//==============================================================================
//
//  MediaRouteStream
//
//  Created by Kwon Keuk Hanb
//  Copyright (c) 2018 AirenSoft. All rights reserved.
//
//==============================================================================

#pragma once


#include <stdint.h>
#include <memory>
#include <vector>
#include <queue>

#include "base/media_route/media_buffer.h"
#include "base/media_route/media_type.h"
#include "base/application/stream_info.h"

#include "bitstream/bitstream_to_annexb.h"
#include "bitstream/bitstream_to_adts.h"
#include "bitstream/bitstream_to_annexa.h"

class MediaRouteStream
{
public:
    MediaRouteStream(std::shared_ptr<StreamInfo> stream_info);
    ~MediaRouteStream();

public:
    // 우너본 스트림 정보 조회
    std::shared_ptr<StreamInfo> GetStreamInfo();

    void SetConnectorType(int32_t type);
    int32_t GetConnectorType();

private:
    std::shared_ptr<StreamInfo> _stream_info;

    int32_t _application_connector_type;



public:
    // 패킷 관리
    bool Push(std::unique_ptr<MediaBuffer> buffer);
    std::unique_ptr<MediaBuffer> Pop();
    uint32_t Size();

    time_t  getLastReceivedTime();
private:
    std::queue<std::unique_ptr<MediaBuffer>> _queue;

private:
    ////////////////////////////
    // 비트 스트림 필터
    ////////////////////////////
	BitstreamToAnnexB	_bsfv;
	BitstreamToADTS		_bsfa;
    BitstreamAnnexA     _bsf_vp8;

    // 마지막으로 받은 패킷의 시간
    time_t              _last_rb_time;
};

