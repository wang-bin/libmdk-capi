/*
 * Copyright (c) 2019-2025 WangBin <wbsecg1 at gmail.com>
 */
#include "mdk/c/MediaInfo.h"
#include "mdk/MediaInfo.h"
#include "mdk/VideoFormat.h"
#include "MediaInfoInternal.h"
#include <cassert>
#include <algorithm>

ColorSpace kColorSpaceMap[] = {
    ColorSpaceUnknown,
    ColorSpaceBT709,
    ColorSpaceBT2100_PQ,
    ColorSpaceSCRGB,
    ColorSpaceExtendedLinearDisplayP3,
    ColorSpaceExtendedSRGB,
    ColorSpaceExtendedLinearSRGB,
    ColorSpaceBT2100_HLG,
};

static MDK_ColorSpace toC(const ColorSpace& cs)
{
    const auto dist = [&](const ColorSpace& cs1, const ColorSpace& cs2) {
        if (cs1 == cs2)
            return 0;
        if ((cs1.range == ColorSpace::Range::Extended || cs2.range == ColorSpace::Range::Extended) && cs1.range != cs2.range)
            return 1000000;
        if (cs1 == ColorSpaceUnknown || cs2 == ColorSpaceUnknown)
            return 99;
        return (cs2.primaries != cs1.primaries) * 100 + (cs2.transfer != cs1.transfer);
    };
    const auto it = std::min_element(std::begin(kColorSpaceMap), std::end(kColorSpaceMap), [&](const ColorSpace& cs1, const ColorSpace& cs2) {
        return dist(cs1, cs) < dist(cs2, cs);
    });
    return MDK_ColorSpace(std::distance(std::begin(kColorSpaceMap), it));
}

static void from_abi(const AudioCodecParameters& in, mdkAudioCodecParameters& out)
{
    out.codec = in.codec.data();
    out.codec_tag = in.codec_tag;
    if (in.extra) {
        out.extra_data = in.extra->constData();
        out.extra_data_size = (int)in.extra->size();
    }
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

static void from_abi(const AudioStreamInfo& in, mdkAudioStreamInfo& out)
{
    out.index = in.index;
    out.start_time = in.start_time;
    out.duration = in.duration;
    out.frames = in.frames;
    out.priv = &in;
}

static void from_abi(const VideoCodecParameters& in, mdkVideoCodecParameters& out)
{
    out.codec = in.codec.data();
    out.codec_tag = in.codec_tag;
    if (in.extra) {
        out.extra_data = in.extra->constData();
        out.extra_data_size = (int)in.extra->size();
    }
    out.bit_rate = in.bit_rate;
    out.profile = in.profile;
    out.level = in.level;
    out.frame_rate = in.frame_rate;

    out.format = (int)in.format;
    out.format_name = VideoFormat(in.format).name();
    out.width = in.width;
    out.height = in.height;
    out.b_frames = in.b_frames;
    out.par = in.par;
    out.color_space = toC(in.color_space);
}

static void from_abi(const VideoStreamInfo& in, mdkVideoStreamInfo& out)
{
    out.index = in.index;
    out.start_time = in.start_time;
    out.duration = in.duration;
    out.frames = in.frames;
    out.rotation = in.rotation;
    out.priv = &in;
}

static void from_abi(const SubtitleCodecParameters& in, mdkSubtitleCodecParameters& out)
{
    out.codec = in.codec.data();
    out.codec_tag = in.codec_tag;
    if (in.extra) {
        out.extra_data = in.extra->constData();
        out.extra_data_size = (int)in.extra->size();
    }
    out.width = in.width;
    out.height = in.height;
}

static void from_abi(const SubtitleStreamInfo& in, mdkSubtitleStreamInfo& out)
{
    out.index = in.index;
    out.start_time = in.start_time;
    out.duration = in.duration;
    out.priv = &in;
}

static void from_abi(const ChapterInfo& in, mdkChapterInfo& out)
{
    out.start_time = in.start_time;
    out.end_time = in.end_time;
    if (in.title.empty())
        out.title = nullptr;
    else
        out.title = in.title.data();
    out.priv = &in;
}

static void from_abi(const ProgramInfo& in, mdkProgramInfo& out)
{
    out.id = in.id;
    out.stream = in.stream.data();
    out.nb_stream = (int)in.stream.size();
    out.priv = &in;
}

static void from_abi(const MediaInfo& in, mdkMediaInfo& out)
{
    out.start_time = in.start_time;
    out.duration = in.duration;
    out.bit_rate = in.bit_rate;
    out.format = in.format.data();
    out.streams = (int)in.streams;
    out.nb_chapters = (int)in.chapters.size();
    out.nb_audio = (int)in.audio.size();
    out.nb_video = (int)in.video.size();
    out.nb_subtitle = (int)in.subtitle.size();
    out.nb_programs = (int)in.program.size();

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
        out->c.push_back(ci);
    }
    out->info.chapters = nullptr;
    if (!out->c.empty())
        out->info.chapters = &out->c[0];

    for (const auto& i : out->abi.program) {
        mdkProgramInfo pi;
        from_abi(i, pi);
        out->p.push_back(pi);
    }
    out->info.programs = nullptr;
    if (!out->p.empty())
        out->info.programs = &out->p[0];

    for (const auto& i : out->abi.audio) {
        mdkAudioStreamInfo si;
        from_abi(i, si);
        out->a.push_back(si);
    }
    if (!out->a.empty())
        out->info.audio = &out->a[0];

    for (const auto& i : out->abi.video) {
        mdkVideoStreamInfo si;
        from_abi(i, si);
        out->v.push_back(si);
    }
    if (!out->v.empty())
        out->info.video = &out->v[0];

    for (const auto& i : out->abi.subtitle) {
        mdkSubtitleStreamInfo si;
        from_abi(i, si);
        out->s.push_back(si);
    }
    if (!out->s.empty())
        out->info.subtitle = &out->s[0];

    from_abi(out->abi, out->info);
}

template<class InfoAbi, class Info>
static bool MDK_GetMetaData(const Info* info, mdkStringMapEntry* entry)
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
    p->dovi_profile = abi->dovi.profile;
}

void MDK_SubtitleStreamCodecParameters(const mdkSubtitleStreamInfo* info, mdkSubtitleCodecParameters* p)
{
    auto abi = reinterpret_cast<const SubtitleStreamInfo*>(info->priv);
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

bool MDK_SubtitleStreamMetadata(const mdkSubtitleStreamInfo* info, mdkStringMapEntry* entry)
{
    return MDK_GetMetaData<SubtitleStreamInfo>(info, entry);
}

bool MDK_ProgramMetadata(const mdkProgramInfo* info, mdkStringMapEntry* entry)
{
    return MDK_GetMetaData<ProgramInfo>(info, entry);
}

const uint8_t* MDK_VideoStreamData(const mdkVideoStreamInfo* info, int* len, int flags)
{
    if (flags == 0) {
        if (auto i = reinterpret_cast<const VideoStreamInfo*>(info->priv)->image) {
            if (len)
                *len = (int)i->size();
            return i->data();
        }
    }
    return nullptr;
}
} // extern "C"