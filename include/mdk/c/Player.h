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
#include "RenderAPI.h"

#ifdef __cplusplus
extern "C" {
#endif
struct mdkMediaInfo;
struct mdkAudioFrame;
struct mdkVideoFrameAPI;
struct mdkPlayer;

enum MDK_SurfaceType {
    MDK_SurfaceType_Auto, /* platform default type */
    MDK_SurfaceType_X11,
    MDK_SurfaceType_GBM,
    MDK_SurfaceType_Wayland,
};

typedef struct mdkCurrentMediaChangedCallback {
    void (*cb)(void* opaque);
    void* opaque;
} mdkCurrentMediaChangedCallback;

/*!
  \brief mdkPrepareCallback
  \param position in callback is the actual position, or <0 (TODO: error code as position) if prepare() failed.
  \param boost in callback can be set by user to boost the first frame rendering
  \return false to unload media immediately when media is loaded and MediaInfo is ready, true to continue.
    example: always return false can be used as media information reader
 */
typedef struct mdkPrepareCallback {
    bool (*cb)(int64_t position, bool* boost, void* opaque);
    void* opaque;
} mdkPrepareCallback;

typedef struct mdkRenderCallback {
    void (*cb)(void* vo_opaque, void* opaque);
    void* opaque;
} mdkRenderCallback;

typedef struct mdkVideoCallback {
    int (*cb)(mdkVideoFrameAPI** pFrame/*in/out*/, int track, void* opaque);
    void* opaque;
} mdkVideoCallback;

typedef struct SwitchBitrateCallback {
    void (*cb)(bool, void* opaque);
    void* opaque;
} SwitchBitrateCallback;

typedef struct mdkSeekCallback {
    void (*cb)(int64_t ms, void* opaque);
    void* opaque;
} mdkSeekCallback;

/*!
  \brief TimeoutCallback
  \param ms elapsed milliseconds
  \return true to abort current operation on timeout.
  A null callback can abort current operation.
  Negative timeout infinit.
  Default timeout is 10s
 */
typedef struct mdkTimeoutCallback {
    bool (*cb)(int64_t ms, void* opaque);
    void* opaque;
} mdkTimeoutCallback;

/*!
  \brief MediaEventCallback
  \return true if event is processed and should stop dispatching.
 */
typedef struct mdkMediaEventCallback {
    bool (*cb)(const mdkMediaEvent*, void* opaque);
    void* opaque;
} mdkMediaEventCallback;

typedef struct mdkLoopCallback {
    void (*cb)(int, void* opaque);
    void* opaque;
} mdkLoopCallback;

typedef struct mdkSnapshotRequest {
/* data: rgba data. Created internally or provided by user.
   If data is provided by user, stride,  height and width MUST be also set, and data MUST be valid until snapshot callback is finished.
 */
    uint8_t* data;
/*
   result width of snapshot image set by user, or the same as current frame width if 0. no renderer transform.
   if both requested width and height are < 0, then result image is scaled image of current frame with ratio=width/height. no renderer transform.
   if only one of width and height < 0, then the result size is video renderer viewport size, and all transforms will be applied.
*/
    int width;
    int height;
    int stride;
    bool subtitle; // not supported yet
} mdkSnapshotRequest;

enum MDK_MapDirection {
    MDK_MapDirection_FrameToViewport, // left-hand
    MDK_MapDirection_ViewportToFrame, // left-hand
};

typedef struct mdkSnapshotCallback {
/* \brief cb
   snapshot callback.
   \param req result request. If null, snapshot failed. Otherwise req.width, height and stride are always >0, data is never null.
   \param frameTime captured frame timestamp(seconds)
   \param opaque user data
   \returns null, or a file path to save as a file(jpeg is recommended, other formats depends on ffmpeg runtime).
   Returned string will be freed internally(assume allocated by malloc family apis).
   Callback is called in a dedicated thread, so time-consuming operations(encode, file io etc.) are allowed in the callback.
 */
    char* (*cb)(mdkSnapshotRequest* req, double frameTime, void* opaque);
    void* opaque;
} mdkSnapshotCallback;

typedef struct mdkSyncCallback {
    double (*cb)(void* opaque);
    void* opaque;
} mdkSyncCallback;


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
/*!
  \brief setNextMedia
  Gapless play the next media after current media playback end
  \param flags seek flags if startPosition > 0, accurate or fast
  setState(State::Stopped) only stops current media. Call setNextMedia(nullptr, -1) first to disable next media.
 */
    void (*setNextMedia)(mdkPlayer*, const char* url, int64_t startPosition, MDKSeekFlag flags);

/* call before setMedia() */
    void (*currentMediaChanged)(mdkPlayer*, mdkCurrentMediaChangedCallback cb);
/* backends can be: AudioQueue(Apple only), OpenSL(Android only), ALSA(linux only), XAudio2(Windows only), OpenAL
  ends with NULL
*/
    void (*setAudioBackends)(mdkPlayer*, const char** names);
    void (*setAudioDecoders)(mdkPlayer*, const char** names);
    void (*setVideoDecoders)(mdkPlayer*, const char** names);

    void (*setTimeout)(mdkPlayer*, int64_t value, mdkTimeoutCallback cb);
/*!
  \brief prepare
  Preload a media and then becomes State::Paused. \sa PrepareCallback
  To play a media from a given position, call prepare(ms) then setState(State::Playing)
  \param startPosition start from position, relative to media start position, i.e. MediaInfo.start_time
  \param flags seek flag if startPosition != 0.
  For fast seek(has flag SeekFlag::Fast), the first frame is a key frame whose timestamp >= startPosition
  For accurate seek(no flag SeekFlag::Fast), the first frame is the nearest frame whose timestamp <= startPosition, but the position passed to callback is the key frame position <= startPosition
 */
    void (*prepare)(mdkPlayer*, int64_t startPosition, mdkPrepareCallback cb, MDKSeekFlag flags);
    const mdkMediaInfo* (*mediaInfo)(mdkPlayer*); /* NOT IMPLEMENTED*/

/*!
  \brief setState
  request a new state.
  setState(State::Stopped) only stops current media. Call setNextMedia(nullptr, -1) before stop to disable next media.
  setState(State::Stopped) will release all resouces and clear video renderer viewport. While a normal playback end will keep renderer resources
  and the last video frame. Manually call setState(State::Stopped) to clear them.
  NOTE: the requested state is not queued. so set one state immediately after another may have no effect.
  e.g. State::Playing after State::Stopped may have no effect if playback have not been stopped and still in Playing state
  so the final state is State::Stopped. Current solution is waitFor(State::Stopped) before setState(State::Playing).
  Usually no waitFor(State::Playing) because we want async load
*/
    void (*setState)(mdkPlayer*, MDK_State value);
    MDK_State (*state)(mdkPlayer*);
    void (*onStateChanged)(mdkPlayer*, mdkStateChangedCallback);
    bool (*waitFor)(mdkPlayer*, MDK_State value, long timeout);

    MDK_MediaStatus (*mediaStatus)(mdkPlayer*);
/*!
  \brief onMediaStatusChanged
  Add a callback to be invoked when MediaStatus is changed
  \param cb null to clear callbacks
  TODO: callback token
 */
    void (*onMediaStatusChanged)(mdkPlayer*, mdkMediaStatusChangedCallback);

/*!
 * \brief updateNativeSurface
 * If surface is not created, create rendering context internally by createSurface() and attached to native surface
 * native surface MUST be not null before destroying player
 type: ignored if win ptr does not change (request to resize)
 */
    void (*updateNativeSurface)(mdkPlayer*, void* surface, int width, int height, MDK_SurfaceType type);

    void (*createSurface)(mdkPlayer*, void* nativeHandle, MDK_SurfaceType type);
    void (*resizeSurface)(mdkPlayer*, int w, int h);
    void (*showSurface)(mdkPlayer*);

/*
  vo_opaque: a ptr to identify the renderer. can be null, then it is the default vo/renderer.
  A vo/renderer (e.g. the default vo/renderer) is gfx context aware, i.e. can render in multiple gfx contexts with a single vo/renderer, but parameters(e.g. surface size)
  must be updated when switch to a new context. So per gfx context vo/renderer can be better because parameters are stored in vo/renderer.
*/
/*!
  \brief getVideoFrame
  get current rendered frame, i.e. the decoded video frame rendered by renderVideo()
 */
    void (*getVideoFrame)(); /* NOT IMPLEMENTED*/
/*
  \brief setVideoSurfaceSize
  Window size, surface size or drawable size. Render callback(if exists) will be invoked if width and height > 0.
  If width or heigh < 0, corresponding video renderer (for vo_opaque) will be removed. But subsequence call with this vo_opaque will create renderer again. So it can be used before destroying the renderer.
 */
    void (*setVideoSurfaceSize)(mdkPlayer*, int width, int height, void* vo_opaque);
    void (*setVideoViewport)(mdkPlayer*, float x, float y, float w, float h, void* vo_opaque);
/*!
  \brief setAspectRatio
  Video display aspect ratio.
  0: ignore aspect ratio and scale to fit renderer viewport
  FLT_EPSILON(default): keep frame aspect ratio and scale as large as possible inside renderer viewport
  -FLT_EPSILON: keep frame aspect ratio and scale as small as possible outside renderer viewport
  other value > 0: keep given aspect ratio and scale as large as possible inside renderer viewport
  other value < 0: keep given aspect ratio and scale as small as possible inside renderer viewport
 */
    void (*setAspectRatio)(mdkPlayer*, float value, void* vo_opaque);
    void (*rotate)(mdkPlayer*, int degree, void* vo_opaque);
    void (*scale)(mdkPlayer*, float x, float y, void* vo_opaque);
/*!
   \brief renderVideo
  Render the next or current(redraw) frame. Foreign render context only (i.e. not created by createSurface()/updateNativeSurface()).
  OpenGL: Can be called in multiple foreign contexts for the same vo_opaque.
   \return timestamp of rendered frame, or < 0 if no frame is rendered
 */
    double (*renderVideo)(mdkPlayer*, void* vo_opaque);
/*!
  \brief setBackgroundColor
  r, g, b, a range is [0, 1]. default is 0. if out of range or a == 0, background color will not be filled
 */
    void (*setBackgroundColor)(mdkPlayer*, float r, float g, float b, float a, void* vo_opaque);

/*!
  \brief setRenderCallback
  set a callback which is invoked when the vo coresponding to vo_opaque needs to update/draw content, e.g. when a new frame is received in the renderer.
  Also invoked in setVideoSurfaceSize(), setVideoViewport(), setAspectRatio() and rotate(), take care of dead lock in callback and above functions.
  with vo_opaque, user can know which vo/renderer is rendering, useful for multiple renderers
  There may be no frames or playback not even started, but renderer update is required internally
*/
    void (*setRenderCallback)(mdkPlayer*, mdkRenderCallback);

/*
  \brief onVideo
  Called before delivering frame to renderers. Can be used to apply filters.
 */
    void (*onVideo)(mdkPlayer*, mdkVideoCallback);
    void (*onAudio)(mdkPlayer*); // NOT IMPLEMENTED
/*
  \brief beforeVideoRender
  NOT IMPLEMENTED. Called after rendering a frame on renderer of vo_opaque on rendering thread. Can be used to apply GPU filters.
 */
    void (*beforeVideoRender)(mdkPlayer*, void (*)(mdkVideoFrameAPI*, void* vo_opaque));
/*
  \brief beforeVideoRender
  NOT IMPLEMENTED. Called after rendering a frame on renderer of vo_opaque on rendering thread. Can be used to draw a watermark.
 */
    void (*afterVideoRender)(mdkPlayer*, void (*)(mdkVideoFrameAPI*, void* vo_opaque));

    int64_t (*position)(mdkPlayer*);
/*!
  \brief seekWithFlags
  \param cb callback to be invoked when seek finished(ret >= 0), error occured(ret < 0, usually -1) or skipped because of unfinished previous seek(ret == -2)
 */
    bool (*seekWithFlags)(mdkPlayer*, int64_t pos, MDK_SeekFlag flags, mdkSeekCallback);
    bool (*seek)(mdkPlayer*, int64_t pos, mdkSeekCallback);

    void (*setPlaybackRate)(mdkPlayer*, float value);
    float (*playbackRate)(mdkPlayer*);
/*!
 * \brief buffered
 * get buffered data(packets) duration and size
 * \return buffered data duration
 */
    int64_t (*buffered)(mdkPlayer*, int64_t* bytes);
/*!
  \brief switchBitrate
  A new media will be played later
  \param delay switch after at least delay ms. TODO: determined by buffered time, e.g. from high bit rate without enough buffered samples to low bit rate
  \param cb (true/false) called when finished/failed
  \param flags seek flags for the next url, accurate or fast
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

    void (*onEvent)(mdkPlayer*, mdkMediaEventCallback cb, MDK_CallbackToken* token);
/*
  \brief bufferRange
  set duration range of buffered data.
  minMs: default 4000. wait for buffered duration >= minMs when before popping a packet from to decode
  maxMs: default 16000. max buffered duration.
  drop = true: drop old non-key frame packets to reduce buffered duration until < maxMs.
  drop = false: wait for buffered duration < maxMs before pushing packets

  Usually you don't need to call this api. This api can be used for low latency live videos, for example setBufferRange(0, 1000, true) will decode as soon as possible when media data received, also it ensures the max delay of rendered video is 1s, and no accumulated delay.
 */
    void (*setBufferRange)(mdkPlayer*, int64_t minMs, int64_t maxMs, bool drop);
/*!
  \brief snapshot
  take a snapshot from current renderer. The result is in bgra format, or null on failure.
  When `snapshot()` is called, redraw is scheduled for `vo_opaque`'s renderer, then renderer will take a snapshot in rendering thread.
  So for a foreign context, if renderer's surface/window/widget is invisible or minimized, snapshot may do nothing because of system or gui toolkit painting optimization.
*/
    void (*snapshot)(mdkPlayer*, mdkSnapshotRequest* request, mdkSnapshotCallback cb, void* vo_opaque);

/*
  \brief setProperty
  Set additional properties. Can be used to store user data, or change player behavior if the property is defined internally.
  Predefined properties are:
  - "video.avfilter": ffmpeg avfilter filter graph string for video track. take effect immediately
  - "audio.avfilter": ffmpeg avfilter filter graph string for audio track. take effect immediately
  - "continue_at_end": do not stop playback when decode and render to end of stream. only setState(State::Stopped) can stop playback
 */
    void (*setProperty)(mdkPlayer*, const char* key, const char* value);
/*!
  \brief setProperty
  \return value for key, or null if no such key
 */
    const char* (*getProperty)(mdkPlayer*, const char* key);
/*
  \brief record
  Start to record or stop recording current media by remuxing packets read. If media is not loaded, recorder will start when playback starts
  \param url destination. null or the same value as recording one to stop recording
  \param format forced format if unable to guess from url suffix
 */
    void (*record)(mdkPlayer*, const char* url, const char* format);

/*!
  \brief setLoopRange
  DEPRECATED! use setLoop+setRange instead
 */
    void (*setLoopRange)(mdkPlayer*, int count, int64_t a, int64_t b);
/*!
  \brief setLoop
  Set A-B loop repeat count.
  \param count repeat count. 0 to disable looping and stop when out of range(B)
 */
    void (*setLoop)(mdkPlayer*, int count);
/*
  \brief onLoop
  add/remove a callback which will be invoked right before a new A-B loop
  \param cb callback with current loop count elapsed
 */
    void (*onLoop)(mdkPlayer*, mdkLoopCallback cb, MDK_CallbackToken* token);
/*!
  \brief setRange
  Set A-B loop range, or playback range
  \param a loop position begin, in ms.
  \param b loop position end, in ms. -1, INT64_MAX or numeric_limit<int64_t>::max() indicates b is the end of media
 */
    void (*setRange)(mdkPlayer*, int64_t a, int64_t b);

/*
  RenderAPI
  RenderAPI provides platform/api dependent resources for video renderer and rendering context corresponding to vo_opaque. It's used by
  1. create internal render context via updateNativeSurface() using given api. MUST be called before any other functions have parameter vo_opaque and updateNativeSurface()!
    To use RenderAPI other than OpenGL, setRenderAPI() MUST be called before add/updateNativeSurface(), and vo_opaque MUST be the surface or nullptr.
    If vo_opaque is nullptr, i.e. the default, then all context will have the same RenderAPI type, and call setRenderAPI() once is enough.
    If vo_opaque is surface(not null), each surface can have it's own RenderAPI type.
    RenderAPI members will be initialized when a rendering context for surface is created, and keep valid in rendering functions like renderVideo()
  2. Set foreign context provided by user. setRenderAPI() and other functions with vo_opaque parameter can be called in any order
  3. render. renderVideo() will use the given api for vo_opaque

  If setRenderAPI() is not called by user, a default one (usually GLRenderAPI) is used, thus renderAPI() always not null.
  setRenderAPI() is not thread safe, so usually called before rendering starts, or native surface is set.
*/
    void (*setRenderAPI)(mdkPlayer*, mdkRenderAPI* api, void* vo_opaque);
/*!
  \brief renderApi()
  get render api. For offscreen rendering, may only api type be valid in setRenderAPI(), and other members are filled internally, and used by user after renderVideo()
 */
    mdkRenderAPI* (*renderAPI)(mdkPlayer*, void* vo_opaque);

/*!
  \brief mapPoint
  map a point from one coordinates to another. a frame must be rendered. coordinates is normalized to [0, 1].
  \param x points to x coordinate of viewport or currently rendered video frame
  \param z not used
*/
    void (*mapPoint)(mdkPlayer*, MDK_MapDirection dir, float* x, float* y, float* z, void* vo_opaque);
    void (*onSync)(mdkPlayer*, mdkSyncCallback cb, int minInterval);

    void (*setVideoEffect)(mdkPlayer*, MDK_VideoEffect effect, const float* values, void* vo_opaque);
    void* reserved[8];
} mdkPlayerAPI;

MDK_API mdkPlayerAPI* mdkPlayerAPI_new();
MDK_API void mdkPlayerAPI_delete(mdkPlayerAPI**);
/* MUST be called when a foreign OpenGL context previously used is being destroyed to release context resources. The context MUST be current.*/
MDK_API void MDK_foreignGLContextDestroyed();

#ifdef __cplusplus
}
#endif
