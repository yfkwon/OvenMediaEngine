//==============================================================================
//
//  MediaRouteStream
//
//  Created by Kwon Keuk Hanb
//  Copyright (c) 2018 AirenSoft. All rights reserved.
//
//==============================================================================


#include "media_route_stream.h"
#include <base/ovlibrary/ovlibrary.h>

#define OV_LOG_TAG "MediaRouteStream"

MediaRouteStream::MediaRouteStream(std::shared_ptr<StreamInfo> stream_info) 
{
	logtd("create media route stream. name(%s) id(%u)"
		, stream_info->GetName().CStr(), stream_info->GetId());

	// 스트림 정보 저정
	_stream_info = stream_info;
	_stream_info->ShowInfo();
}

MediaRouteStream::~MediaRouteStream()
{
	logtd("Delete media route stream name(%s) id(%u)"
		, _stream_info->GetName().CStr(), _stream_info->GetId());
}

std::shared_ptr<StreamInfo> MediaRouteStream::GetStreamInfo()
{
	return _stream_info;
}

void MediaRouteStream::SetConnectorType(int32_t type)
{
	_application_connector_type = type;
}

int32_t MediaRouteStream::GetConnectorType()
{
	return _application_connector_type;
}

bool MediaRouteStream::Push(std::unique_ptr<MediaBuffer> buffer)
{
	MediaType media_type = buffer->GetMediaType();
	uint32_t track_id = buffer->GetTrackId();
	auto 	 media_track = _stream_info->GetTrack(track_id);

	if(media_track == nullptr)
	{
		logte("can not find media track. type(%s), id(%d)"
			, (media_type == MediaType::MEDIA_TYPE_VIDEO)?"video":"audio", track_id);
		return false;
	}

	if(media_type == MediaType::MEDIA_TYPE_VIDEO && media_track->GetCodecId() == MediaCodecId::CODEC_ID_H264)
	{
		_bsfv.convert_to(buffer.get());
	}
	else if(media_type == MediaType::MEDIA_TYPE_VIDEO && media_track->GetCodecId() == MediaCodecId::CODEC_ID_VP8)
	{
		_bsf_vp8.convert_to(buffer.get());
	}
	else if(media_type == MediaType::MEDIA_TYPE_AUDIO && media_track->GetCodecId() == MediaCodecId::CODEC_ID_AAC)
	{
		_bsfa.convert_to(buffer.get());
	}
	else if(media_type == MediaType::MEDIA_TYPE_AUDIO && media_track->GetCodecId() == MediaCodecId::CODEC_ID_OPUS)
	{
		// logtd("%s", ov::Dump(buffer->GetBuffer(), buffer->GetDataSize()).CStr());
		// _bsfa.convert_to(buffer.GetBuffer());
	}

#if 0
	buffer->SetPts(media_track->GetStartFrameTime() + buffer->GetPts());
	
	// 수신된 버퍼의 PTS값이 초기화되면 마지막에 받은 PTS값을 기준으로 증가시켜준다
	if( (media_track->GetLastFrameTime()-1000000) > buffer->GetPts())
	{
		logtw("change start time");
		logtd("start buffer : %.0f, last buffer : %.0f, buffer : %.0f", (float)media_track->GetStartFrameTime(), (float)media_track->GetLastFrameTime(), (float)buffer->GetPts());

		media_track->SetStartFrameTime(media_track->GetLastFrameTime());
	}

	media_track->SetLastFrameTime(buffer->GetPts());
#endif


	// 변경된 스트림을 큐에 넣음
	_queue.push(std::move(buffer));

	time(&_last_rb_time);
	// logtd("last time : %s", asctime(gmtime(&_last_rb_time)) );
	return true;
}

std::unique_ptr<MediaBuffer> MediaRouteStream::Pop()
{
	if(_queue.empty())
	{
		return nullptr;
	}

	auto p2 = std::move(_queue.front());
	_queue.pop();
	
	return p2;
}

uint32_t MediaRouteStream::Size()
{
	return _queue.size();	
}


time_t MediaRouteStream::getLastReceivedTime()
{
	return _last_rb_time;
}