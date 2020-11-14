#pragma once
#ifdef __APPLE__
#import <Metal/Metal.h> /* to define swift bool type. but -fcxx-module error if include stdbool.h */
#endif
#include "MediaInfo.h"
#include "VideoFrame.h"
#include "RenderAPI.h"
#include "Player.h"
