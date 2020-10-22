set(root "${CMAKE_CURRENT_LIST_DIR}/..")

###############################################
#library
###############################################
set(PUBLIC_HDRS
    ${root}/inc/Audio/AudioEngine.h
)

set(SRCS
    ${root}/src/AudioEngine.cpp
    ${root}/src/Constants.h
    ${root}/src/OutputConversion.h
    ${root}/src/OutputConversion.cpp
    ${root}/src/Sample.h
    ${root}/src/TestTone.h
    ${root}/src/TestTone.cpp
    ${root}/src/Windows/AudioDevice.h
    ${root}/src/Windows/AudioDevice.cpp
)

set(BUILD
    ${root}/build/build.cmake
)

add_library(audio  
	${PUBLIC_HDRS} 
	${SRCS} 
	${SRCS_SHADERS} 
	${BUILD}
)
	
settingsCR(audio)	
usePCH(audio core)	

target_include_directories(audio PUBLIC
	"${root}/inc"
)
target_include_directories(audio PRIVATE
	"${root}/src"
	"${root}/src/Windows"
)		

target_link_libraries(audio PUBLIC
    doctest
    robinmap
    core
    function2
	samplerate
)
					
###############################################
#unit tests
###############################################
set(SRCS
  ${root}/tests/main.cpp
  ${root}/tests/AudioEngine.cpp
)

add_executable(audio_tests
					${SRCS}
)

settingsCR(audio_tests)	
usePCH(audio_tests core)
					
target_include_directories(audio_tests PRIVATE
	"${root}/src"
)	

target_link_libraries(audio_tests 
	doctest
	fmt
	core
	platform
	audio
	Rtworkq.lib
	samplerate
)

add_dependencies(audio_tests AudioProcessor)

add_custom_command(TARGET audio_tests POST_BUILD
    COMMAND $<TARGET_FILE:AudioProcessor> -i ${root}/tests/data/BGM_Menu.wav -o $<TARGET_FILE_DIR:audio_tests>/BGM_Menu
)

set_property(TARGET audio_tests APPEND PROPERTY FOLDER tests)