#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include "head.hpp"

int main(int argc, char **argv) {
    if (argc < 3) {
        std::cerr << "using:\n"
                  << "  differentiator --eval \"statement\" [var=value ...]\n"
                  << "  differentiator --diff \"statement\" --by var\n";
        return 1;
    }

    std::string mode = argv[1];

    try {
        if (mode == "--eval") {
            std::string exprStr = argv[2];
            ExpressionParser parser(exprStr);
            auto expr = parser.parse();

            std::map<std::string, double> vars;
            for (int i = 3; i < argc; ++i) {
                std::string assignment = argv[i];
                size_t pos = assignment.find('=');
                if (pos == std::string::npos) {
                    std::cerr << "ERR Variable: " << assignment << std::endl;
                    return 1;
                }
                std::string var = assignment.substr(0, pos);
                std::string valStr = assignment.substr(pos + 1);
                double value = std::stod(valStr);
                vars[var] = value;
            }

            double result = expr.evaluate(vars);
            std::cout << result << std::endl;
        } else if (mode == "--diff") {
            if (argc < 5) {
                std::cerr << "using differ: differentiator --diff \"выражение\" --by var\n";
                return 1;
            }
            std::string exprStr = argv[2];
            std::string byFlag = argv[3];
            if (byFlag != "--by") {
                std::cerr << "Wait flag '--by', получено: " << byFlag << std::endl;
                return 1;
            }
            std::string diffVar = argv[4];

            ExpressionParser parser(exprStr);
            auto expr = parser.parse();
            auto deriv = expr.differentiate(diffVar);
            std::cout << deriv.to_string() << std::endl;
        } else {
            std::cerr << "Unknown metod: " << mode << std::endl;
            return 1;
        }
    } catch (const std::exception &ex) {
        std::cerr << "ERR: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
