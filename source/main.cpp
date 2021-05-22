#include "okex/okex_v5.h"
#include "mogodb.h"
#include "redis.h"
#include "orderbook.h"
#include <iostream>

void main()
{
	std::vector<std::unique_ptr<DataStorageBase>> pMDataStorages;
	pMDataStorages.push_back(create_mongodb_data_storage());
	pMDataStorages.push_back(create_redis_data_storage());
	OrderBookService obs(create_okex_data_collector(), std::move(pMDataStorages));
	obs.start();
	system("pause");
	obs.stop();
}
