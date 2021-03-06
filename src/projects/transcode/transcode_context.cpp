//==============================================================================
//
//  Transcode
//
//  Created by Kwon Keuk Han
//  Copyright (c) 2018 AirenSoft. All rights reserved.
//
//==============================================================================

#include <iostream>
#include <unistd.h>

#include "transcode_context.h"

#define OV_LOG_TAG "TranscodeContext"


TranscodeContext::TranscodeContext() 
{
	SetAudioTimeBase(1, 1000000);
	SetVideoTimeBase(1, 1000000);
}

TranscodeContext::~TranscodeContext()
{
}

void TranscodeContext::SetVideoCodecId(MediaCodecId val)
{
	_video_codec_id = val;
}

MediaCodecId TranscodeContext::GetVideoCodecId()
{
	return _video_codec_id;
}

void TranscodeContext::SetAudioCodecId(MediaCodecId val)
{
	_audio_codec_id = val;
}

MediaCodecId TranscodeContext::GetAudioCodecId()
{
	return _audio_codec_id;
}

void TranscodeContext::SetVideoBitrate(int32_t val)
{
	_video_bitrate = val;
}

int32_t TranscodeContext::GetVideoBitrate()
{
	return _video_bitrate;
}

int32_t TranscodeContext::GetAudioBitrate()
{
	return _audio_bitrate;
}

void TranscodeContext::SetAudioBitrate(int32_t val)
{
	_audio_bitrate = val;
}

void TranscodeContext::SetAudioSampleRate(int32_t val)
{
	_audio_sample.SetRate((AudioSample::Rate)val);
	// _audio_sample_rate = val;
}

int32_t TranscodeContext::GetAudioSampleRate()
{
	return (int32_t)_audio_sample.GetRate();
}

void TranscodeContext::SetVideoWidth(uint32_t val)
{
	_video_width = val;
}

void TranscodeContext::SetVideoHeight(uint32_t val)
{
	_video_height = val;
}

uint32_t TranscodeContext::GetVideoWidth()
{
	return _video_width;
}

uint32_t TranscodeContext::GetVideoHeight()
{
	return _video_height;
}

void TranscodeContext::SetFrameRate(float val)
{
	_video_frame_rate = val;
}

float TranscodeContext::GetFrameRate()
{
	return _video_frame_rate;
}

Timebase& TranscodeContext::GetVideoTimeBase()
{
	return _video_time_base;
}

void TranscodeContext::SetVideoTimeBase(int32_t num, int32_t den)
{
	_video_time_base.Set(num, den);
}

Timebase& TranscodeContext::GetAudioTimeBase()
{
	return _audio_time_base;
}

void TranscodeContext::SetAudioTimeBase(int32_t num, int32_t den)
{
	_audio_time_base.Set(num, den);
}


void TranscodeContext::SetGOP(int32_t val)
{
	_video_gop = val;
}

int32_t TranscodeContext::GetGOP()
{
	return _video_gop;
}

void TranscodeContext::SetAudioSampleForamt(AudioSample::Format val)
{
	_audio_sample.SetFormat(val);
}

void TranscodeContext::SetAudioChannelLayout(AudioChannel::Layout val)
{
	_audio_channel.SetLayout(val);
}