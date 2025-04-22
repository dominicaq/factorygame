#pragma once

#include <json/json.hpp>
#include <iostream>

using json = nlohmann::json;

int main() {
    json j = {
        {"name", "Factory"},
        {"version", 1.0}
    };

    std::cout << j.dump(2) << std::endl;
}
