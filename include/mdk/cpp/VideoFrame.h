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
/*!
  \brief VideoFrame
  Construct a video frame for given format, size. If strides is not null, a single contiguous memory for all planes will be allocated.
  If data is not null, data is copied to allocated memory.
  \param width visual width
  \param height visual height
  \sa setBuffers
   NOTE: Unkine setBuffers(), no memory is allocated for null strides.
 */
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
/*!
  \brief addBuffer
  Add an external buffer to nth plane, store external buffer data ptr. The old buffer will be released.
  \param data external buffer data ptr
  \param stride stride of data. if <=0, it's the stride of current format at this plane
  \param buf external buffer ptr. user should ensure the buffer is alive before frame is destroyed.
  \param bufDeleter to delete buf when frame is destroyed
 */
    bool addBuffer(const uint8_t* data, int stride, void* buf, void (*bufDeleter)(void** pBuf), int plane = -1) {
        return MDK_CALL(p, addBuffer, data, stride, buf, bufDeleter, plane);
    }
/*
   \brief setBuffers
   Add buffers with data copied from given source. Unlike constructor, a single contiguous memory for all planes is always allocated.
   If data is not null, data is copied to allocated memory.
   \param data array of source data planes, array size MUST >= plane count of format if not null. Can be null and allocate memory without copying.
   NOTE: data[i] will be filled with allocated plane address if necessary(data != null && strides != null).
   If data[0] != null but data[i] == null, assume copying from contiguous source data.
   \param strides array of plane strides, size MUST >= plane count of format if not null. Can be null and strides[i] can be <=0 indicating no padding bytes (for plane i).
   NOTE: strides[i] will be filled with allocated plane i stride if necessary(strides[i] <= 0)
 */
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
/*!
  \brief to
  The result frame data is always on host memory. If it's already on host memory and the same as target format, return the current frame.
  \param fmt output format. if invalid, same as format()
  \param width output width. if invalid(<=0), same as width()
  \param height output height. if invalid(<=0), same as height()
  if all output parameters(invalid) are the same as input, return self
  \return Invalid frame if failed
 */
    VideoFrame to(PixelFormat format, int width = -1, int height = -1) {
        return VideoFrame(MDK_CALL(p, to, MDK_PixelFormat(format), width, height));
    }
private:
    mdkVideoFrameAPI* p = nullptr;
    bool owner_ = true;
};

MDK_NS_END