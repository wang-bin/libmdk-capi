/*
 * Copyright (c) 2020-2025 WangBin <wbsecg1 at gmail.com>
 * This file is part of MDK
 * MDK SDK: https://github.com/wang-bin/mdk-sdk
 * Free for opensource softwares or non-commercial use.
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 */
#pragma once
#include "global.h"

#ifdef __cplusplus
extern "C" {
#endif

/* native resource import */
struct ID3D11Device;
struct ID3D11DeviceChild;
struct IDirect3DSurface9;
typedef struct mdkDX11Resource {
    int size; /* struct size, for binary compatibility */
    /* ID3D11Texture2D or ID3D11VideoDecoderOutputView as input, ID3D11Texture2D as output */
    struct ID3D11DeviceChild* resource;
    /* subresource index for texture array, 0 otherwise */
    int subResource;
    struct ID3D11Texture2D* plane[4];  // ID3D11Texture2D for each plane. plane[0] == resource. usually each plane is an array indexed by subResource
    int planeCount;                     // plane count
} mdkDX11Resource;

typedef struct mdkDX9Resource {
    int size; /* struct size, for binary compatibility */
    struct IDirect3DSurface9* surface;
} mdkDX9Resource;

#ifndef VA_VERSION
typedef unsigned int VASurfaceID;
typedef void* VADisplay;
#endif

typedef struct mdkVAAPIResource {
    int size; /* struct size, for binary compatibility */
    VASurfaceID surface;
    VADisplay display;
    void* x11Display;   /* can be null, then global option "X11Display" is used when required*/
    /* surface is not ref counted, so unref() is required */
    const void* opaque;
    void (*unref)(const void* opaque);
} mdkVAAPIResource;

typedef struct mdkVideoBufferPool mdkVideoBufferPool;

struct mdkVideoFrame;

typedef enum MDK_PixelFormat {
    MDK_PixelFormat_Unknown = -1, // TODO: 0 in next major version
    MDK_PixelFormat_YUV420P,
    MDK_PixelFormat_NV12,
    MDK_PixelFormat_YUV422P,
    MDK_PixelFormat_YUV444P,
    MDK_PixelFormat_P010LE,
    MDK_PixelFormat_P016LE,
    MDK_PixelFormat_YUV420P10LE,
    MDK_PixelFormat_UYVY422,
    MDK_PixelFormat_RGB24,
    MDK_PixelFormat_RGBA,
    MDK_PixelFormat_RGBX,
    MDK_PixelFormat_BGRA,
    MDK_PixelFormat_BGRX,
    MDK_PixelFormat_RGB565LE,
    MDK_PixelFormat_RGB48LE,
    MDK_PixelFormat_RGB48 = MDK_PixelFormat_RGB48LE,  // name: "rgb48le"
    MDK_PixelFormat_GBRP,
    MDK_PixelFormat_GBRP10LE,
    MDK_PixelFormat_XYZ12LE,
    MDK_PixelFormat_YUVA420P,
    MDK_PixelFormat_BC1,
    MDK_PixelFormat_BC3,
    MDK_PixelFormat_RGBA64, // name: "rgba64le"
    MDK_PixelFormat_BGRA64, // name: "bgra64le"
    MDK_PixelFormat_RGBP16, // name: "rgbp16le"
    MDK_PixelFormat_RGBPF32, // name: "rgbpf32le"
    MDK_PixelFormat_BGRAF32, // name: "bgraf32le"
} MDK_PixelFormat;

typedef struct mdkCUDAResource {
    int size; /* struct size, for binary compatibility */
    void* ptr[4]; /* CUdeviceptr. ptr[0] can be null, others can */
    int width;   /* can't be 0 */
    int height;  /* can't be 0 */
    int stride[4]; /* can be 0 */
    enum MDK_PixelFormat format; /* can't be unknown */
    void* context; /* CUcontext, can be null */
    void* stream;  /* CUstream, can be null */
    /* surface is not ref counted, so unref() is required */
    const void* opaque;
    void (*unref)(const void* opaque);
} mdkCUDAResource;


typedef struct mdkVideoFrameAPI {
    struct mdkVideoFrame* object;

    int (*planeCount)(struct mdkVideoFrame*);
    int (*width)(struct mdkVideoFrame*, int plane /*=-1*/);
    int (*height)(struct mdkVideoFrame*, int plane /*=-1*/);
    enum MDK_PixelFormat (*format)(struct mdkVideoFrame*);
    bool (*addBuffer)(struct mdkVideoFrame*, const uint8_t* data, int stride, void* buf, void (*bufDeleter)(void** pBuf), int plane);
    void (*setBuffers)(struct mdkVideoFrame*, uint8_t const** const data, int* strides/*in/out = nullptr*/);
    const uint8_t* (*bufferData)(struct mdkVideoFrame*, int plane);
    int (*bytesPerLine)(struct mdkVideoFrame*, int plane);
    void (*setTimestamp)(struct mdkVideoFrame*, double t);
    double (*timestamp)(struct mdkVideoFrame*);

    struct mdkVideoFrameAPI* (*to)(struct mdkVideoFrame*, enum MDK_PixelFormat format, int width/*= -1*/, int height/*= -1*/);
    bool (*save)(struct mdkVideoFrame*, const char* fileName, const char* format, float quality);

    struct mdkVideoFrameAPI* (*onDestroy)(struct mdkVideoFrame*);
/*!
  \brief mdkVideoFrameAPI_fromDX11
  create a frame containing dx11
  \param pool if *pool not null, the pool will be used, otherwise a new pool will be created and returned. Users usually have to keep the pool object for the same resource producer, release by mdkVideoBufferPoolFree
  \param width frame width, can be 0, then the width is the texture width
  \param height frame height, can be 0, then the height is the texture height
*/
    bool (*fromDX11)(struct mdkVideoFrame*, mdkVideoBufferPool** pool, const mdkDX11Resource* res, int width, int height);
    bool (*fromDX9)(struct mdkVideoFrame*, mdkVideoBufferPool** pool, const mdkDX9Resource* res, int width, int height);
    bool (*fromVAAPI)(struct mdkVideoFrame*, mdkVideoBufferPool** pool, const mdkVAAPIResource* res, int width, int height);
    bool (*fromCUDA)(struct mdkVideoFrame*, mdkVideoBufferPool** pool, const mdkCUDAResource* res, int width, int height);
    bool (*getDX11)(struct mdkVideoFrame*, mdkDX11Resource* res, struct ID3D11Device* dev);
/* The followings are not implemented */
    int (*rotation)(struct mdkVideoFrame*);
    bool (*fromMetal)();
    bool (*fromVk)();
    bool (*fromGL)();
    bool (*fromDX12)();
    bool (*toHost)(struct mdkVideoFrame*);
    void* reserved[8];
} mdkVideoFrameAPI;


MDK_API mdkVideoFrameAPI* mdkVideoFrameAPI_new(int width/*=0*/, int height/*=0*/, MDK_PixelFormat format/*=Unknown*/);
MDK_API void mdkVideoFrameAPI_delete(mdkVideoFrameAPI**);
MDK_API mdkVideoFrameAPI* mdkVideoFrameAPI_ref(mdkVideoFrameAPI* p);
MDK_API void mdkVideoFrameAPI_unref(mdkVideoFrameAPI** pp);

/*
  \brief mdkVideoBufferPoolFree
  free *pool and set null
*/
MDK_API void mdkVideoBufferPoolFree(mdkVideoBufferPool** pool);

#ifdef __cplusplus
}
#endif
