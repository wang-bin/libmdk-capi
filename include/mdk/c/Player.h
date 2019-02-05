/*
 * Copyright (c) 2019 WangBin <wbsecg1 at gmail.com>
 */
#pragma once
#include "global.h"

#ifdef __cplusplus
extern "C" {
#endif
struct MDK_MediaInfo;
struct MDK_AudioFrame;
struct MDK_VideoFrame;
struct MDK_Window;
struct MDK_Player;
MDK_API MDK_Player* MDK_Player_new();
MDK_API void MDK_Player_delete(MDK_Player**);

/* MUST be called when a foreign OpenGL context previously used is being destroyed to release context resources. The context MUST be current.*/
MDK_API void MDK_foreignGLContextDestroyed();

MDK_API void MDK_Player_setMute(MDK_Player*, bool value);
MDK_API void MDK_Player_setVolume(MDK_Player*, float value);

/* MUST call setActiveTracks() after setMedia(), otherwise the 1st track in the media is used*/
MDK_API void MDK_Player_setMedia(MDK_Player*, const char* url);
/* Set individual source for type, e.g. audio track file. If url is not empty, an individual pipeline will be used for 'type' tracks.
  If url is empty, use 'type' tracks in MediaType::Video url.
  TODO: default type is Unknown
*/
MDK_API void MDK_Player_setMediaForType(MDK_Player*, const char* url, MDK_MediaType type);
MDK_API const char* MDK_Player_url(MDK_Player*);

MDK_API void MDK_Player_setPreloadImmediately(MDK_Player*, bool value);
MDK_API void MDK_Player_setNextMedia(MDK_Player*, const char* url, int64_t startPosition);

typedef struct MDK_CurrentMediaChangedCallback {
    void (*cb)(void* opaque);
    void* opaque;
} MDK_CurrentMediaChangedCallback;
/* call before setMedia() */
MDK_API void MDK_Player_currentMediaChanged(MDK_Player*, MDK_CurrentMediaChangedCallback cb);
/* backends can be: AudioQueue(Apple only), OpenSL(Android only), ALSA(linux only), XAudio2(Windows only), OpenAL
  ends with NULL
*/
MDK_API void MDK_Player_setAudioBackends(MDK_Player*, const char** names);
MDK_API void MDK_Player_setAudioDecoders(MDK_Player*, const char** names);
MDK_API void MDK_Player_setVideoDecoders(MDK_Player*, const char** names);
MDK_API void MDK_Player_setTimeout(MDK_Player*, int64_t value, MDK_TimeoutCallback cb);
/*!
   \brief prepare
   To play a media from a given position, call prepare(ms) then setState(State::Playing)
   parameter position in callback is the actual position, or <0 (TODO: error code as position) if prepare() failed.
   parameter boost in callback can be set by user to boost the first frame rendering
 */
typedef struct MDK_PrepareCallback {
    void (*cb)(int64_t position, bool* boost, void* opaque);
    void* opaque;
} MDK_PrepareCallback;

MDK_API void MDK_Player_prepare(MDK_Player*, int64_t startPosition, MDK_PrepareCallback cb);
MDK_API const MDK_MediaInfo* MDK_Player_mediaInfo(MDK_Player*);

/*!
  \brief setState
  request a new state.
  setState(State::Stopped) only stops current media. Call setNextMedia(nullptr, -1) before stop to disable next media.
  NOTE: the requested state is not queued. so set one state immediately after another may have no effect.
  e.g. State::Playing after State::Stopped may have no effect if playback have not been stopped and still in Playing state
  so the final state is State::Stopped. Current solution is waitFor(State::Stopped) before setState(State::Playing).
  Usually no waitFor(State::Playing) because we want async load
*/
MDK_API void MDK_Player_setState(MDK_Player*, MDK_State value);
MDK_API MDK_State MDK_Player_state(MDK_Player*);

typedef struct MDK_StateChangedCallback {
    void (*cb)(MDK_State, void* opaque);
    void* opaque;
} MDK_StateChangedCallback;

MDK_API void MDK_Player_onStateChanged(MDK_Player*, MDK_StateChangedCallback);
MDK_API bool MDK_Player_waitFor(MDK_Player*, MDK_State value, long timeout);

MDK_API MDK_MediaStatus MDK_Player_mediaStatus(MDK_Player*);

typedef struct MDK_MediaStatusChangedCallback {
    bool (*cb)(MDK_MediaStatus, void* opaque);
    void* opaque;
} MDK_MediaStatusChangedCallback;

MDK_API void MDK_Player_onMediaStatusChanged(MDK_Player*, MDK_MediaStatusChangedCallback);

enum MDK_SurfaceType {
    Auto, /* platform default type */
    X11,
    GBM,
    Wayland,
};
/*!
 * \brief updateNativeWindow
 * If window is not created, create rendering context internally by createWindow() and attached to native window
 * native window MUST be not null before destroying player
 type: ignored if win ptr does not change (request to resize)
 */
MDK_API void MDK_Player_updateNativeWindow(MDK_Player*, void* win, int width, int height, MDK_SurfaceType type);

MDK_API void MDK_Player_createWindow(MDK_Player*, void* nativeHandle, MDK_SurfaceType type);
MDK_API void MDK_Player_resizeWindow(MDK_Player*, int w, int h);
MDK_API void MDK_Player_showWindow(MDK_Player*);

/*
  vo_opaque: a ptr to identify the renderer. cam be null, then it is the default vo/renderer.
  A vo/renderer (e.g. the default vo/renderer) is gfx context aware, i.e. can render in multiple gfx contexts with a single vo/renderer, but parameters(e.g. surface size)
  must be updated when switch to a new context. So per gfx context vo/renderer can be better because parameters are stored in vo/renderer.
*/
/*!
  \brief getVideoFrame
  get current rendered frame, i.e. the decoded video frame rendered by renderVideo()
 */
MDK_API void MDK_Player_getVideoFrame(MDK_Player*, MDK_VideoFrame* frame, void* vo_opaque);
MDK_API void MDK_Player_setVideoSurfaceSize(MDK_Player*, int width, int height, void* vo_opaque);
MDK_API void MDK_Player_setVideoViewport(MDK_Player*, float x, float y, float w, float h, void* vo_opaque);
MDK_API void MDK_Player_setAspectRatio(MDK_Player*, float value, void* vo_opaque);
MDK_API void MDK_Player_rotate(MDK_Player*, int degree, void* vo_opaque);
MDK_API void MDK_Player_scale(MDK_Player*, float x, float y, void* vo_opaque);
/*!
   \brief renderVideo
   Render the next/current frame. Call only in Window.onDraw() (not created by createWindow()/updateNativeWindow()) or external graphics context's rendering function.
   Can be called in multiple foreign gfx contexts for the same vo_opaque.
   \return timestamp of rendered frame, or < 0 if no frame is rendered
 */
MDK_API double MDK_Player_renderVideo(MDK_Player*, void* vo_opaque);

/*
  callback is invoked when the vo coresponding to vo_opaque needs to update/draw content, e.g. when a new frame is received in the renderer.
  Also invoked in setVideoSurfaceSize(), setVideoViewport(), setAspectRatio() and rotate(), take care of dead lock in callback and above functions.
  with vo_opaque, user can know which vo/renderer is rendering, useful for multiple renderers
*/
typedef struct MDK_RenderCallback {
    void (*cb)(void* vo_opaque, void* opaque);
    void* opaque;
} MDK_RenderCallback;

MDK_API void MDK_Player_setRenderCallback(MDK_Player*, MDK_RenderCallback);

/*
  \brief onFrame
  called before delivering frame to renderers
 */
MDK_API void MDK_Player_onVideoFrame(MDK_Player*, void (*)(MDK_VideoFrame*));
MDK_API void MDK_Player_onAudioFrame(MDK_Player*, void (*)(MDK_AudioFrame*));

MDK_API int64_t MDK_Player_position(MDK_Player*);

typedef struct MDK_SeekCallback {
    void (*cb)(int64_t ms, void* opaque);
    void* opaque;
} MDK_SeekCallback;

MDK_API bool MDK_Player_seekWithFlags(MDK_Player*, int64_t pos, MDK_SeekFlag flags, MDK_SeekCallback);
inline bool MDK_Player_seek(MDK_Player* player, int64_t pos, MDK_SeekCallback cb) {
    return MDK_Player_seekWithFlags(player, pos, MDK_SeekFlag_Default, cb);
}

MDK_API void MDK_Player_setPlaybackRate(MDK_Player*, float value);
MDK_API float MDK_Player_playbackRate(MDK_Player*);
/*!
 * \brief buffered
 * get buffered packets' duration and size
 * \return buffered packets' duration
 */
MDK_API int64_t MDK_Player_buffered(MDK_Player*, int64_t* bytes);
/*!
 * \brief switchBitrate
 * A new media will be played later
 * \param delay (default -1) switch after at least delay ms. TODO: determined by buffered time, e.g. from high bit rate without enough buffered samples to low bit rate
 * \param cb (true/false) called when finished/failed
 */
typedef struct SwitchBitrateCallback {
    void (*cb)(bool, void* opaque);
    void* opaque;
} SwitchBitrateCallback;

MDK_API void MDK_Player_switchBitrate(MDK_Player*, const char* url, int64_t delay, SwitchBitrateCallback cb);
/*!
 * \brief switchBitrateSingalConnection
 * Only 1 media is loaded. The previous media is unloaded and the playback continues. When new media is preloaded, stop the previous media at some point
 * MUST call setPreloadImmediately(false) because PreloadImmediately for singal connection preload is not possible.
 * \return false if preload immediately
 * This will not affect next media set by user
 */
MDK_API bool MDK_Player_switchBitrateSingleConnection(MDK_Player*, const char *url, SwitchBitrateCallback cb);

/*!
 * \brief addListener
 * listener the events from player, control and FrameReader
 * \return listener id
 */
MDK_API int64_t MDK_Player_addListener(MDK_Player*, MDK_MediaEventListener cb);
MDK_API void MDK_Player_removeListener(MDK_Player*, int64_t listener);

#ifdef __cplusplus
}
#endif
