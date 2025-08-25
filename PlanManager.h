#pragma once

#include <map>
#include "Plan.h"


namespace Dualys {
    /**
     * @class PlanManager
     * @brief Manages and maintains plans within the system.
     *
     * The PlanManager class is responsible for handling active plans and providing
     * utilities for creating, accessing, and managing plan states. It utilizes
     * efficient memory management mechanisms for organizing plans and their templates,
     * ensuring consistent operation across the system.
     */
    class PlanManager {

        /**
         * @brief A map storing the active plans managed by the PlanManager.
         *
         * This map associates a unique string key with a shared pointer to a Plan object.
         * The string key is used as an identifier for the active plans, and the shared
         * pointer allows for shared ownership and efficient memory management of the Plan instances.
         * This structure is used to manage, access, and manipulate active plans within the system.
         */
        std::map<std::string, std::shared_ptr<Plan> > active_plans;

        /**
         * @brief A shared pointer to a constant Plan object representing the initial state template.
         *
         * This object serves as a template for initializing or creating new plans within
         * the PlanManager. Its immutability ensures that the template remains unchanged,
         * providing a consistent reference for creating or restoring plan states. The shared
         * pointer enables efficient memory management and ensures shared access to the template
         * across the system.
         */
        std::shared_ptr<const Plan> initial_state_template;
    };
}


