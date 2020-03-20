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