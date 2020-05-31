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
        Metal = 3,
        D3D11 = 4,
    };

    Type type() const { return type_;}
protected:
    Type type_ = Type::Invalid;
};


struct GLRenderAPI final: RenderAPI {
    GLRenderAPI() {
        type_ = RenderAPI::OpenGL;
    }
/*** Render Context Resources. Foreign context (provided by user) only ***/
    int fbo = -1; // if >=0, will draw in given fbo. no need to bind in user code
    int unused;
    void* (*getProcAddress)(const char* name, void* opaque); /* NOT IMPLENETED */
    void* (*getCurrentNativeContext)(void* opaque); /* NOT IMPLENETED */
    void* opaque; /* NOT IMPLENETED */

/***
  Render Context Creation Options.
  as input, they are desired values to create an internal context(ignored if context is provided by user). as output, they are result values(if context is not provided by user)
***/
    enum class Profile : uint8_t {
        No,
        Core,
        Compatibility,
    };

    bool debug = false; /* default false. NOT IMPLENETED */
    int8_t egl = -1; /* default -1. -1: auto. 0: no, 1: try */
/* if any one of opengl and opengles is 0, then another is treated as 1 */
    int8_t opengl = -1; /* default -1. -1: auto. 0: no, 1: try */
    int8_t opengles = -1; /* default -1. -1: auto. 0: no, 1: try */
    Profile profile = Profile::Core; /* default 3. 0: no profile, 1: core profile, 2: compatibility profile */
    float version = 0; /* default 0, ignored if < 2.0. requested version major.minor. result version may < requested version if not supported */
    int8_t reserved[32];
};

struct MetalRenderAPI final: RenderAPI {
    MetalRenderAPI() {
        type_ = RenderAPI::Metal;
    }
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
struct D3D11RenderAPI : RenderAPI {
    D3D11RenderAPI(ID3D11DeviceContext* c = nullptr, ID3D11DeviceChild* r = nullptr) : context(c), rtv(r) {
        type_ = RenderAPI::D3D11;
    }
/*** Render Context Resources. Foreign context (provided by user) only ***/
/*
  context and rtv can be set by user if user can provide. then rendering becomes foreign context mode.
  if rtv is not null, no need to set context
  \sa Player.setRenderAPI()
 */
    ID3D11DeviceContext* context = nullptr;
    // rtv or texture. usually user can provide a texture from gui easly, no d3d code to create a view
    ID3D11DeviceChild* rtv = nullptr; // optional. the render target(view). ID3D11RenderTargetView or ID3D11Texture2D. can be null if context is not null. if not null, no need to set context
    void* reserved[2];

/***
  Render Context Creation Options.
  as input, they are desired values to create an internal context(ignored if context is provided by user). as output, they are result values(if context is not provided by user)
***/
    bool debug = false;
    int buffers = 2; /* UWP must >= 2. */
    int adapter = 0; /* adapter index */
    float feature_level = 0; /* 0 is the highest */
};
#endif

MDK_NS_END