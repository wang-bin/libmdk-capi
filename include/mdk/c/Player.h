/*
 * Copyright (c) 2019 WangBin <wbsecg1 at gmail.com>
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 */
#pragma once
#include "global.h"

#ifdef __cplusplus
extern "C" {
#endif
struct mdkMediaInfo;
struct mdkAudioFrame;
struct mdkVideoFrame;
struct mdkWindow;
struct mdkPlayer;

enum MDK_SurfaceType {
    MDK_SurfaceType_Auto, /* platform default type */
    MDK_SurfaceType_X11,
    MDK_SurfaceType_GBM,
    MDK_SurfaceType_Wayland,
};

typedef struct MDK_CurrentMediaChangedCallback {
    void (*cb)(void* opaque);
    void* opaque;
} MDK_CurrentMediaChangedCallback;

typedef struct MDK_PrepareCallback {
    void (*cb)(int64_t position, bool* boost, void* opaque);
    void* opaque;
} MDK_PrepareCallback;

typedef struct MDK_RenderCallback {
    void (*cb)(void* vo_opaque, void* opaque);
    void* opaque;
} MDK_RenderCallback;

typedef struct SwitchBitrateCallback {
    void (*cb)(bool, void* opaque);
    void* opaque;
} SwitchBitrateCallback;

typedef struct MDK_SeekCallback {
    void (*cb)(int64_t ms, void* opaque);
    void* opaque;
} MDK_SeekCallback;

typedef struct mdkPlayerAPI {
    mdkPlayer* object;

    void (*setMute)(mdkPlayer*, bool value);
    void (*setVolume)(mdkPlayer*, float value);

/* MUST call setActiveTracks() after setMedia(), otherwise the 1st track in the media is used*/
    void (*setMedia)(mdkPlayer*, const char* url);
/* Set individual source for type, e.g. audio track file. If url is not empty, an individual pipeline will be used for 'type' tracks.
  If url is empty, use 'type' tracks in MediaType::Video url.
  TODO: default type is Unknown
*/
    void (*setMediaForType)(mdkPlayer*, const char* url, MDK_MediaType type);
    const char* (*url)(mdkPlayer*);

    void (*setPreloadImmediately)(mdkPlayer*, bool value);
    void (*setNextMedia)(mdkPlayer*, const char* url, int64_t startPosition);

/* call before setMedia() */
    void (*currentMediaChanged)(mdkPlayer*, MDK_CurrentMediaChangedCallback cb);
/* backends can be: AudioQueue(Apple only), OpenSL(Android only), ALSA(linux only), XAudio2(Windows only), OpenAL
  ends with NULL
*/
    void (*setAudioBackends)(mdkPlayer*, const char** names);
    void (*setAudioDecoders)(mdkPlayer*, const char** names);
    void (*setVideoDecoders)(mdkPlayer*, const char** names);

    void (*setTimeout)(mdkPlayer*, int64_t value, MDK_TimeoutCallback cb);
/*!
   \brief prepare
   To play a media from a given position, call prepare(ms) then setState(State::Playing)
   parameter position in callback is the actual position, or <0 (TODO: error code as position) if prepare() failed.
   parameter boost in callback can be set by user to boost the first frame rendering
 */

    void (*prepare)(mdkPlayer*, int64_t startPosition, MDK_PrepareCallback cb);
    const mdkMediaInfo* (*mediaInfo)(mdkPlayer*); /* NOT IMPLEMENTED*/

/*!
  \brief setState
  request a new state.
  setState(State::Stopped) only stops current media. Call setNextMedia(nullptr, -1) before stop to disable next media.
  NOTE: the requested state is not queued. so set one state immediately after another may have no effect.
  e.g. State::Playing after State::Stopped may have no effect if playback have not been stopped and still in Playing state
  so the final state is State::Stopped. Current solution is waitFor(State::Stopped) before setState(State::Playing).
  Usually no waitFor(State::Playing) because we want async load
*/
    void (*setState)(mdkPlayer*, MDK_State value);
    MDK_State (*state)(mdkPlayer*);
    void (*onStateChanged)(mdkPlayer*, MDK_StateChangedCallback);
    bool (*waitFor)(mdkPlayer*, MDK_State value, long timeout);

    MDK_MediaStatus (*mediaStatus)(mdkPlayer*);
    void (*onMediaStatusChanged)(mdkPlayer*, MDK_MediaStatusChangedCallback);

/*!
 * \brief updateNativeWindow
 * If window is not created, create rendering context internally by createWindow() and attached to native window
 * native window MUST be not null before destroying player
 type: ignored if win ptr does not change (request to resize)
 */
    void (*updateNativeWindow)(mdkPlayer*, void* win, int width, int height, MDK_SurfaceType type);

    void (*createWindow)(mdkPlayer*, void* nativeHandle, MDK_SurfaceType type);
    void (*resizeWindow)(mdkPlayer*, int w, int h);
    void (*showWindow)(mdkPlayer*);

/*
  vo_opaque: a ptr to identify the renderer. cam be null, then it is the default vo/renderer.
  A vo/renderer (e.g. the default vo/renderer) is gfx context aware, i.e. can render in multiple gfx contexts with a single vo/renderer, but parameters(e.g. surface size)
  must be updated when switch to a new context. So per gfx context vo/renderer can be better because parameters are stored in vo/renderer.
*/
/*!
  \brief getVideoFrame
  get current rendered frame, i.e. the decoded video frame rendered by renderVideo()
 */
    void (*getVideoFrame)(mdkPlayer*, mdkVideoFrame* frame, void* vo_opaque); /* NOT IMPLEMENTED*/

    void (*setVideoSurfaceSize)(mdkPlayer*, int width, int height, void* vo_opaque);
    void (*setVideoViewport)(mdkPlayer*, float x, float y, float w, float h, void* vo_opaque);
    void (*setAspectRatio)(mdkPlayer*, float value, void* vo_opaque);
    void (*rotate)(mdkPlayer*, int degree, void* vo_opaque);
    void (*scale)(mdkPlayer*, float x, float y, void* vo_opaque);
/*!
   \brief renderVideo
   Render the next/current frame. Call only in Window.onDraw() (not created by createWindow()/updateNativeWindow()) or external graphics context's rendering function.
   Can be called in multiple foreign gfx contexts for the same vo_opaque.
   \return timestamp of rendered frame, or < 0 if no frame is rendered
 */
    double (*renderVideo)(mdkPlayer*, void* vo_opaque);

/*
  callback is invoked when the vo coresponding to vo_opaque needs to update/draw content, e.g. when a new frame is received in the renderer.
  Also invoked in setVideoSurfaceSize(), setVideoViewport(), setAspectRatio() and rotate(), take care of dead lock in callback and above functions.
  with vo_opaque, user can know which vo/renderer is rendering, useful for multiple renderers
*/
    void (*setRenderCallback)(mdkPlayer*, MDK_RenderCallback);

/*
  \brief onFrame
  called before delivering frame to renderers
 
    void (*onVideoFrame)(mdkPlayer*, void (*)(mdkVideoFrame*));
    void (*onAudioFrame)(mdkPlayer*, void (*)(mdkAudioFrame*));
*/

    int64_t (*position)(mdkPlayer*);
    bool (*seekWithFlags)(mdkPlayer*, int64_t pos, MDK_SeekFlag flags, MDK_SeekCallback);
    bool (*seek)(mdkPlayer*, int64_t pos, MDK_SeekCallback);

    void (*setPlaybackRate)(mdkPlayer*, float value);
    float (*playbackRate)(mdkPlayer*);
/*!
 * \brief buffered
 * get buffered data(packets) duration and size
 * \return buffered data duration
 */
    int64_t (*buffered)(mdkPlayer*, int64_t* bytes);
/*!
 * \brief switchBitrate
 * A new media will be played later
 * \param delay (default -1) switch after at least delay ms. TODO: determined by buffered time, e.g. from high bit rate without enough buffered samples to low bit rate
 * \param cb (true/false) called when finished/failed
 */
    void (*switchBitrate)(mdkPlayer*, const char* url, int64_t delay, SwitchBitrateCallback cb);
/*!
 * \brief switchBitrateSingalConnection
 * Only 1 media is loaded. The previous media is unloaded and the playback continues. When new media is preloaded, stop the previous media at some point
 * MUST call setPreloadImmediately(false) because PreloadImmediately for singal connection preload is not possible.
 * \return false if preload immediately
 * This will not affect next media set by user
 */
    bool (*switchBitrateSingleConnection)(mdkPlayer*, const char *url, SwitchBitrateCallback cb);

/*!
 * \brief addListener
 * listener the events from player, control and FrameReader
 * \return listener id
 */
    int64_t (*addListener)(mdkPlayer*, MDK_MediaEventListener cb);
    void (*removeListener)(mdkPlayer*, int64_t listener);

/*
  \brief bufferRange
  duration range of buffered data.
  minMs: wait for buffered duration >= minMs when before popping a packet to decode
  drop = true: drop old non-key frame data to reduce buffered duration until less then maxMs.
  drop = false: wait for buffered duration less than maxMs before buffering more data
 */
    void (*setBufferRange)(mdkPlayer*, int64_t minMs, int64_t maxMs, bool drop);
} mdkPlayerAPI;

MDK_API mdkPlayerAPI* mdkPlayerAPI_new();
MDK_API void mdkPlayerAPI_delete(mdkPlayerAPI**);
/* MUST be called when a foreign OpenGL context previously used is being destroyed to release context resources. The context MUST be current.*/
MDK_API void MDK_foreignGLContextDestroyed();

#ifdef __cplusplus
}
#endif
