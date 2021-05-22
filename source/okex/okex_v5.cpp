#include "../data_collector.h"
#include "utils.h"
#include <cpprest/ws_client.h>
#include <cpprest/json.h>
#include <cpprest/filestream.h>
#include "pplx_utils.h"

#define BUFLEN 65536

using namespace web;
using namespace web::websockets::client;
using namespace concurrency::streams;       // Asynchronous streams
using namespace std;

bool replace(std::string& str, const std::string& from, const std::string& to) {
	size_t start_pos = str.find(from);
	if (start_pos == std::string::npos)
		return false;
	str.replace(start_pos, from.length(), to);
	return true;
}

class OkExV5 {
	websocket_client client;
	Concurrency::task<void>  task;
	//pplx::cancellation_token_source cts;
	//Concurrency::cancellation_token token;

	struct SubsInfo
	{
		std::function<void(json::value const&)> callback;
	};

	SubsInfo    subs_info_order_;
	SubsInfo    subs_info_bal_;

	Concurrency::task<void> receive_loop()
	{
		printf(">>start receive\n");
		Concurrency::task<void> t = client.receive().then([this](websocket_incoming_message msg) {
			// Write response body into the file.
			unsigned char buf[BUFLEN] = { 0 };
			char data[BUFLEN] = { 0 };

			auto buflen = msg.body().streambuf().scopy(buf, BUFLEN);
			uLong datalen = sizeof(data);

			if (msg.message_type() == websocket_message_type::binary_message) {
				gzDecompress((Byte*)buf, buflen, (Byte*)data, &datalen);
			}
			else {
				strcpy_s((char*)data, datalen, (char*)buf);
			}
			printf("receive success: \n");
			printf("\t data: %s\n", data);
			printf("\t datalen: %ld\n\n\n", datalen);

			if (string(data) == "pong")
			{
				printf("it is pong message\n");
			}
			else
			{
				auto json = json::value::parse(string(data));
				if (json.has_field(L"arg"))
				{
					auto channel = json.at(L"arg").at(L"channel").as_string();
					if (channel == L"balance_and_position")
					{
						if (subs_info_bal_.callback)
							subs_info_bal_.callback(json);
					}
					else if (channel == L"orders")
					{
						if (subs_info_order_.callback)
							subs_info_order_.callback(json);
					}
				}
			}

			// Q: will tping be deleted very soon?
			// A: No. it seems the system has pushed the task to its task queue
			// TODO: the task can be cancelled if there is new message received before timeout
			pplx::task<void> tping = pplx::complete_after(25000).then([this]() {
				{
					websocket_outgoing_message msg;
					msg.set_utf8_message("ping");
					pplx::task<void> t = client.send(msg);
					printf("send success: ping\n");
					return t;
				}});
			return receive_loop();
		});
		return t;
	}

	
	void connect(string const& url)
	{
		uri wsuri(s2w(url));
		client.connect(wsuri).wait();   // throw exception if abnormal
		printf("connect success: %s\n", url.c_str());
	}

	std::string send_and_rcv(std::string const& str)
	{
		{
			websocket_outgoing_message msg;
			msg.set_utf8_message(str);
			client.send(msg).wait();
			printf("send success: %s\n", str.c_str());
		}

		{
			websocket_incoming_message msg = client.receive().get();
			// Write response body into the file.
			unsigned char buf[BUFLEN] = { 0 };
			char data[BUFLEN] = { 0 };

			auto buflen = msg.body().streambuf().scopy(buf, BUFLEN);
			uLong datalen = sizeof(data);

			if (msg.message_type() == websocket_message_type::binary_message) {
				gzDecompress((Byte*)buf, buflen, (Byte*)data, &datalen);
			}
			else {
				strcpy_s((char*)data, datalen, (char*)buf);
			}

			printf("receive success: \n");
			printf("\t data: %s\n", data);
			printf("\t datalen: %ld\n\n\n", datalen);
			return string(data);
		}
	}

