/*
 * Copyright (c) 2019 WangBin <wbsecg1 at gmail.com>
 * This file is part of MDK
 * MDK SDK: https://sourceforge.net/projects/mdk-sdk/files
 * Free for GPL softwares or non-commercial use.
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 */
#pragma once
#include "global.h"
#include "../c/MediaInfo.h"
#include <cstring>
#include <unordered_map>
#include <vector>

MDK_NS_BEGIN

struct AudioCodecParameters {
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
};

struct AudioStreamInfo {
    int index;
    int64_t start_time; /* ms */
    int64_t duration; /* ms */
    int64_t frames;

    const void* priv = nullptr; // internal

    std::unordered_map<std::string, std::string> metadata;
    AudioCodecParameters codec;
};

struct VideoCodecParameters {
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
};

struct VideoStreamInfo {
    int index;
    int64_t start_time;
    int64_t duration;
    int64_t frames;
    int rotation;

    const void* priv = nullptr; // internal

    std::unordered_map<std::string, std::string> metadata;
    VideoCodecParameters codec;
};

struct ChapterInfo {
    int64_t start_time = 0;
    int64_t end_time = 0;
    std::string title;
};

struct MediaInfo
{
    int64_t start_time; // ms
    int64_t duration;
    int64_t bit_rate;
    int64_t size;
    const char* format;
    int streams;

    std::vector<ChapterInfo> chapters;
    std::unordered_map<std::string, std::string> metadata;
    std::vector<AudioStreamInfo> audio;
    std::vector<VideoStreamInfo> video;
};

// the following functions MUST be built into user's code because user's c++ stl abi is unknown
// used by Player.mediaInfo()
static void from_c(const mdkMediaInfo* cinfo, MediaInfo* info)
{
    *info = MediaInfo();
    if (!cinfo)
        return;
    info->start_time = cinfo->start_time;
    info->duration = cinfo->duration;
    info->bit_rate = cinfo->bit_rate;
    info->size = cinfo->size;
    info->format = cinfo->format;
    info->streams = cinfo->streams;

    mdkStringMapEntry entry{};
    while (MDK_MediaMetadata(cinfo, &entry))
        info->metadata[entry.key] = entry.value;
    for (int i = 0; i < cinfo->nb_chapters; ++i) {
        auto cci = &cinfo->chapters[i];
        std::string title;
        if (cci->title)
            title = cci->title;
        info->chapters.push_back({cci->start_time, cci->end_time, title});
    }
    for (int i = 0; i < cinfo->nb_audio; ++i) {
        AudioStreamInfo si{};
        auto csi = &cinfo->audio[i];
        memcpy(&si, csi, sizeof(*csi));
        MDK_AudioStreamCodecParameters(csi, (mdkAudioCodecParameters*)&si.codec);
        mdkStringMapEntry e{};
        while (MDK_AudioStreamMetadata(csi, &e))
            si.metadata[e.key] = e.value;
        info->audio.push_back(std::move(si));
    }
    for (int i = 0; i < cinfo->nb_video; ++i) {
        VideoStreamInfo si{};
        auto csi = &cinfo->video[i];
        memcpy(&si, csi, sizeof(*csi));
        MDK_VideoStreamCodecParameters(csi, (mdkVideoCodecParameters*)&si.codec);
        mdkStringMapEntry e{};
        while (MDK_VideoStreamMetadata(csi, &e))
            si.metadata[e.key] = e.value;
        info->video.push_back(std::move(si));
    }
}

MDK_NS_END