#include <cstdint>
#include <iostream>
#include <vector>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/instance.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>
#include "data_collector.h"

// http://mongocxx.org/mongocxx-v3/tutorial/

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

class MongoDBStorage : public DataStorageBase {
	mongocxx::instance instance{}; // This should be done only once.
	mongocxx::client client{ mongocxx::uri{} };
	mongocxx::database db = client["okex"];
	mongocxx::collection coll_order = db["order"];
	mongocxx::collection coll_balance = db["balance"];
public:

	void store_order(Order const& order) override
	{
		auto builder = bsoncxx::builder::stream::document{};
		bsoncxx::document::value doc_value = builder
#define COUT(x) << #x << order.x
			COUT(instType)
			COUT(instId)
			COUT(ordId)
			COUT(tradeId)
			COUT(cTime)
			<< bsoncxx::builder::stream::finalize;
		bsoncxx::document::view view = doc_value.view();

		bsoncxx::stdx::optional<mongocxx::result::insert_one> result =
			coll_order.insert_one(view);

		std::cout << __FUNCTION__ << order.tradeId << std::endl;
	}
	void store_balance(Balance const& balance) override
	{
		auto builder = bsoncxx::builder::stream::document{};
		bsoncxx::document::value doc_value = builder
#define COUT(x) << #x << balance.x
			COUT(ccy)
			COUT(cashBal)
			COUT(uTime)
			<< bsoncxx::builder::stream::finalize;
		bsoncxx::document::view view = doc_value.view();

		bsoncxx::stdx::optional<mongocxx::result::insert_one> result =
			coll_balance.insert_one(view);

		std::cout << __FUNCTION__ << balance.cashBal << std::endl;
	}
};

std::unique_ptr<DataStorageBase>  create_mongodb_data_storage()
{
	std::unique_ptr<DataStorageBase> p(new MongoDBStorage);
	return p;
}
