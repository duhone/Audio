cmake_minimum_required (VERSION 3.12)
set(CMAKE_CONFIGURATION_TYPES "Debug;RelWithDebInfo;Profile;Final") 

project (Audio)

include (${CMAKE_CURRENT_SOURCE_DIR}/../../build/common.cmake)
include (${CMAKE_CURRENT_SOURCE_DIR}/../../3rdParty/build/build_cli11.cmake)
include (${CMAKE_CURRENT_SOURCE_DIR}/../../3rdParty/build/build_doctest.cmake)
include (${CMAKE_CURRENT_SOURCE_DIR}/../../3rdParty/build/build_fmt.cmake)
include (${CMAKE_CURRENT_SOURCE_DIR}/../../3rdParty/build/build_function2.cmake)
include (${CMAKE_CURRENT_SOURCE_DIR}/../../3rdParty/build/build_glm.cmake)
include (${CMAKE_CURRENT_SOURCE_DIR}/../../3rdParty/build/build_libsamplerate.cmake)
include (${CMAKE_CURRENT_SOURCE_DIR}/../../3rdParty/build/build_opus.cmake)
include (${CMAKE_CURRENT_SOURCE_DIR}/../../3rdParty/build/build_robinmap.cmake)
include (${CMAKE_CURRENT_SOURCE_DIR}/../../3rdParty/build/build_spdlog.cmake)
include (${CMAKE_CURRENT_SOURCE_DIR}/../../core/build/build.cmake)
include (${CMAKE_CURRENT_SOURCE_DIR}/../../Platform/build/build.cmake)
include (${CMAKE_CURRENT_SOURCE_DIR}/../../AudioProcessor/build/build.cmake)
include (${CMAKE_CURRENT_SOURCE_DIR}/build.cmake)
