//==============================================================================
//
//  OvenMediaEngine
//
//  Copyright (c) 2018 AirenSoft. All rights reserved.
//
//==============================================================================
#pragma once
#include "base/common_types.h"
#include "common_attr.h"
#include "media_description.h"

// OvenMediaEngine 스펙만 SDP로 나타낸다. 모든 SDP를 지원하지 않아도 문제 되지 않는 이유는
// OvenMediaEngine이 무조건 OFFER를 보내는 Peer이기 때문에 Remote Peer가 OME의 SDP에 따라 동작하게
// 되기 때문이다.

/*
 [Minimal SDP]

	"v=0\r\n"
	"o=OvenMediaEngine 1882243660 2 IN IP4 127.0.0.1\r\n"
	"s=-\r\n"
	"t=0 0\r\n"
	"c=IN IP4 0.0.0.0\r\n"
	"a=group:BUNDLE video\r\n"

	"a=fingerprint:sha-256 D7:81:CF:01:46:FB:2D:93:8E:04:AF:47:76:0A:88:08:FF:73:37:C6:A7:45:0B:31:FE:12:49:DE:A7:E4:1F:3A\r\n"
	"a=ice-options:trickle\r\n"
	"a=ice-pwd:c32d4070c67e9782bea90a9ab46ea838\r\n"
	"a=ice-ufrag:0dfa46c9\r\n"

	"a=msid-semantic:WMS *\r\n"

	"m=video 9 UDP/TLS/RTP/SAVPF 97\r\n"
	"a=rtpmap:97 VP8/50000\r\n"
	"a=framesize:97 426-240\r\n"

	"a=mid:video\r\n"
	"a=rtcp-mux\r\n"
	"a=setup:actpass\r\n"
	"a=sendonly\r\n"

	"a=ssrc:2064629418 cname:{b2266c86-259f-4853-8662-ea94cf0835a3}\r\n";
 */

class SessionDescription : public SdpBase,
						   public CommonAttr,
						   public ov::EnableSharedFromThis<SessionDescription>
{
public:
	SessionDescription();
	~SessionDescription();

	bool			FromString(const ov::String &sdp) override ;

	// v=0
	void 			SetVersion(uint8_t version);
	uint8_t 		GetVersion();

	// o=OvenMediaEngine 1882243660 2 IN IP4 127.0.0.1
	void 			SetOrigin(ov::String user_name, uint32_t session_id, uint32_t session_version,
							  ov::String net_type, uint8_t ip_version, ov::String address);
	ov::String		GetUserName();
	uint32_t 		GetSessionId();
	uint32_t 		GetSessionVersion();
	ov::String		GetNetType();
	uint8_t 		GetIpVersion();
	ov::String		GetAddress();

	// s=-
	void 			SetSessionName(ov::String session_name);
	ov::String		GetSessionName();

	// t=0 0
	void 			SetTiming(uint32_t start, uint32_t stop);
	uint32_t 		GetStartTime();
	uint32_t 		GetStopTime();

	// a=msid-semantic:WMS *
	void 			SetMsidSemantic(const ov::String& semantic, const ov::String& token);
	ov::String		GetMsidSemantic() const;
	ov::String		GetMsidToken() const;

	// m=video 9 UDP/TLS/RTP/SAVPF 97
	// a=group:BUNDLE 에 AddMedia의 mid를 추가한다. OME는 BUNDLE-ONLY만 지원한다. (2018.05.01)
	void 			AddMedia(std::shared_ptr<MediaDescription> media);
	const std::shared_ptr<MediaDescription> GetFirstMedia();
	const std::shared_ptr<MediaDescription> GetMediaByMid(const ov::String& mid);
	const std::vector<std::shared_ptr<MediaDescription>>& GetMediaList();

	// Commin attr
	ov::String		GetFingerprintAlgorithm() override;
	ov::String		GetFingerprintValue() override;
	ov::String		GetIceOption() override;
	ov::String		GetIceUfrag() override;
	ov::String		GetIcePwd() override;

	bool operator ==(const SessionDescription &description) const
	{
		// TODO(getroot): this와 description이 같은지를 판단할 수 있게 해주세요
		return (_session_id == description._session_id);
	}

private:
	bool 			UpdateData(ov::String &sdp) override;
	bool			ParsingSessionLine(char type, std::string content);
	// version
	uint8_t 		_version;
	// origin
	ov::String		_user_name;
	uint32_t 		_session_id;
	uint32_t 		_session_version;
	ov::String		_net_type;
	uint8_t 		_ip_version;
	ov::String		_address;
	// session
	ov::String		_session_name;
	// timing
	uint32_t 		_start_time;
	uint32_t 		_stop_time;
	// msid-semantic
	ov::String		_msid_semantic;
	ov::String		_msid_token;
	// group:Bundle
	std::vector<ov::String>	_bundles;
	// Media
	std::vector<std::shared_ptr<MediaDescription>>	_media_list;
};