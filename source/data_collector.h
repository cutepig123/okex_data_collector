#pragma once
#include <functional>
#include <string>

struct Order{
	std::string instType, instId, ordId, tradeId;
	std::string cTime;
};

struct Balance {
	std::string ccy, cashBal;
	std::string uTime;
};

class DataCollectorBase{
public:
	virtual ~DataCollectorBase() = 0{};
	virtual void subscribe(std::function<void(Order const&)>, std::function<void(Balance const&)>) = 0;
	virtual void wait() = 0;
};

class DataStorageBase{
public:
	virtual ~DataStorageBase() = 0{};
	virtual void store_order(Order const&) = 0;
	virtual void store_balance(Balance const&) = 0;
};
