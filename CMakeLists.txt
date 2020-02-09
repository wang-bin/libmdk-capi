set(MODULE c)
set(SRC_C
  global.cpp
  MediaInfo.cpp
  Player.cpp
  RenderAPI.cpp
  VideoFrame.cpp
)
add_library(${MODULE} OBJECT ${SRC_C})
