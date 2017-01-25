/// @file	
///	@copyright	Copyright (c) 2017, Cycling '74
/// @author		Timothy Place
///	@license	Usage of this file and its contents is governed by the MIT License

#include "../zero.h"


class zero_browse : public object<zero_browse>, public zero_base {
public:

	MIN_DESCRIPTION { "Browse available services published using ZeroConf" };
	MIN_TAGS		{ "network" };
	MIN_AUTHOR		{ "Cycling '74" };
	MIN_RELATED		{ "zero.announce, zero.resolve, udpsend, udpreceive" };
	
	inlet<>		input	{ this, "(bang) refresh the listing of services" };
	outlet<>	output	{ this, "(list) a list of available services" };


	zero_browse(const atoms& = {}) {
		bang();
	}


	attribute<symbol> type { this, "type", "_http._tcp",
		description { "Type of service. " }
	};


	attribute<symbol> domain { this, "domain", "local",
		description { "Domain for the service. The name 'local' is reserved for ZeroConf usage. " }
	};


	message<> bang { this, "bang", "Post the greeting.",
		MIN_FUNCTION {
			m_dns_service_browser = std::make_unique<dns_service_browser>(this, domain, type);
			poll.delay(k_poll_rate);
			return {};
		}
	};


	message<> notify { this, "notify",
		MIN_FUNCTION {
			symbol msg = args[2];
			if (msg == "attr_modified")
				bang();
			return { c74::max::MAX_ERR_NONE };
		}
	};


	queue q { this,
		MIN_FUNCTION {
			if (m_dns_service_browser) {
				auto success = m_dns_service_browser->poll();
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
		output.send(args);
	}

	virtual void error(const char* message) override {
		cerr << message << endl;
	}

private:
	std::unique_ptr<dns_service_browser> m_dns_service_browser;
};


MIN_EXTERNAL(zero_browse);
