/*
 * Copyright (c) 2019 WangBin <wbsecg1 at gmail.com>
 */
#pragma once
#include <inttypes.h>
#include <limits.h>

#if defined(_WIN32)
#define MDK_EXPORT __declspec(dllexport)
#define MDK_IMPORT __declspec(dllimport)
#else
#define MDK_EXPORT __attribute__((visibility("default")))
#define MDK_IMPORT __attribute__((visibility("default")))
#endif

#ifdef BUILD_MDK_STATIC
# define MDK_API
#else
# if defined(BUILD_MDK_LIB)
#  define MDK_API MDK_EXPORT
# else
#  define MDK_API MDK_IMPORT
# endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

static const float MDK_IgnoreAspectRatio = 0; /* stretch, ROI etc.*/
static const float MDK_KeepAspectRatio = -1;
static const float MDK_KeepAspectRatioCrop = -2; /* by expending and cropping*/

enum MDK_MediaType {
    MDK_MediaType_Unknown = -1,
    MDK_MediaType_Video,
    MDK_MediaType_Audio,
    MDK_MediaType_Data,
    MDK_MediaType_Subtitle,
    MDK_MediaType_Attachment,
    MDK_MediaType_Count
};

/*!
  \brief The MediaStatus enum
  Defines the io status of a media stream,
  Use flags_added/removed() to check the change, for example buffering after seek is Loaded|Prepared|Buffering, and changes to Loaded|Prepared|Buffered when seek completed
 */
enum MDK_MediaStatus
{
    MDK_MediaStatus_NoMedia = 0, /* initial status, not invalid. // what if set an empty url and closed?*/
    MDK_MediaStatus_Unloaded = 1, /* unloaded // (TODO: or when a source(url) is set?)*/
    MDK_MediaStatus_Loading = 1<<1, /* when source is set*/
    MDK_MediaStatus_Loaded = 1<<2, /* if auto load and source is set. player is stopped state*/
    MDK_MediaStatus_Prepared = 1<<8, /* all tracks are buffered and ready to decode frames. tracks failed to open decoder are ignored*/
    MDK_MediaStatus_Stalled = 1<<3, /* insufficient buffering or other interruptions (timeout, user interrupt)*/
    MDK_MediaStatus_Buffering = 1<<4, /* NOT IMPLEMENTED*/
    MDK_MediaStatus_Buffered = 1<<5, /* when playing //NOT IMPLEMENTED*/
    MDK_MediaStatus_End = 1<<6, /* Playback has reached the end of the current media. The player is in the State::Stopped.*/
    MDK_MediaStatus_Seeking = 1<<7, /* can be used with Buffering, Loaded. FIXME: NOT IMPLEMENTED*/
    MDK_MediaStatus_Invalid = 1<<31, /*  invalid media source*/
};

/*!
 * \brief The State enum
 * Defines the current state of a media player. Can be set by user
 */
enum MDK_State {
    MDK_State_NotRunning,
    MDK_State_Stopped = MDK_State_NotRunning,
    MDK_State_Running,
    MDK_State_Playing = MDK_State_Running, /* start/resume to play*/
    MDK_State_Paused,
};
typedef MDK_State MDK_PlaybackState;

enum MDK_BufferMode {
    MDK_BufferTime,
    MDK_BufferBytes,
    MDK_BufferPackets
};

enum MDK_SeekFlag {
    /* choose one of SeekFromX */
    MDK_SeekFlag_From0       = 1,    /* relative to time 0*/
    MDK_SeekFlag_FromStart   = 1<<1, /* relative to media start position*/
    MDK_SeekFlag_FromNow     = 1<<2, /* relative to current position, the seek position can be negative*/
    MDK_SeekFlag_Byte        = 1<<5,
    /* combine the above values with one of the following*/
    MDK_SeekFlag_KeyFrame    = 1<<8, /* fast key-frame seek, forward if Backward is not set. If not set, it's accurate seek but slow, implies backward seek internally*/
    MDK_SeekFlag_Fast        = MDK_SeekFlag_KeyFrame,
    MDK_SeekFlag_AnyFrame    = 1<<9, /* fast, broken image if video format has key frames. TODO: remove?*/

    /* Useful if seek backward repeatly, .i.e. target < playback(not buffered) position. result positions may be the same repeatly if seek forward w/ this flag, or seek backward w/o this flag*/
    MDK_SeekFlag_Backward    = 1<<16, /* for KeyFrame seek only. NOTE: FrameReader/PacketReader only. It has no effect to (un)set this flag in MediaControl/MediaPlayer and higher level apis*/
    MDK_SeekFlag_Default     = MDK_SeekFlag_KeyFrame|MDK_SeekFlag_FromStart
};

/*!
 * \brief javaVM
 * Set/Get current java vm
 * \param vm null to get current vm
 * \return current vm
 */
MDK_API void* MDK_javaVM(void* vm);

enum MDK_LogLevel {
    MDK_LogLevel_Off,
    MDK_LogLevel_Error,
    MDK_LogLevel_Warning,
    MDK_LogLevel_Info,
    MDK_LogLevel_Debug,
    MDK_LogLevel_All
};
MDK_API void MDK_setLogLevel(MDK_LogLevel value);
MDK_API MDK_LogLevel MDK_logLevel();
/* \brief setLogHandler
  if log handler is not set, i.e. setLogHandler() was not called, log is disabled.
  if set to non-null handler, log will be passed to the handler.
  if previous handler is set by user and not null, then call setLogHandler(nullptr) will print to stderr, and call setLogHandler(nullptr) again to silence the log
*/
typedef struct MDK_LogHandler {
    void (*cb)(MDK_LogLevel, const char*, void* opaque);
    void* opaque;
} MDK_LogHandler;
MDK_API void MDK_setLogHandler(MDK_LogHandler);

/*
  events:
  "decoder.audio/video" + "open" + stream: decoder of a stream is open, or failed to open if error != 0
  progress + "reader.buffering": buffering progress
*/
struct MDK_MediaEvent {
    int64_t error; /* result <0: error code(fourcc?). >=0: special value depending on event*/
    const char* category;
    const char* detail; /* if error, detail can be error string*/

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
typedef struct MDK_MediaEventListener {
    bool (*cb)(const MDK_MediaEvent*, void* opaque);
    void* opaque;
} MDK_MediaEventListener;

static const int64_t kTimeout = 10000;
/*!
 * \brief TimeoutCallback
 * \param ms elapsed milliseconds since restart
 * return true to abort current operation on timeout.
 * A null callback will abort current operation when timeout.
 * Setting a negative timeout value means timeout is inf.
 */
typedef struct MDK_TimeoutCallback {
    bool (*cb)(int64_t ms, void* opaque);
    void* opaque;
} MDK_TimeoutCallback;
#ifdef __cplusplus
}
#endif
