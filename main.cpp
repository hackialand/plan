#include <iostream>
#include <ostream>
#include "Plan.h"
using namespace Dualys;

int main(const int argc, const char **argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <file>" << std::endl;
        return 1;
    }
    const auto file_path = argv[1];
    const auto p = new Plan("0",nullptr);
    return 0;
}
