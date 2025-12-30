#include "../Public/log.h"
#include <iostream>

void Log::write(const std::string& msg) noexcept {
    if (!msg.empty()) {
        std::cout << msg << std::endl;
    }
}