#include <sw/redis++/redis++.h>
#include <iostream>
#include <unordered_set>
#include "data_collector.h"

using namespace sw::redis;

class RedisStorage : public DataStorageBase {
	Redis redis = Redis("tcp://127.0.0.1:6379");

	long long inc(const char* key)
	{
		return redis.incr(key);
	}
public:
	RedisStorage()
	{
	}

	void store_order(Order const& order) override
	{
		// key
		auto ordernum = inc("ordernum");

		std::stringstream hashkeyss;
		hashkeyss << "order" << ordernum;

		// value
		std::unordered_map<std::string, std::string> m = {
#define COUT(x)  {  #x ,  order.x }
			COUT(instType),
			COUT(instId),
			COUT(ordId),
			COUT(tradeId),
			COUT(cTime)
#undef COUT
		};

		redis.hmset(hashkeyss.str(), m.begin(), m.end());

		std::cout << __FUNCTION__ << hashkeyss.str() << std::endl;
	}
	void store_balance(Balance const& balance) override
	{
		// key
		auto ordernum = inc("balancenum");

		std::stringstream hashkeyss;
		hashkeyss << "balance" << ordernum;

		// value
		std::unordered_map<std::string, std::string> m = {
#define COUT(x)  {  #x ,  balance.x }
			COUT(ccy),
			COUT(cashBal),
			COUT(uTime)
		};

		redis.hmset(hashkeyss.str(), m.begin(), m.end());

		std::cout << __FUNCTION__ << hashkeyss.str() << std::endl;
	}
};

std::unique_ptr<DataStorageBase>  create_redis_data_storage()
{
	std::unique_ptr<DataStorageBase> p(new RedisStorage);
	return p;
}
