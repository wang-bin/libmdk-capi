# libmdk-capi. MUST be built in source tree

set(MODULE c)
set(SRC_C
  global.cpp
  MediaInfo.cpp
  Player.cpp
  RenderAPI.cpp
  AudioFrame.cpp
  VideoFrame.cpp
)
if(EXISTS ${Vulkan_INCLUDE_DIR}) # FindVulkan will cache Vulkan_INCLUDE_DIR even if library is not found
  set_property(SOURCE RenderAPI.cpp
    APPEND PROPERTY INCLUDE_DIRECTORIES "${Vulkan_INCLUDE_DIR}") # or COMPILE_OPTIONS "-I..."
endif()

add_library(${MODULE} OBJECT ${SRC_C})
target_include_directories(${MODULE} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)
