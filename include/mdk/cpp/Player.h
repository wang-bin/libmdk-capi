/*
 * Copyright (c) 2016-2020 WangBin <wbsecg1 at gmail.com>
 * This file is part of MDK
 * MDK SDK: https://sourceforge.net/projects/mdk-sdk/files
 * Free for GPL softwares or non-commercial use.
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 */
#pragma once
#include "global.h"
#include "MediaInfo.h"
#include "RenderAPI.h"
#include "../c/Player.h"
#include "VideoFrame.h"
#include <cinttypes>
#include <cstdlib>
#include <map>
#include <vector>

MDK_NS_BEGIN

/*!
  \brief PrepareCallback
  \param position in callback is the actual position, or <0 (TODO: error code as position) if prepare() failed.
  \param boost in callback can be set by user(*boost = true/false) to boost the first frame rendering. default is true.
  \return false to unload media immediately when media is loaded and MediaInfo is ready, true to continue.
    example: always return false can be used as media information reader
 */
using PrepareCallback = std::function<bool(int64_t position, bool* boost)>;

/*!
 * \brief The Player class
 * High level API with basic playback function.
 */
class AudioFrame;
class VideoFrame;
class Player
{
public:
    // MUST be called when a foreign OpenGL context previously used is being destroyed to release context resources. The context MUST be current.
    static void foreignGLContextDestroyed() {
        MDK_foreignGLContextDestroyed();
    }

    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;
    Player() : p(mdkPlayerAPI_new()) {}
    ~Player() {
        mdkPlayerAPI_delete(&p);
    }

    void setMute(bool value = true) {
        MDK_CALL(p, setMute, value);
    }

    void setVolume(float value) {
        MDK_CALL(p, setVolume, value);
    }

    // MUST call setActiveTracks() after setMedia(), otherwise the 1st track in the media is used
    void setMedia(const char* url) {
        MDK_CALL(p, setMedia, url);
    }
    // Set individual source for type, e.g. audio track file. If url is not empty, an individual pipeline will be used for 'type' tracks.
    // If url is empty, use 'type' tracks in MediaType::Video url.
    // TODO: default type is Unknown
    void setMedia(const char* url, MediaType type) {
        MDK_CALL(p, setMediaForType, url, (MDK_MediaType)type);
    }

    const char* url() const {
        return MDK_CALL(p, url);
    }

    void setPreloadImmediately(bool value = true) {
        MDK_CALL(p, setPreloadImmediately, value);
    }
/*!
  \brief setNextMedia
  Gapless play the next media after current media playback end
  \param flags seek flags if startPosition > 0, accurate or fast
  setState(State::Stopped) only stops current media. Call setNextMedia(nullptr, -1) first to disable next media.
 */
    void setNextMedia(const char* url, int64_t startPosition = 0, SeekFlag flags = SeekFlag::FromStart) {
        MDK_CALL(p, setNextMedia, url, startPosition, MDKSeekFlag(flags));
    }

    void currentMediaChanged(std::function<void()> cb) { // call before setMedia()
        current_cb_ = cb;
        mdkCurrentMediaChangedCallback callback;
        callback.cb = [](void* opaque){
            auto f = (std::function<void()>*)opaque;
            (*f)();
        };
        callback.opaque = current_cb_ ? (void*)&current_cb_ : nullptr;
        MDK_CALL(p, currentMediaChanged, callback);
    }

    // backends can be: AudioQueue(Apple only), OpenSL(Android only), ALSA(linux only), XAudio2(Windows only), OpenAL
    void setAudioBackends(const std::vector<std::string>& names) {
        std::vector<const char*> s(names.size() + 1, nullptr);
        for (size_t i = 0; i < names.size(); ++i)
            s[i] = names[i].data();
        MDK_CALL(p, setAudioBackends, s.data());
    }

