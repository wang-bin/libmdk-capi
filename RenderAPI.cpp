/*
 * Copyright (c) 2019-2024 WangBin <wbsecg1 at gmail.com>
 */
// include gl, d3d, vk headers to enable RenderAPI structures
#if defined(_WIN32)
# include <d3d11.h>
# include <d3d12.h>
#endif
#if __has_include(<vulkan/vulkan_core.h>)
# include <vulkan/vulkan_core.h>
#endif
#include "mdk/RenderAPI.h"
#include "mdk/c/RenderAPI.h"
#include <memory>

using namespace std;
using namespace MDK_NS;

unique_ptr<RenderAPI> from_c(MDK_RenderAPI type, void* data)
{
    [[maybe_unused]] const int version = (type >> 16);
    const int struct_sz = ((type << 2) >> 18) & 0xffff;
    const bool is_size = (type >> 30) & 0b11;
    type = MDK_RenderAPI(type & 0xffff);
//    assert(!is_size || version >= (MDK_VERSION_INT(0, 17, 0) >> 8));
    switch (type) {
    case MDK_RenderAPI_OpenGL: {
        auto c = static_cast<mdkGLRenderAPI*>(data);
        auto api = make_unique<GLRenderAPI>();
        api->fbo = c->fbo;
        api->getProcAddress = c->getProcAddress;
        api->getCurrentNativeContext = c->getCurrentNativeContext;
        api->opaque = c->opaque;
        api->debug = c->debug;
        api->egl = c->egl;
        api->opengl = c->opengl;
        api->opengles = c->opengles;
        api->profile = GLRenderAPI::Profile(c->profile);
        api->version = c->version;
        return api;
    }
    case MDK_RenderAPI_Metal: {
        auto c = static_cast<mdkMetalRenderAPI*>(data);
        auto api = make_unique<MetalRenderAPI>();
        api->device = c->device;
        api->cmdQueue = c->cmdQueue;
        api->texture = c->texture;
        api->opaque = c->opaque;
        api->currentRenderTarget = c->currentRenderTarget;
        api->layer = c->layer;
        api->device_index = c->device_index;
        if (is_size && struct_sz >= sizeof(mdkMetalRenderAPI)) {
            api->currentCommand = c->currentCommand;
            api->colorFormat = c->colorFormat;
            api->depthStencilFormat = c->depthStencilFormat;
        }
        return api;
    }
#if defined(D3D11_SDK_VERSION)
    case MDK_RenderAPI_D3D11: {
        auto c = static_cast<mdkD3D11RenderAPI*>(data);
        auto api = make_unique<D3D11RenderAPI>(c->context, c->rtv);
        api->debug = c->debug;
        api->buffers = c->buffers;
        api->adapter = c->adapter;
        api->feature_level = c->feature_level;
        if (version >= (MDK_VERSION_INT(0, 17, 0) >> 8)) {
            api->vendor = c->vendor;
        }
        return api;
    }
#endif // defined(D3D11_SDK_VERSION)
#if defined(__d3d12_h__)
    case MDK_RenderAPI_D3D12: {
        auto c = static_cast<mdkD3D12RenderAPI*>(data);
        auto api = make_unique<D3D12RenderAPI>(c->cmdQueue, c->rt);
        api->rtvHandle = c->rtvHandle;
        api->opaque = c->opaque;
        api->currentRenderTarget = c->currentRenderTarget;
        //api->swapChain =
        api->currentCommandList = c->currentCommandList;
        api->colorFormat = c->colorFormat;
        api->depthStencilFormat = c->depthStencilFormat;

        api->debug = c->debug;
        api->buffers = c->buffers;
        api->adapter = c->adapter;
        api->feature_level = c->feature_level;
        api->vendor = c->vendor;
        return api;
    }
#endif // defined(__d3d12_h__)
#if (VK_VERSION_1_0+0)
    case MDK_RenderAPI_Vulkan: {
        auto c = static_cast<mdkVulkanRenderAPI*>(data);
        auto api = make_unique<VulkanRenderAPI>();
        api->instance = c->instance;
        api->phy_device = c->phy_device;
        api->device = c->device;
        api->graphics_queue = c->graphics_queue;
        api->rt = c->rt;
        api->render_pass = c->render_pass;
        api->opaque = c->opaque;
        api->renderTargetInfo = c->renderTargetInfo;
        api->beginFrame = c->beginFrame;
        api->currentCommandBuffer = c->currentCommandBuffer;
        api->endFrame = c->endFrame;
        api->graphics_family = c->graphics_family;
        api->compute_family = c->compute_family;
        api->transfer_family = c->transfer_family;
        api->present_family = c->present_family;
        api->debug = c->debug;
        api->buffers = c->buffers;
        api->device_index = c->device_index;
        api->max_version = c->max_version;
        api->gfx_queue_index = c->gfx_queue_index;
        api->transfer_queue_index = c->transfer_queue_index;
        api->compute_queue_index = c->compute_queue_index;
        api->depth = c->depth;
        return api;
    }
#endif
    default:
        break;
    }
    return nullptr;
}