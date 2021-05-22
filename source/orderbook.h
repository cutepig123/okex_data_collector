#include "data_collector.h"
#include <vector>
#include <memory>

class OrderBookService
{
    std::unique_ptr<DataCollectorBase> pDataCollector_; 
    std::vector<std::unique_ptr<DataStorageBase>>  pMDataStorages_;
public:
    OrderBookService(std::unique_ptr<DataCollectorBase> &&pDataCollector, 
        std::vector<std::unique_ptr<DataStorageBase>>&& pMDataStorages);

    void start();
    void stop();
};