    void setAudioDecoders(const std::vector<std::string>& names) {
        std::vector<const char*> s(names.size() + 1, nullptr);
        for (size_t i = 0; i < names.size(); ++i)
            s[i] = names[i].data();
        MDK_CALL(p, setAudioDecoders, s.data());
    }

// see https://github.com/wang-bin/mdk-sdk/wiki/Player-APIs#setdecodernamesconst-stdvectorstdstring-names
    void setVideoDecoders(const std::vector<std::string>& names) {
        std::vector<const char*> s(names.size() + 1, nullptr);
        for (size_t i = 0; i < names.size(); ++i)
            s[i] = names[i].data();
        MDK_CALL(p, setVideoDecoders, s.data());
    }

/*!
  \brief setTimeout
  callback ms: elapsed milliseconds
  callback return: true to abort current operation on timeout.
  A null callback can abort current operation.
  Negative timeout infinit.
  Default timeout is 10s
 */
    void setTimeout(int64_t ms, std::function<bool(int64_t ms)> cb = nullptr) {
        timeout_cb_ = cb;
        mdkTimeoutCallback callback;
        callback.cb = [](int64_t ms, void* opaque){
            auto f = (std::function<bool(int64_t ms)>*)opaque;
            return (*f)(ms);
        };
        callback.opaque = timeout_cb_ ? (void*)&timeout_cb_ : nullptr;
        MDK_CALL(p, setTimeout, ms, callback);
    }

/*!
  \brief prepare
  Preload a media and then becomes State::Paused. \sa PrepareCallback
  To play a media from a given position, call prepare(ms) then setState(State::Playing)
  \param startPosition start from position, relative to media start position(i.e. MediaInfo.start_time)
  \param flags seek flag if startPosition != 0.
  For fast seek(has flag SeekFlag::Fast), the first frame is a key frame whose timestamp >= startPosition
  For accurate seek(no flag SeekFlag::Fast), the first frame is the nearest frame whose timestamp <= startPosition, but the position passed to callback is the key frame position <= startPosition
 */
    void prepare(int64_t startPosition = 0, PrepareCallback cb = nullptr, SeekFlag flags = SeekFlag::FromStart) {
        prepare_cb_ = cb;
        mdkPrepareCallback callback;
        callback.cb = [](int64_t position, bool* boost, void* opaque){
            auto f = (PrepareCallback*)opaque;
            return (*f)(position, boost);
        };
        callback.opaque = prepare_cb_ ? (void*)&prepare_cb_ : nullptr;
        MDK_CALL(p, prepare, startPosition, callback, MDKSeekFlag(flags));
    }

    const MediaInfo& mediaInfo() const {
        from_c(MDK_CALL(p, mediaInfo), &info_);
        return info_;
    }

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
    void setState(PlaybackState value) {
        MDK_CALL(p, setState, MDK_State(value));
    }

    PlaybackState state() const {
        return (PlaybackState)MDK_CALL(p, state);
    }

    Player& onStateChanged(std::function<void(State)> cb) {
        state_cb_ = cb;
        mdkStateChangedCallback callback;
        callback.cb = [](MDK_State value, void* opaque){
            auto f = (std::function<void(PlaybackState)>*)opaque;
            (*f)(State(value));
        };
        callback.opaque = state_cb_ ? (void*)&state_cb_ : nullptr;
        MDK_CALL(p, onStateChanged, callback);
        return *this;
    }

    bool waitFor(State value, long timeout = -1) {
        return MDK_CALL(p, waitFor, (MDK_State)value, timeout);
    }

    MediaStatus mediaStatus() const {
        return (MediaStatus)MDK_CALL(p, mediaStatus);
    }
/*!
  \brief onMediaStatusChanged
  Add a callback to be invoked when MediaStatus is changed
  \param cb null to clear callbacks
 */
    Player& onMediaStatusChanged(std::function<bool(MediaStatus)> cb) {
        status_cb_ = cb;
        mdkMediaStatusChangedCallback callback;
        callback.cb = [](MDK_MediaStatus value, void* opaque){
            auto f = (std::function<bool(MediaStatus)>*)opaque;
            return (*f)(MediaStatus(value));
        };
        callback.opaque = status_cb_ ? (void*)&status_cb_ : nullptr;
        MDK_CALL(p, onMediaStatusChanged, callback);
        return *this;
    }

    enum SurfaceType {
        Auto, // platform default type
        X11,
        GBM,
        Wayland,
    };
/*!
 * \brief updateNativeSurface
 * If surface is not created, create rendering context internally by createSurface() and attached to native surface
 * native surface MUST be not null before destroying player
 */
// type: ignored if win ptr does not change (request to resize)
    void updateNativeSurface(void* surface, int width = -1, int height = -1, SurfaceType type = SurfaceType::Auto) {
        MDK_CALL(p, updateNativeSurface, surface, width, height, (MDK_SurfaceType)type);
    }

