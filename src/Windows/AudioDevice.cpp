#include "AudioDevice.h"

#include "Constants.h"

#include "core/Log.h"
#include "core/ScopeExit.h"

#include <atlbase.h>
#include <audioclient.h>
#include <mmdeviceapi.h>
#include <rtworkq.h>

#include <chrono>
#include <cstdint>
#include <thread>

using namespace CR;
using namespace CR::Audio;
using namespace std::chrono_literals;

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator    = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient3          = __uuidof(IAudioClient3);
const IID IID_IAudioRenderClient     = __uuidof(IAudioRenderClient);

namespace CR::Audio {
	class AudioDeviceImpl : public IRtwqAsyncCallback {
	  public:
		AudioDeviceImpl(AudioDevice::DeviceCallback_t a_callback);
		virtual ~AudioDeviceImpl();

		AudioDeviceImpl(const AudioDeviceImpl&) = delete;
		AudioDeviceImpl(AudioDeviceImpl&&)      = delete;
		AudioDeviceImpl& operator=(const AudioDeviceImpl&) = delete;
		AudioDeviceImpl& operator=(AudioDeviceImpl&&) = delete;

		STDMETHODIMP GetParameters(DWORD* a_flags, DWORD* a_queue) override;
		STDMETHODIMP Invoke(IRtwqAsyncResult* a_result) override;

		STDMETHODIMP QueryInterface(const IID&, void**) override { return 0; }
		ULONG AddRef() override { return 0; }
		ULONG Release() override { return 0; }

		void PutWorkItem();

		CComPtr<IAudioClient3> m_audioClient;
		CComPtr<IAudioRenderClient> m_audioRenderClient;
		uint32_t m_frameSamples = 0;
		uint32_t m_bufferFrames = 0;
		uint32_t m_frameSize    = 0;
		uint32_t m_channels     = 0;
		uint32_t m_sampleRate   = 0;
		HANDLE m_audioEvent;

		DWORD m_rtWorkQueueId = 0;

		std::atomic<uint64_t> m_rtWorkItemKey = 0;
		std::atomic_bool m_finish             = false;
		std::atomic_bool m_finished           = false;
		std::atomic_int32_t m_finalFramesLeft = 4;

		AudioDevice::DeviceCallback_t m_callback;
	};
}    // namespace CR::Audio

