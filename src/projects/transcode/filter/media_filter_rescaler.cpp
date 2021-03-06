//==============================================================================
//
//  Transcode
//
//  Created by Kwon Keuk Han
//  Copyright (c) 2018 AirenSoft. All rights reserved.
//
//==============================================================================

#include "media_filter_rescaler.h"
#include <base/ovlibrary/ovlibrary.h>

#define OV_LOG_TAG "MediaFilter"

#define DEBUG_RESCALER 	0

MediaFilterRescaler::MediaFilterRescaler() : 
	_frame(nullptr),
	// _frame_out(nullptr),
	_buffersink_ctx(nullptr),
	_buffersrc_ctx(nullptr),
	_filter_graph(nullptr),
	_outputs(nullptr),
	_inputs(nullptr)
{
	avfilter_register_all();
	
	_frame = av_frame_alloc();

	// _frame_out = av_frame_alloc();

	_outputs = avfilter_inout_alloc();

	_inputs  = avfilter_inout_alloc();	


#if DEBUG_PREVIEW_ENABLE && DEBUG_RESCALER
	cv::namedWindow( "Decoded", WINDOW_AUTOSIZE );
	cv::namedWindow( "Rescaled", WINDOW_AUTOSIZE );
#endif
}

MediaFilterRescaler::~MediaFilterRescaler()
{
	if(_frame)
	{
		av_frame_free(&_frame); 
	}

	// if(_frame_out)
	// {
	// 	av_frame_free(&_frame_out); 
	// }

	if(_outputs)
	{
		avfilter_inout_free(&_outputs);
	}

	if(_inputs)
	{
		avfilter_inout_free(&_inputs);
	}
}


int32_t MediaFilterRescaler::Configure(std::shared_ptr<MediaTrack> input_media_track, std::shared_ptr<TranscodeContext> context)
{
	int ret;
	const AVFilter *buffersrc  = avfilter_get_by_name("buffer");
	const AVFilter *buffersink = avfilter_get_by_name("buffersink");

	_filter_graph = avfilter_graph_alloc();

	if (!_outputs || !_inputs || !_filter_graph) {
		logte("cannot allocated filter graph");
		return 1;
	}

	// 입력 트랙의 정보를 설정함
	ov::String input_formats = ov::String::FormatString("video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d"
		,input_media_track->GetWidth()
		,input_media_track->GetHeight()
		,AV_PIX_FMT_YUV420P
		,input_media_track->GetTimeBase().GetNum()			// Timebase 1 
		,input_media_track->GetTimeBase().GetDen()			// Timebase 1000
		,1													// PAR 
		,1													// PAR
	);	

	ret = avfilter_graph_create_filter(&_buffersrc_ctx, buffersrc, "in", input_formats.CStr(), NULL, _filter_graph);
	if (ret < 0) 
	{
		logte( "Cannot create buffer source\n");
		return 1;																									
	}

	// TODO: Timebase의 값을 설정이 가능하도록 할지, 기본값으로 고정할지 정해야함.
	ov::String output_filter_descr = ov::String::FormatString("fps=fps=%.2f:round=near,scale=%dx%d:flags=bicubic,settb=expr=%f"
		,context->GetFrameRate()
		,context->GetVideoWidth()
		,context->GetVideoHeight()
		,(float)context->GetVideoTimeBase().GetExpr()			
	);

	logtd("rescale track[%u] %s -> %s", input_media_track->GetId(), input_formats.CStr(), output_filter_descr.CStr());

	enum AVPixelFormat pix_fmts[] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE };

	ret = avfilter_graph_create_filter(&_buffersink_ctx, buffersink, "out", NULL, NULL, _filter_graph);
	if (ret < 0) 
	{
		logte( "Cannot create video buffer sink\n");
		return 1;
	}

    ret = av_opt_set_int_list(_buffersink_ctx, "pix_fmts", pix_fmts, AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) 
    {
        logte("Cannot set output pixel format\n");
        return 1;
    }

	// 필터 연결
	_outputs->name       = av_strdup("in");
	_outputs->filter_ctx = _buffersrc_ctx;
	_outputs->pad_idx    = 0;
	_outputs->next       = NULL;


	// 버퍼 싱크의 임력 설정
	_inputs->name       = av_strdup("out");
	_inputs->filter_ctx = _buffersink_ctx;
	_inputs->pad_idx    = 0;
	_inputs->next       = NULL;


	if ((ret = avfilter_graph_parse_ptr(_filter_graph, output_filter_descr.CStr(), &_inputs, &_outputs, NULL)) < 0)
	{
		logte( "Cannot create filter graph\n");
		return 1;
	}

	if ((ret = avfilter_graph_config(_filter_graph, NULL)) < 0)
	{
		logte( "Cannot validation filter graph\n");
		return 1;
	}

