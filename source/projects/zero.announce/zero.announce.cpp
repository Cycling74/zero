/// @file	
///	@ingroup 	minexamples
///	@copyright	Copyright (c) 2016, Cycling '74
/// @author		Timothy Place
///	@license	Usage of this file and its contents is governed by the MIT License

#include "../zero.h"

class zero_announce : public object<zero_announce> {
private:
	bool m_initialized { false };

public:

	MIN_DESCRIPTION { "Publish an available service using ZeroConf" };
	MIN_TAGS		{ "network" };
	MIN_AUTHOR		{ "Cycling '74" };
	MIN_RELATED		{ "zero.browse, udpsend, udpreceive" };
	
	inlet<>		input	{ this, "(bang) refresh the listing of services" };


	zero_announce(const atoms& = {}) {
		m_initialized = true;
		c74::max::object_attach_byptr_register(maxobj(), maxobj(), k_sym_box);
		bang();
	}


	~zero_announce() {
		delete m_dns_service;
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


	attribute<int> port { this, "port", 7400,
		description { "Network port on which the service is provided. " }
	};


	message<> bang { this, "bang", "Post the greeting.",
		MIN_FUNCTION {
			if (m_dns_service)
				delete m_dns_service;
			m_dns_service = new dns_service(domain, type, name, port);
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

private:
	dns_service* m_dns_service = nullptr;

};


MIN_EXTERNAL(zero_announce);
