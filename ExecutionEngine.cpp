#include "ExecutionEngine.h"
#include <iostream>
#include <stdexcept>

namespace Dualys {

    // --- Implémentation de WasmStrategy ---

    /**
     * C'est ici que la logique d'exécution de WebAssembly prendrait place.
     * Pour l'instant, nous mettons un placeholder qui simule l'action.
     */
    void WasmStrategy::execute(const Plan& plan) const {
        std::cout << "--- [WasmStrategy] Début de l'exécution du Plan: " << plan.getId() << " ---" << std::endl;

        // 1. Obtenir l'état du système de fichiers du Plan.
        std::cout << "[WasmStrategy] Matérialisation du système de fichiers virtuel..." << std::endl;
        auto const fs_state = plan.getFileSystemState();

        // 2. Rechercher le fichier .wasm principal à exécuter dans le FS.
        // (Exemple : on cherche un fichier nommé "main.wasm" à la racine).
        std::string wasm_entry_point = "/main.wasm";
        if (fs_state.count(wasm_entry_point)) {
            std::cout << "[WasmStrategy] Fichier d'entrée '" << wasm_entry_point << "' trouvé." << std::endl;
            std::cout << "[WasmStrategy] Hash du contenu: " << fs_state.at(wasm_entry_point) << std::endl;

            // 3. Initialiser un runtime WASM (ex: Wasmtime, WAVM).
            std::cout << "[WasmStrategy] Initialisation du bac à sable (sandbox) WASM..." << std::endl;

            // 4. Charger le code WASM et l'exécuter dans le sandbox.
            //    Le sandbox aurait accès à une version en lecture seule du fs_state.
            std::cout << "[WasmStrategy] Exécution du code..." << std::endl;
            // ... ici irait le vrai code d'exécution ...
            std::cout << "[WasmStrategy] Exécution terminée avec succès." << std::endl;

        } else {
            std::cerr << "[WasmStrategy] ERREUR: Point d'entrée '" << wasm_entry_point << "' non trouvé dans le Plan." << std::endl;
        }

        std::cout << "--- [WasmStrategy] Fin de l'exécution du Plan: " << plan.getId() << " ---" << std::endl;
    }


    // --- Implémentation de ExecutionEngine ---

    void ExecutionEngine::setStrategy(std::unique_ptr<IExecutionStrategy> strategy) {
        m_strategy = std::move(strategy);
    }

    void ExecutionEngine::run(const Plan& plan) const {
        // On vérifie d'abord si une stratégie a été définie.
        if (!m_strategy) {
            throw std::runtime_error("ExecutionEngine: Aucune stratégie d'exécution n'a été définie.");
        }
        // On délègue l'exécution à l'objet stratégie.
        m_strategy->execute(plan);
    }

}