	void subscribe(json::value const& channels)
	{
		string op = "subscribe";
		json::value obj;
		obj[L"op"] = json::value::string(s2w(op));
		obj[L"args"] = channels;

		send_and_rcv(w2s(obj.serialize()));
	}
public:
	void connect_public()
	{
		//TODO: error handling
		string url = "wss://wspap.okex.com:8443/ws/v5/public?brokerId=9999";
		connect(url);
	}

	void connect_private(string const& api_key, string const& secret_key, string const& passphrase)
	{
		//TODO: error handling
		string url = "wss://wspap.okex.com:8443/ws/v5/private?brokerId=9999";
		connect(url);

		char timestamp[32];
		time_t t;
		time(&t);
		sprintf_s(timestamp, 32, "%ld", t);
		std::string sign = GetSign(secret_key, timestamp, "GET", "/users/self/verify", "");

		string login_str = "{ \"op\": \"login\", \"args\" : ["
			"{\"apiKey\":\"$api_key\", \"passphrase\" : \"$passphrase\", \"timestamp\" : \"$timestamp\", \"sign\" : \"$sign\"}"
			"]}";

		replace(login_str, "$api_key", api_key);
		replace(login_str, "$passphrase", passphrase);
		replace(login_str, "$timestamp", timestamp);
		replace(login_str, "$sign", sign);
		send_and_rcv(login_str);
	}

	void begin_subscribe_order(std::function<void(Order const&)> callback)
	{
		json::value channels =
			json::value::parse(L"[{\"channel\":\"orders\", \"instType\":\"ANY\"}]");
		subs_info_order_.callback = [callback](json::value const& json) {
			for (auto& t : json.at(L"data").as_array())
			{
				Order order;
				order.instType = w2s(t.at(L"instType").as_string());
				order.instId = w2s(t.at(L"instId").as_string());
				order.ordId = w2s(t.at(L"ordId").as_string());
				order.tradeId = w2s(t.at(L"tradeId").as_string());
				order.cTime = w2s(t.at(L"cTime").as_string());
				callback(order);
			}
		};
		subscribe(channels);
	}

	void begin_subscribe_balance(std::function<void(Balance const&)> callback)
	{
		json::value channels =
			json::value::parse(L"[{\"channel\":\"balance_and_position\"}]");
		subs_info_bal_.callback = [callback](json::value const& json) {
			for (auto& data : json.at(L"data").as_array())
			{
				for (auto& t : data.at(L"balData").as_array())
				{
					Balance balance;
					balance.ccy = w2s(t.at(L"ccy").as_string());
					balance.cashBal = w2s(t.at(L"cashBal").as_string());
					balance.uTime = w2s(t.at(L"uTime").as_string());
					callback(balance);
				}

			}
		};
		subscribe(channels);
	}

	void subscribe_enable_callback()
	{
		task = receive_loop();
	}

	void wait()
	{
		printf("wait...\n");
		//cts.cancel();
		try
		{
			task.wait();
		}
		catch (const exception& e)
		{
			cout << L"Caught exception." << e.what() << endl;
		}
	}
	
	void cancel()
	{
		// TODO
	}
};

class OkExDataCollector : public DataCollectorBase {
	OkExV5 okex;
public:
	void subscribe(std::function<void(Order const&)> callback1,
		std::function<void(Balance const&)> callback2) override
	{
		string api_key = "NA";
		string secret_key = "NA";
		string passphrase = "NA";

		okex.connect_private(api_key, secret_key, passphrase);
		okex.begin_subscribe_order(callback1);
		okex.begin_subscribe_balance(callback2);
		okex.subscribe_enable_callback();
	}

	void wait() override
	{
		okex.wait();
	}
};

std::unique_ptr<DataCollectorBase>  create_okex_data_collector()
{
	std::unique_ptr<DataCollectorBase> pDataCollector(new OkExDataCollector);
	return pDataCollector;
}
