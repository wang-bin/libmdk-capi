/*
 * Copyright (c) 2019 WangBin <wbsecg1 at gmail.com>
 */
#include "mdk/c/Player.h"
#include "mdk/Player.h"
#include "mdk/MediaInfo.h"
#include <iostream>

using namespace std;
using namespace MDK_NS;
struct MDK_Player : Player{
    MediaInfo info;
};

extern "C" {

MDK_Player* MDK_Player_new()
{
    return new MDK_Player();
}

void MDK_Player_delete(MDK_Player** p) {
    delete *p;
    *p = nullptr;
}

// MUST be called when a foreign OpenGL context previously used is being destroyed to release context resources. The context MUST be current.
void MDK_foreignGLContextDestroyed()
{
    Player::foreignGLContextDestroyed();
}

void MDK_Player_setMute(MDK_Player* p, bool value)
{
    p->setMute(value);
}

void MDK_Player_setVolume(MDK_Player* p, float value)
{
    p->setVolume(value);
}

void MDK_Player_setMedia(MDK_Player* p, const char* url)
{
    p->setMedia(url);
}

void MDK_Player_setMediaForType(MDK_Player* p, const char* url, MDK_MediaType type)
{
    p->setMedia(url, MediaType(type));
}

const char* MDK_Player_url(MDK_Player* p)
{
    return p->url();
}

void MDK_Player_setPreloadImmediately(MDK_Player* p, bool value)
{
    p->setPreloadImmediately(value);
}

void MDK_Player_setNextMedia(MDK_Player* p, const char* url, int64_t startPosition)
{
    p->setNextMedia(url, startPosition);
}

void MDK_Player_currentMediaChanged(MDK_Player* p, MDK_CurrentMediaChangedCallback cb)
{
    if (!cb.opaque) {
        p->currentMediaChanged(nullptr);
        return;
    }
    p->currentMediaChanged([cb]{
        cb.cb(cb.opaque);
    });
}

void MDK_Player_setAudioBackends(MDK_Player* p, const char** names)
{
    if (!names) // TODO: default
        return;
    vector<string> s;
    while (*names) {
        s.push_back(*names++);
    }
    p->setAudioBackends(s);
}

void MDK_Player_setAudioDecoders(MDK_Player* p, const char** names)
{
    if (!names) // TODO: default
        return;
    vector<string> s;
    while (*names) {
        s.push_back(*names++);
    }
    p->setAudioDecoders(s);
}

void MDK_Player_setVideoDecoders(MDK_Player* p, const char** names)
{
    if (!names) // TODO: default
        return;
    vector<string> s;
    while (*names) {
        s.push_back(*names++);
    }
    p->setVideoDecoders(s);
}

void MDK_Player_setTimeout(MDK_Player* p, int64_t value, MDK_TimeoutCallback cb)
{
    if (!cb.opaque) {
        p->setTimeout(value, nullptr);
        return;
    }
    p->setTimeout(value, [cb](int64_t ms){
        return cb.cb(ms, cb.opaque);
    });
}

void MDK_Player_prepare(MDK_Player* p, int64_t startPosition, MDK_PrepareCallback cb)
{
    if (!cb.opaque) {
        p->prepare(startPosition, nullptr);
        return;
    }
    p->prepare(startPosition, [cb](int64_t position, bool* boost){
        cb.cb(position, boost, cb.opaque);
    });
}

const MDK_MediaInfo* MDK_Player_mediaInfo(MDK_Player*);

void MDK_Player_setState(MDK_Player* p, MDK_State value)
{
    p->setState(State(value));
}

MDK_State MDK_Player_state(MDK_Player* p)
{
    return (MDK_State)p->state();
}

void MDK_Player_onStateChanged(MDK_Player* p, MDK_StateChangedCallback cb)
{
    if (!cb.opaque) {
        p->onStateChanged(nullptr);
        return;
    }
    p->onStateChanged([cb](State value){
        return cb.cb(MDK_State(value), cb.opaque);
    });
}

bool MDK_Player_waitFor(MDK_Player* p, MDK_State value, long timeout)
{
    return p->waitFor(State(value), timeout);
}

MDK_MediaStatus MDK_Player_mediaStatus(MDK_Player* p)
{
    return (MDK_MediaStatus)p->mediaStatus();
}

void MDK_Player_onMediaStatusChanged(MDK_Player* p, MDK_MediaStatusChangedCallback cb)
{
    if (!cb.opaque) {
        p->onMediaStatusChanged(nullptr);
        return;
    }
    p->onMediaStatusChanged([cb](MediaStatus value){
        return cb.cb(MDK_MediaStatus(value), cb.opaque);
    });
}

void MDK_Player_updateNativeWindow(MDK_Player* p, void* win, int width, int height, MDK_SurfaceType type)
{
    p->updateNativeWindow(win, width, height, Player::SurfaceType(type));
}

void MDK_Player_createWindow(MDK_Player* p, void* nativeHandle, MDK_SurfaceType type)
{
    p->createWindow(nativeHandle, Player::SurfaceType(type));
}

void MDK_Player_resizeWindow(MDK_Player* p, int w, int h)
{
    p->resizeWindow(w, h);
}

void MDK_Player_showWindow(MDK_Player* p)
{
    p->showWindow();
}

void MDK_Player_getVideoFrame(MDK_Player* p, MDK_VideoFrame* frame, void* vo_opaque);

void MDK_Player_setVideoSurfaceSize(MDK_Player* p, int width, int height, void* vo_opaque)
{
    p->setVideoSurfaceSize(width, height, vo_opaque);
}

void MDK_Player_setVideoViewport(MDK_Player* p, float x, float y, float w, float h, void* vo_opaque)
{
    p->setVideoViewport(x, y, w, h, vo_opaque);
}

void MDK_Player_setAspectRatio(MDK_Player* p, float value, void* vo_opaque)
{
    p->setAspectRatio(value, vo_opaque);
}

void MDK_Player_rotate(MDK_Player* p, int degree, void* vo_opaque)
{
    p->rotate(degree, vo_opaque);
}

void MDK_Player_scale(MDK_Player* p, float x, float y, void* vo_opaque)
{
    p->scale(x, y, vo_opaque);
}

double MDK_Player_renderVideo(MDK_Player* p, void* vo_opaque)
{
    return p->renderVideo(vo_opaque);
}

void MDK_Player_setRenderCallback(MDK_Player* p, MDK_RenderCallback cb)
{
    if (!cb.opaque) {
        p->setRenderCallback(nullptr);
        return;
    }
    p->setRenderCallback([cb](void* vo_opaque){
        cb.cb(vo_opaque, cb.opaque);
    });
}

void MDK_Player_onVideoFrame(MDK_Player*, void (*)(MDK_VideoFrame*));
void MDK_Player_onAudioFrame(MDK_Player*, void (*)(MDK_AudioFrame*));

int64_t MDK_Player_position(MDK_Player* p)
{
    return p->position();
}

bool MDK_Player_seekWithFlags(MDK_Player* p, int64_t pos, MDK_SeekFlag flags, MDK_SeekCallback cb)
{
    if (!cb.opaque) {
        return p->seek(pos, SeekFlag(flags), nullptr);
    }
    return p->seek(pos, SeekFlag(flags), [cb](int64_t value){
        cb.cb(value, cb.opaque);
    });
}

void MDK_Player_setPlaybackRate(MDK_Player* p, float value)
{
    p->setPlaybackRate(value);
}

float MDK_Player_playbackRate(MDK_Player* p)
{
    return p->playbackRate();
}

int64_t MDK_Player_buffered(MDK_Player* p, int64_t* bytes)
{
    return p->buffered(bytes);
}

void MDK_Player_switchBitrate(MDK_Player* p, const char* url, int64_t delay, SwitchBitrateCallback cb)
{
    if (!cb.opaque) {
        p->switchBitrate(url, delay, nullptr);
        return;
    }
    p->switchBitrate(url, delay, [cb](bool value){
        cb.cb(value, cb.opaque);
    });
}

bool MDK_Player_switchBitrateSingleConnection(MDK_Player* p, const char *url, SwitchBitrateCallback cb)
{
    if (!cb.opaque) {
        return p->switchBitrateSingleConnection(url, nullptr);
    }
    return p->switchBitrateSingleConnection(url, [cb](bool value){
        cb.cb(value, cb.opaque);
    });
}

int64_t MDK_Player_addListener(MDK_Player* p, MDK_MediaEventListener cb)
{
    if (!cb.opaque) {
        return p->addListener(nullptr);
    }
    return p->addListener([cb](const MediaEvent& e){
        MDK_MediaEvent me{};
        me.error = e.error;
        me.category = e.category.data();
        me.detail = e.detail.data();
        me.decoder.stream = e.decoder.stream;
        return cb.cb(&me, cb.opaque);
    });
}

void MDK_Player_removeListener(MDK_Player* p, int64_t listener)
{
    p->removeListener(listener);
}

} // extern "C"