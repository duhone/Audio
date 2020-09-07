#include "AudioDevice.h"

#include "core/Log.h"
#include "core/ScopeExit.h"

#include <mmdeviceapi.h>

using namespace CR;
using namespace CR::Audio;

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator    = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient3          = __uuidof(IAudioClient3);
const IID IID_IAudioRenderClient     = __uuidof(IAudioRenderClient);

AudioDevice::AudioDevice() noexcept {
	/*const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
	const IID IID_IMMDeviceEnumerator    = __uuidof(IMMDeviceEnumerator);
	const IID IID_IAudioClient3          = __uuidof(IAudioClient3);
	const IID IID_IAudioRenderClient     = __uuidof(IAudioRenderClient);*/

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
	if(waveFormatDevice->Format.nSamplesPerSec != 48000) {
		Core::Log::Error("Unsupported number of samples per second");
	}
	if(waveFormatDevice->Format.wBitsPerSample != 32) { Core::Log::Error("Unsupported number of bits per sample"); }
	if(waveFormatDevice->Format.nChannels != 2) { Core::Log::Error("Unsupported number of channels"); }

	UINT32 defaultPeriod     = 0;
	UINT32 fundamentalPeriod = 0;
	UINT32 minPeriod         = 0;
	UINT32 maxPeriod         = 0;

	m_audioClient->GetSharedModeEnginePeriod(&waveFormatDevice->Format, &defaultPeriod, &fundamentalPeriod, &minPeriod,
	                                         &maxPeriod);
	Core::Log::Info("Default Period: {} Fundamental Period: {} Minimum Period: {} Maximum Period: {}", defaultPeriod,
	                fundamentalPeriod, minPeriod, maxPeriod);

	Core::Log::Info("**** Audio Device Properties End ****\n");

	AudioClientProperties audioProps;
	audioProps.cbSize     = sizeof(AudioClientProperties);
	audioProps.bIsOffload = offloadCapable;
	audioProps.eCategory  = AUDIO_STREAM_CATEGORY::AudioCategory_GameEffects;
	// Can probably save a little bit of latency by using a raw stream if supported. May not be desirable though, would
	// loose any user settings, equalizers, whatever.
	audioProps.Options = AUDCLNT_STREAMOPTIONS::AUDCLNT_STREAMOPTIONS_NONE;
	hr                 = m_audioClient->SetClientProperties(&audioProps);
	Core::Log::Require(hr == S_OK, "Failed to set wasapi audio client properties");

	m_frameSize    = waveFormatDevice->Format.nBlockAlign;
	m_frameSamples = minPeriod;
	hr             = m_audioClient->InitializeSharedAudioStream(AUDCLNT_STREAMFLAGS_EVENTCALLBACK, m_frameSamples,
                                                    &waveFormatDevice->Format, nullptr);
	Core::Log::Require(hr == S_OK, "Failed to set initialize wasapi audio stream");

	m_audioEvent = CreateEventA(nullptr, FALSE, FALSE, nullptr);
	Core::Log::Require(m_audioEvent != NULL, "Failed to create an event to use with event driven wasapi");

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
}

AudioDevice ::~AudioDevice() noexcept {
	CloseHandle(m_audioEvent);
	m_audioRenderClient.Release();
	m_audioClient.Release();
	CoUninitialize();
}