    void createSurface(void* nativeHandle = nullptr, SurfaceType type = SurfaceType::Auto) {
        MDK_CALL(p, createSurface, nativeHandle, (MDK_SurfaceType)type);
    }

    void resizeSurface(int w, int h) {
        MDK_CALL(p, resizeSurface, w, h);
    }

    void showSurface() {
        MDK_CALL(p, showSurface);
    }

// vo_opaque: a ptr to identify the renderer. can be null, then it is the default vo/renderer.
    struct SnapshotRequest {
/* data: rgba data. Created internally or provided by user.
   If data is provided by user, stride,  height and width MUST be also set, and data MUST be valid until snapshot callback is finished.
 */
        uint8_t* data = nullptr;
        // result width of snapshot image set by user, or the same as current frame width if 0. no renderer transform.
        // if both requested width and height are < 0, then result image is scaled image of current frame with ratio=width/height. no renderer transform.
        // if only one of width and height < 0, then the result size is video renderer viewport size, and all transforms will be applied.
        int width = 0;
        int height = 0;
        int stride = 0;
        bool subtitle = false; // not supported yet
    };
/* \brief SnapshotCallback
   snapshot callback.
   \param req result request. If null, snapshot failed. Otherwise req.width, height and stride are always >0, data is never null.
   \param frameTime captured frame timestamp(seconds)
   \param opaque user data
   \returns a file path to save as a file(jpeg is recommended, other formats depends on ffmpeg runtime). or empty to do nothing.
   Returned string will be freed internally(assume allocated by malloc family apis).
   Callback is called in a dedicated thread, so time-consuming operations(encode, file io etc.) are allowed in the callback.
 */
    using SnapshotCallback = std::function<std::string(SnapshotRequest*, double frameTime)>;
/*!
  \brief snapshot
  take a snapshot from current renderer. The result is in bgra format, or null on failure.
  When `snapshot()` is called, redraw is scheduled for `vo_opaque`'s renderer, then renderer will take a snapshot in rendering thread.
  So for a foreign context, if renderer's surface/window/widget is invisible or minimized, snapshot may do nothing because of system or gui toolkit painting optimization.
*/
    void snapshot(SnapshotRequest* request, SnapshotCallback cb, void* vo_opaque = nullptr) {
        snapshot_cb_ = cb;
        mdkSnapshotCallback callback;
        callback.cb = [](mdkSnapshotRequest* req, double frameTime, void* opaque){
            auto f = (SnapshotCallback*)opaque;
            auto file = (*f)((SnapshotRequest*)req, frameTime);
            if (file.empty())
                return (char*)nullptr;
            auto filec = (char*)malloc(file.size() + 1);
            memcpy(filec, file.data(), file.size());
            filec[file.size()] = 0;
            return filec;
        };
        callback.opaque = snapshot_cb_ ? (void*)&snapshot_cb_ : nullptr;
        return MDK_CALL(p, snapshot, (mdkSnapshotRequest*)request, callback, vo_opaque);
    }

/*
  \brief setProperty
  Set additional properties. Can be used to store user data, or change player behavior if the property is defined internally.
  Predefined properties are:
  - "video.avfilter": ffmpeg avfilter filter graph string for video track. take effect immediately
  - "audio.avfilter": ffmpeg avfilter filter graph string for audio track. take effect immediately
  - "continue_at_end": do not stop playback when decode and render to end of stream. only setState(State::Stopped) can stop playback
 */
    void setProperty(const std::string& key, const std::string& value) {
        MDK_CALL(p, setProperty, key.data(), value.data());
    }

    std::string property(const std::string& key, const std::string& defaultValue = std::string()) const {
        auto value = MDK_CALL(p, getProperty, key.data());
        if (!value)
            return defaultValue;
        return value;
    }
// A vo/renderer (e.g. the default vo/renderer) is gfx context aware, i.e. can render in multiple gfx contexts with a single vo/renderer, but parameters(e.g. surface size)
// must be updated when switch to a new context. So per gfx context vo/renderer can be better because parameters are stored in vo/renderer.
/*!
  \brief getVideoFrame
  get current rendered frame, i.e. the decoded video frame rendered by renderVideo()
 */
    void getVideoFrame(VideoFrame* frame, void* vo_opaque = nullptr);
/*
  \brief setVideoSurfaceSize
  Window size, surface size or drawable size. Render callback(if exists) will be invoked if width and height > 0.
  If width or heigh < 0, corresponding video renderer (for vo_opaque) will be removed. But subsequence call with this vo_opaque will create renderer again. So it can be used before destroying the renderer.
 */
    void setVideoSurfaceSize(int width, int height, void* vo_opaque = nullptr) {
        MDK_CALL(p, setVideoSurfaceSize, width, height, vo_opaque);
    }

