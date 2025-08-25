#include "Plan.h"
#include <stdexcept> // Pour les exceptions futures
#include <utility>   // Pour std::move

namespace Dualys {

    // Implémentation du constructeur
    Plan::Plan(std::string id, std::shared_ptr<const Plan> base)
        : m_id(std::move(id)), m_base_plan(std::move(base)) {
        // Le constructeur initialise les membres. Le vecteur de layers est vide par défaut.
    }

    // Retourne l'ID du plan
    const std::string& Plan::getId() const {
        return m_id;
    }

    // Ajoute un layer à la pile
    void Plan::applyLayer(const Layer& new_layer) {
        m_layers.push_back(new_layer);
    }

    /**
     * Le clonage est l'une des opérations les plus puissantes et efficaces.
     * On crée un nouveau plan dont la "base" est le plan actuel.
     * Comme la base et les layers du plan actuel sont constants et partagés,
     * il n'y a aucune copie de données lourde. C'est quasi-instantané.
     */
    std::unique_ptr<Plan> Plan::clone(const std::string& new_id) const {
        // On utilise shared_from_this() pour obtenir un shared_ptr de l'instance actuelle (`this`).
        // Cela permet au nouveau plan de partager la propriété de l'original.
        // C'est la manière la plus propre de gérer le partage de la base.
        auto self_ptr = shared_from_this();
        auto cloned_plan = std::make_unique<Plan>(new_id, std::const_pointer_cast<const Plan>(self_ptr));
        return cloned_plan;
    }

    /**
     * C'est le cœur du système. Cette fonction calcule l'état final du FS.
     * Elle est récursive : elle demande d'abord l'état de sa base, puis applique
     * ses propres modifications par-dessus.
     */
    std::map<std::string, std::string> Plan::getFileSystemState() const {
        std::map<std::string, std::string> currentState;

        // 1. Point de départ : obtenir l'état de la base.
        if (m_base_plan) {
            currentState = m_base_plan->getFileSystemState(); // Appel récursif
        }
        // Si m_base_plan est nullptr, on part d'une map vide (l'État Initial 0).

        // 2. Appliquer tous nos propres calques, dans l'ordre.
        for (const auto& layer : m_layers) {
            for (const auto& change : layer.changes) {
                switch (change.type) {
                    case ChangeType::ADDED:
                    case ChangeType::MODIFIED:
                        // Pour ADDED et MODIFIED, on insère ou met à jour la valeur.
                        // La structure `std::map` gère ça automatiquement.
                        if (change.new_content_hash.data()) {
                            currentState[change.path] = *change.new_content_hash.data();
                        }
                        break;

                    case ChangeType::REMOVED:
                        // On supprime l'entrée de la map.
                        currentState.erase(change.path);
                        break;

                    // Le cas PERMISSION_CHANGED pourrait être géré ici
                    // en modifiant une structure de données plus complexe que la map.
                    case ChangeType::PERMISSION_CHANGED:
                        // Logique future...
                        break;
                }
            }
        }

        return currentState;
    }

    /**
     * La fusion combine les changements de deux plans.
     * Pour que la fusion soit logique, les deux plans doivent partager la même base.
     * Stratégie de conflit simple : "Last Write Wins". Les calques du plan B
     * sont appliqués après ceux du plan A, donc ils écrasent les conflits.
     */
    std::unique_ptr<Plan> Plan::merge(const std::string& new_id, const Plan& planA, const Plan& planB) {
        // Vérification de la compatibilité : les deux plans doivent dériver de la même base.
        if (planA.m_base_plan != planB.m_base_plan) {
            // Pour l'instant, on n'autorise que la fusion de plans avec une base commune.
            // Une fusion plus complexe (type "3-way merge") serait nécessaire sinon.
            return nullptr;
        }

        // Le nouveau plan fusionné a la même base que ses parents.
        auto merged_plan = std::make_unique<Plan>(new_id, planA.m_base_plan);

        // Appliquer les calques du premier parent.
        for (const auto& layer : planA.m_layers) {
            merged_plan->applyLayer(layer);
        }

        // Appliquer les calques du second parent.
        for (const auto& layer : planB.m_layers) {
            merged_plan->applyLayer(layer);
        }

        return merged_plan;
    }

} // namespace Dualys
