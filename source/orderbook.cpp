#include "orderbook.h"
#include "data_collector.h"
#include <vector>
#include <memory>

OrderBookService::OrderBookService(std::unique_ptr<DataCollectorBase>&& pDataCollector,
	std::vector<std::unique_ptr<DataStorageBase>>&& pMDataStorages)
	:pDataCollector_(std::move(pDataCollector))
	, pMDataStorages_(std::move(pMDataStorages))
{}

void OrderBookService::start()
{
	pDataCollector_->subscribe(
		[this](Order const& t) {
		for (auto& storage : pMDataStorages_)
		{
			storage->store_order(t);
		}
	},
		[this](Balance const& t) {
		for (auto& storage : pMDataStorages_)
		{
			storage->store_balance(t);
		}
	});

	pDataCollector_->wait();
}

void OrderBookService::stop()
{
	// TODO
}
