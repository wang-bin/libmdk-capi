/*
 * Copyright (c) 2019-2022 WangBin <wbsecg1 at gmail.com>
 */
#include "mdk/c/MediaInfo.h"
#include "mdk/MediaInfo.h"
#include "mdk/VideoFormat.h"
#include "MediaInfoInternal.h"
#include <cassert>

void from_abi(const AudioCodecParameters& in, mdkAudioCodecParameters& out)
{
    out.codec = in.codec.data();
    out.codec_tag = in.codec_tag;
    out.extra_data = in.extra.data();
    out.extra_data_size = (int)in.extra.size();
    out.bit_rate = in.bit_rate;
    out.profile = in.profile;
    out.level = in.level;
    out.frame_rate = in.frame_rate;

    out.is_float = IsFloat(in.format);
    out.is_unsigned = IsUnsigned(in.format);
    out.is_planar = IsPlanar(in.format);
    out.raw_sample_size = RawSampleSize(in.format);
    out.channels = in.channels;
    out.sample_rate = in.sample_rate;
    out.block_align = in.block_align;
    out.frame_size = in.frame_size;
}

void from_abi(const AudioStreamInfo& in, mdkAudioStreamInfo& out)
{
    out.index = in.index;
    out.start_time = in.start_time;
    out.duration = in.duration;
    out.frames = in.frames;
    out.priv = &in;
}

void from_abi(const VideoCodecParameters& in, mdkVideoCodecParameters& out)
{
    out.codec = in.codec.data();
    out.codec_tag = in.codec_tag;
    out.extra_data = in.extra.data();
    out.extra_data_size = (int)in.extra.size();
    out.bit_rate = in.bit_rate;
    out.profile = in.profile;
    out.level = in.level;
    out.frame_rate = in.frame_rate;

    out.format = (int)in.format;
    out.format_name = VideoFormat(in.format).name();
    out.width = in.width;
    out.height = in.height;
    out.b_frames = in.b_frames;
}

void from_abi(const VideoStreamInfo& in, mdkVideoStreamInfo& out)
{
    out.index = in.index;
    out.start_time = in.start_time;
    out.duration = in.duration;
    out.frames = in.frames;
    out.rotation = in.rotation;
    out.priv = &in;
}

void from_abi(const ChapterInfo& in, mdkChapterInfo& out)
{
    out.start_time = in.start_time;
    out.end_time = in.end_time;
    if (in.title.empty())
        out.title = nullptr;
    else
        out.title = in.title.data();
    out.priv = &in;
}

void from_abi(const MediaInfo& in, mdkMediaInfo& out)
{
    out.start_time = in.start_time;
    out.duration = in.duration;
    out.bit_rate = in.bit_rate;
    out.format = in.format.data();
    out.streams = (int)in.streams;
    out.nb_chapters = (int)in.chapters.size();
    out.nb_audio = (int)in.audio.size();
    out.nb_video = (int)in.video.size();

    out.priv = &in;
}

void MediaInfoToC(const MediaInfo& abi, MediaInfoInternal* out)
{
    if (!out)
        return;
    *out = MediaInfoInternal{};
    out->abi = abi;
    for (const auto& i : out->abi.chapters) {
        mdkChapterInfo ci;
        from_abi(i, ci);
        out->c.push_back(std::move(ci));
    }
    out->info.chapters = nullptr;
    if (!out->c.empty())
        out->info.chapters = &out->c[0];
    for (const auto& i : out->abi.audio) {
        mdkAudioStreamInfo si;
        from_abi(i, si);
        out->a.push_back(std::move(si));
    }
    if (!out->a.empty())
        out->info.audio = &out->a[0];
    for (const auto& i : out->abi.video) {
        mdkVideoStreamInfo si;
        from_abi(i, si);
        out->v.push_back(std::move(si));
    }
    if (!out->v.empty())
        out->info.video = &out->v[0];
    from_abi(out->abi, out->info);
}

template<class InfoAbi, class Info>
bool MDK_GetMetaData(const Info* info, mdkStringMapEntry* entry)
{
    if (!info)
        return false;
    assert(entry && "entry can not be null");
    auto abi = reinterpret_cast<const InfoAbi*>(info->priv);
    auto it = abi->metadata.cend();
    if (entry->priv) {
        auto pit = (decltype(it)*)entry->priv;
        it = *pit;
        it++;
        delete pit;
    } else if (entry->key) {
        it = abi->metadata.find(entry->key);
    } else {
        it = abi->metadata.cbegin();
    }
    if (it == abi->metadata.cend())
        return false;
    entry->key = it->first.data();
    entry->value = it->second.data();
    entry->priv = new decltype(it)(it);
    return true;
}

extern "C" {

void MDK_AudioStreamCodecParameters(const mdkAudioStreamInfo* info, mdkAudioCodecParameters* p)
{
    auto abi = reinterpret_cast<const AudioStreamInfo*>(info->priv);
    from_abi(abi->codec, *p);
}

void MDK_VideoStreamCodecParameters(const mdkVideoStreamInfo* info, mdkVideoCodecParameters* p)
{
    auto abi = reinterpret_cast<const VideoStreamInfo*>(info->priv);
    from_abi(abi->codec, *p);
}

bool MDK_AudioStreamMetadata(const mdkAudioStreamInfo* info, mdkStringMapEntry* entry)
{
    return MDK_GetMetaData<AudioStreamInfo>(info, entry);
}

bool MDK_VideoStreamMetadata(const mdkVideoStreamInfo* info, mdkStringMapEntry* entry)
{
    return MDK_GetMetaData<VideoStreamInfo>(info, entry);
}

bool MDK_MediaMetadata(const mdkMediaInfo* info, mdkStringMapEntry* entry)
{
    return MDK_GetMetaData<MediaInfo>(info, entry);
}

} // extern "C"