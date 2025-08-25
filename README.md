# Plan

## Table of Contents
- [Overview](#overview)
- [Core Concepts](#core-concepts)
- [Data Model](#data-model)
- [Class Reference](#class-reference)
- [Life Cycle of a Plan](#life-cycle-of-a-plan)
- [Typical Workflows](#typical-workflows)
- [Merge Semantics](#merge-semantics)
- [Error Handling and Constraints](#error-handling-and-constraints)
- [Performance Characteristics](#performance-characteristics)
- [Thread-Safety](#thread-safety)
- [Extensibility](#extensibility)
- [Build and Install](#build-and-install)
- [Getting Started Example](#getting-started-example)
- [Limitations and Future Work](#limitations-and-future-work)
- [FAQ](#faq)

## Overview
Plan is a lightweight framework to model and manipulate a virtual application environment (e.g., a virtual filesystem state) through immutable bases and composable layers of changes. It allows:
- Inexpensive cloning of environments
- Deterministic materialization of the final state
- Simple two-way merge of plans sharing the same base
- Pluggable execution strategies for running a plan (e.g., WASM)

The project targets C++26.

## Core Concepts

- Plan
    - Represents an environment identified by an ID.
    - Has an optional immutable base plan.
    - Holds an ordered list of layers (delta changes).
    - Materializes a final state by starting from its base’s state and applying its layers in order.

- Layer
    - An ordered collection of file changes.
    - Each change describes a path, a change type, and optionally the new content hash.

- Change Types
    - ADDED: Introduce a new file/path with content hash.
    - MODIFIED: Update an existing file/path with a new content hash.
    - REMOVED: Delete a file/path from the state.
    - PERMISSION_CHANGED: Placeholder for future metadata change handling.

- Execution Strategy
    - A pluggable interface to execute a plan.
    - Example: WasmStrategy to run WASM-based content.

## Data Model

- State Representation: map<string path, string content_hash>
    - Materialized by Plan::getFileSystemState()
    - Deterministic and derived from base plus applied layers (in order).

- Immutability by Convention:
    - A plan’s base is shared and treated as immutable.
    - Layers appended to a plan do not mutate its base.

## Class Reference

- Dualys::Layer
    - Fields:
        - std::vector<FileChange> changes
        - std::string id
    - FileChange:
        - std::string path
        - ChangeType type
        - std::string new_content_hash (used for ADDED/MODIFIED)

- Dualys::Plan
    - Constructor: Plan(std::string id, std::shared_ptr<const Plan> base)
    - Methods:
        - const std::string& getId() const
        - void applyLayer(const Layer& new_layer)
        - std::unique_ptr<Plan> clone(const std::string& new_id) const
            - Creates a new plan whose base is the current plan (inexpensive clone).
        - std::map<std::string, std::string> getFileSystemState() const
            - Materializes the final state by recursively accumulating the base state and applying local layers.
        - static std::unique_ptr<Plan> merge(const std::string& new_id, const Plan& planA, const Plan& planB)
            - Requires both plans to share the same base.
            - Conflict policy: last-write-wins by applying planB’s layers after planA’s layers.

- Dualys::IExecutionStrategy
    - Interface with: void execute(const Plan& plan) const = 0

- Dualys::WasmStrategy
    - Implements IExecutionStrategy::execute for WASM execution.

- Dualys::ExecutionEngine
    - Methods:
        - void setStrategy(std::unique_ptr<IExecutionStrategy> strategy)
        - void run(const Plan& plan) const

## Life Cycle of a Plan
1. Create an initial plan with a null base (the “initial state 0”).
2. Append layers to record changes over time.
3. Clone to branch into a new plan cheaply; the original becomes the base of the clone.
4. Merge sibling plans that share the same base (two-way merge).
5. Materialize with getFileSystemState() to get the final view.

## Typical Workflows

- Creating a base plan:
```c++
#include <memory>
#include "Plan.h"

using namespace Dualys;

auto base = std::make_shared<Plan>("base", nullptr);
```


- Applying a layer:
```c++
#include "Layer.h"

Layer l;
l.id = "init";
l.changes.push_back(FileChange{
    .path = "/README.md",
    .type = ChangeType::ADDED,
    .new_content_hash = "hash_readme_v1"
});
base->applyLayer(l);
```


- Cloning to branch:
```c++
auto feature = base->clone("feature-1");
// Now feature’s base is the original `base` plan.
```


- Modifying the branch:
```c++
Layer l2;
l2.id = "feature-change";
l2.changes.push_back(FileChange{
    .path = "/README.md",
    .type = ChangeType::MODIFIED,
    .new_content_hash = "hash_readme_v2"
});
feature->applyLayer(l2);
```


- Materializing final state:
```c++
auto state = feature->getFileSystemState();
// state["/README.md"] == "hash_readme_v2"
```


- Merging two siblings:
```c++
auto a = base->clone("A");
auto b = base->clone("B");

// ... apply layers to a and b ...

auto merged = Plan::merge("A+B", *a, *b);
if (!merged) {
    // Handle incompatible bases
}
```


- Executing a plan:
```c++
#include "ExecutionEngine.h"

ExecutionEngine engine;
engine.setStrategy(std::make_unique<WasmStrategy>());
engine.run(*feature);
```


## Merge Semantics

- Precondition: planA.m_base_plan == planB.m_base_plan
- Process:
    1. Create a new plan with the common base.
    2. Apply layers of A in order.
    3. Apply layers of B in order.
- Effect: For overlapping paths, B’s later changes win (“last write wins”).
- Unsupported: Divergent bases (would require a three-way merge approach with a common ancestor and conflict resolution).

## Error Handling and Constraints

- Clone requires that the plan instance is managed by a shared_ptr because it uses shared_from_this().
- getFileSystemState:
    - ADDED/MODIFIED expect new_content_hash to be meaningful; missing or empty values should be treated carefully in client code if used beyond hashing.
- merge returns nullptr when bases are incompatible.

## Performance Characteristics

- Cloning is O(1) and does not copy heavy data.
- Materialization cost is proportional to:
    - Depth of the base chain + total number of changes across all layers.
- std::map is used for deterministic ordering; consider the trade-offs versus unordered_map.

## Thread-Safety

- The Plan class is not inherently thread-safe.
- If multiple threads interact with the same Plan instance, external synchronization is required.
- Read-only operations on a fully constructed Plan can be safe if no concurrent mutation occurs.

## Extensibility

- Execution Strategies:
    - Implement IExecutionStrategy to add new execution modes (JIT, Unikernels, interpreters, etc.).
- Metadata and Permissions:
    - Extend state representation to include metadata if PERMISSION_CHANGED or other attributes need concrete behavior.
- Advanced Merging:
    - Introduce three-way merges and conflict descriptors for richer VCS-like behaviors.

## Build and Install

- Requirements:
    - CMake (project sets CMAKE_CXX_STANDARD 26)
    - A C++26-capable compiler

- Build:
    - Library target: Plan (static)
    - Executable target: plan

- Install:
    - Installs the library to lib, header to include, and the executable to bin.

Example build steps:
```shell script
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
cmake --install build --prefix /your/prefix
```


## Getting Started Example

```c++
#include <iostream>
#include <memory>
#include "Plan.h"
#include "Layer.h"
#include "ExecutionEngine.h"

using namespace Dualys;

int main() {
    auto base = std::make_shared<Plan>("base", nullptr);

    Layer init;
    init.id = "init";
    init.changes.push_back({"/app/main.wasm", ChangeType::ADDED, "hash_wasm_v1"});
    base->applyLayer(init);

    auto feature = base->clone("feature-A");
    Layer patch;
    patch.id = "patch";
    patch.changes.push_back({"/app/main.wasm", ChangeType::MODIFIED, "hash_wasm_v2"});
    feature->applyLayer(patch);

    auto state = feature->getFileSystemState();
    std::cout << "main.wasm -> " << state["/app/main.wasm"] << "\n";

    ExecutionEngine engine;
    engine.setStrategy(std::make_unique<WasmStrategy>());
    engine.run(*feature);

    return 0;
}
```


## Limitations and Future Work

- Merge is limited to plans sharing the same base.
- PERMISSION_CHANGED is a placeholder; a richer state model is needed for permissions/metadata.
- No built-in content store; hashes are treated as opaque identifiers.
- No concurrency primitives within Plan; users must add synchronization if needed.

## FAQ

- Why use base + layers instead of copying states?
    - To enable inexpensive branching, fast cloning, and minimal duplication of unchanged data.

- How are conflicts resolved in merges?
    - Last write wins by ordering the application of layers; more advanced strategies can be introduced later.

- Can I persist plans?
    - The model is designed to be serializable, but persistence is outside the current scope and left to integrators.
