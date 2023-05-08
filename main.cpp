#include "FormulaParser.hpp"
#include <string>
#include <iostream>

int main() {
    FormulaParser parser{};
    std::string input;
    getline(std::cin, input);

    parser.parse(input);
    //std::cout << m;
    
}