AudioDeviceImpl::AudioDeviceImpl(AudioDevice::DeviceCallback_t a_callback) : m_callback(std::move(a_callback)) {
	HRESULT hr = CoInitialize(nullptr);
	Core::Log::Require(hr == S_OK, "Failed to initialize com for audio");

	CComPtr<IMMDeviceEnumerator> deviceEnumerator;
	hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator,
	                      (void**)&deviceEnumerator);

	Core::Log::Require(deviceEnumerator && hr == S_OK, "Failed to create audio device enumerator");

	CComPtr<IMMDevice> defaultDevice;
	hr = deviceEnumerator->GetDefaultAudioEndpoint(EDataFlow::eRender, ERole::eConsole, &defaultDevice);

	Core::Log::Require(defaultDevice && hr == S_OK, "Failed to create audio device");

	defaultDevice->Activate(IID_IAudioClient3, CLSCTX_ALL, nullptr, (void**)&m_audioClient);

	Core::Log::Require(m_audioClient && hr == S_OK, "Failed to create audio client");

	BOOL offloadCapable = false;
	m_audioClient->IsOffloadCapable(AUDIO_STREAM_CATEGORY::AudioCategory_GameEffects, &offloadCapable);

	AudioClientProperties audioProps;
	audioProps.cbSize     = sizeof(AudioClientProperties);
	audioProps.bIsOffload = offloadCapable;
	audioProps.eCategory  = AUDIO_STREAM_CATEGORY::AudioCategory_GameEffects;
	// Can probably save a little bit of latency by using a raw stream if supported. May not be desirable though, would
	// loose any user settings, equalizers, whatever.
	audioProps.Options = AUDCLNT_STREAMOPTIONS::AUDCLNT_STREAMOPTIONS_NONE;
	hr                 = m_audioClient->SetClientProperties(&audioProps);
	Core::Log::Require(hr == S_OK, "Failed to set wasapi audio client properties");

	WAVEFORMATEXTENSIBLE* waveFormatDevice = nullptr;
	m_audioClient->GetMixFormat((WAVEFORMATEX**)&waveFormatDevice);
	auto freeWaveFormat = std23::make_scope_exit([&]() { CoTaskMemFree(waveFormatDevice); });

	Core::Log::Info("\n**** Audio Device Properties Start ****");
	Core::Log::Info("OffloadCapable: {}", (offloadCapable == TRUE ? true : false));
	Core::Log::Info("Channels: {}", waveFormatDevice->Format.nChannels);
	Core::Log::Info("Samples Per Second: {}", waveFormatDevice->Format.nSamplesPerSec);
	Core::Log::Info("Bits Per Sample: {}", waveFormatDevice->Format.wBitsPerSample);
	if(waveFormatDevice->Format.wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
		Core::Log::Info("Valid Bits per Samples: {}", waveFormatDevice->Samples.wValidBitsPerSample);
		if(waveFormatDevice->SubFormat == KSDATAFORMAT_SUBTYPE_PCM) {
			Core::Log::Info("Format: PCM");
		} else if(waveFormatDevice->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT) {
			Core::Log::Info("Format: Float");
		} else {
			Core::Log::Error("Format: Unknown or Unsupported");
		}
	}

	if(waveFormatDevice->Format.nChannels != 2) { Core::Log::Error("Unsupported number of channels"); }
	if((waveFormatDevice->Format.nSamplesPerSec % 48000) != 0) {
		Core::Log::Error("Unsupported number of samples per second, require an even multiple of 48000");
	}
	if(waveFormatDevice->Format.wBitsPerSample != 32) { Core::Log::Error("Unsupported number of bits per sample"); }
	if(waveFormatDevice->Format.nChannels != 2) { Core::Log::Error("Unsupported number of channels"); }
	if(waveFormatDevice->Format.wFormatTag != WAVE_FORMAT_EXTENSIBLE) {
		Core::Log::Error("require support for float audio format");
	}
	if(waveFormatDevice->SubFormat != KSDATAFORMAT_SUBTYPE_IEEE_FLOAT) {
		Core::Log::Error("require support for float audio format");
	}

	UINT32 defaultPeriod     = 0;
	UINT32 fundamentalPeriod = 0;
	UINT32 minPeriod         = 0;
	UINT32 maxPeriod         = 0;

	hr = m_audioClient->GetSharedModeEnginePeriod(&waveFormatDevice->Format, &defaultPeriod, &fundamentalPeriod,
	                                              &minPeriod, &maxPeriod);
	Core::Log::Require(hr == S_OK, "Failed to retrieve the low latency audio period");
	Core::Log::Info(
	    "Default Period: {} Fundamental Period: {} Minimum Period: {} Maximum Period: {} Min Period Time {}ms",
	    defaultPeriod, fundamentalPeriod, minPeriod, maxPeriod,
	    (1000 * minPeriod / waveFormatDevice->Format.nSamplesPerSec));

	Core::Log::Info("**** Audio Device Properties End ****\n");

	m_frameSize    = waveFormatDevice->Format.nBlockAlign;
	m_frameSamples = minPeriod;
	m_channels     = waveFormatDevice->Format.nChannels;
	m_sampleRate   = waveFormatDevice->Format.nSamplesPerSec;
	hr             = m_audioClient->InitializeSharedAudioStream(AUDCLNT_STREAMFLAGS_EVENTCALLBACK, m_frameSamples,
                                                    &waveFormatDevice->Format, nullptr);
	Core::Log::Require(hr == S_OK, "Failed to set initialize wasapi audio stream");

	m_audioEvent = CreateEventA(nullptr, FALSE, FALSE, nullptr);
	Core::Log::Require(m_audioEvent != NULL, "Failed to create an event to use with event driven wasapi");
	m_audioClient->SetEventHandle(m_audioEvent);

	m_audioClient->GetBufferSize(&m_bufferFrames);

	hr = m_audioClient->GetService(IID_IAudioRenderClient, (void**)&m_audioRenderClient);
	Core::Log::Require(hr == S_OK && m_audioRenderClient, "Failed to create a wasapi render client");

	// prime with silence
	BYTE* buffer = nullptr;
	hr           = m_audioRenderClient->GetBuffer(m_bufferFrames, &buffer);
	Core::Log::Require(hr == S_OK, "Failed to get wasapi buffer");
	memset(buffer, 0, m_bufferFrames * m_frameSize);
	hr = m_audioRenderClient->ReleaseBuffer(m_bufferFrames, 0);
	Core::Log::Require(hr == S_OK, "Failed to release wasapi buffer");

	hr = RtwqStartup();
	Core::Log::Require(hr == S_OK, "Failed to startup real time work queues");

	m_rtWorkQueueId = RTWQ_MULTITHREADED_WORKQUEUE;

	DWORD taskId = 0;
	hr           = RtwqLockSharedWorkQueue(L"Pro Audio", 0, &taskId, &m_rtWorkQueueId);
	Core::Log::Require(hr == S_OK, "Failed to lock pro audio work queue");

	PutWorkItem();

	m_audioClient->Start();
}

