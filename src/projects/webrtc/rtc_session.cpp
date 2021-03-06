#include "rtc_session.h"
#include "rtc_application.h"

std::shared_ptr<RtcSession> RtcSession::Create(std::shared_ptr<Application> application,
											   std::shared_ptr<Stream> stream,
											   std::shared_ptr<SessionDescription> offer_sdp,
											   std::shared_ptr<SessionDescription> peer_sdp,
											   std::shared_ptr<IcePort> ice_port)
{
	auto session = std::make_shared<RtcSession>(application, stream, offer_sdp, peer_sdp, ice_port);
	if(!session->Start())
	{
		return nullptr;
	}
	return session;
}

RtcSession::RtcSession(std::shared_ptr<Application> application,
					   std::shared_ptr<Stream> stream,
					   std::shared_ptr<SessionDescription> offer_sdp,
                       std::shared_ptr<SessionDescription> peer_sdp,
                       std::shared_ptr<IcePort> ice_port)
	: Session(application, stream)
{
	_offer_sdp = offer_sdp;
	_peer_sdp = peer_sdp;
	_ice_port = ice_port;
}

RtcSession::~RtcSession()
{
	logd("WEBRTC", "RtcSession(%d) has been terminated finally", GetId());

	_rtp_rtcp_map.clear();
}

std::shared_ptr<RtcStream> RtcSession::GetRtcStream()
{
	return std::static_pointer_cast<RtcStream>(GetStream());
}

bool RtcSession::Start()
{
	auto session = std::static_pointer_cast<Session>(GetSharedPtr());

	// SRTP 생성
	_srtp_transport = std::make_shared<SrtpTransport>(SRTP, session);

	// DTLS 생성
	_dtls_transport = std::make_shared<DtlsTransport>(DTLS, session);
	std::shared_ptr<RtcApplication> application = std::static_pointer_cast<RtcApplication>(GetApplication());
	_dtls_transport->SetLocalCertificate(application->GetCertificate());
	_dtls_transport->StartDTLS();

	// ICE-DTLS 생성
	_dtls_ice_transport = std::make_shared<DtlsIceTransport>(ICE, session, _ice_port);

	// RtpRtcp를 생성하면서 SRTP와 연결한다.

	// Player가 준 SDP를 기준으로(플레이어가 받고자 하는 Track) RTP_RTCP를 생성한다.
	auto offer_media_desc_list = _offer_sdp->GetMediaList();
	auto peer_media_desc_list = _peer_sdp->GetMediaList();

	if(offer_media_desc_list.size() != peer_media_desc_list.size())
	{
		loge("WEBRTC", "m= line of answer does not correspod with offer");
		return false;
	}

	// RFC3264
	// For each "m=" line in the offer, there MUST be a corresponding "m=" line in the answer.
	for(int i=0; i<peer_media_desc_list.size(); i++)
	{
		auto peer_media_desc = peer_media_desc_list[i];
		auto offer_media_desc = offer_media_desc_list[i];

		// 첫번째 Payload가 가장 우선순위가 높다. Server에서 제시한 Payload중 첫번째 이므로 이것으로 통신하면 됨
		auto payload = peer_media_desc->GetFirstPayload();
		if(payload == nullptr)
		{
			loge("WEBRTC", "Failed to get the first payload type of peer sdp");
			return false;
		}

		// Payload값을 가지고 track을 찾는다.
		auto track = GetRtcStream()->GetRtcTrack(payload->GetId());
		if(track == nullptr)
		{
			loge("WEBRTC", "Failed to find track associated with %d payload");
			return false;
		}

		// TODO(getroot): 향후 player에서 m= line을 선택하여 받는 기능이 만들어지면
		// peer에서 받기 거부한 m= line이 있는지 체크하여 track에서 뺀다, 현재는 다 받기 때문에 모두 보낸다.

		auto rtp_rtcp = std::make_shared<RtpRtcp>(track->GetId(),
												  session,
												  peer_media_desc->GetMediaType() == MediaDescription::MediaType::AUDIO);
		rtp_rtcp->Initialize();
		rtp_rtcp->SetSSRC(offer_media_desc->GetSsrc());
		rtp_rtcp->SetPayloadType(payload->GetId());

		rtp_rtcp->RegisterUpperNode(nullptr);
		rtp_rtcp->RegisterLowerNode(_srtp_transport);
		_srtp_transport->RegisterUpperNode(rtp_rtcp);
		_rtp_rtcp_map[track->GetId()] = rtp_rtcp;

		rtp_rtcp->Start();
	}

	// 나머지를 모두 연결한다.
	_srtp_transport->RegisterLowerNode(_dtls_transport);
	_srtp_transport->Start();
	_dtls_transport->RegisterUpperNode(_srtp_transport);
	_dtls_transport->RegisterLowerNode(_dtls_ice_transport);
	_dtls_transport->Start();
	_dtls_ice_transport->RegisterUpperNode(_dtls_transport);
	_dtls_ice_transport->RegisterLowerNode(nullptr);
	_dtls_ice_transport->Start();

	return Session::Start();
}

bool RtcSession::Stop()
{
	logd("WEBRTC", "Stop session. Peer sdp session id : %u", GetPeerSDP()->GetSessionId());

	// 연결된 세션을 정리한다.
	_dtls_ice_transport->Stop();
	_dtls_transport->Stop();
	_srtp_transport->Stop();

	for(const auto &x : _rtp_rtcp_map)
	{
		auto rtp_rtcp = x.second;
		rtp_rtcp->Stop();
	}

	_rtp_rtcp_map.clear();

	return Session::Stop();
}

std::shared_ptr<SessionDescription> RtcSession::GetPeerSDP()
{
	return _peer_sdp;
}

void RtcSession::OnPacketReceived(std::shared_ptr<SessionInfo> session_info, const std::shared_ptr<const ov::Data> data)
{
	// NETWORK에서 받은 Packet은 DTLS로 넘긴다.
	// ICE -> DTLS -> SRTP | SCTP -> RTP|RTCP

	_dtls_ice_transport->OnDataReceived(NONE, data);
}

bool RtcSession::SendOutgoingVideoData(std::shared_ptr<MediaTrack> track,
									   FrameType frame_type,
									   uint32_t timestamp,
									   const uint8_t *payload_data,
									   size_t payload_size,
									   const FragmentationHeader *fragmentation,
									   const RTPVideoHeader *rtp_video_header)
{
	// Track ID로 rtp_rtcp를 찾는다.
	auto it = _rtp_rtcp_map.find(track->GetId());
	// 없으면 peer가 원하는 track이 아니므로 돌려보냄
	if(it == _rtp_rtcp_map.end())
	{
		return false;
	}

	auto rtp_rtcp = it->second;

	//logd("WEBRTC", "RtcSession Send first node : %d", payload_size);
	// SendOutgoingData 를 호출하면 최종적으로 SendRtpToNetwork가 호출된다.
	return rtp_rtcp->SendOutgoingData(frame_type,
									  timestamp,
									  payload_data,
									  payload_size,
									  fragmentation,
									  rtp_video_header);
}