#pragma once

#include <string>
#include <vector>
#include <memory> // Pour std::shared_ptr, std::unique_ptr, std::enable_shared_from_this
#include <map>
#include "Layer.h" // Assure-toi que Layer.h est dans le même dossier

namespace Dualys {

    class Plan : public std::enable_shared_from_this<Plan> {
        /**
         * @brief Represents the unique identifier of the plan.
         *
         * Holds a string that uniquely identifies the current instance of the plan.
         * This identifier is used to differentiate between various plans in a system
         * and to track them across operations such as cloning, merging, or retrieving
         * stored plans.
         */
        std::string m_id;

        /**
         * @brief Represents the foundational plan upon which the current plan is built.
         *
         * Holds a shared pointer to a constant `Plan` object that serves as the base
         * for this plan. It defines the initial state from which modifications or layers
         * of the current plan are applied. If `m_base_plan` is null, the current plan
         * starts from an empty or initial state.
         */
        std::shared_ptr<const Plan> m_base_plan;

        /**
         * @brief Stores the collection of layers associated with the plan.
         *
         * Maintains a sequential list of `Layer` objects that represent the modifications
         * or changes applied to the current plan. These layers are applied in order
         * during operations like computing the filesystem state or merging plans.
         */
        std::vector<Layer> m_layers;

    public:
        /**
         *
         */
        Plan(std::string id, std::shared_ptr<const Plan> base);

        /**
         * @brief Retrieves the identifier of the entity.
         *
         * Provides the unique identifier that represents the entity.
         *
         * @return The unique identifier as an integer.
         */
        const std::string &getId() const;

        /**
         *
         * @brief Applies a new layer to the current plan.
         *
         * This method adds the provided layer to the list of layers in the current plan,
         * allowing further modifications to the plan's state.
         *
         * @param new_layer The new layer to be added.
         *
         */
        void applyLayer(const Layer &new_layer);

        /**
         * @brief Creates a clone of the current plan with a new identifier.
         *
         * This method creates a new plan which uses the current plan as its base.
         * The cloning process is efficient, as the new plan shares its base and layers
         * with the original plan without duplicating heavy data. The operation is nearly instantaneous.
         *
         * @param new_id The identifier for the newly created plan.
         *
         * @return A unique pointer to the newly cloned plan.
         *
         */
        std::unique_ptr<Plan> clone(const std::string &new_id) const;

        /**
         * @brief Computes and returns the final state of the virtual filesystem based on the current plan.
         *
         * This method calculates the resulting state of the filesystem by starting from the state of its base plan
         * (if any) and applying all the modifications described by its own layers, in order. If the current plan
         * has no base (i.e., initial state), the computation starts from an empty state. Modifications can include
         * added, modified, or removed entries. The changes are applied recursively for all base plans.
         *
         * @return A map representing the final virtual filesystem state, where the keys are paths (strings) and
         *         the values are content hashes or other related string information.
         */
        std::map<std::string, std::string> getFileSystemState() const;

        /**
         *
         * @brief Merges two plans into a new plan with combined layers.
         *
         * This method combines the changes of two plans, presuming both share the same base plan.
         * The conflict resolution strategy used is "Last Write Wins", where the layers of `planB`
         * overwrite conflicting layers from `planA`. The result is a new plan with a unique identifier.
         *
         * @param new_id The identifier for the newly created plan.
         * @param planA The first plan to merge, whose layers are applied first.
         * @param planB The second plan to merge, whose layers overwrite conflicts from planA.
         *
         * @return A unique pointer to the new merged Plan instance, or nullptr if the base plans of
         *         `planA` and `planB` are incompatible.
         */
        static std::unique_ptr<Plan> merge(const std::string &new_id, const Plan &planA, const Plan &planB);
    };
}
