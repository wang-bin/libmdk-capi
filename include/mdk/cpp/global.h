/*
 * Copyright (c) 2016-2019 WangBin <wbsecg1 at gmail.com>
 */
#pragma once
#include <cinttypes> //_UINT8_MAX. c++11 <cstdint> defines _STDC_LIMIT_MACROS
#include <climits> // INT_MAX
#include <functional>
#include <memory>
#include <string>
#include "../c/global.h"

#ifndef MDK_NS
#define MDK_NS mdk
#endif
# define MDK_NS_BEGIN namespace MDK_NS {
# define MDK_NS_END }
# define MDK_NS_PREPEND(X) ::MDK_NS::X

MDK_NS_BEGIN

static const float IgnoreAspectRatio = 0; // stretch, ROI etc.
static const float KeepAspectRatio = -1;
static const float KeepAspectRatioCrop = -2; // by expending and cropping

#if defined(_MSC_VER) && _MSC_VER < 1900
# ifdef constexpr
#  undef constexpr
# endif
# define constexpr //inline // constexpr implies inline. but we can not declare a var as inline like constexpr
#endif
/*!
 * \brief is_flag
 * if enum E is of enum type, to enable flag(bit) operators, define
 * \code template<> struct is_flag<E> : std::true_type {}; \endcode
 */
template<typename T> struct is_flag; // 
template<typename T>
using if_flag = std::enable_if<std::is_enum<T>::value && is_flag<T>::value>;
template<typename E, typename = if_flag<E>>
constexpr E operator~(E e1) { return E(~typename std::underlying_type<E>::type(e1));}
template<typename E, typename = if_flag<E>>
constexpr E operator|(E e1, E e2) { return E(typename std::underlying_type<E>::type(e1) | typename std::underlying_type<E>::type(e2));}
template<typename E, typename = if_flag<E>>
constexpr E operator^(E e1, E e2) { return E(typename std::underlying_type<E>::type(e1) ^ typename std::underlying_type<E>::type(e2));}
template<typename E, typename = if_flag<E>>
constexpr E operator&(E e1, E e2) { return E(typename std::underlying_type<E>::type(e1) & typename std::underlying_type<E>::type(e2));}
// assign in constexpr requires c++14 for clang/gcc, but not msvc(2013+), so the following functions are not constexpr for now. check c++ version?
template<typename E, typename = if_flag<E>>
constexpr E& operator|=(E& e1, E e2) { return e1 = e1 | e2;}
template<typename E, typename = if_flag<E>>
constexpr E& operator^=(E& e1, E e2) { return e1 = e1 ^ e2;}
template<typename E, typename = if_flag<E>>
constexpr E& operator&=(E& e1, E e2) { return e1 = e1 & e2;}
// convenience functions to test whether a flag exists. REQUIRED by scoped enum
template<typename E>
constexpr bool test_flag(E e) { return typename std::underlying_type<E>::type(e);}
template<typename E1, typename E2>
constexpr bool test_flag(E1 e1, E2 e2) { return test_flag(e1 & e2);}
template<typename E>
constexpr bool flags_added(E oldFlags, E newFlags, E testFlags) { return test_flag(newFlags, testFlags) && !test_flag(oldFlags, testFlags);}
template<typename E>
constexpr bool flags_removed(E oldFlags, E newFlags, E testFlags) { return !test_flag(newFlags, testFlags) && test_flag(oldFlags, testFlags);}

enum class MediaType : int8_t {
    Unknown = -1,
    Video,
    Audio,
    Data, // e.g. timed metadata tracks in ffmpeg 3.2
    Subtitle,
    Attachment,
    Count
};

/*!
  \brief The MediaStatus enum
  Defines the io status of a media stream,
  Use flags_added/removed() to check the change, for example buffering after seek is Loaded|Prepared|Buffering, and changes to Loaded|Prepared|Buffered when seek completed
 */
enum MediaStatus
{
    NoMedia = 0, // initial status, not invalid. // what if set an empty url and closed?
    Unloaded = 1, // unloaded // (TODO: or when a source(url) is set?)
    Loading = 1<<1, // when source is set
    Loaded = 1<<2, // if auto load and source is set. player is stopped state
    Prepared = 1<<8, // all tracks are buffered and ready to decode frames. tracks failed to open decoder are ignored
    Stalled = 1<<3, // insufficient buffering or other interruptions (timeout, user interrupt)
    Buffering = 1<<4, // NOT IMPLEMENTED
    Buffered = 1<<5, // when playing //NOT IMPLEMENTED
    End = 1<<6, // Playback has reached the end of the current media. The player is in the State::Stopped.
    Seeking = 1<<7, // can be used with Buffering, Loaded. FIXME: NOT IMPLEMENTED
    Invalid = 1<<31, //  invalid media source
};
template<> struct is_flag<MediaStatus> : std::true_type {};
// MediaStatusCallback

