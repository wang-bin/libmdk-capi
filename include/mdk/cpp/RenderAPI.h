/*
 * Copyright (c) 2019-2020 WangBin <wbsecg1 at gmail.com>
 */
#pragma once
#include "global.h"

MDK_NS_BEGIN

/*!
  \brief RenderAPI
  use concrete types in user code, for example D3D11RenderAPI
 */
struct RenderAPI {
    enum Type {
        Invalid,
        OpenGL = 1,
        D3D11 = 4,
    };

    Type type() const { return type_;}
protected:
    Type type_ = Type::Invalid;
};

/*!
  NOTE: include d3d11.h first to use D3D11RenderAPI
 */
#if defined(D3D11_SDK_VERSION)
struct D3D11RenderAPI : RenderAPI {
    D3D11RenderAPI(ID3D11DeviceContext* c = nullptr, ID3D11RenderTargetView* r = nullptr) : context(c), rtv(r) {
        type_ = RenderAPI::D3D11;
    }
/*
  context and rtv can be set by user if user can provide. then rendering becomes foreign context mode.
  \sa Player.setRenderAPI()
 */
    ID3D11DeviceContext* context = nullptr;
    ID3D11RenderTargetView* rtv = nullptr;
    void* reserved[2];
};
#endif

MDK_NS_END