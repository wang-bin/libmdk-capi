/*
 * Copyright (c) 2019-2023 WangBin <wbsecg1 at gmail.com>
 */
#pragma once
#include "mdk/c/MediaInfo.h"
#include "mdk/MediaInfo.h"

using namespace std;
using namespace MDK_NS;

struct MediaInfoInternal {
    MediaInfo abi;
    mdkMediaInfo info;
    vector<mdkChapterInfo> c;
    vector<mdkAudioStreamInfo> a;
    vector<mdkVideoStreamInfo> v;
    vector<mdkSubtitleStreamInfo> s;
    vector<mdkProgramInfo> p;
};

void MediaInfoToC(const MediaInfo& abi, MediaInfoInternal* out);
