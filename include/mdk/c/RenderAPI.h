/*
 * Copyright (c) 2019-2020 WangBin <wbsecg1 at gmail.com>
 */
#pragma once
#include "global.h"

enum MDK_RenderAPI {
    MDK_RenderAPI_Invalid,
    MDK_RenderAPI_OpenGL = 1,
    MDK_RenderAPI_Metal = 3,
    MDK_RenderAPI_D3D11 = 4,
};

/*!
  \brief mdkRenderAPI
  use concrete types in user code, for example mdkD3D11RenderAPI
 */
typedef struct mdkRenderAPI mdkRenderAPI;

struct mdkGLRenderAPI {
    MDK_RenderAPI type;
/*** Render Context Resources. Foreign context (provided by user) only ***/
    int fbo; // if >=0, will draw in given fbo. no need to bind in user code
    int unused;
    void* (*getProcAddress)(const char* name, void* opaque); /* NOT IMPLENETED */
    void* (*getCurrentNativeContext)(void* opaque); /* NOT IMPLENETED */
    void* opaque; /* NOT IMPLENETED */

/***
  Render Context Creation Options.
  as input, they are desired values to create an internal context(ignored if context is provided by user). as output, they are result values(if context is not provided by user)
***/
    bool debug; /* default false. NOT IMPLENETED */
    int8_t egl; /* default -1. -1: auto. 0: no, 1: yes */
/* if any one of opengl and opengles is 0, then another is treated as 1 */
    int8_t opengl; /* default -1. -1: auto. 0: no, 1: yes */
    int8_t opengles; /* default -1. -1: auto. 0: no, 1: yes */
    uint8_t profile; /* default 3. 0: no profile, 1: core profile, 2: compatibility profile */
    float version; /* default 0, ignored if < 2.0. requested version major.minor. result version may < requested version if not supported */
    int8_t reserved[32];
};

struct mdkMetalRenderAPI {
    MDK_RenderAPI type;
/*** Render Context Resources. Foreign context (provided by user) only ***/
// id<?> => void*: to be compatible with c++
    void* device = nullptr; // MUST set if metal is provided by user
    void* cmdQueue = nullptr; // optional. if not null, device can be null. currentQueue callback to share the same command buffer?
/* one of texture and currentRenderTarget MUST be set if metal is provided by user */
    void* texture = nullptr; // optional. id<MTLTexture>. if not null, device can be null. usually for offscreen rendering. render target for MTLRenderPassDescriptor if encoder is not provided by user. set once for offscreen rendering
    void* opaque = nullptr; // optional. callback opaque
    void* (*currentRenderTarget)(void* opaque) = nullptr; // optional. usually for on screen rendering. return id<MTLTexture>.
    // no encoder because we need own render pass
    void* reserved[2];

/***
  Render Context Creation Options.
  as input, they are desired values to create an internal context(ignored if context is provided by user). as output, they are result values(if context is not provided by user)
***/
    // device options: macOS only
    int device_index = -1; // -1 will use system default device. callback with index+name?
};

/*!
  NOTE: include d3d11.h first to use D3D11RenderAPI
 */
#if defined(D3D11_SDK_VERSION)
struct mdkD3D11RenderAPI {
    MDK_RenderAPI type;
/*** Render Context Resources. Foreign context (provided by user) only ***/
/*
  context and rtv can be set by user if user can provide. then rendering becomes foreign context mode.
  if rtv is not null, no need to set context
  \sa Player.setRenderAPI()
 */
    ID3D11DeviceContext* context;
    // rtv or texture. usually user can provide a texture from gui easly, no d3d code to create a view
    ID3D11DeviceChild* rtv; // optional. the render target(view). ID3D11RenderTargetView or ID3D11Texture2D. can be null if context is not null. if not null, no need to set context
    void* reserved[2];

/***
  Render Context Creation Options.
  as input, they are desired values to create an internal context(ignored if context is provided by user). as output, they are result values(if context is not provided by user)
***/
    bool debug;
    int buffers; /* UWP must >= 2. */
    int adapter; /* adapter index */
    float feature_level; /* 0 is the highest */
};
#endif