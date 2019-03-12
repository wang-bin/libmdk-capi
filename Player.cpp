/*
 * Copyright (c) 2019 WangBin <wbsecg1 at gmail.com>
 */
#include "mdk/c/Player.h"
#include "mdk/c/MediaInfo.h"
#include "mdk/Player.h"
#include "mdk/MediaInfo.h"
#include "MediaInfoInternal.h"

using namespace std;
using namespace MDK_NS;

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

void MDK_Player_setNextMedia(mdkPlayer* p, const char* url, int64_t startPosition)
{
    p->setNextMedia(url, startPosition);
}

void MDK_Player_currentMediaChanged(mdkPlayer* p, MDK_CurrentMediaChangedCallback cb)
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
        s.push_back(*names++);
    }
    p->setAudioBackends(s);
}

void MDK_Player_setAudioDecoders(mdkPlayer* p, const char** names)
{
    if (!names) // TODO: default
        return;
    vector<string> s;
    while (*names) {
        s.push_back(*names++);
    }
    p->setAudioDecoders(s);
}

void MDK_Player_setVideoDecoders(mdkPlayer* p, const char** names)
{
    if (!names) // TODO: default
        return;
    vector<string> s;
    while (*names) {
        s.push_back(*names++);
    }
    p->setVideoDecoders(s);
}

void MDK_Player_setTimeout(mdkPlayer* p, int64_t value, MDK_TimeoutCallback cb)
{
    if (!cb.opaque) {
        p->setTimeout(value, nullptr);
        return;
    }
    p->setTimeout(value, [cb](int64_t ms){
        return cb.cb(ms, cb.opaque);
    });
}

void MDK_Player_prepare(mdkPlayer* p, int64_t startPosition, MDK_PrepareCallback cb)
{
    if (!cb.opaque) {
        p->prepare(startPosition, nullptr);
        return;
    }
    p->prepare(startPosition, [cb](int64_t position, bool* boost){
        cb.cb(position, boost, cb.opaque);
    });
}

const mdkMediaInfo* MDK_Player_mediaInfo(mdkPlayer* p)
{
    MediaInfoToC(p->mediaInfo(), &p->media_info);
    return &p->media_info.info;
}

void MDK_Player_setState(mdkPlayer* p, MDK_State value)
{
    p->setState(State(value));
}

MDK_State MDK_Player_state(mdkPlayer* p)
{
    return (MDK_State)p->state();
}

void MDK_Player_onStateChanged(mdkPlayer* p, MDK_StateChangedCallback cb)
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

void MDK_Player_onMediaStatusChanged(mdkPlayer* p, MDK_MediaStatusChangedCallback cb)
{
    if (!cb.opaque) {
        p->onMediaStatusChanged(nullptr);
        return;
    }
    p->onMediaStatusChanged([cb](MediaStatus value){
        return cb.cb(MDK_MediaStatus(value), cb.opaque);
    });
}

void MDK_Player_updateNativeWindow(mdkPlayer* p, void* win, int width, int height, MDK_SurfaceType type)
{
    p->updateNativeWindow(win, width, height, Player::SurfaceType(type));
}

void MDK_Player_createWindow(mdkPlayer* p, void* nativeHandle, MDK_SurfaceType type)
{
    p->createWindow(nativeHandle, Player::SurfaceType(type));
}

void MDK_Player_resizeWindow(mdkPlayer* p, int w, int h)
{
    p->resizeWindow(w, h);
}

void MDK_Player_showWindow(mdkPlayer* p)
{
    p->showWindow();
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

double MDK_Player_renderVideo(mdkPlayer* p, void* vo_opaque)
{
    return p->renderVideo(vo_opaque);
}

void MDK_Player_setRenderCallback(mdkPlayer* p, MDK_RenderCallback cb)
{
    if (!cb.opaque) {
        p->setRenderCallback(nullptr);
        return;
    }
    p->setRenderCallback([cb](void* vo_opaque){
        cb.cb(vo_opaque, cb.opaque);
    });
}

void MDK_Player_onVideoFrame(mdkPlayer*, void (*)(mdkVideoFrame*));
void MDK_Player_onAudioFrame(mdkPlayer*, void (*)(mdkAudioFrame*));

int64_t MDK_Player_position(mdkPlayer* p)
{
    return p->position();
}

bool MDK_Player_seekWithFlags(mdkPlayer* p, int64_t pos, MDK_SeekFlag flags, MDK_SeekCallback cb)
{
    if (!cb.opaque) {
        return p->seek(pos, SeekFlag(flags), nullptr);
    }
    return p->seek(pos, SeekFlag(flags), [cb](int64_t value){
        cb.cb(value, cb.opaque);
    });
}

bool MDK_Player_seek(mdkPlayer* p, int64_t pos, MDK_SeekCallback cb)
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

int64_t MDK_Player_addListener(mdkPlayer* p, MDK_MediaEventListener cb)
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

void MDK_Player_removeListener(mdkPlayer* p, int64_t listener)
{
    p->removeListener(listener);
}


mdkPlayerAPI* mdkPlayerAPI_new()
{
    mdkPlayerAPI* p = new mdkPlayerAPI();
    p->object = new mdkPlayer();
#define SET_API(FN) p->FN = MDK_Player_##FN
    SET_API(setMute);
    SET_API(setVolume);
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
    //SET_API(mediaInfo);
    SET_API(setState);
    SET_API(state);
    SET_API(onStateChanged);
    SET_API(waitFor);
    SET_API(mediaStatus);
    SET_API(onMediaStatusChanged);
    SET_API(updateNativeWindow);
    SET_API(createWindow);
    SET_API(resizeWindow);
    SET_API(showWindow);
    //SET_API(getVideoFrame);
    SET_API(setVideoSurfaceSize);
    SET_API(setVideoViewport);
    SET_API(setAspectRatio);
    SET_API(rotate);
    SET_API(scale);
    SET_API(renderVideo);
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
    SET_API(addListener);
    SET_API(removeListener);
#undef SET_API
    return p;
}

void mdkPlayerAPI_delete(mdkPlayerAPI** pp)
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