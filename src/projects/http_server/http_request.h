//==============================================================================
//
//  OvenMediaEngine
//
//  Created by Hyunjun Jang
//  Copyright (c) 2018 AirenSoft. All rights reserved.
//
//==============================================================================
#pragma once

#include "http_datastructure.h"
#include "interceptors/http_request_interceptor.h"

#include <memory>

#include <base/ovlibrary/ovlibrary.h>

class HttpRequest : public ov::EnableSharedFromThis<HttpRequest>
{
public:
	friend class HttpRequestInterceptor;

	HttpRequest(const std::shared_ptr<HttpResponse> &response, const std::shared_ptr<HttpRequestInterceptor> &interceptor);
	~HttpRequest() override = default;

	/// HttpRequest 객체 초기화를 위해, client에서 보낸 데이터를 처리함
	///
	/// @param data 수신한 데이터
	///
	/// @return HTTP 파싱에 사용한 데이터 크기. 만약 파싱 도중 오류가 발생하면 -1L을 반환함
	ssize_t ProcessData(const std::shared_ptr<const ov::Data> &data);

	/// 헤더 파싱 상태 (ProcessData() 안에서 갱신됨)
	///
	/// @return HttpStatusCode::PartialContent = 데이터가 더 필요함.
	///         HttpStatusCode::OK = 데이터 수신이 모두 완료되었음.
	///         기타 = 오류 발생.
	HttpStatusCode ParseStatus() const
	{
		return _parse_status;
	}

	HttpMethod GetMethod() const noexcept
	{
		return _method;
	}

	ov::String GetHttpVersion() const noexcept
	{
		return _http_version;
	}

	double GetHttpVersionAsNumber() const noexcept
	{
		auto tokens = _http_version.Split("/");

		if(tokens.size() != 2)
		{
			return 0.0;
		}

		return ov::Converter::ToDouble(tokens[1]);
	}

	// GetRequestTarget의 alias
	const ov::String &GetUri() const noexcept
	{
		return GetRequestTarget();
	}

	const ov::String &GetRequestTarget() const noexcept
	{
		return _request_target;
	}

	/// HTTP body 데이터 길이 반환
	///
	/// @return body 데이터 길이. 파싱이 제대로 되지 않았거나, request header에 명시가 안되어 있으면 0이 반환됨.
	ssize_t GetContentLength() const noexcept
	{
		return _content_length;
	}

	const std::map<ov::String, ov::String, ov::CaseInsensitiveComparator> &GetRequestHeader() const noexcept
	{
		return _request_header;
	}

	const ov::String &GetHeader(const ov::String &key) const noexcept;
	const ov::String &GetHeader(const ov::String &key, const ov::String &default_value) const noexcept;
	const bool IsHeaderExists(const ov::String &key) const noexcept;

	std::shared_ptr<HttpResponse> &GetHttpResponse() noexcept
	{
		return _response;
	}

	bool SetRequestInterceptor(const std::shared_ptr<HttpRequestInterceptor> &interceptor) noexcept
	{
		if(interceptor == nullptr)
		{
			OV_ASSERT2(false);
			return false;
		}

		_interceptor = interceptor;
		return true;
	}

	const std::shared_ptr<HttpRequestInterceptor> &GetRequestInterceptor()
	{
		return _interceptor;
	}

	ov::String ToString() const;

protected:
	// HttpRequestInterceptorInterface를 통해, 다른 interceptor에서 사용됨
	const std::shared_ptr<ov::Data> &GetRequestBody()
	{
		if(_request_body == nullptr)
		{
			_request_body = ov::Data::CreateData();
		}

		return _request_body;
	}

	HttpStatusCode ParseMessage();
	HttpStatusCode ParseRequestLine(const ov::String &line);
	HttpStatusCode ParseHeader(const ov::String &line);

	void PostProcess();

	// request 처리를 담당하는 객체
	std::shared_ptr<HttpRequestInterceptor> _interceptor;

	HttpStatusCode _parse_status;

	// request 관련 정보 저장
	HttpMethod _method;
	ov::String _request_target;
	ov::String _http_version;

	// request 헤더
	bool _is_header_found;
	// 헤더 영역을 추출해내기 위해 임시로 사용되는 문자열 버퍼
	ov::String _request_string;
	std::map<ov::String, ov::String, ov::CaseInsensitiveComparator> _request_header;

	// 자주 사용하는 헤더 값은 미리 저장해놓음
	ssize_t _content_length;

	// HTTP body
	std::shared_ptr<ov::Data> _request_body;

	std::shared_ptr<HttpResponse> _response;
};
