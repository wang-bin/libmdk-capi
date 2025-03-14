/*
 * Copyright (c) 2024-2025 WangBin <wbsecg1 at gmail.com>
 */
#include "mdk/c/AudioFrame.h"
#include "mdk/AudioFrame.h"
#include <cassert>
#include <cstdlib>

using namespace std;
using namespace MDK_NS;

struct mdkAudioFrame {
    AudioFrame frame;
};

AudioFormat::SampleFormat fromC(MDK_SampleFormat fmt)
{
#define CASE_FMT(X) case MDK_SampleFormat_##X: return AudioFormat::SampleFormat::X;
    switch (fmt) {
    CASE_FMT(U8);
    CASE_FMT(S16);
    CASE_FMT(S32);
    CASE_FMT(F32);
    CASE_FMT(F64);
    CASE_FMT(U8P);
    CASE_FMT(S16P);
    CASE_FMT(S32P);
    CASE_FMT(F32P);
    CASE_FMT(F64P);
    default: return AudioFormat::SampleFormat::Unknown;
    }
#undef CASE_FMT
}

MDK_SampleFormat toC(AudioFormat::SampleFormat fmt)
{
#define CASE_FMT(X) case AudioFormat::SampleFormat::X: return MDK_SampleFormat_##X;
    switch (fmt) {
    CASE_FMT(U8);
    CASE_FMT(S16);
    CASE_FMT(S32);
    CASE_FMT(F32);
    CASE_FMT(F64);
    CASE_FMT(U8P);
    CASE_FMT(S16P);
    CASE_FMT(S32P);
    CASE_FMT(F32P);
    CASE_FMT(F64P);
    default: return MDK_SampleFormat_Unknown;
    }
#undef CASE_FMT
}

mdkAudioFrameAPI* MDK_AudioFrame_toC(const AudioFrame& frame);

extern "C" {
static void init_mdkAudioFrameAPI(mdkAudioFrameAPI* p);


void init_mdkAudioFrameAPI(mdkAudioFrameAPI* p)
{
    p->planeCount = [](struct mdkAudioFrame* f) { return f->frame.format().planeCount();};
    p->sampleFormat = [](struct mdkAudioFrame* f) { return toC(f->frame.format().sampleFormat());};
    p->channelMask = [](struct mdkAudioFrame* f) { return (uint64_t)f->frame.format().channelMap();};
    p->channels = [](struct mdkAudioFrame* f) { return f->frame.format().channels();};
    p->sampleRate = [](struct mdkAudioFrame* f) { return f->frame.format().sampleRate();};
    p->addBuffer = [](struct mdkAudioFrame* f, const uint8_t* data, size_t size, int plane, void* buf, void (*bufDeleter)(void** pBuf)) {
        return f->frame.addBuffer(data, size, plane, buf, bufDeleter);
    };
    p->setBuffers = [](struct mdkAudioFrame* f, uint8_t const** const data, int bytesPerPlane) {
        f->frame.setBuffers(data, bytesPerPlane);
    };
    p->bufferData = [](struct mdkAudioFrame* f, int plane) { return f->frame.buffer(plane)->constData();};
    p->bytesPerPlane = [](struct mdkAudioFrame* f) { return (int)f->frame.buffer(0)->size(); };
    p->setSamplesPerChannel = [](struct mdkAudioFrame* f, int samples) { f->frame.setSamplesPerChannel(samples);};
    p->samplesPerChannel = [](struct mdkAudioFrame* f) { return f->frame.samplesPerChannel();};
    p->setTimestamp = [](struct mdkAudioFrame* f, double t) { f->frame.setTimestamp(t); };
    p->timestamp = [](struct mdkAudioFrame* f) { return f->frame.timestamp(); };
    p->duration = [](struct mdkAudioFrame* f) { return f->frame.duration(); };
    p->to = [](struct mdkAudioFrame* f, enum MDK_SampleFormat format, int channels, int sampleRate) {
        return MDK_AudioFrame_toC(f->frame.to({fromC(format), channels, sampleRate}));
    };
}

mdkAudioFrameAPI* mdkAudioFrameAPI_new(enum MDK_SampleFormat format, int channels, int sampleRate, int samples)
{
    mdkAudioFrameAPI* p = new mdkAudioFrameAPI();
    p->object = new mdkAudioFrame();
    p->object->frame = AudioFrame({fromC(format), channels, sampleRate});
    init_mdkAudioFrameAPI(p);
    return p;
}

void mdkAudioFrameAPI_delete(mdkAudioFrameAPI** pp)
{
    if (!pp || !*pp)
        return;
    delete (*pp)->object;
    delete *pp;
    *pp = nullptr;
}

} // extern "C"

mdkAudioFrameAPI* MDK_AudioFrame_toC(const AudioFrame& frame)
{
    if (!frame && frame.timestamp() != TimestampEOS) // TODO: special frames, e.g. EOS
        return nullptr;
    mdkAudioFrameAPI* api = new mdkAudioFrameAPI();
    api->object = new mdkAudioFrame();
    api->object->frame = frame;
    init_mdkAudioFrameAPI(api);
    return api;
}

AudioFrame MDK_AudioFrame_fromC(mdkAudioFrameAPI* p)
{
    if (!p)
        return {};
    return p->object->frame;
}
