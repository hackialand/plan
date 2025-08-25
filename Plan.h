#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include "Layer.h"

namespace Dualys {
    class Plan {
        std::string id;
        std::shared_ptr<const Plan> base_plan;
        std::vector<Layer> layers;
    };
}