AudioDeviceImpl::~AudioDeviceImpl() {
	HRESULT hr;

	m_finish.store(true, std::memory_order_release);
	while(!m_finished.load(std::memory_order_acquire)) { std::this_thread::sleep_for(1ms); }

	m_audioClient->Stop();

	hr = RtwqUnlockWorkQueue(m_rtWorkQueueId);
	Core::Log::Assert(hr == S_OK, "Failed to unlock pro audio work queue");
	hr = RtwqShutdown();
	Core::Log::Assert(hr == S_OK, "Failed to shutdown real time work queues");

	CloseHandle(m_audioEvent);
	m_audioRenderClient.Release();
	m_audioClient.Release();
	CoUninitialize();
}

STDMETHODIMP AudioDeviceImpl::GetParameters(DWORD* a_flags, DWORD* a_queue) {
	*a_flags = 0;
	*a_queue = m_rtWorkQueueId;

	return S_OK;
}

STDMETHODIMP AudioDeviceImpl::Invoke(IRtwqAsyncResult*) {
	Core::Log::Require(m_finished.load(std::memory_order_acquire) != true,
	                   "Should never have a work item queued, if the device is already finsished");

	HRESULT hr;

	uint32_t padding;
	hr = m_audioClient->GetCurrentPadding(&padding);
	Core::Log::Assert(hr == S_OK, "Failed to get current padding");

	uint32_t numFrames = m_bufferFrames - padding;

	float* buffer = nullptr;
	hr            = m_audioRenderClient->GetBuffer(numFrames, (BYTE**)&buffer);
	Core::Log::Require(hr == S_OK, "Failed to get wasapi buffer");

	bool finish = m_finish.load(std::memory_order_acquire);

	bool fillSilence = m_callback(Core::Span<float>{buffer, numFrames * m_channels}, m_channels, m_sampleRate, finish);
	if(fillSilence) {
		hr = m_audioRenderClient->ReleaseBuffer(numFrames, AUDCLNT_BUFFERFLAGS_SILENT);
		Core::Log::Require(hr == S_OK, "Failed to release wasapi buffer");

	} else {
		hr = m_audioRenderClient->ReleaseBuffer(numFrames, 0);
		Core::Log::Require(hr == S_OK, "Failed to release wasapi buffer");
	}

	bool finished = m_finished.load(std::memory_order_acquire);
	if(finish && fillSilence && !finished) {
		uint32_t framesLeft = framesLeft = m_finalFramesLeft.fetch_add(-1);
		if(finish && framesLeft == 0) {
			m_finished.store(true, std::memory_order_release);
			finished = true;
		}
	}

	if(!finished) { PutWorkItem(); }

	return S_OK;
}

void AudioDeviceImpl::PutWorkItem() {
	HRESULT hr;
	CComPtr<IRtwqAsyncResult> asyncResult;
	hr = RtwqCreateAsyncResult(nullptr, this, nullptr, &asyncResult);
	Core::Log::Require(hr == S_OK, "Failed to create real time work queue async result");

	uint64_t workItemKey;
	hr = RtwqPutWaitingWorkItem(m_audioEvent, 1, asyncResult, &workItemKey);
	m_rtWorkItemKey.store(workItemKey, std::memory_order_release);
	Core::Log::Require(hr == S_OK, "Failed to put a work item to real time work queue for audio");
}

AudioDevice::AudioDevice(AudioDevice::DeviceCallback_t a_callback) {
	m_pimpl = std::make_unique<AudioDeviceImpl>(std::move(a_callback));
}

AudioDevice::~AudioDevice() {}
