/// @file
/// @copyright	Copyright (c) 2017, Cycling '74
/// @author		Timothy Place
/// @license	Usage of this file and its contents is governed by the MIT License

#include "../zero.h"

class zero_announce : public object<zero_announce>, public zero_base {
public:

	MIN_DESCRIPTION { "Publish an available service using ZeroConf" };
	MIN_TAGS		{ "network" };
	MIN_AUTHOR		{ "Cycling '74" };
	MIN_RELATED		{ "zero.browse, zero.resolve, udpsend, udpreceive" };
	
	inlet<>		input	{ this, "(bang) re-publish the service" };


	zero_announce(const atoms& = {}) {
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


	attribute<int> port { this, "port", 7400,
		description { "Network port on which the service is provided." }
	};


	message<> bang { this, "bang", "Publish the service.",
		MIN_FUNCTION {
			m_dns_service = std::make_unique<dns_service>(this, domain, type, name, port);
			m_dns_service->publish();
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


	virtual void update(const atoms&) override {}
	virtual void error(const char*) override {}


private:
	std::unique_ptr<dns_service> m_dns_service;
};


MIN_EXTERNAL(zero_announce);