    void setVideoViewport(float x, float y, float w, float h, void* vo_opaque = nullptr) {
        MDK_CALL(p, setVideoViewport, x, y, w, h, vo_opaque);
    }

/*!
  \brief setAspectRatio
  Video display aspect ratio.
  0: ignore aspect ratio and scale to fit renderer viewport
  -1(default): keep frame aspect ratio and scale as large as possible inside renderer viewport
  -2: keep frame aspect ratio and scale as small as possible outside renderer viewport
 */
    void setAspectRatio(float value, void* vo_opaque = nullptr) {
        MDK_CALL(p, setAspectRatio, value, vo_opaque);
    }

    void rotate(int degree, void* vo_opaque = nullptr) {
        MDK_CALL(p, rotate, degree, vo_opaque);
    }

    void scale(float x, float y, void* vo_opaque = nullptr) {
        MDK_CALL(p, scale, x, y, vo_opaque);
    }

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
    Player& setRenderAPI(RenderAPI* api, void* vo_opaque = nullptr) {
        MDK_CALL(p, setRenderAPI, reinterpret_cast<mdkRenderAPI*>(api), vo_opaque);
        return *this;
    }
/*!
  \brief renderApi()
  get render api. For offscreen rendering, may only api type be valid in setRenderAPI(), and other members are filled internally, and used by user after renderVideo()
 */
    RenderAPI* renderAPI(void* vo_opaque = nullptr) {
        return reinterpret_cast<RenderAPI*>(MDK_CALL(p, renderAPI, vo_opaque));
    }

/*!
   \brief renderVideo
  Render the next or current(redraw) frame. Foreign render context only (i.e. not created by createSurface()/updateNativeSurface()).
  OpenGL: Can be called in multiple foreign contexts for the same vo_opaque.
   \return timestamp of rendered frame, or < 0 if no frame is rendered
 */
    double renderVideo(void* vo_opaque = nullptr) {
        return MDK_CALL(p, renderVideo, vo_opaque);
    }
/*!
  \brief setBackgroundColor
  r, g, b, a range is [0, 1]. default is 0. if out of range, background color will not be filled
 */
    void setBackgroundColor(float r, float g, float b, float a, void* vo_opaque = nullptr) {
        return MDK_CALL(p, setBackgroundColor, r, g, b, a, vo_opaque);
    }

/*!
  \brief setRenderCallback
  set a callback which is invoked when the vo coresponding to vo_opaque needs to update/draw content, e.g. when a new frame is received in the renderer.
  Also invoked in setVideoSurfaceSize(), setVideoViewport(), setAspectRatio() and rotate(), take care of dead lock in callback and above functions.
  with vo_opaque, user can know which vo/renderer is rendering, useful for multiple renderers
  There may be no frames or playback not even started, but renderer update is required internally
*/
    void setRenderCallback(std::function<void(void* vo_opaque)> cb) { // per vo?
        render_cb_ = cb;
        mdkRenderCallback callback;
        callback.cb = [](void* vo_opaque, void* opaque){
            auto f = (std::function<void(void* vo_opaque)>*)opaque;
            (*f)(vo_opaque);
        };
        callback.opaque = render_cb_ ? (void*)&render_cb_ : nullptr;
        MDK_CALL(p, setRenderCallback, callback);
    }

/*!
  \brief onFrame
  A callback to be invoked before delivering a frame to renderers. Frame can be VideoFrame and AudioFrame(NOT IMPLEMENTED).
  The callback can be used as a filter.
  TODO: frames not in host memory
  \param cb callback to be invoked. returns pending number of frames. callback parameter is input and output frame. if input frame is an invalid frame, output a pending frame.
  For most filters, 1 input frame generates 1 output frame, then return 0.
 */
    template<class Frame>
    Player& onFrame(std::function<int(Frame&, int/*track*/)> cb);

