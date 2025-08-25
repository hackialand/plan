#pragma once

#include <map>
#include "Plan.h"


namespace Dualys {
    class PlanManager {
    private:
        std::map<std::string, std::shared_ptr<Plan> > active_plans;
        std::shared_ptr<const Plan> initial_state_template;

    public:
    };
}


