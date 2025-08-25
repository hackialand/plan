#pragma once
#include <iostream>
#include <string>
#include <vector>

namespace Dualys {
    /**
     *
     * @enum ChangeType
     *
     * Represents the type of change affecting a file.
     *
     * Possible values:
     * - ADDED: The file was added to the system.
     * - MODIFIED: The file was modified.
     * - REMOVED: The file was removed from the system.
     * - PERMISSION_CHANGED: The file's permissions were changed.
     */
    enum class ChangeType {
        ADDED,
        MODIFIED,
        REMOVED,
        PERMISSION_CHANGED
    };

    /**
     *
     * @struct FileChange
     *
     * Represents a change made to a file in the system.
     *
     * Attributes:
     * - path: The path of the file that has changed.
     * - type: The type of change affecting the file, represented as a value of the ChangeType enum.
     * - new_content_hash: The hash has been replaced with new content, if applicable.
     *
     */
    struct FileChange {
        std::string path;
        ChangeType type;
        std::string new_content_hash;
    };

    /**
     *
     * @class Layer
     *
     * Represents a set of changes applied to a system or repository.
     *
     * A Layer encapsulates a collection of modifications related to files,
     * represented as `FileChange` instances, and is uniquely identifiable
     * by its `id`.
     */
    class Layer {
    public:
        /**
         *
         * @var changes
         *
         * Represent a collection of file changes associated with a Layer.
         *
         * Each entry in the vector corresponds to a specific file alteration,
         * such as addition, modification, or deletion, encapsulated by the
         * `FileChange` structure.
         *
         */
        std::vector<FileChange> changes;

        /**
         *
         * @var id
         *
         * Represents the unique identifier associated with the Layer.
         *
         * This identifier serves as a primary reference for distinguishing
         * a specific Layer within a system or repository.
         *
         */
        std::string id;
    };
}
