#pragma once
#include "data_collector.h"
#include <memory>

std::unique_ptr<DataStorageBase>  create_redis_data_storage();
