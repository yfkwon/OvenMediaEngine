LOCAL_PATH := $(call get_local_path)
include $(DEFAULT_VARIABLES)

LOCAL_STATIC_LIBRARIES := \
	webrtc \
	ice \
	transcoder \
	rtc_signalling \
	jsoncpp \
	http_server \
	dtls_srtp \
	rtp_rtcp \
	sdp \
	publisher \
	mediarouter \
	application \
	physical_port \
	socket \
	ovcrypto \
	config \
	ovlibrary \
	rtmpprovider \
	jsoncpp \
	sqlite

LOCAL_PREBUILT_LIBRARIES := \
	librtmp.a \
	libpugixml.a

LOCAL_LDFLAGS := \
	-lpthread \
	-ldl \
	`pkg-config --libs libavformat` \
	`pkg-config --libs libavfilter` \
	`pkg-config --libs libavcodec` \
	`pkg-config --libs libswresample` \
	`pkg-config --libs libswscale` \
	`pkg-config --libs libavutil` \
	`pkg-config --libs openssl` \
	`pkg-config --libs vpx` \
	`pkg-config --libs opus` \
	`pkg-config --libs libsrtp2`

LOCAL_TARGET := main

include $(BUILD_EXECUTABLE)
