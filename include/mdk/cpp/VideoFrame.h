/*
 * Copyright (c) 2019-2020 WangBin <wbsecg1 at gmail.com>
 * This file is part of MDK
 * MDK SDK: https://sourceforge.net/projects/mdk-sdk/files
 * Free for GPL softwares or non-commercial use.
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 */
#pragma once
#include "global.h"
#include "../c/VideoFrame.h"
#include <algorithm>

MDK_NS_BEGIN

enum class PixelFormat
{
    Unknown = -1,
    YUV420P,
    NV12,
    P010LE,
    RGB24,
    RGBA,
};

class VideoFrame
{
public:
    VideoFrame(const VideoFrame&) = delete;
    VideoFrame& operator=(const VideoFrame&) = delete;
    VideoFrame(VideoFrame&& that) {
        std::swap(p, that.p);
        std::swap(owner_, that.owner_);
    }
    VideoFrame& operator=(VideoFrame&& that)  {
        std::swap(p, that.p);
        std::swap(owner_, that.owner_);
        return *this;
    }

    VideoFrame() = default;

    VideoFrame(int width, int height, PixelFormat format, int* strides/*in/out*/ = nullptr, uint8_t const** const data/*in/out*/ = nullptr) {
        p = mdkVideoFrameAPI_new(width, height, MDK_PixelFormat(format));
        if (data)
            MDK_CALL(p, setBuffers, data, strides);
    }

    VideoFrame(mdkVideoFrameAPI* pp) : p(pp) {}

    ~VideoFrame() {
        if (owner_)
            mdkVideoFrameAPI_delete(&p);
    }

    void attach(mdkVideoFrameAPI* api) {
        if (owner_)
            mdkVideoFrameAPI_delete(&p);
        p = api;
        owner_ = false;
    }

    mdkVideoFrameAPI* detach() {
        auto ptr = p;
        p = nullptr;
        return ptr;
    }

    int planeCount() const { return MDK_CALL(p, planeCount); }

    int width(int plane = -1) const {
        return MDK_CALL(p, width, plane);
    }

    int height(int plane = -1) const {
        return MDK_CALL(p, height, plane);
    }

    PixelFormat format() const {
        return (PixelFormat)MDK_CALL(p, format);
    }

    void setBuffers(mdkVideoFrame*, uint8_t const** const data, int* strides/*in/out*/ = nullptr) {
        MDK_CALL(p, setBuffers, data, strides);
    }

    const uint8_t* bufferData(int plane = 0) const {
        return MDK_CALL(p, bufferData, plane);
    }

    int bytesPerLine(int plane = 0) const {
        return MDK_CALL(p, bytesPerLine, plane);
    }

    void setTimestamp(double t) {
        return MDK_CALL(p, setTimestamp, t);
    }

    double timestamp() const {
        return MDK_CALL(p, timestamp);
    }

    VideoFrame to(PixelFormat format, int width = -1, int height = -1) {
        return VideoFrame(MDK_CALL(p, to, MDK_PixelFormat(format), width, height));
    }
private:
    mdkVideoFrameAPI* p = nullptr;
    bool owner_ = true;
};

MDK_NS_END