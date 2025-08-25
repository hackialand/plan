# Plan.h — API Documentation

This document describes the public API and semantics of the Dualys::Plan class as declared in plan.h. It is suitable for use as reference documentation and as guidance for correct usage.

## Overview

- Dualys::Plan models a virtual environment state as:
    - An optional immutable base plan (shared ancestry)
    - An ordered list of modification layers
- The final state is materialized by starting from the base and applying layers in order.
- Cloning is inexpensive: a clone references the original as its base without a deep-copying state.
- A simple two-way merge is provided for plans that share the same base.

The state is represented as a mapping:
- key: string path
- value: string content hash

## Key Concepts

- Base plan: Optional ancestor plan treated as immutable. If null, the plan is an “initial state.”
- Layer: An ordered collection of file changes (add, modify, remove, etc.). Layers are applied in insertion order.
- Materialization: Computing the final map by applying a plan’s layers on top of its base’s computed state.
- Merge: Combining layers from two sibling plans (with the same base) using last-write-wins.

Note: Dualys::Layer and its change model are used by the API to describe deltas. The plan API does not store file contents, only identifiers (hash strings).

## Class: Dualys::Plan

Represents a virtual environment state. Instances are intended to be shared via smart pointers and cloned frequently. Inherits from std::enable_shared_from_this<Plan> to enable safe self-referencing (used by clone).

### Construction

- Plan(std::string id, std::shared_ptr<const Plan> base)
    - Create a new plan with a unique identifier and an optional base plan.
    - If the base is nullptr, the plan represents an initial empty state.
    - The base is held as std::shared_ptr<const Plan>, signaling immutability by design and allowing structural sharing.

Preconditions:
- id should uniquely identify the plan within your domain.
- If you intend to call clone(), the instance must be owned by a std::shared_ptr (enable_shared_from_this requirement).

Complexity:
- O(1).

### Identification

- const std::string& getId() const
    - Returns the plan’s identifier as provided at construction.

Complexity:
- O(1).

### Mutation: Layers

- void applyLayer(const Layer& new_layer)
    - Appends a new layer to the plan.
    - Layers are applied in the order they are added when materializing the final state.

Notes:
- The plan does not validate the semantics of the layer’s changes (e.g., whether a modified path exists); materialization applies deltas deterministically, with later changes overriding earlier ones on the same path.
- Consider using small, well-scoped layers to keep reasoning and diffs simple.

Complexity:
- Amortized O(1).

Thread-safety:
- Not thread-safe. External synchronization is required for concurrent writers/readers.

### Cloning

- std::unique_ptr<Plan> clone(const std::string& new_id) const
    - Returns a new plan whose base is the current plan.
    - The clone starts with no additional layers; it inherits behavior by referencing this instance as its base.
    - Does not deep-copy the base or layers: O(1).

Preconditions:
- The current plan must be managed by a std::shared_ptr, as the clone internally uses shared_from_this().

Postconditions:
- The returned plan has id == new_id and base == this.
- The two plans are independent for future layer additions.

Complexity:
- O(1).

### Materialization

- std::map<std::string, std::string> getFileSystemState() const
    - Recursively computes the final state:
        - If a base exists, starts from base.getFileSystemState()
        - Applies all local layers in insertion order
    - Change effects:
        - ADDED/MODIFIED: set path -> new content hash
        - REMOVED: erase path
        - PERMISSION_CHANGED: currently ignored unless an extended state is introduced

Return:
- A map representing the finalized view (path -> content hash).

Complexity:
- O(B + L + C) where:
    - B = number of base chain plans
    - L = number of layers across the chain
    - C = number of changes across those layers

Determinism:
- Deterministic given the same base and layer order.

### Merge

- static std::unique_ptr<Plan> merge(const std::string& new_id, const Plan& planA, const Plan& planB)
    - Produces a new plan with the same base as A and B, and with layers equal to A’s layers followed by B’s layers.
    - Conflict policy: last-write-wins due to application order (B after A).
    - If A and B do not share the same base (pointer equality), merge returns nullptr.

Preconditions:
- planA and planB must share the same base pointer (i.e., are siblings).

Postconditions:
- If successful, the merged plan’s base is the shared base of A and B.
- The resulting plan’s state equals materialize(base), then apply(A.layers), then apply(B.layers).

Complexity:
- O(|A.layers| + |B.layers|) to construct the merged plan (not counting materialization).

Limitation:
- No three-way merge with a computed common ancestor for divergent bases.
- No explicit conflict reporting; resolution is implicit by ordering.

## Behavioral Notes

- Immutability by design:
    - The base pointer is to const Plan and generally treated as immutable.
    - Cloning does not duplicate data; it links structure via shared_ptr.
- Determinism:
    - Outcome depends solely on base state and ordered layers.
- Content model:
    - Content hashes are opaque strings; the class does not verify content existence or format.
- Error handling:
    - The API favors simple preconditions and returns nullptr on unsupported merges rather than throwing exceptions.

## Complexity Summary

- Construct, getId, applyLayer, clone: O(1) amortized
- getFileSystemState: proportional to total changes across ancestry
- merge: O(number of layers in A plus B)

## Thread-Safety

- Instances are not thread-safe for concurrent mutation or materialization.
- If multiple threads must read and write plans, protect with external synchronization.
- Read-only usage across threads is safe if the plan graph is not mutated concurrently.

## Usage Examples

- Creating a base plan:
```c++
#include <memory>
#include "Plan.h"

using namespace Dualys;

auto base = std::make_shared<Plan>("base", nullptr);
```


- Adding a layer:
```c++
#include "Layer.h"

Layer init;
init.id = "init";
// init.changes.push_back({"/README.md", ChangeType::ADDED, "hash_v1"});
base->applyLayer(init);
```


- Cloning and branching:
```c++
auto feature = base->clone("feature-A"); // feature’s base is `base`
```


- Materializing:
```c++
auto state = feature->getFileSystemState();
// state is a map<string, string> of path -> content hash
```


- Merging siblings:
```c++
auto a = base->clone("A");
auto b = base->clone("B");
// ... add layers to a and b ...

auto merged = Plan::merge("A+B", *a, *b);
if (!merged) {
    // bases differ: handle error path
}
```


## Best Practices

- Keep layers small and logically grouped to simplify merges and audits.
- Use clone to branch instead of copying states.
- Validate business invariants at higher layers (e.g., ensure MODIFIED targets exist if your domain requires it).
- Consider augmenting the state model if you need permissions/metadata; PERMISSION_CHANGED is a placeholder.
