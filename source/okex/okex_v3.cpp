#include "../data_collector.h"
#include "utils.h"
#include <cpprest/ws_client.h>
#include <cpprest/json.h>
#include <cpprest/filestream.h>

#define BUFLEN 65536

using namespace web;
using namespace web::websockets::client;
using namespace concurrency::streams;       // Asynchronous streams
using namespace std;

class OkExV3 {
	websocket_client client;
    pplx::cancellation_token_source cts;
    std::function<void(Order const&)> callback_;

    void receive_loop()
    {
        auto token = cts.get_token();
        if (token.is_canceled())
        {
            printf("cancelled!\n");
            return;
        }
        
        client.receive().then([this](websocket_incoming_message msg) {
            // Write response body into the file.
            unsigned char buf[BUFLEN] = { 0 };
            unsigned char data[BUFLEN] = { 0 };

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

            receive_loop();
        });
        
    }
public:
	void connect()
	{
        // Create http_client to send the request.
        string url = ("wss://real.okex.com:8443/ws/v3");
        uri wsuri(s2w(url));
        client.connect(wsuri).wait();   // throw exception if abnormal
        printf("connect success: %s\n", url.c_str());
	}

	bool login();
	void subscribe(string const & channels)
	{
        string op = "subscribe";
        json::value obj;
        obj[L"op"] = json::value::string(s2w(op));
        obj[L"args"] = json::value::string(s2w(channels));
        
        {
            websocket_outgoing_message msg;
            msg.set_utf8_message(w2s(obj.serialize()));
            client.send(msg).wait();
            printf("send success: %s\n", w2s(obj.serialize()).c_str());
        }

        {
            websocket_incoming_message msg = client.receive().get();
            // Write response body into the file.
            unsigned char buf[BUFLEN] = { 0 };
            unsigned char data[BUFLEN] = { 0 };

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
        }

        auto token = cts.get_token();

        create_task([this]() {this->receive_loop(); }, token);
	}
    void unsubscribe()
    {
        cts.cancel();
    }
};

void main()
{
    OkExV3 okex;
    okex.connect();
    okex.subscribe("swap/ticker:BTC-USD-SWAP");
    system("pause");
    okex.unsubscribe();
    system("pause");
}
