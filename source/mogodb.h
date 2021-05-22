#pragma once
#include "data_collector.h"
#include <memory>

std::unique_ptr<DataStorageBase>  create_mongodb_data_storage();
