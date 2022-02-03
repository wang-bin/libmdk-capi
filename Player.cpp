/*
 * Copyright (c) 2019-2022 WangBin <wbsecg1 at gmail.com>
 */
#include "mdk/c/Player.h"
#include "mdk/c/MediaInfo.h"
#include "mdk/c/VideoFrame.h"
#include "mdk/Player.h"
#include "mdk/MediaInfo.h"
#include "mdk/VideoFrame.h"
#include "mdk/RenderAPI.h"
#include "MediaInfoInternal.h"
#include <cassert>
#include <cstdlib>
#include <iostream>

using namespace std;
using namespace MDK_NS;

extern mdkVideoFrameAPI* MDK_VideoFrame_toC(const VideoFrame& frame);
extern VideoFrame MDK_VideoFrame_fromC(mdkVideoFrameAPI* p);
extern unique_ptr<RenderAPI> from_c(MDK_RenderAPI type, void* data);

struct mdkPlayer : Player{
    MediaInfoInternal media_info;
};

extern "C" {

void MDK_Player_setMute(mdkPlayer* p, bool value)
{
    p->setMute(value);
}

void MDK_Player_setVolume(mdkPlayer* p, float value)
{
    p->setVolume(value);
}

void MDK_Player_setChannelVolume(mdkPlayer* p, float value, int channel)
{
    p->setVolume(value, channel);
}

void MDK_Player_setMedia(mdkPlayer* p, const char* url)
{
    p->setMedia(url);
}

void MDK_Player_setMediaForType(mdkPlayer* p, const char* url, MDK_MediaType type)
{
    p->setMedia(url, MediaType(type));
}

const char* MDK_Player_url(mdkPlayer* p)
{
    return p->url();
}

void MDK_Player_setPreloadImmediately(mdkPlayer* p, bool value)
{
    p->setPreloadImmediately(value);
}

void MDK_Player_setNextMedia(mdkPlayer* p, const char* url, int64_t startPosition, MDKSeekFlag flag)
{
    p->setNextMedia(url, startPosition, SeekFlag(flag));
}

void MDK_Player_currentMediaChanged(mdkPlayer* p, mdkCurrentMediaChangedCallback cb)
{
    if (!cb.opaque) {
        p->currentMediaChanged(nullptr);
        return;
    }
    p->currentMediaChanged([cb]{
        cb.cb(cb.opaque);
    });
}

void MDK_Player_setAudioBackends(mdkPlayer* p, const char** names)
{
    if (!names) // TODO: default
        return;
    vector<string> s;
    while (*names) {
        s.emplace_back(*names++);
    }
    p->setAudioBackends(s);
}

void MDK_Player_setDecoders(mdkPlayer* p, MDK_MediaType type, const char* names[])
{
    if (!names) // TODO: default
        return;
    vector<string> s;
    while (*names) {
        s.emplace_back(*names++);
    }
    p->setDecoders(MediaType(type), s);
}

void MDK_Player_setAudioDecoders(mdkPlayer* p, const char** names)
{
    MDK_Player_setDecoders(p, MDK_MediaType_Audio, names);
}

void MDK_Player_setVideoDecoders(mdkPlayer* p, const char* names[])
{
    MDK_Player_setDecoders(p, MDK_MediaType_Video, names);
}

void MDK_Player_setTimeout(mdkPlayer* p, int64_t value, mdkTimeoutCallback cb)
{
    if (!cb.opaque) {
        p->setTimeout(value, nullptr);
        return;
    }
    p->setTimeout(value, [cb](int64_t ms){
        return cb.cb(ms, cb.opaque);
    });
}

void MDK_Player_prepare(mdkPlayer* p, int64_t startPosition, mdkPrepareCallback cb, MDKSeekFlag flag)
{
    if (!cb.opaque) {
        p->prepare(startPosition, nullptr, SeekFlag(flag));
        return;
    }
    p->prepare(startPosition, [cb](int64_t position, bool* boost){
        return cb.cb(position, boost, cb.opaque);
    }, SeekFlag(flag));
}

const mdkMediaInfo* MDK_Player_mediaInfo(mdkPlayer* p)
{
    MediaInfoToC(p->mediaInfo(), &p->media_info);
    return &p->media_info.info;
}

void MDK_Player_setState(mdkPlayer* p, MDK_State value)
{
    p->set(State(value));
}

MDK_State MDK_Player_state(mdkPlayer* p)
{
    return (MDK_State)p->state();
}

void MDK_Player_onStateChanged(mdkPlayer* p, mdkStateChangedCallback cb)
{
    if (!cb.opaque) {
        p->onStateChanged(nullptr);
        return;
    }
    p->onStateChanged([cb](State value){
        return cb.cb(MDK_State(value), cb.opaque);
    });
}

bool MDK_Player_waitFor(mdkPlayer* p, MDK_State value, long timeout)
{
    return p->waitFor(State(value), timeout);
}

MDK_MediaStatus MDK_Player_mediaStatus(mdkPlayer* p)
{
    return (MDK_MediaStatus)p->mediaStatus();
}

void MDK_Player_onMediaStatusChanged(mdkPlayer* p, mdkMediaStatusChangedCallback cb)
{
    if (!cb.opaque) {
        p->onMediaStatusChanged(nullptr);
        return;
    }
    p->onMediaStatusChanged([cb](MediaStatus value){
        return cb.cb(MDK_MediaStatus(value), cb.opaque);
    });
}

void MDK_Player_updateNativeSurface(mdkPlayer* p, void* win, int width, int height, MDK_SurfaceType type)
{
    p->updateNativeSurface(win, width, height, Player::SurfaceType(type));
}

void MDK_Player_createSurface(mdkPlayer* p, void* nativeHandle, MDK_SurfaceType type)
{
    p->createSurface(nativeHandle, Player::SurfaceType(type));
}

void MDK_Player_resizeSurface(mdkPlayer* p, int w, int h)
{
    p->resizeSurface(w, h);
}

void MDK_Player_showSurface(mdkPlayer* p)
{
    p->showSurface();
}

void MDK_Player_getVideoFrame(mdkPlayer* p, mdkVideoFrame* frame, void* vo_opaque);

void MDK_Player_setVideoSurfaceSize(mdkPlayer* p, int width, int height, void* vo_opaque)
{
    p->setVideoSurfaceSize(width, height, vo_opaque);
}

void MDK_Player_setVideoViewport(mdkPlayer* p, float x, float y, float w, float h, void* vo_opaque)
{
    p->setVideoViewport(x, y, w, h, vo_opaque);
}

void MDK_Player_setAspectRatio(mdkPlayer* p, float value, void* vo_opaque)
{
    p->setAspectRatio(value, vo_opaque);
}

void MDK_Player_rotate(mdkPlayer* p, int degree, void* vo_opaque)
{
    p->rotate(degree, vo_opaque);
}

void MDK_Player_scale(mdkPlayer* p, float x, float y, void* vo_opaque)
{
    p->scale(x, y, vo_opaque);
}

void MDK_Player_setRenderAPI(mdkPlayer* p, mdkRenderAPI* api, void* vo_opaque)
{
    p->setRenderAPI(from_c(*reinterpret_cast<MDK_RenderAPI*>(api), api).get(), vo_opaque);
}

mdkRenderAPI* MDK_Player_renderAPI(mdkPlayer* p, void* vo_opaque)
{
    return reinterpret_cast<mdkRenderAPI*>(p->renderAPI(vo_opaque));
}

double MDK_Player_renderVideo(mdkPlayer* p, void* vo_opaque)
{
    return p->renderVideo(vo_opaque);
}

void MDK_Player_setBackgroundColor(mdkPlayer* p, float r, float g, float b, float a, void* vo_opaque)
{
    p->setBackgroundColor(r, g, b, a, vo_opaque);
}

void MDK_Player_setRenderCallback(mdkPlayer* p, mdkRenderCallback cb)
{
    if (!cb.opaque) {
        p->setRenderCallback(nullptr);
        return;
    }
    p->setRenderCallback([cb](void* vo_opaque){
        cb.cb(vo_opaque, cb.opaque);
    });
}

void MDK_Player_onVideo(mdkPlayer* p, mdkVideoCallback cb)
{
    if (!cb.opaque) {
        p->onFrame<VideoFrame>(nullptr);
        return;
    }
    p->onFrame<VideoFrame>([cb](VideoFrame& frame, int track){
        auto f = MDK_VideoFrame_toC(frame);
        auto f0 = f;
        auto ret = cb.cb(&f, track, cb.opaque);
        if (f != f0)
            frame = MDK_VideoFrame_fromC(f);
        mdkVideoFrameAPI_delete(&f);
        return ret;
    });
}

void MDK_Player_onAudio(mdkPlayer*);

int64_t MDK_Player_position(mdkPlayer* p)
{
    return p->position();
}

bool MDK_Player_seekWithFlags(mdkPlayer* p, int64_t pos, MDK_SeekFlag flags, mdkSeekCallback cb)
{
    if (!cb.opaque) {
        return p->seek(pos, SeekFlag(flags), nullptr);
    }
    return p->seek(pos, SeekFlag(flags), [cb](int64_t value){
        cb.cb(value, cb.opaque);
    });
}

bool MDK_Player_seek(mdkPlayer* p, int64_t pos, mdkSeekCallback cb)
{
    return MDK_Player_seekWithFlags(p, pos, MDK_SeekFlag_Default, cb);
}

void MDK_Player_setPlaybackRate(mdkPlayer* p, float value)
{
    p->setPlaybackRate(value);
}

float MDK_Player_playbackRate(mdkPlayer* p)
{
    return p->playbackRate();
}

int64_t MDK_Player_buffered(mdkPlayer* p, int64_t* bytes)
{
    return p->buffered(bytes);
}

void MDK_Player_setBufferRange(mdkPlayer* p, int64_t minMs, int64_t maxMs, bool drop)
{
    p->setBufferRange(minMs, maxMs, drop);
}

void MDK_Player_switchBitrate(mdkPlayer* p, const char* url, int64_t delay, SwitchBitrateCallback cb)
{
    if (!cb.opaque) {
        p->switchBitrate(url, delay, nullptr);
        return;
    }
    p->switchBitrate(url, delay, [cb](bool value){
        cb.cb(value, cb.opaque);
    });
}

bool MDK_Player_switchBitrateSingleConnection(mdkPlayer* p, const char *url, SwitchBitrateCallback cb)
{
    if (!cb.opaque) {
        return p->switchBitrateSingleConnection(url, nullptr);
    }
    return p->switchBitrateSingleConnection(url, [cb](bool value){
        cb.cb(value, cb.opaque);
    });
}

void MDK_Player_onEvent(mdkPlayer* p, mdkMediaEventCallback cb, MDK_CallbackToken* token)
{
    if (!cb.opaque) {
        p->onEvent(nullptr, token);
        return;
    }
    p->onEvent([cb](const MediaEvent& e){
        mdkMediaEvent me{};
        me.error = e.error;
        me.category = e.category.data();
        me.detail = e.detail.data();
        me.decoder.stream = e.decoder.stream;
        return cb.cb(&me, cb.opaque);
    }, token);
}

void MDK_Player_snapshot(mdkPlayer* p, mdkSnapshotRequest* request, mdkSnapshotCallback cb, void* vo_opaque)
{
    assert(cb.cb && "mdkSnapshotCallback.cb can not be null");
    Player::SnapshotRequest r;
    r.width = request->width;
    r.height = request->height;
    r.stride = request->stride;
    r.subtitle = request->subtitle;
    if (request->data)
        r.buf = make_shared<Buffer2DView>(request->stride, request->height, request->data);
    p->snapshot(&r, [cb](Player::SnapshotRequest* req, double frameTime){
        if (!req)
            return string();
        mdkSnapshotRequest q;
        q.data = (uint8_t*)req->buf->constData();
        q.width = req->width;
        q.height = req->height;
        q.stride = req->stride;
        q.subtitle = req->subtitle;
        auto filec = cb.cb(&q, frameTime, cb.opaque);
        if (!filec)
            return string();
        string file(filec);
        free(filec);
        return file;
    }, vo_opaque);
}

void MDK_Player_setProperty(mdkPlayer* p, const char* key, const char* value)
{
    p->setProperty(key, value);
}

const char* MDK_Player_getProperty(mdkPlayer* p, const char* key)
{
    const auto& value = p->property(key);
    if (value.empty())
        return nullptr;
    return value.data();
}

void MDK_Player_record(mdkPlayer* p, const char* url, const char* format)
{
    p->record(url, format);
}

void MDK_Player_setLoopRange(mdkPlayer* p, int count, int64_t a, int64_t b)
{
    p->setLoop(count);
    p->setRange(a, b);
}

void MDK_Player_onLoop(mdkPlayer* p, mdkLoopCallback cb, MDK_CallbackToken* token)
{
    if (!cb.opaque) {
        p->onLoop(nullptr, token);
        return;
    }
    p->onLoop([cb](int count){
        cb.cb(count, cb.opaque);
    }, token);
}

void MDK_Player_setLoop(mdkPlayer* p, int count)
{
    p->setLoop(count);
}

void MDK_Player_setRange(mdkPlayer* p, int64_t a, int64_t b)
{
    p->setRange(a, b);
}

void MDK_Player_mapPoint(mdkPlayer* p, MDK_MapDirection dir, float* x, float* y, float* z, void* vo_opaque)
{
    p->mapPoint(Player::MapDirection(dir), x, y, z, vo_opaque);
}

void MDK_Player_setPointMap(mdkPlayer* p, const float* videoRoi, const float* viewRoi, int count, void* vo_opaque)
{
    p->setPointMap(videoRoi, viewRoi, count, vo_opaque);
}

void MDK_Player_onSync(mdkPlayer* p, mdkSyncCallback cb, int minInterval)
{
    p->onSync([cb]{
        return cb.cb(cb.opaque);
    }, minInterval);
}

void MDK_Player_setVideoEffect(mdkPlayer* p, MDK_VideoEffect effect, const float* values, void* vo_opaque)
{
    p->set(VideoEffect(effect), *values, vo_opaque);
}

void MDK_Player_setActiveTracks(mdkPlayer* p, MDK_MediaType type, const int* tracks, size_t count)
{
    set<int> t;
    for (int i = 0; i < count; ++i)
        t.insert(tracks[i]);
    p->setActiveTracks(MediaType(type), set<int>(tracks, tracks + count));
}

void MDK_Player_setFrameRate(mdkPlayer* p, float value)
{
    p->setFrameRate(value);
}

const mdkPlayerAPI* mdkPlayerAPI_new()
{
    mdkPlayerAPI* p = new mdkPlayerAPI();
    p->object = new mdkPlayer();
#define SET_API(FN) p->FN = MDK_Player_##FN
    SET_API(setMute);
    SET_API(setVolume);
    SET_API(setChannelVolume);
    SET_API(setMedia);
    SET_API(setMediaForType);
    SET_API(url);
    SET_API(setPreloadImmediately);
    SET_API(setNextMedia);
    SET_API(currentMediaChanged);
    SET_API(setAudioBackends);
    SET_API(setAudioDecoders);
    SET_API(setVideoDecoders);
    SET_API(setTimeout);
    SET_API(prepare);
    SET_API(mediaInfo);
    SET_API(setState);
    SET_API(state);
    SET_API(onStateChanged);
    SET_API(waitFor);
    SET_API(mediaStatus);
    SET_API(onMediaStatusChanged);
    SET_API(updateNativeSurface);
    SET_API(createSurface);
    SET_API(resizeSurface);
    SET_API(showSurface);
    //SET_API(getVideoFrame);
    SET_API(setVideoSurfaceSize);
    SET_API(setVideoViewport);
    SET_API(setAspectRatio);
    SET_API(rotate);
    SET_API(scale);
    SET_API(renderVideo);
    SET_API(setBackgroundColor);
    SET_API(setRenderCallback);
    SET_API(position);
    SET_API(seekWithFlags);
    SET_API(seek);
    SET_API(setPlaybackRate);
    SET_API(playbackRate);
    SET_API(buffered);
    SET_API(setBufferRange);
    SET_API(switchBitrate);
    SET_API(switchBitrateSingleConnection);
    SET_API(onEvent);
    SET_API(snapshot);
    SET_API(setProperty);
    SET_API(getProperty);
    SET_API(record);
    SET_API(setLoopRange);
    SET_API(setLoop);
    SET_API(onLoop);
    SET_API(setRange);
    SET_API(setRenderAPI);
    SET_API(renderAPI);
    SET_API(onVideo);
    SET_API(mapPoint);
    SET_API(onSync);
    SET_API(setVideoEffect);
    SET_API(setActiveTracks);
    SET_API(setDecoders);
    SET_API(setFrameRate);
    SET_API(setPointMap);
#undef SET_API
    return p;
}

void mdkPlayerAPI_delete(const mdkPlayerAPI** pp)
{
    if (!pp || !*pp)
        return;
    delete (*pp)->object;
    delete *pp;
    *pp = nullptr;
}

void MDK_foreignGLContextDestroyed()
{
    Player::foreignGLContextDestroyed();
}

} // extern "C"