/*!
 * \brief The State enum
 * Defines the current state of a media player. Can be set by user
 */
enum class State : int8_t {
    NotRunning,
    Stopped = NotRunning,
    Running,
    Playing = Running, /// start/resume to play
    Paused,
};
typedef State PlaybackState;

enum BufferMode { // TODO: scoped
    BufferTime,
    BufferBytes,
    BufferPackets
};

enum class SeekFlag {
    /// choose one of SeekFromX
    From0       = 1,    /// relative to time 0
    FromStart   = 1<<1, /// relative to media start position
    FromNow     = 1<<2, /// relative to current position, the seek position can be negative
    Byte        = 1<<5,
    /// combine the above values with one of the following
    KeyFrame    = 1<<8, // fast key-frame seek, forward if Backward is not set. If not set, it's accurate seek but slow, implies backward seek internally
    Fast        = KeyFrame,
    AnyFrame    = 1<<9, // fast, broken image if video format has key frames. TODO: remove?

    // Useful if seek backward repeatly, .i.e. target < playback(not buffered) position. result positions may be the same repeatly if seek forward w/ this flag, or seek backward w/o this flag
    Backward    = 1<<16, // for KeyFrame seek only. NOTE: FrameReader/PacketReader only. It has no effect to (un)set this flag in MediaControl/MediaPlayer and higher level apis
    /// default values
    Default     = KeyFrame|FromStart
};
template<> struct is_flag<SeekFlag> : std::true_type {};

/*!
 * \brief javaVM
 * Set/Get current java vm
 * \param vm null to get current vm
 * \return current vm
 */
// TODO: mdk/ugl/ugs set/getGlobal(): XDisplay, JavaVM. use std::any or void*?
static inline void* javaVM(void* vm = nullptr) {
    return MDK_javaVM(vm);
}

enum LogLevel {
    Off,
    Error,
    Warning,
    Info,
    Debug,
    All
};
static inline void setLogLevel(LogLevel value) {
    MDK_setLogLevel(MDK_LogLevel(value));
}

static inline LogLevel logLevel() {
    return (LogLevel)MDK_logLevel();
}

/* \brief setLogHandler
  if log handler is not set, i.e. setLogHandler() was not called, log is disabled.
  if set to non-null handler, log will be passed to the handler.
  if previous handler is set by user and not null, then call setLogHandler(nullptr) will print to stderr, and call setLogHandler(nullptr) again to silence the log
*/
static inline void setLogHandler(std::function<void(LogLevel, const char*)> cb) {
    static std::function<void(LogLevel, const char*)> scb;
    scb = cb;
    MDK_LogHandler h;
    h.cb = [](MDK_LogLevel level, const char* msg, void* opaque){
        auto f = (std::function<void(LogLevel, const char*)>*)opaque;
        (*f)(LogLevel(level), msg);
    };
    h.opaque = scb ? (void*)&scb : nullptr;
    MDK_setLogHandler(h);
}

/*
  events:
  "decoder.audio/video" + "open" + stream: decoder of a stream is open, or failed to open if error != 0
  progress + "reader.buffering": buffering progress
*/
class MediaEvent { // why no export?
public:
    int64_t error = 0; // result <0: error code(fourcc?). >=0: special value depending on event
    std::string category;
    std::string detail; // if error, detail can be error string

    union {
        struct {
            int stream;
        } decoder;
    };
};
/*!
 * \brief MediaEventListener
 * return true if event is processed and dispatching.
 */
typedef std::function<bool(const MediaEvent&)> MediaEventListener;

static const int64_t kTimeout = 10000;
/*!
 * \brief TimeoutCallback
 * \param ms elapsed milliseconds since restart
 * return true to abort current operation on timeout.
 * A null callback will abort current operation when timeout.
 * Setting a negative timeout value means timeout is inf.
 */
typedef std::function<bool(int64_t ms)> TimeoutCallback;
MDK_NS_END

