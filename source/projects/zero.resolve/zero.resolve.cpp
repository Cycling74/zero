/// @file	
///	@copyright	Copyright (c) 2017, Cycling '74
/// @author		Timothy Place
///	@license	Usage of this file and its contents is governed by the MIT License

#include "../zero.h"

class zero_resolve : public object<zero_resolve>, public zero_base {
public:

	MIN_DESCRIPTION { "Resolve a ZeroConf service name to it's IP address and port" };
	MIN_TAGS		{ "network" };
	MIN_AUTHOR		{ "Cycling '74" };
	MIN_RELATED		{ "zero.announce, zero.browse, udpsend, udpreceive" };
	
	inlet<>		input	{ this, "(bang) refresh the address and port for the named service" };
	outlet<>	output	{ this, "(list) a list of available services" };


	zero_resolve(const atoms& = {}) {
		bang();
	}


	attribute<symbol> name { this, "name", "Zero",
		description{ "Name of the service to publish." }
	};


	attribute<symbol> type { this, "type", "_http._tcp",
		description { "Type of service." }
	};


	attribute<symbol> domain { this, "domain", "local",
		description { "Domain for the service. The name 'local' is reserved for ZeroConf usage." }
	};


	message<> bang { this, "bang", "Refresh the address and port for the named service.",
		MIN_FUNCTION {
			m_dns_service = std::make_unique<dns_service>(this, domain, type, name);
			m_dns_service->resolve();
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


	timer poll { this, true,
		MIN_FUNCTION {
			if (m_dns_service) {
				auto success = m_dns_service->poll();
				if (!success)
					poll.delay(k_poll_rate);
			}
			return {};
		}
	};


	virtual void update(const atoms& args) override {
		auto host = args[0];
		auto port = args[1];

		output.send("host", host);
		output.send("port", port);
	}

	virtual void error(const char* message) override {}

private:
	std::unique_ptr<dns_service> m_dns_service;
};


MIN_EXTERNAL(zero_resolve);
