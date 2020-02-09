/*
 * Copyright (c) 2019-2020 WangBin <wbsecg1 at gmail.com>
 */
// include gl, d3d, vk headers to enable RenderAPI structures
#if defined(_WIN32)
# include <d3d11.h>
#endif
#include "mdk/RenderAPI.h"
#include "mdk/c/RenderAPI.h"
#include <memory>

using namespace std;
using namespace MDK_NS;

unique_ptr<RenderAPI> from_c(MDK_RenderAPI type, void* data)
{
    switch (type) {
    case MDK_RenderAPI_OpenGL: {
        auto api = make_unique<GLRenderAPI>();
        return api;
    }
#if defined(D3D11_SDK_VERSION)
    case MDK_RenderAPI_D3D11: {
        auto c = static_cast<mdkD3D11RenderAPI*>(data);
        auto api = make_unique<D3D11RenderAPI>(c->context, c->rtv);
        return api;
    }
#endif // defined(D3D11_SDK_VERSION)
    default:
        break;
    }
    return nullptr;
}