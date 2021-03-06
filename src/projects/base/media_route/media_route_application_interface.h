#pragma once


#include <stdint.h>
#include <memory>
#include <vector>

#include "base/ovlibrary/ovlibrary.h"

class MediaRouteApplicationObserver;
class MediaRouteApplicationConnector;
class StreamInfo;
class MediaBuffer;

class MediaRouteApplicationInterface :  public ov::EnableSharedFromThis<MediaRouteApplicationInterface>
{
public:
	virtual bool OnCreateStream(
		std::shared_ptr<MediaRouteApplicationConnector> application, 
		std::shared_ptr<StreamInfo> stream) { return false; }

	virtual bool OnDeleteStream(
		std::shared_ptr<MediaRouteApplicationConnector> application, 
		std::shared_ptr<StreamInfo> stream) { return false; }

	virtual bool OnReceiveBuffer(
		std::shared_ptr<MediaRouteApplicationConnector> application, 
		std::shared_ptr<StreamInfo> stream,
		std::unique_ptr<MediaBuffer> buffer) { return false; }
};

