#include "FormulaParser.hpp"
#include <string>
#include <iostream>

int main() {
    FormulaParser parser{};
    std::string input;
    getline(std::cin, input);

    auto m = parser.parse(input);
    std::cout << m;
    
}