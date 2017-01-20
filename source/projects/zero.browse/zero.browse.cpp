/// @file	
///	@ingroup 	minexamples
///	@copyright	Copyright (c) 2016, Cycling '74
/// @author		Timothy Place
///	@license	Usage of this file and its contents is governed by the MIT License

#include "c74_min.h"
#include "dns_sd.h"

using namespace c74::min;

const double k_poll_rate = 100;

class zero_base {
public:
	virtual void update(const atoms& args) = 0;
};


void DNSSD_API dns_service_browse_reply(DNSServiceRef, DNSServiceFlags, uint32_t, DNSServiceErrorType, const char*, const char*, const char*, void*);


class dns_service {
public:
	dns_service(const symbol& domain, const symbol& type, const symbol& name)
	: m_domain { domain }
	, m_type { type }
	, m_name { name }
	{}


	symbol name() {
		return m_name;
	}


	inline friend bool operator == (dns_service& a, const dns_service& b) {
		return a.m_domain == b.m_domain && a.m_type == b.m_type && a.m_name == b.m_name;
	}


private:
	symbol m_domain;
	symbol m_type;
	symbol m_name;
};


class dns_service_browser {
public:

	dns_service_browser(zero_base& owner, const symbol& type, const symbol& domain)
	: m_owner	{ owner }
	, m_type	{ type }
	, m_domain	{ domain }
	{
		auto a_type = type.c_str();
		auto a_domain = domain.c_str();
		auto err = DNSServiceBrowse(&m_client, 0, kDNSServiceInterfaceIndexAny, a_type, a_domain, dns_service_browse_reply, this);

		if (!m_client || err != kDNSServiceErr_NoError) {
			if (m_client)
				DNSServiceRefDeallocate(m_client);
			m_client = NULL;
		}
	}


	~dns_service_browser() {
		if (m_client)
			DNSServiceRefDeallocate(m_client);
		m_client = nullptr;
	}


	void handle_reply(const DNSServiceFlags flags, const char* name, const char* type, const char* domain) {
		if (flags & kDNSServiceFlagsAdd) {
			dns_service match(domain, type, name);
			auto iter = std::find(m_services.begin(), m_services.end(), match);
			if (iter == m_services.end()) // only add to the list if we don't have it already
				m_services.push_back( dns_service(domain, type, name) );
		}
		else {
			dns_service match(domain, type, name);
			auto iter = std::find(m_services.begin(), m_services.end(), match);
			if (iter != m_services.end())
				m_services.erase(iter);
		}

		if (flags & kDNSServiceFlagsMoreComing)
			; // wait for the rest of the replies...
		else {
			//zero_browse_update(m_owner);
			atoms as(m_services.size());
			for (auto i=0; i<m_services.size(); ++i)
				as[i] = m_services[i].name();
			m_owner.update(as);
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

private:
	zero_base&					m_owner;
	symbol						m_type;
	symbol						m_domain;
	DNSServiceRef				m_client = nullptr;
	std::vector<dns_service>	m_services;
	std::thread					m_thread;
};



void DNSSD_API dns_service_browse_reply(DNSServiceRef client, DNSServiceFlags flags, uint32_t index, DNSServiceErrorType err,
										const char *name, const char *type, const char *domain, void *context)
{
	auto self = (dns_service_browser*)context;
	self->handle_reply(flags, name, type, domain);
}



class zero_browse : public object<zero_browse>, public zero_base {
private:
	bool m_initialized { false };

public:

	MIN_DESCRIPTION { "Browse available services published using ZeroConf" };
	MIN_TAGS		{ "network" };
	MIN_AUTHOR		{ "Cycling '74" };
	MIN_RELATED		{ "udpsend, udpreceive" };
	
	inlet<>		input	{ this, "(bang) refresh the listing of services" };
	outlet<>	output	{ this, "(list) a list of available services" };


	zero_browse(const atoms& = {}) {
		m_initialized = true;
		c74::max::object_attach_byptr_register(maxobj(), maxobj(), k_sym_box);
		bang();
	}


	attribute<symbol> type { this, "type", "_http._tcp",
		description {
			"Type of service. "
		}
	};


	attribute<symbol> domain { this, "domain", "local",
		description {
			"Domain for the service. The name 'local' is reserved for ZeroConf usage. "
		},
	};


	message<> bang { this, "bang", "Post the greeting.",
		MIN_FUNCTION {
			if (m_dns_service_browser)
				delete m_dns_service_browser;
			m_dns_service_browser = new dns_service_browser(*this, type, domain);
			poll.delay(k_poll_rate);
			return {};
		}
	};


	message<> notify { this, "notify",
		MIN_FUNCTION {
			symbol msg = args[2];
			if (msg == "attr_modified")
				bang();
			return { 0 };
		}
	};


	timer poll { this,
		MIN_FUNCTION {
			if (m_dns_service_browser) {
				auto success = m_dns_service_browser->poll();
				if (!success)
					poll.delay(k_poll_rate);
			}
			return {};
		}
	};


	void update(const atoms& args) {
		output.send(args);
	}

private:
	dns_service_browser* m_dns_service_browser = nullptr;
};


MIN_EXTERNAL(zero_browse);
