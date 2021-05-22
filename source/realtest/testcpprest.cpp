#include "gtest/gtest.h"
#include <cpprest/json.h>
#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <cpprest/http_listener.h>              // HTTP server
#include <cpprest/json.h>                       // JSON library
#include <cpprest/uri.h>                        // URI library
#include <cpprest/ws_client.h>                  // WebSocket client
#include <cpprest/containerstream.h>            // Async streams backed by STL containers
#include <cpprest/interopstream.h>              // Bridges for integrating Async streams with STL and WinRT streams
#include <cpprest/rawptrstream.h>               // Async streams backed by raw pointer to memory
#include <cpprest/producerconsumerstream.h>     // Async streams for producer consumer scenarios

using namespace utility;                    // Common utilities like string conversions
using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features
using namespace concurrency::streams;       // Asynchronous streams

TEST(testcpprest, json)
{
	web::json::value obj;
	obj[L"bar"] = web::json::value::string(L"value");
	obj[L"bob"] = 1;
	obj[L"2"] = 3;

	EXPECT_EQ(3, obj[L"2"].as_integer());
}

TEST(testcpprest, testpplx)
{
	auto fileStream = std::make_shared<ostream>();

	// Open stream to output file.
	pplx::task<void> requestTask = fstream::open_ostream(U("results.html")).then([=](ostream outFile)
	{
		*fileStream = outFile;

		// Create http_client to send the request.
		http_client client(U("http://www.bing.com/"));

		// Build request URI and start the request.
		uri_builder builder(U("/search"));
		builder.append_query(U("q"), U("cpprestsdk github"));
		return client.request(methods::GET, builder.to_string());
	}).then([=](http_response response)
	{
		printf("Received response status code:%u\n", response.status_code());
		// Write response body into the file.
		return response.body().read_to_end(fileStream->streambuf());
	}).then([=](size_t)
	{
		return fileStream->close();
	});

	try
	{
		requestTask.wait();
	}
	catch (const std::exception& e)
	{
		printf("Error exception:%s\n", e.what());
	}
}

TEST(testcpprest, test_ws)
{
	websockets::client::websocket_client cli;
	pplx::task<std::string> ret = cli.connect(uri(U("wss://echo.websocket.org")))
		.then([&]() {
		web::websockets::client::websocket_outgoing_message msg;
		msg.set_utf8_message(("Rock it with HTML5 WebSocket"));
		return cli.send(msg);
	}).then([&]() {
		return cli.receive();
	}).then([](web::websockets::client::websocket_incoming_message const &m) {
		return m.extract_string();
	});
	
	try
	{
		ret.wait();
		EXPECT_EQ(ret.get(), "Rock it with HTML5 WebSocket");
	}
	catch (const std::exception& e)
	{
		printf("Error exception:%s\n", e.what());
	}
}