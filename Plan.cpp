#include "Plan.h"
#include <utility>

using namespace Dualys;


Plan::Plan(std::string id, std::shared_ptr<const Plan> base)
    : m_id(std::move(id)), m_base_plan(std::move(base)) {
}

const std::string &Plan::getId() const {
    return m_id;
}

void Plan::applyLayer(const Layer &new_layer) {
    m_layers.push_back(new_layer);
}

std::unique_ptr<Plan> Plan::clone(const std::string &new_id) const {
    const auto self_ptr = shared_from_this();
    auto cloned_plan = std::make_unique<Plan>(new_id, std::const_pointer_cast<const Plan>(self_ptr));
    return cloned_plan;
}

std::map<std::string, std::string> Plan::getFileSystemState() const {
    std::map<std::string, std::string> currentState;

    if (m_base_plan) {
        currentState = m_base_plan->getFileSystemState();
    }

    for (const auto &[changes, id]: m_layers) {
        for (const auto &[path, type, new_content_hash]: changes) {
            switch (type) {
                case ChangeType::ADDED:
                case ChangeType::MODIFIED:
                    if (new_content_hash.data()) {
                        currentState[path] = *new_content_hash.data();
                    }
                    break;

                case ChangeType::REMOVED:
                    currentState.erase(path);
                    break;
                case ChangeType::PERMISSION_CHANGED:
                    break;
            }
        }
    }
    return currentState;
}

void Plan::loadFromFile(const char *file_path) {

}


std::unique_ptr<Plan> Plan::merge(const std::string &new_id, const Plan &planA, const Plan &planB) {
    if (planA.m_base_plan != planB.m_base_plan) {
        return nullptr;
    }
    auto merged_plan = std::make_unique<Plan>(new_id, planA.m_base_plan);

    for (const auto &layer: planA.m_layers) {
        merged_plan->applyLayer(layer);
    }

    for (const auto &layer: planB.m_layers) {
        merged_plan->applyLayer(layer);
    }

    return merged_plan;
}
