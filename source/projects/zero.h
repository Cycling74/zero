/// @file	
///	@ingroup 	minexamples
///	@copyright	Copyright (c) 2016, Cycling '74
/// @author		Timothy Place
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

#include "c74_min.h"
#include "dns_sd.h"

using namespace c74::min;

const double k_poll_rate = 100;


class zero_base {
public:
	virtual void update(const atoms& args) = 0;
};


class dns_service {
public:

	dns_service(const symbol& domain, const symbol& type, const symbol& name, uint16_t port = 0)
	: m_domain { domain }
	, m_type { type }
	, m_name { name }
	, m_port { port }
	{}


	~dns_service() {
		if (m_client)
			DNSServiceRefDeallocate(m_client);
	}


	symbol name() {
		return m_name;
	}



	void publish() {
//		DNSServiceFlags flags = 0;
//		uint32_t interfaceIndex = kDNSServiceInterfaceIndexAny;
		auto		name		= m_name.c_str();
		auto		type		= m_type.c_str();
		auto		domain		= m_domain.c_str();
		auto		host		= "";
//		uint16_t PortAsNumber	= mPort;
//		Opaque16	registerPort = {{ m_port >> 8, m_port & 0xFF }};
//  uint16_t txtLen			= 0;
//  const void *txtRecord	= "";		                        /* may be NULL */
//		DNSServiceRegisterReply callBack = (DNSServiceRegisterReply)&register_reply;	/* may be NULL */
//  void* context			= this;		                        /* may be NULL */
		DNSServiceErrorType result = DNSServiceRegister(&m_client, 0, kDNSServiceInterfaceIndexAny,
														name, type, domain, host, byteorder_swap(m_port),
														0, "", nullptr, this);

		if (result != kDNSServiceErr_NoError) {
//	  if (mpListener)
//	  {
//		  mpListener->didNotPublish(this);
//	  }
			if (m_client)
				DNSServiceRefDeallocate(m_client);
			m_client = NULL;
		}
		else {
//	  if (mpListener) {
//		  mpListener->willPublish(this);
//	  }
		}
	}


	inline friend bool operator == (dns_service& a, const dns_service& b) {
		return a.m_domain == b.m_domain && a.m_type == b.m_type && a.m_name == b.m_name;
	}


private:
	symbol			m_domain;
	symbol			m_type;
	symbol			m_name;
	uint16_t		m_port;
	DNSServiceRef	m_client = nullptr;
};
