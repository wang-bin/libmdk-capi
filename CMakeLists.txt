# libmdk-capi. MUST be built in source tree

set(MODULE c)
set(SRC_C
  global.cpp
  MediaInfo.cpp
  Player.cpp
  RenderAPI.cpp
  VideoFrame.cpp
)
if(EXISTS ${Vulkan_INCLUDE_DIR}) # FindVulkan will cache Vulkan_INCLUDE_DIR even if library is not found
  set_property(SOURCE RenderAPI.cpp
    APPEND PROPERTY INCLUDE_DIRECTORIES "${Vulkan_INCLUDE_DIR}") # or COMPILE_OPTIONS "-I..."
endif()

add_library(${MODULE} OBJECT ${SRC_C})
target_include_directories(${MODULE} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)

# no subdir is creaded by PUBLIC_HEADER property
if(APPLE)
  install(DIRECTORY include/${PROJECT_NAME}/c include/${PROJECT_NAME}/cpp
    DESTINATION lib/${PROJECT_NAME}.framework/Headers
  )
else()
  install(DIRECTORY include/${PROJECT_NAME}/c include/${PROJECT_NAME}/cpp
    DESTINATION include/mdk
  )
endif()