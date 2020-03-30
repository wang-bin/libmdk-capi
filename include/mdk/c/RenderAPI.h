/*
 * Copyright (c) 2019-2020 WangBin <wbsecg1 at gmail.com>
 */
#pragma once
#include "global.h"

enum MDK_RenderAPI {
    MDK_RenderAPI_Invalid,
    MDK_RenderAPI_OpenGL = 1,
    MDK_RenderAPI_D3D11 = 4,
};

/*!
  \brief mdkRenderAPI
  use concrete types in user code, for example mdkD3D11RenderAPI
 */
typedef struct mdkRenderAPI mdkRenderAPI;

struct mdkGLRenderAPI {
    MDK_RenderAPI type;
    void* (*getProcAddress)(const char* name, void* userData); /* NOT IMPLENETED */
    void* (*getCurrentNativeContext)(void* userData); /* NOT IMPLENETED */
    void* userData; /* NOT IMPLENETED */
    void* nativeContext; /* NOT IMPLENETED. prefer this over getProcAddress if not null */

/* context creation options. */
    bool debug; /* default false. NOT IMPLENETED */
    int8_t egl; /* default -1. -1: auto. 0: no, 1: yes */
/* if any one of opengl and opengles is 0, then another is treated as 1 */
    int8_t opengl; /* default -1. -1: auto. 0: no, 1: yes */
    int8_t opengles; /* default -1. -1: auto. 0: no, 1: yes */
    uint8_t profile; /* default 3. 0: no profile, 1: core profile, 2: compatibility profile */
    float version; /* default 0, ignored if < 2.0. requested version major.minor. result version may < requested version if not supported */
};

/*!
  NOTE: include d3d11.h first to use D3D11RenderAPI
 */
#if defined(D3D11_SDK_VERSION)
struct mdkD3D11RenderAPI {
    MDK_RenderAPI type;
/*
  context and rtv can be set by user if user can provide. then rendering becomes foreign context mode.
  \sa Player.setRenderAPI()
 */
    ID3D11DeviceContext* context;
    ID3D11RenderTargetView* rtv;
    void* reserved[2];

/*
  options. as input, they are desired values. as output, they are result values(if context is not provided by user)
*/
    bool debug;
    int buffers; /* UWP must >= 2. */
    int adapter; /* adapter index */
    float feature_level; /* 0 is the highest */
};
#endif