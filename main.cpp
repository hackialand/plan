#include <iostream>
#include <memory>
#include <vector>

// Inclusion de toutes nos briques de base
#include "PlanManager.h"
#include "Plan.h"
#include "Layer.h"
#include "ExecutionEngine.h"

// Fonction utilitaire pour afficher proprement l'état d'un système de fichiers
void printFileSystemState(const Dualys::Plan& plan) {
    std::cout << ">>> État du système de fichiers pour le Plan '" << plan.getId() << "':" << std::endl;
    auto const fs_state = plan.getFileSystemState();
    if (fs_state.empty()) {
        std::cout << "    (Système de fichiers vide)" << std::endl;
    } else {
        for (const auto& entry : fs_state) {
            std::cout << "    - " << entry.first << " (hash: " << entry.second << ")" << std::endl;
        }
    }
    std::cout << std::endl;
}


int main() {
    // --- 1. DÉMARRAGE DU SYSTÈME ---
    std::cout << "--- [Dualys OS] Démarrage du gestionnaire de Plans ---" << std::endl;
    auto planManager = std::make_unique<Dualys::PlanManager>();
    std::cout << std::endl;


    // --- 2. CRÉATION DE PLANS DE BASE ---
    std::cout << "--- Création de deux Plans de base ---" << std::endl;
    std::shared_ptr<Dualys::Plan> web_server_plan = planManager->createPlan("web-server");
    std::shared_ptr<Dualys::Plan> database_plan = planManager->createPlan("database");

    // Vérifions leur état initial (ils doivent être vides, car basés sur "l'État Initial 0")
    printFileSystemState(*web_server_plan);


    // --- 3. MODIFICATION DES PLANS (APPLICATION DE CALQUES) ---
    std::cout << "--- Application de calques pour configurer les Plans ---" << std::endl;

    // Configuration du serveur web
    Dualys::Layer web_config_layer;
    web_config_layer.id = "nginx_config";
    web_config_layer.changes.push_back({"/etc/nginx.conf", Dualys::ChangeType::ADDED, "hash_nginx_conf_v1", std::nullopt});
    web_config_layer.changes.push_back({"/var/www/index.html", Dualys::ChangeType::ADDED, "hash_html_hello", std::nullopt});
    web_server_plan->applyLayer(web_config_layer);

    // Ajout de l'exécutable WASM au serveur web
    Dualys::Layer web_exec_layer;
    web_exec_layer.id = "wasm_app";
    web_exec_layer.changes.push_back({"/app/main.wasm", Dualys::ChangeType::ADDED, "hash_webapp_123", std::nullopt});
    web_server_plan->applyLayer(web_exec_layer);

    // Configuration de la base de données
    Dualys::Layer db_config_layer;
    db_config_layer.id = "postgres_config";
    db_config_layer.changes.push_back({"/etc/postgres.conf", Dualys::ChangeType::ADDED, "hash_pg_conf_standard", std::nullopt});
    db_config_layer.changes.push_back({"/data/db_init.sql", Dualys::ChangeType::ADDED, "hash_init_script", std::nullopt});
    database_plan->applyLayer(db_config_layer);

    // Affichons leur nouvel état
    printFileSystemState(*web_server_plan);
    printFileSystemState(*database_plan);


    // --- 4. OPÉRATION DE CLONAGE ---
    std::cout << "--- Clonage du Plan 'web-server' pour un test ---" << std::endl;
    // Note: Pour que clone() fonctionne avec shared_from_this, le PlanManager doit retourner des shared_ptr, ce qu'il fait.
    std::unique_ptr<Dualys::Plan> test_plan = web_server_plan->clone("web-server-test");

    // Modifions le clone sans affecter l'original
    Dualys::Layer test_layer;
    test_layer.id = "test_modification";
    test_layer.changes.push_back({"/var/www/index.html", Dualys::ChangeType::MODIFIED, "hash_html_test_page", "hash_html_hello"});
    test_plan->applyLayer(test_layer);

    std::cout << "État de l'original après clonage et modification du clone :" << std::endl;
    printFileSystemState(*web_server_plan);
    std::cout << "État du clone :" << std::endl;
    printFileSystemState(*test_plan);


    // --- 5. OPÉRATION DE FUSION ---
    std::cout << "--- Fusion de 'web-server' et 'database' pour créer un Plan 'full-stack' ---" << std::endl;
    std::unique_ptr<Dualys::Plan> full_stack_plan = Dualys::Plan::merge("full-stack", *web_server_plan, *database_plan);
    if (full_stack_plan) {
        printFileSystemState(*full_stack_plan);
    } else {
        std::cerr << "La fusion a échoué (les plans n'ont peut-être pas la même base)." << std::endl;
    }


    // --- 6. EXÉCUTION D'UN PLAN ---
    std::cout << "--- Exécution du Plan 'web-server' ---" << std::endl;
    Dualys::ExecutionEngine engine;
    engine.setStrategy(std::make_unique<Dualys::WasmStrategy>());

    // Le plan web_server a déjà un fichier /app/main.wasm grâce au layer "web_exec_layer"
    engine.run(*web_server_plan);


    return 0;
}
