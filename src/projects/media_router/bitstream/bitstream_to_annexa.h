//==============================================================================
//
//  MediaRouter
//
//  Created by Kwon Keuk Hanb
//  Copyright (c) 2018 AirenSoft. All rights reserved.
//
//==============================================================================

#pragma once

#include "base/media_route/media_buffer.h"
#include "base/media_route/media_type.h"
#include <stdint.h>

class BitstreamAnnexA
{
public:
    BitstreamAnnexA();
    ~BitstreamAnnexA();

 	void convert_to(MediaBuffer* pkt);
private:

	std::vector<uint8_t> 	_sps;
	std::vector<uint8_t> 	_pps;
};