    int64_t position() const {
        return MDK_CALL(p, position);
    }
/*!
  \brief seek
  \param cb callback to be invoked when seek finished(ret >= 0), error occured(ret < 0, usually -1) or skipped because of unfinished previous seek(ret == -2)
 */
    bool seek(int64_t pos, SeekFlag flags, std::function<void(int64_t)> cb = nullptr) {
        seek_cb_ = cb;
        mdkSeekCallback callback;
        callback.cb = [](int64_t ms, void* opaque){
            auto f = (std::function<void(int64_t)>*)opaque;
            (*f)(ms);
        };
        callback.opaque = seek_cb_ ? (void*)&seek_cb_ : nullptr;
        return MDK_CALL(p, seekWithFlags, pos, MDK_SeekFlag(flags), callback);
    }

    bool seek(int64_t pos, std::function<void(int64_t)> cb = nullptr) {
        return seek(pos, SeekFlag::Default, cb);
    }

    void setPlaybackRate(float value) {
        MDK_CALL(p, setPlaybackRate, value);
    }

    float playbackRate() const {
        return MDK_CALL(p, playbackRate);
    }
/*!
 * \brief buffered
 * get buffered data duration and size
 * \return buffered data(packets) duration
 */
    int64_t buffered(int64_t* bytes = nullptr) const {
        return MDK_CALL(p, buffered, bytes);
    }
/*
  \brief bufferRange
  set duration range of buffered data.
  minMs: default 4000. wait for buffered duration >= minMs when before popping a packet from to decode
  maxMs: default 16000. max buffered duration.
  drop = true: drop old non-key frame packets to reduce buffered duration until < maxMs.
  drop = false: wait for buffered duration < maxMs before pushing packets

  Usually you don't need to call this api. This api can be used for low latency live videos, for example setBufferRange(0, 1000, true) will decode as soon as possible when media data received, also it ensures the max delay of rendered video is 1s, and no accumulated delay.
 */
    void setBufferRange(int64_t minMs = 4000, int64_t maxMs = 16000, bool drop = false) {
        MDK_CALL(p, setBufferRange, minMs, maxMs, drop);
    }
/*!
  \brief switchBitrate
  A new media will be played later
  \param delay switch after at least delay ms. TODO: determined by buffered time, e.g. from high bit rate without enough buffered samples to low bit rate
  \param cb (true/false) called when finished/failed
  \param flags seek flags for the next url, accurate or fast
 */
    void switchBitrate(const char* url, int64_t delay = -1, std::function<void(bool)> cb = nullptr) {
        switch_cb_ = cb;
        SwitchBitrateCallback callback;
        callback.cb = [](bool value, void* opaque){
            auto f = (std::function<void(bool)>*)opaque;
            (*f)(value);
        };
        callback.opaque = switch_cb_ ? (void*)&switch_cb_ : nullptr;
        return MDK_CALL(p, switchBitrate, url, delay, callback);
    }
/*!
 * \brief switchBitrateSingalConnection
 * Only 1 media is loaded. The previous media is unloaded and the playback continues. When new media is preloaded, stop the previous media at some point
 * MUST call setPreloadImmediately(false) because PreloadImmediately for singal connection preload is not possible.
 * \return false if preload immediately
 * This will not affect next media set by user
 */
    bool switchBitrateSingleConnection(const char *url, std::function<void(bool)> cb = nullptr) {
        switch_cb_ = cb;
        SwitchBitrateCallback callback;
        callback.cb = [](bool value, void* opaque){
            auto f = (std::function<void(bool)>*)opaque;
            (*f)(value);
        };
        callback.opaque = switch_cb_ ? (void*)&switch_cb_ : nullptr;
        return MDK_CALL(p, switchBitrateSingleConnection, url, callback);
    }

/*!
  \brief onEvent
  callback return: true if event is processed and should stop dispatching.
 */
    Player& onEvent(std::function<bool(const MediaEvent&)> cb, CallbackToken* token = nullptr) {
        mdkMediaEventCallback callback{};
        if (!cb) {
            MDK_CALL(p, onEvent, callback, token ? &event_cb_key_[*token] : nullptr);
            if (token) {
                event_cb_.erase(*token);
                event_cb_key_.erase(*token);
            } else {
                event_cb_.clear();
                event_cb_key_.clear();
            }
        } else {
            static CallbackToken k = 1;
            event_cb_[k] = cb;
            callback.cb = [](const mdkMediaEvent* me, void* opaque){
                auto f = (std::function<bool(const MediaEvent&)>*)opaque;
                MediaEvent e;
                e.error = me->error;
                e.category = me->category;
                e.detail = me->detail;
                e.decoder.stream = me->decoder.stream;
                return (*f)(e);
            };
            callback.opaque = &event_cb_[k];
            CallbackToken t;
            MDK_CALL(p, onEvent, callback, &t);
            event_cb_key_[k] = t;
            if (token)
                *token = t;
            k++;
        }
        return *this;
    }
/*
  \brief record
  Start to record or stop recording current media by remuxing packets read. If media is not loaded, recorder will start when playback starts
  \param url destination. null or the same value as recording one to stop recording
  \param format forced format if unable to guess from url suffix
 */
    void record(const char* url = nullptr, const char* format = nullptr) {
        MDK_CALL(p, record, url, format);
    }

/*!
  \brief setLoop
  DEPRECATED! use setLoop+setRange instead
 */
    void setLoop(int count, int64_t a, int64_t b = INT64_MAX) {
        setLoop(count);
        setRange(a, b);
    }
/*!
  \brief setLoop
  Set A-B loop repeat count.
  \param count repeat count. 0 to disable looping and stop when out of range(B)
 */
    void setLoop(int count) {
        MDK_CALL(p, setLoop, count);
    }
/*
  \brief onLoop
  add/remove a callback which will be invoked right before a new A-B loop
  \param cb callback with current loop count elapsed
 */
    Player& onLoop(std::function<void(int)> cb, CallbackToken* token = nullptr) {
        mdkLoopCallback callback{};
        if (!cb) {
            MDK_CALL(p, onLoop, callback, token ? &loop_cb_key_[*token] : nullptr);
            if (token) {
                loop_cb_.erase(*token);
                loop_cb_key_.erase(*token);
            } else {
                loop_cb_.clear();
                loop_cb_key_.clear();
            }
        } else {
            static CallbackToken k = 1;
            loop_cb_[k] = cb;
            callback.cb = [](int countNow, void* opaque){
                auto f = (std::function<void(int)>*)opaque;
                return (*f)(countNow);
            };
            callback.opaque = &loop_cb_[k];
            CallbackToken t;
            MDK_CALL(p, onLoop, callback, &t);
            loop_cb_key_[k] = t;
            if (token)
                *token = t;
            k++;
        }
        return *this;
    }
/*!
  \brief setRange
  Set A-B loop range, or playback range
  \param a loop position begin, in ms.
  \param b loop position end, in ms. -1, INT64_MAX or numeric_limit<int64_t>::max() indicates b is the end of media
 */
    void setRange(int64_t a, int64_t b = INT64_MAX) {
        MDK_CALL(p, setRange, a, b);
    }
private:
    mdkPlayerAPI* p = nullptr;
    std::function<void()> current_cb_ = nullptr;
    std::function<bool(int64_t ms)> timeout_cb_ = nullptr;
    std::function<bool(int64_t position, bool* boost)> prepare_cb_ = nullptr;
    std::function<void(State)> state_cb_ = nullptr;
    std::function<bool(MediaStatus)> status_cb_ = nullptr;
    std::function<void(void* vo_opaque)> render_cb_ = nullptr;
    std::function<void(int64_t)> seek_cb_ = nullptr;
    std::function<void(bool)> switch_cb_ = nullptr;
    SnapshotCallback snapshot_cb_ = nullptr;
    std::function<int(VideoFrame&, int/*track*/)> video_cb_ = nullptr;
    std::map<CallbackToken, std::function<bool(const MediaEvent&)>> event_cb_; // rb tree, elements never destroyed
    std::map<CallbackToken,CallbackToken> event_cb_key_;
    std::map<CallbackToken, std::function<void(int)>> loop_cb_; // rb tree, elements never destroyed
    std::map<CallbackToken,CallbackToken> loop_cb_key_;

    mutable MediaInfo info_;
};


template<>
inline Player& Player::onFrame(std::function<int(VideoFrame&, int/*track*/)> cb)
{
    video_cb_ = cb;
    mdkVideoCallback callback;
    callback.cb = [](mdkVideoFrameAPI** pFrame/*in/out*/, int track, void* opaque){
        VideoFrame frame;
        frame.attach(*pFrame);
        auto f = (std::function<int(VideoFrame&, int)>*)opaque;
        auto pendings = (*f)(frame, track);
        *pFrame = frame.detach();
        return pendings;
    };
    callback.opaque = video_cb_ ? (void*)&video_cb_ : nullptr;
    MDK_CALL(p, onVideo, callback);
    return *this;
}

MDK_NS_END