#if 0
	// 출력 프레임 설정
	_frame_out->format = AV_PIX_FMT_YUV420P;
	_frame_out->width  = context->GetVideoWidth();
	_frame_out->height = context->GetVideoHeight();;

	// logti("input frame pts : %f", (float)_frame->pts);
	if(av_frame_get_buffer(_frame_out, 32) < 0)
	{
		logte("Could not allocate the video frame data\n");
		return 1;
	}
	
	if(av_frame_make_writable(_frame_out) < 0)
	{
		logte("Could not make sure the frame data is writable\n");
		return 1;
	}
#endif
	logtd("media rescaler filter created.");

	return 0;
}

int32_t MediaFilterRescaler::SendBuffer(std::unique_ptr<MediaBuffer> buf)
{
	
	_pkt_buf.push_back(std::move(buf));

	return 0;
}

std::pair<int32_t, std::unique_ptr<MediaBuffer>> MediaFilterRescaler::RecvBuffer()
{
	// 출력될 프레임이 있는지 확인함
	int ret = av_buffersink_get_frame(_buffersink_ctx, _frame);
	if(ret == AVERROR(EAGAIN))
	{
	// printf("eagain : %d\r\n", ret);
	}
	else if(ret == AVERROR_EOF)
	{
		logte("eof : %d\r\n", ret);
		return std::make_pair(-1, nullptr);
	}
	else if(ret < 0)
	{
		logte("error : %d\r\n", ret);
		return std::make_pair(-1, nullptr);
	}
	else
	{
		auto out_buf = std::make_unique<MediaBuffer>();
		out_buf->_width = _frame->width;
		out_buf->_height = _frame->height;
		out_buf->_format = _frame->format;

		// logte("============ %d %d %d / %d %d %d", _frame_out->width, _frame_out->height, _frame_out->_format, _frame_out->linesize[0], _frame_out->linesize[1], _frame_out->linesize[2]);
		out_buf->SetPts((_frame->pts==AV_NOPTS_VALUE)?-1.0f:_frame->pts);
		
		out_buf->SetStride(_frame->linesize[0], 0);
		out_buf->SetStride(_frame->linesize[1], 1);
		out_buf->SetStride(_frame->linesize[2], 2);
		
		out_buf->SetBuffer(_frame->data[0], out_buf->GetStride(0) * out_buf->_height, 0);		// Y-Plane
		out_buf->SetBuffer(_frame->data[1], out_buf->GetStride(1) * out_buf->_height/2, 1); 	// Cb Plane
		out_buf->SetBuffer(_frame->data[2], out_buf->GetStride(2) * out_buf->_height/2, 2);		// Cr Plane

		av_frame_unref(_frame);

		// TODO: Got Frame!
		return std::make_pair(0, std::move(out_buf));
	}


	while (_pkt_buf.size() > 0) 
	{
		MediaBuffer* cur_pkt = _pkt_buf[0].get();

		_frame->format = cur_pkt->_format;
		_frame->width  = cur_pkt->_width;
		_frame->height = cur_pkt->_height;
		_frame->pts    = cur_pkt->GetPts();
		
		_frame->linesize[0] = cur_pkt->GetStride(0);
		_frame->linesize[1] = cur_pkt->GetStride(1);
		_frame->linesize[2] = cur_pkt->GetStride(2);

		// logti("input frame pts : %f", (float)_frame->pts);
		if(av_frame_get_buffer(_frame, 32) < 0)
		{
			logte("Could not allocate the video frame data\n");
			return std::make_pair(-1, nullptr);
		}

		if(av_frame_make_writable(_frame) < 0)
		{
			logte("Could not make sure the frame data is writable\n");
			return std::make_pair(-1, nullptr);
		}

		// 데이터를 복사함.
		memcpy(_frame->data[0], cur_pkt->GetBuffer(0), cur_pkt->GetBufferSize(0));
		memcpy(_frame->data[1], cur_pkt->GetBuffer(1), cur_pkt->GetBufferSize(1));
		memcpy(_frame->data[2], cur_pkt->GetBuffer(2), cur_pkt->GetBufferSize(2));

#if DEBUG_PREVIEW_ENABLE && DEBUG_RESCALER	// DEBUG for OpenCV
			Mat *display_frame = nullptr;

			AVUtilities::FrameConvert::AVFrameToCvMat(_frame, &display_frame, _frame->width);

			cv::imshow("Decoded", *display_frame );  
			waitKey(1);   
			delete display_frame;
#endif

		if(av_buffersrc_add_frame_flags(_buffersrc_ctx, _frame, AV_BUFFERSRC_FLAG_KEEP_REF) < 0) 
		{
			logte( "Error while feeding the audio filtergraph. frame.format(%d), buffer.pts(%.0f) buffer.linesize(%d), buf.size(%d)\n"
				, _frame->format
				, (double)_frame->pts
				, _frame->linesize[0]
				, _pkt_buf.size());			
		}

		// 처리가 완료된 패킷은 큐에서 삭제함.
		_pkt_buf.erase(_pkt_buf.begin(), _pkt_buf.begin()+1);

		av_frame_unref(_frame);
	}

	return std::make_pair(-1, nullptr);
}