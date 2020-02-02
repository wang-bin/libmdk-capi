/*
 * Copyright (c) 2020 WangBin <wbsecg1 at gmail.com>
 * This file is part of MDK
 * MDK SDK: https://sourceforge.net/projects/mdk-sdk/files
 * Free for GPL softwares or non-commercial use.
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 */
#pragma once
#include "global.h"

#ifdef __cplusplus
extern "C" {
#endif
// fromPlanarYUV(w, h, pixdesc, data, strides)
// void* pixLayout()
// fromGL(id, internalfmt, w, h)
struct mdkVideoFrame;

enum MDK_PixelFormat {
    MDK_PixelFormat_Unknown = -1,
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
    MDK_PixelFormat_GBRP,
    MDK_PixelFormat_GBRP10LE,
    MDK_PixelFormat_XYZ12LE,
};

typedef struct mdkVideoFrameAPI {
    mdkVideoFrame* object;

    int (*planeCount)(mdkVideoFrame*);
    int (*width)(mdkVideoFrame*, int plane /*=-1*/);
    int (*height)(mdkVideoFrame*, int plane /*=-1*/);
    MDK_PixelFormat (*format)(mdkVideoFrame*);
    bool (*addBuffer)(mdkVideoFrame*, const uint8_t* data, int stride, void* buf, void (*bufDeleter)(void** pBuf), int plane);
    void (*setBuffers)(mdkVideoFrame*, uint8_t const** const data, int* strides/*in/out = nullptr*/);
    const uint8_t* (*bufferData)(mdkVideoFrame*, int plane);
    int (*bytesPerLine)(mdkVideoFrame*, int plane);
    void (*setTimestamp)(mdkVideoFrame*, double t);
    double (*timestamp)(mdkVideoFrame*);

    mdkVideoFrameAPI* (*to)(mdkVideoFrame*, MDK_PixelFormat format, int width/*= -1*/, int height/*= -1*/);

/* The followings are not implemented */
    mdkVideoFrameAPI* (*toHost)(mdkVideoFrame*);
    mdkVideoFrameAPI* (*fromGL)();
    mdkVideoFrameAPI* (*fromMetal)();
    mdkVideoFrameAPI* (*fromD3D11)();
    mdkVideoFrameAPI* (*fromVk)();
    void* reserved[16];
} mdkVideoFrameAPI;


MDK_API mdkVideoFrameAPI* mdkVideoFrameAPI_new(int width/*=0*/, int height/*=0*/, MDK_PixelFormat format/*=Unknown*/);
MDK_API void mdkVideoFrameAPI_delete(mdkVideoFrameAPI**);

#ifdef __cplusplus
}
#endif
