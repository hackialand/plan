#pragma once

#include <string>
#include <vector>
#include <memory> // Pour std::shared_ptr, std::unique_ptr, std::enable_shared_from_this
#include <map>
#include "Layer.h" // Assure-toi que Layer.h est dans le même dossier

namespace Dualys {

    /**
     * @class Plan
     * @brief Représente un environnement applicatif virtuel et son état.
     *
     * Un Plan est défini par un état de base (potentiellement un autre Plan) et une
     * série de "calques" (Layers) de modifications. Cette structure permet une
     * gestion efficace de l'état, un partage de données maximal et des opérations
     * rapides comme le clonage.
     * La classe hérite de std::enable_shared_from_this pour permettre d'obtenir
     * un std::shared_ptr de l'instance courante, ce qui est utile pour le clonage.
     */
    class Plan : public std::enable_shared_from_this<Plan> {
    private:
        // L'identifiant unique du Plan.
        std::string m_id;

        // Un pointeur partagé vers le Plan de base.
        // C'est un pointeur vers une instance constante pour garantir que la base est immuable.
        // Si ce pointeur est nullptr, ce Plan est un "État Initial 0".
        std::shared_ptr<const Plan> m_base_plan;

        // La pile ordonnée de calques de modifications.
        // L'ordre est crucial : les calques sont appliqués séquentiellement.
        std::vector<Layer> m_layers;

    public:
        /**
         * @brief Construit un nouveau Plan.
         * @param id L'identifiant unique pour ce nouveau Plan.
         * @param base Un pointeur partagé vers le Plan de base (peut être nullptr).
         */
        Plan(std::string id, std::shared_ptr<const Plan> base);

        /**
         * @brief Retourne l'identifiant du Plan.
         */
        const std::string& getId() const;

        /**
         * @brief Applique un nouveau calque de modifications au Plan.
         * Le calque est ajouté à la fin de la pile.
         * @param new_layer Le calque à appliquer.
         */
        void applyLayer(const Layer& new_layer);

        /**
         * @brief Clone le Plan actuel pour en créer un nouveau.
         * Le clonage est une opération très légère qui ne duplique pas les données.
         * Le nouveau Plan partagera la même base et la même pile de calques que l'original,
         * prêt à recevoir ses propres modifications.
         * @param new_id L'identifiant unique pour le Plan cloné.
         * @return Un pointeur unique vers le nouveau Plan.
         */
        std::unique_ptr<Plan> clone(const std::string& new_id) const;

        /**
         * @brief Calcule et retourne l'état final et complet du système de fichiers virtuel.
         * Cette méthode "matérialise" l'arborescence en partant de l'état de base
         * et en appliquant séquentiellement tous les calques.
         * @return Une map représentant l'arborescence (chemin -> hash du contenu).
         */
        std::map<std::string, std::string> getFileSystemState() const;

        /**
         * @brief Fusionne deux Plans pour en créer un nouveau.
         * Cette opération statique prend deux plans, combine leurs calques et résout les conflits.
         * @param new_id L'identifiant du nouveau Plan fusionné.
         * @param planA Le premier Plan source.
         * @param planB Le second Plan source.
         * @return Un pointeur unique vers le nouveau Plan fusionné, ou nullptr en cas d'échec.
         */
        static std::unique_ptr<Plan> merge(const std::string& new_id, const Plan& planA, const Plan& planB);
    };
}
