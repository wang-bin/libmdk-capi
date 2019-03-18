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

typedef struct mdkAudioCodecParameters {
    const char* codec;
    uint32_t codec_tag;
    const uint8_t* extra_data; /* without padding data */
    int extra_data_size;
    int64_t bit_rate;
    int profile;
    int level;
    float frame_rate;

    bool is_float;
    bool is_unsigned;
    bool is_planar;
    int raw_sample_size;

    int channels;
    int sample_rate;
    int block_align;
    int frame_size; /* const samples per channel in a frame */
} mdkAudioCodecParameters;

typedef struct mdkAudioStreamInfo {
    int index;
    int64_t start_time; /* ms */
    int64_t duration; /* ms */
    int64_t frames;

    const void* priv;
} mdkAudioStreamInfo;

MDK_API const mdkAudioCodecParameters* MDK_AudioStreamCodecParameters(const mdkAudioStreamInfo*);
/*
entry: in/out. can be null to return the 1st entry
return: next entry, or null if no more entry
*/
MDK_API mdkStringMapEntry MDK_AudioStreamMetadata(const mdkAudioStreamInfo*, mdkStringMapEntry* entry);

struct mdkVideoCodecParameters {
    const char* codec;
    uint32_t codec_tag;
    const uint8_t* extra_data; /* without padding data */
    int extra_data_size;
    int64_t bit_rate;
    int profile;
    int level;
    float frame_rate;
    int format;
    const char* format_name;

    int width;
    int height;
    int b_frames;

    int rotation;
};

struct mdkVideoStreamInfo {
    int index;
    int64_t start_time;
    int64_t duration;
    int64_t frames;
    
    const void* priv;
};

MDK_API void MDK_VideoStreamCodecParametersGet(const mdkVideoStreamInfo*, mdkVideoCodecParameters* p);
/*
entry: in/out. can be null to return the 1st entry
return: next entry, or null if no more entry
*/
MDK_API mdkStringMapEntry MDK_VideoStreamMetadata(const mdkVideoStreamInfo*, mdkStringMapEntry* entry);

typedef struct mdkMediaInfo
{
    int64_t start_time; // ms
    int64_t duration;
    int64_t bit_rate;
    int64_t size;
    const char* format;
    int streams;

    mdkAudioStreamInfo* audio;
    int nb_audio;
    mdkVideoStreamInfo* video;
    int nb_video;

    const void* priv;
} mdkMediaInfo;

/* see document of mdkStringMapEntry */
MDK_API bool MDK_MediaMetadata(const mdkMediaInfo*, mdkStringMapEntry* entry);

#ifdef __cplusplus
}
#endif