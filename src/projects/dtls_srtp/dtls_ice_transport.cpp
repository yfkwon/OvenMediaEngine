//==============================================================================
//
//  OvenMediaEngine
//
//  Created by getroot
//  Copyright (c) 2018 AirenSoft. All rights reserved.
//
//==============================================================================

#include "dtls_ice_transport.h"


DtlsIceTransport::DtlsIceTransport(uint32_t node_id, std::shared_ptr<Session> session, std::shared_ptr<IcePort> ice_port)
	:SessionNode(node_id, ICE, session)
{
	_ice_port = ice_port;
}

DtlsIceTransport::~DtlsIceTransport()
{
}

// Implement SessionNode Interface
// 데이터를 upper에서 받는다. lower node로 보낸다.
bool DtlsIceTransport::SendData(SessionNodeType from_node, const std::shared_ptr<ov::Data> &data)
{
	// Node 시작 전에는 아무것도 하지 않는다.
	if(GetState() != SessionNode::NodeState::Started)
	{
		logd("DTLS-ICE", "SessionNode has not started, so the received data has been canceled.");
		return false;
	}

	//logd("WEBRTC", "DtlsIceTransport Send by ice port : %d", data->GetLength());
	// ICE_PORT로 최종 전송한다. (Out of session)
	_ice_port->Send(GetSession(), data);
	return true;
}

// 데이터를 lower에서 받는다. upper node로 보낸다.
bool DtlsIceTransport::OnDataReceived(SessionNodeType from_node, const std::shared_ptr<const ov::Data> &data)
{
	// Node 시작 전에는 아무것도 하지 않는다.
	if(GetState() != SessionNode::NodeState::Started)
	{
		logd("DTLS-ICE", "SessionNode has not started, so the received data has been canceled.");
		return false;
	}

	// upper node를 구한다. DTLS와 연결되는 ICE는 무조건 DTLS 하나와면 연결된다.
	auto node = GetUpperNode();
	if(node == nullptr)
	{
		return false;
	}

	// 여기서는 할일이 없다. 그냥 상위 노드로 넘긴다.
	node->OnDataReceived(from_node, data);

	return true;
}