//==============================================================================
//
//  OvenMediaEngine
//
//  Created by getroot
//  Copyright (c) 2018 AirenSoft. All rights reserved.
//
//
//==============================================================================

#pragma once

#include "sdp_base.h"
#include "common_attr.h"
#include "payload_attr.h"

class SessionDescription;

class MediaDescription : public SdpBase, public CommonAttr
{
public:

	enum class MediaType
	{
		UNKNOWN,
		VIDEO,
		AUDIO,
		APPLICATION
	};

	enum class Direction
	{
		UNKNOWN,
		SENDRECV,
		RECVONLY,
		SENDONLY,
		INACTIVE
	};

	enum class SetupType
	{
		UNKNOWN,
		ACTIVE,
		PASSIVE,
		ACTPASS
	};

	MediaDescription(const std::shared_ptr<SessionDescription>& session_description);
	virtual ~MediaDescription();

	bool				FromString(const ov::String& desc) override;

	// m=video 9 UDP/TLS/RTP/SAVPF 97
	void 				SetMediaType(const MediaType type);
	bool				SetMediaType(const ov::String &type);
	const MediaType 	GetMediaType();
	void 				SetPort(const uint16_t port);
	const uint16_t 		GetPort();
	void 				UseDtls(const bool flag);
	const bool			IsUseDtls();

	void 				AddPayload(const std::shared_ptr<PayloadAttr>& payload);
	const std::shared_ptr<PayloadAttr> GetPayload(uint8_t id);
	const std::shared_ptr<PayloadAttr> GetFirstPayload();

	// a=rtcp-mux
	void 				UseRtcpMux(const bool flag = true);
	const bool			IsUseRtcpMux();

	// a=sendonly
	void 				SetDirection(const Direction dir);
	bool				SetDirection(const ov::String &dir);
	const Direction 	GetDirection();

	// a=mid:video
	void 				SetMid(const ov::String& mid);
	const ov::String	GetMid();

	// a=setup:actpass
	void 				SetSetup(const SetupType type);
	bool 				SetSetup(const ov::String& type);

	// c=IN IP4 0.0.0.0
	void 				SetConnection(const uint8_t version, const ov::String& ip);

	// a=framerate:29.97
	void 				SetFramerate(const float framerate);
	const float 		GetFramerate();

	// a=rtpmap:96 VP8/50000
	bool 				AddRtpmap(const uint8_t payload, const ov::String &codec, const uint32_t rate,
								  const ov::String parameters);
	void 				AddRtpmap(const uint8_t payload, const PayloadAttr::SupportCodec codec, const uint32_t rate,
								  const ov::String parameters);
	// a=rtcp-fb:96 nack pli
	bool 				EnableRtcpFb(const uint8_t id, const ov::String &type, const bool on);
	void 				EnableRtcpFb(const uint8_t id, const PayloadAttr::RtcpFbType &type, const bool on);

	// a=ssrc:2064629418 cname:{b2266c86-259f-4853-8662-ea94cf0835a3}
	void 				SetCname(const uint32_t ssrc, const ov::String& cname);

	const uint32_t 		GetSsrc();
	const ov::String	GetCname();

private:
	bool 				UpdateData(ov::String &sdp) override ;
	bool 				ParsingMediaLine(char type, std::string content);

	MediaType 		_media_type;
	ov::String		_media_type_str;

	uint16_t 		_port;
	bool			_use_dtls_flag;
	ov::String		_protocol;

	bool			_use_rtcpmux_flag;

	Direction 		_direction;
	ov::String		_direction_str;

	ov::String		_mid;

	SetupType 		_setup;
	ov::String		_setup_str;

	uint8_t 		_connection_ip_version;
	ov::String		_connection_ip;

	float 			_framerate;

	uint32_t 		_ssrc;
	ov::String		_cname;


	std::shared_ptr<SessionDescription>			_session_description;
	std::vector<std::shared_ptr<PayloadAttr>>	_payload_list;
};
