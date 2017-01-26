/// @file
/// @copyright	Copyright (c) 2017, Cycling '74
/// @author		Timothy Place
/// @license	Usage of this file and its contents is governed by the MIT License

#pragma once

#include "c74_min.h"
#include "dns_sd.h"

using namespace c74::min;

const double k_poll_rate = 100;


class zero_base {
public:
	virtual void update(const atoms& args) = 0;
	virtual void error(const char* message) = 0;
};


void DNSSD_API dns_service_browse_reply(DNSServiceRef, DNSServiceFlags, uint32_t, DNSServiceErrorType, const char*, const char*, const char*, void*);
void DNSSD_API dns_service_resolve_reply(DNSServiceRef, DNSServiceFlags, uint32_t, DNSServiceErrorType, const char*, const char*, uint16_t, uint16_t, const unsigned char*, void*);


class dns_service {
public:

	dns_service(zero_base* owner, const symbol& domain, const symbol& type, const symbol& name, uint16_t port = 0)
	: m_owner	{ owner }
	, m_domain	{ domain }
	, m_type	{ type }
	, m_name	{ name }
	, m_port	{ port }
	{}


	virtual ~dns_service() {
		if (m_client)
			DNSServiceRefDeallocate(m_client);
	}

	dns_service(const dns_service& other) = default;
	dns_service& operator = (const dns_service& value) = default;
	// TODO: worry about m_client getting copied!?!


	symbol name() {
		return m_name;
	}


	void publish() {
		auto		name		= m_name.c_str();
		auto		type		= m_type.c_str();
		auto		domain		= m_domain.c_str();
		auto		host		= "";
		DNSServiceErrorType result = DNSServiceRegister(&m_client, 0, kDNSServiceInterfaceIndexAny,
														name, type, domain, host, byteorder_swap(m_port),
														0, "", nullptr, this);

		if (result != kDNSServiceErr_NoError) {
			if (m_client)
				DNSServiceRefDeallocate(m_client);
			m_client = nullptr;
		}
	}


	void resolve() {
		auto		name		= m_name.c_str();
		auto		type		= m_type.c_str();
		auto		domain		= m_domain.c_str();
		DNSServiceErrorType result = DNSServiceResolve(&m_client, 0, kDNSServiceInterfaceIndexAny,
													name, type, domain, dns_service_resolve_reply, this);

		if (!m_client || result != kDNSServiceErr_NoError) {
			m_owner->error("could not resolve name");
			if (m_client)
				DNSServiceRefDeallocate(m_client);
			m_client = nullptr;
		}
		// if successful then m_client is freed in handle_resolve()
	}


	void handle_resolve(DNSServiceErrorType err, uint16_t port, const char* name) {
		if (err == kDNSServiceErr_NoError) {
			m_port = byteorder_swap(port);
			m_name = symbol(name);
			m_owner->update({{ m_name, m_port }});
		}
		if (m_client) {
			DNSServiceRefDeallocate(m_client);
			m_client = nullptr;
		}
	}


	bool poll() {
		if (!m_client)
			return true; // fail silently so we don't continue to poll

		DNSServiceErrorType	err = kDNSServiceErr_NoError;
		int					dns_sd_fd = DNSServiceRefSockFD(m_client);
		int					nfds = dns_sd_fd + 1;
		timeval				tv {0, 1000};	// 1 millisecond timeout
		fd_set				readfds {};

		FD_SET(dns_sd_fd, &readfds);

		int result = select(nfds, &readfds, nullptr, nullptr, &tv);
		if (result > 0) {
			DNSServiceErrorType err = kDNSServiceErr_NoError;
			if (FD_ISSET(dns_sd_fd, &readfds))
				err = DNSServiceProcessResult(m_client);
			if (err)
				fprintf(stderr, "DNSServiceProcessResult returned %d\n", err);
			return true;
		}
		else {
			err = kDNSServiceErr_NoError;
			return false;
		}
	}


	inline friend bool operator == (dns_service& a, const dns_service& b) {
		return a.m_domain == b.m_domain && a.m_type == b.m_type && a.m_name == b.m_name;
	}


protected:
	zero_base*		m_owner;
	symbol			m_domain;
	symbol			m_type;
	symbol			m_name;
	uint16_t		m_port;
	DNSServiceRef	m_client = nullptr;
};


class dns_service_browser : public dns_service {
public:

	dns_service_browser(zero_base* owner, const symbol& domain, const symbol& type)
	: dns_service { owner, domain, type, symbol("##browser##") }
	{
		auto a_type = type.c_str();
		auto a_domain = domain.c_str();
		auto err = DNSServiceBrowse(&m_client, 0, kDNSServiceInterfaceIndexAny, a_type, a_domain, dns_service_browse_reply, this);

		if (!m_client || err != kDNSServiceErr_NoError) {
			if (m_client)
				DNSServiceRefDeallocate(m_client);
				m_client = nullptr;
		}
	}


	void handle_reply(const DNSServiceFlags flags, const char* name, const char* type, const char* domain) {
		if (flags & kDNSServiceFlagsAdd) {
			dns_service match(m_owner, domain, type, name);
			auto iter = std::find(m_services.begin(), m_services.end(), match);
			if (iter == m_services.end()) // only add to the list if we don't have it already
				m_services.push_back( dns_service(m_owner, domain, type, name) );
		}
		else {
			dns_service match(m_owner, domain, type, name);
			auto iter = std::find(m_services.begin(), m_services.end(), match);
			if (iter != m_services.end())
				m_services.erase(iter);
		}

		if (flags & kDNSServiceFlagsMoreComing)
			; // wait for the rest of the replies...
		else {
			atoms as(m_services.size());
			for (auto i=0; i<m_services.size(); ++i)
				as[i] = m_services[i].name();
			m_owner->update(as);
		}
	}


private:
	std::vector<dns_service>	m_services;
};


void DNSSD_API dns_service_browse_reply(DNSServiceRef client, DNSServiceFlags flags, uint32_t index, DNSServiceErrorType err,
										const char *name, const char *type, const char *domain, void *context)
{
	auto self = (dns_service_browser*)context;
	self->handle_reply(flags, name, type, domain);
}


void DNSSD_API dns_service_resolve_reply(DNSServiceRef client, DNSServiceFlags flags, uint32_t index, DNSServiceErrorType err,
										 const char* fullname, const char* hosttarget, uint16_t port,
										 uint16_t txtLen, const unsigned char* txtRecord, void* context)
{
	auto self = (dns_service*)context;
	self->handle_resolve(err, port, hosttarget);
}

