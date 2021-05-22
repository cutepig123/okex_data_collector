#include <string>
#include "../orderbook.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::AtLeast;  
using ::testing::_;
using ::testing::Field;

class MockDataCollector : public DataCollectorBase {
	std::function<void(Order const&)>	order_callback_;
	std::function<void(Balance const&)>	balance_callback_;
public:
	void subscribe(std::function<void(Order const&)> c1,
		std::function<void(Balance const&)> c2) override
	{
		order_callback_ = c1;
		balance_callback_ = c2;
	}

	MOCK_METHOD(void, wait, (), (override));

	void new_order(Order const& t)
	{
		order_callback_(t);
	}
	void new_balance(Balance const& t)
	{
		balance_callback_(t);
	}
};

class MockDataStorage : public DataStorageBase {
public:
	MOCK_METHOD(void, store_order, (Order const&), (override));
	MOCK_METHOD(void, store_balance, (Balance const&), (override));
};

TEST(TestOrderBook, TestNewOrderBalance) {
	auto collector = std::make_unique<MockDataCollector>();    
	auto storage = std::make_unique<MockDataStorage>();

	EXPECT_CALL(*storage, store_order(Field(&Order::tradeId, "123")))
		.Times(1);
	EXPECT_CALL(*storage, store_balance(Field(&Balance::cashBal, "234")))
		.Times(1);

	std::vector<std::unique_ptr<DataStorageBase>> pDataStorages;
	pDataStorages.push_back(std::move(storage));

	auto& collector2 = *collector;

	OrderBookService obs(std::move(collector), std::move(pDataStorages));
	obs.start();
	
	Order order; order.tradeId = "123";
	collector2.new_order(order);
	Balance bal; bal.cashBal = "234";
	collector2.new_balance(bal);

	obs.stop();
}

