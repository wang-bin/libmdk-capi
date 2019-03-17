/*
 * Copyright (c) 2016-2019 WangBin <wbsecg1 at gmail.com>
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 */
#pragma once
#include <functional>
#include <map>
#include <vector>
#include "global.h"
#include "../c/Player.h"

MDK_NS_BEGIN
/*!
 * \brief The Player class
 * High level API with basic playback function.
 */
class MediaInfo;
class AudioFrame;
class VideoFrame;
class Window;
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

    void setNextMedia(const char* url, int64_t startPosition = 0) {
        MDK_CALL(p, setNextMedia, url, startPosition);
    }

    void currentMediaChanged(std::function<void()> cb) { // call before setMedia()
        current_cb_ = cb;
        MDK_CurrentMediaChangedCallback callback;
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

    void setVideoDecoders(const std::vector<std::string>& names) {
        std::vector<const char*> s(names.size() + 1, nullptr);
        for (size_t i = 0; i < names.size(); ++i)
            s[i] = names[i].data();
        MDK_CALL(p, setVideoDecoders, s.data());
    }

    void setTimeout(int64_t value, TimeoutCallback cb = nullptr) {
        timeout_cb_ = cb;
        MDK_TimeoutCallback callback;
        callback.cb = [](int64_t ms, void* opaque){
            auto f = (TimeoutCallback*)opaque;
            return (*f)(ms);
        };
        callback.opaque = timeout_cb_ ? (void*)&timeout_cb_ : nullptr;
        MDK_CALL(p, setTimeout, value, callback);
    }

/*!
   \brief prepare
   To play a media from a given position, call prepare(ms) then setState(State::Playing)
   parameter position in callback is the actual position, or <0 (TODO: error code as position) if prepare() failed.
   parameter boost in callback can be set by user to boost the first frame rendering
 */
    void prepare(int64_t startPosition = 0, std::function<void(int64_t position, bool* boost)> cb = nullptr) {
        prepare_cb_ = cb;
        MDK_PrepareCallback callback;
        callback.cb = [](int64_t position, bool* boost, void* opaque){
            auto f = (std::function<void(int64_t position, bool* boost)>*)opaque;
            (*f)(position, boost);
        };
        callback.opaque = prepare_cb_ ? (void*)&prepare_cb_ : nullptr;
        MDK_CALL(p, prepare, startPosition, callback);
    }

    const MediaInfo& mediaInfo() const;

/*!
  \brief setState
  request a new state.
  setState(State::Stopped) only stops current media. Call setNextMedia(nullptr, -1) before stop to disable next media.
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

    void onStateChanged(std::function<void(State)> cb) {
        state_cb_ = cb;
        MDK_StateChangedCallback callback;
        callback.cb = [](MDK_State value, void* opaque){
            auto f = (std::function<void(PlaybackState)>*)opaque;
            (*f)(State(value));
        };
        callback.opaque = state_cb_ ? (void*)&state_cb_ : nullptr;
        MDK_CALL(p, onStateChanged, callback);
    }

    bool waitFor(State value, long timeout = -1) {
        return MDK_CALL(p, waitFor, (MDK_State)value, timeout);
    }

    MediaStatus mediaStatus() const {
        return (MediaStatus)MDK_CALL(p, mediaStatus);
    }

    void onMediaStatusChanged(std::function<bool(MediaStatus)> cb) {
        status_cb_ = cb;
        MDK_MediaStatusChangedCallback callback;
        callback.cb = [](MDK_MediaStatus value, void* opaque){
            auto f = (std::function<bool(MediaStatus)>*)opaque;
            return (*f)(MediaStatus(value));
        };
        callback.opaque = status_cb_ ? (void*)&status_cb_ : nullptr;
        MDK_CALL(p, onMediaStatusChanged, callback);
    }

    enum SurfaceType {
        Auto, // platform default type
        X11,
        GBM,
        Wayland,
    };
/*!
 * \brief updateNativeWindow
 * If window is not created, create rendering context internally by createWindow() and attached to native window
 * native window MUST be not null before destroying player
 */
// type: ignored if win ptr does not change (request to resize)
    void updateNativeWindow(void* win, int width = -1, int height = -1, SurfaceType type = SurfaceType::Auto) {
        MDK_CALL(p, updateNativeWindow, win, width, height, (MDK_SurfaceType)type);
    }

    void createWindow(void* nativeHandle = nullptr, SurfaceType type = SurfaceType::Auto) {
        MDK_CALL(p, createWindow, nativeHandle, (MDK_SurfaceType)type);
    }

    void resizeWindow(int w, int h) {
        MDK_CALL(p, resizeWindow, w, h);
    }

    void showWindow() {
        MDK_CALL(p, showWindow);
    }

// vo_opaque: a ptr to identify the renderer. cam be null, then it is the default vo/renderer.
// A vo/renderer (e.g. the default vo/renderer) is gfx context aware, i.e. can render in multiple gfx contexts with a single vo/renderer, but parameters(e.g. surface size)
// must be updated when switch to a new context. So per gfx context vo/renderer can be better because parameters are stored in vo/renderer.
/*!
  \brief getVideoFrame
  get current rendered frame, i.e. the decoded video frame rendered by renderVideo()
 */
    void getVideoFrame(VideoFrame* frame, void* vo_opaque = nullptr);

    void setVideoSurfaceSize(int width, int height, void* vo_opaque = nullptr) {
        MDK_CALL(p, setVideoSurfaceSize, width, height, vo_opaque);
    }

    void setVideoViewport(float x, float y, float w, float h, void* vo_opaque = nullptr) {
        MDK_CALL(p, setVideoViewport, x, y, w, h, vo_opaque);
    }

/// IgnoreAspectRatio, KeepAspectRatio, KeepAspectRatio or any positive float
    void setAspectRatio(float value, void* vo_opaque = nullptr) {
        MDK_CALL(p, setAspectRatio, value, vo_opaque);
    }

    void rotate(int degree, void* vo_opaque = nullptr) {
        MDK_CALL(p, rotate, degree, vo_opaque);
    }

    void scale(float x, float y, void* vo_opaque = nullptr) {
        MDK_CALL(p, scale, x, y, vo_opaque);
    }

/*!
   \brief renderVideo
   Render the next/current frame. Call only in Window.onDraw() (not created by createWindow()/updateNativeWindow()) or external graphics context's rendering function.
   Can be called in multiple foreign gfx contexts for the same vo_opaque.
   \return timestamp of rendered frame, or < 0 if no frame is rendered
 */
    double renderVideo(void* vo_opaque = nullptr) {
        return MDK_CALL(p, renderVideo, vo_opaque);
    }

    // callback is invoked when the vo coresponding to vo_opaque needs to update/draw content, e.g. when a new frame is received in the renderer.
    // Also invoked in setVideoSurfaceSize(), setVideoViewport(), setAspectRatio() and rotate(), take care of dead lock in callback and above functions.
    // with vo_opaque, user can know which vo/renderer is rendering, useful for multiple renderers
    void setRenderCallback(std::function<void(void* vo_opaque)> cb) { // per vo?
        render_cb_ = cb;
        MDK_RenderCallback callback;
        callback.cb = [](void* vo_opaque, void* opaque){
            auto f = (std::function<void(void* vo_opaque)>*)opaque;
            (*f)(vo_opaque);
        };
        callback.opaque = render_cb_ ? (void*)&render_cb_ : nullptr;
        MDK_CALL(p, setRenderCallback, callback);
    }

/*
  \brief onFrame
  called before delivering frame to renderers
 */
    template<class Frame>
    void onFrame(std::function<void(Frame&)> cb);

    int64_t position() const {
        return MDK_CALL(p, position);
    }

    bool seek(int64_t pos, SeekFlag flags, std::function<void(int64_t)> cb = nullptr) {
        seek_cb_ = cb;
        MDK_SeekCallback callback;
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
  duration range of buffered data.
  minMs: wait for buffered duration >= minMs when before popping a packet to decode
  drop = true: drop old non-key frame data to reduce buffered duration until less then maxMs.
  drop = false: wait for buffered duration less than maxMs before buffering more data
 */
    void setBufferRange(int64_t minMs, int64_t maxMs, bool drop = false) {
        MDK_CALL(p, setBufferRange, minMs, maxMs, drop);
    }
/*!
 * \brief switchBitrate
 * A new media will be played later
 * \param delay switch after at least delay ms. TODO: determined by buffered time, e.g. from high bit rate without enough buffered samples to low bit rate
 * \param cb (true/false) called when finished/failed
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
 * \brief addListener
 * listener the events from player, control and FrameReader
 * \return listener id
 */
    int64_t addListener(MediaEventListener cb) {
        static int64_t k = 0;
        listener_[k] = cb;
        MDK_MediaEventListener callback;
        callback.cb = [](const MDK_MediaEvent* me, void* opaque){
            auto f = (MediaEventListener*)opaque;
            MediaEvent e;
            e.error = me->error;
            e.category = me->category;
            e.detail = me->detail;
            e.decoder.stream = me->decoder.stream;
            return (*f)(e);
        };
        callback.opaque = &listener_[k];
        listener_key_[k] = MDK_CALL(p, addListener, callback);
        return k++;
    }

    void removeListener(int64_t listener) {
        MDK_CALL(p, removeListener, listener_key_[listener]);
        listener_.erase(listener);
        listener_key_.erase(listener);
    }

private:
    mdkPlayerAPI* p = nullptr;
    std::function<void()> current_cb_ = nullptr;
    TimeoutCallback timeout_cb_ = nullptr;
    std::function<void(int64_t position, bool* boost)> prepare_cb_ = nullptr;
    std::function<void(State)> state_cb_ = nullptr;
    std::function<bool(MediaStatus)> status_cb_ = nullptr;
    std::function<void(void* vo_opaque)> render_cb_ = nullptr;
    std::function<void(int64_t)> seek_cb_ = nullptr;
    std::function<void(bool)> switch_cb_ = nullptr;
    std::map<int64_t, MediaEventListener> listener_; // rb tree, elements never destroyed
    std::map<int64_t,int64_t> listener_key_;
};
MDK_NS_END
