#pragma once
#include "../data_collector.h"
#include <memory>

std::unique_ptr<DataCollectorBase>  create_okex_data_collector();
