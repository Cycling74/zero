/// @file	
///	@ingroup 	minexamples
///	@copyright	Copyright (c) 2016, Cycling '74
/// @author		Timothy Place
///	@license	Usage of this file and its contents is governed by the MIT License

#include "../zero.h"

class zero_resolve : public object<zero_resolve>, public zero_base {
private:
	bool m_initialized { false };

public:

	MIN_DESCRIPTION { "Resolve a ZeroConf service name to it's IP address and port" };
	MIN_TAGS		{ "network" };
	MIN_AUTHOR		{ "Cycling '74" };
	MIN_RELATED		{ "zero.announce, zero.browse, udpsend, udpreceive" };
	
	inlet<>		input	{ this, "(bang) refresh the listing of services" };
	outlet<>	output	{ this, "(list) a list of available services" };

	zero_resolve(const atoms& = {}) {
		m_initialized = true;
		c74::max::object_attach_byptr_register(maxobj(), maxobj(), k_sym_box);
		bang();
	}


	attribute<symbol> name { this, "name", "Zero",
		description{ "Name of the service to publish. " }
	};


	attribute<symbol> type { this, "type", "_http._tcp",
		description { "Type of service. " }
	};


	attribute<symbol> domain { this, "domain", "local",
		description { "Domain for the service. The name 'local' is reserved for ZeroConf usage. " }
	};


	message<> bang { this, "bang", "Post the greeting.",
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


	queue q { this,
		MIN_FUNCTION {
			if (m_dns_service) {
				auto success = m_dns_service->poll();
				if (!success)
					poll.delay(k_poll_rate);
			}
			return {};
		}
	};


	timer poll { this,
		MIN_FUNCTION {
			q.set();
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
