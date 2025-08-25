#pragma once
#include <iostream>
#include <string>
#include <vector>

namespace Dualys {
    enum class ChangeType {
        ADDED,
        MODIFIED,
        REMOVED
    };
    struct FileChange {
        std::string path;
        ChangeType type;
        std::string new_content_hash;
    };

    class Layer {
    public:
        std::vector<FileChange> changes;
    };
}
