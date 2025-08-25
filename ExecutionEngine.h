#pragma once

#include <memory>
#include <string>
#include "Plan.h" // On a besoin de connaître la classe Plan

namespace Dualys {
    /**
     * @class IExecutionStrategy
     * @brief Interface pour toutes les stratégies d'exécution possibles.
     *
     * C'est le contrat que chaque "méthode d'exécution" (WASM, JIT, etc.)
     * doit respecter. Elle définit une seule action : exécuter un Plan.
     */
    class IExecutionStrategy {
    public:
        virtual ~IExecutionStrategy() = default;

        /**
         * @brief Méthode virtuelle pure pour exécuter un Plan.
         * @param plan Le Plan à exécuter (passé par référence constante).
         */
        virtual void execute(const Plan &plan) const = 0;
    };

    /**
     * @class WasmStrategy
     * @brief Une implémentation concrète de stratégie pour exécuter du WebAssembly.
     *
     * Cette classe sait comment prendre un Plan qui contient du code WASM
     * et le faire tourner dans un runtime sécurisé.
     */
    class WasmStrategy : public IExecutionStrategy {
    public:
        void execute(const Plan &plan) const override;
    };

    // On pourrait ajouter d'autres stratégies ici à l'avenir :
    // class UnikernelStrategy : public IExecutionStrategy { ... };
    // class JitSourceStrategy : public IExecutionStrategy { ... };


    /**
     * @class ExecutionEngine
     * @brief Le moteur principal qui orchestre l'exécution d'un Plan.
     *
     * Il ne sait pas *comment* exécuter un Plan, mais il sait *qui* appeler
     * pour le faire (la stratégie courante).
     */
    class ExecutionEngine {
    private:
        // Un pointeur unique vers la stratégie d'exécution actuellement sélectionnée.
        std::unique_ptr<IExecutionStrategy> m_strategy;

    public:
        ExecutionEngine() = default;

        /**
         * @brief Définit la stratégie d'exécution à utiliser.
         * @param strategy Un pointeur unique vers l'objet stratégie.
         */
        void setStrategy(std::unique_ptr<IExecutionStrategy> strategy);

        /**
         * @brief Exécute un Plan en utilisant la stratégie actuellement définie.
         * @param plan Le Plan à exécuter.
         */
        void run(const Plan &plan) const;
    };
}
