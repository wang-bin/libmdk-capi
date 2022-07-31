/*
 * Copyright (c) 2020-2022 WangBin <wbsecg1 at gmail.com>
 */
#include "mdk/c/VideoFrame.h"
#include "mdk/VideoFrame.h"
#include <cassert>
#include <cstdlib>

using namespace std;
using namespace MDK_NS;

struct mdkVideoFrame {
    VideoFrame frame;
};

PixelFormat fromC(MDK_PixelFormat fmt)
{
#define CASE_FMT(X) case MDK_PixelFormat_##X: return PixelFormat::X;
    switch (fmt) {
    CASE_FMT(YUVA420P);
    CASE_FMT(YUV420P);
    CASE_FMT(NV12);
    CASE_FMT(YUV422P);
    CASE_FMT(YUV444P);
    CASE_FMT(P010LE);
    CASE_FMT(P016LE);
    CASE_FMT(YUV420P10LE);
    CASE_FMT(UYVY422);
    CASE_FMT(RGB24);
    CASE_FMT(RGBA);
    CASE_FMT(RGBX);
    CASE_FMT(BGRA);
    CASE_FMT(BGRX);
    CASE_FMT(RGB565LE);
    CASE_FMT(RGB48LE);
    CASE_FMT(GBRP);
    CASE_FMT(GBRP10LE);
    CASE_FMT(XYZ12LE);
    CASE_FMT(BC1);
    CASE_FMT(BC3);
    CASE_FMT(RGBA64);
    CASE_FMT(BGRA64);
    CASE_FMT(RGBP16);
    CASE_FMT(RGBPF32);
    CASE_FMT(BGRAF32);
    default: return PixelFormat::Unknown;
    }
#undef CASE_FMT
}

MDK_PixelFormat toC(PixelFormat fmt)
{
#define CASE_FMT(X) case PixelFormat::X: return MDK_PixelFormat_##X;
    switch (fmt) {
    CASE_FMT(YUVA420P);
    CASE_FMT(YUV420P);
    CASE_FMT(NV12);
    CASE_FMT(YUV422P);
    CASE_FMT(YUV444P);
    CASE_FMT(P010LE);
    CASE_FMT(P016LE);
    CASE_FMT(YUV420P10LE);
    CASE_FMT(UYVY422);
    CASE_FMT(RGB24);
    CASE_FMT(RGBA);
    CASE_FMT(RGBX);
    CASE_FMT(BGRA);
    CASE_FMT(BGRX);
    CASE_FMT(RGB565LE);
    CASE_FMT(RGB48LE);
    CASE_FMT(GBRP);
    CASE_FMT(GBRP10LE);
    CASE_FMT(XYZ12LE);
    CASE_FMT(BC1);
    CASE_FMT(BC3);
    CASE_FMT(RGBA64);
    CASE_FMT(BGRA64);
    CASE_FMT(RGBP16);
    CASE_FMT(RGBPF32);
    CASE_FMT(BGRAF32);
    default: return MDK_PixelFormat_Unknown;
    }
#undef CASE_FMT
}

mdkVideoFrameAPI* MDK_VideoFrame_toC(const VideoFrame& frame);

extern "C" {
static void init_mdkVideoFrameAPI(mdkVideoFrameAPI* p);

int MDK_VideoFrame_planeCount(mdkVideoFrame* p)
{
    return p->frame.format().planeCount();
}

int MDK_VideoFrame_width(mdkVideoFrame* p, int plane /*=-1*/)
{
    return p->frame.width(plane);
}

int MDK_VideoFrame_height(mdkVideoFrame* p, int plane /*=-1*/)
{
    return p->frame.height(plane);
}

MDK_PixelFormat MDK_VideoFrame_format(mdkVideoFrame* p)
{
    return toC(p->frame.format());
}

bool MDK_VideoFrame_addBuffer(mdkVideoFrame* p, const uint8_t* data, int stride, void* buf, void (*bufDeleter)(void** pBuf), int plane)
{
    return p->frame.addBuffer(data, stride, buf, bufDeleter, plane);
}

void MDK_VideoFrame_setBuffers(mdkVideoFrame* p, uint8_t const** const data, int* strides/*in/out = nullptr*/)
{
    p->frame.setBuffers(data, strides);
}

const uint8_t* MDK_VideoFrame_bufferData(mdkVideoFrame* p, int plane)
{
    return p->frame.buffer(plane)->data();
}

int MDK_VideoFrame_bytesPerLine(mdkVideoFrame* p, int plane)
{
    return p->frame.bytesPerLine(plane);
}

void MDK_VideoFrame_setTimestamp(mdkVideoFrame* p, double t)
{
    p->frame.setTimestamp(t);
}

double MDK_VideoFrame_timestamp(mdkVideoFrame* p)
{
    return p->frame.timestamp();
}

mdkVideoFrameAPI* MDK_VideoFrame_to(mdkVideoFrame* p, MDK_PixelFormat format, int width/*= -1*/, int height/*= -1*/)
{
    return MDK_VideoFrame_toC(p->frame.to(fromC(format), width, height));
}

bool MDK_VideoFrame_save(mdkVideoFrame* p, const char* fileName, const char* format, float quality)
{
    return p->frame.save(fileName, format, quality);
}

void init_mdkVideoFrameAPI(mdkVideoFrameAPI* p)
{
#define SET_API(FN) p->FN = MDK_VideoFrame_##FN
    SET_API(planeCount);
    SET_API(width);
    SET_API(height);
    SET_API(format);
    SET_API(setBuffers);
    SET_API(bufferData);
    SET_API(bytesPerLine);
    SET_API(setTimestamp);
    SET_API(timestamp);
    SET_API(to);
    SET_API(save);
#undef SET_API
}

mdkVideoFrameAPI* mdkVideoFrameAPI_new(int width/*=0*/, int height/*=0*/, MDK_PixelFormat format/*=Unknown*/)
{
    mdkVideoFrameAPI* p = new mdkVideoFrameAPI();
    p->object = new mdkVideoFrame();
    p->object->frame = VideoFrame(width, height, fromC(format));
    init_mdkVideoFrameAPI(p);
    return p;
}

void mdkVideoFrameAPI_delete(mdkVideoFrameAPI** pp)
{
    if (!pp || !*pp)
        return;
    delete (*pp)->object;
    delete *pp;
    *pp = nullptr;
}
} // extern "C"

mdkVideoFrameAPI* MDK_VideoFrame_toC(const VideoFrame& frame)
{
    if (!frame) // TODO: special frames, e.g. EOS
        return nullptr;
    mdkVideoFrameAPI* api = new mdkVideoFrameAPI();
    api->object = new mdkVideoFrame();
    api->object->frame = frame;
    init_mdkVideoFrameAPI(api);
    return api;
}

VideoFrame MDK_VideoFrame_fromC(mdkVideoFrameAPI* p)
{
    if (!p)
        return {};
    return p->object->frame;
}