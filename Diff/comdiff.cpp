#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <complex>
#include <stdexcept>
#include <cctype>
#include "head.hpp"

std::string removeSpaces(const std::string& s) {
    std::string res;
    for (char c : s)
        if (!std::isspace(static_cast<unsigned char>(c)))
            res.push_back(c);
    return res;
}

std::complex<double> parseComplex(const std::string& s) {
    std::string str = removeSpaces(s);
    if (str.empty())
        throw std::runtime_error("Empty complex number string");

    size_t pos_i = str.find('i');
    if (pos_i == std::string::npos) {
        double real = std::stod(str);
        return std::complex<double>(real, 0);
    }

    // Если строка равна "i", "+i" или "-i"
    if (str == "i" || str == "+i")
        return std::complex<double>(0, 1);
    if (str == "-i")
        return std::complex<double>(0, -1);
    std::string without_i = str;
    without_i.erase(pos_i, 1);

    size_t pos_sign = std::string::npos;
    for (size_t i = 1; i < without_i.size(); ++i) {
        if (without_i[i] == '+' || without_i[i] == '-')
            pos_sign = i;
    }

    double realPart = 0.0, imagPart = 0.0;
    if (pos_sign == std::string::npos) {

        imagPart = std::stod(without_i);
    } else {
        std::string realStr = without_i.substr(0, pos_sign);
        std::string imagStr = without_i.substr(pos_sign);
        if (!realStr.empty())
            realPart = std::stod(realStr);
        if (imagStr == "+")
            imagPart = 1;
        else if (imagStr == "-")
            imagPart = -1;
        else
            imagPart = std::stod(imagStr);
    }
    return std::complex<double>(realPart, imagPart);
}

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
            bool useComplex = false;
            for (int i = 3; i < argc; ++i) {
                std::string assignment = argv[i];
                size_t pos = assignment.find('=');
                if (pos == std::string::npos) {
                    std::cerr << "ERR Variable: " << assignment << std::endl;
                    return 1;
                }
                std::string valStr = assignment.substr(pos + 1);
                if (valStr.find('i') != std::string::npos) {
                    useComplex = true;
                    break;
                }
            }

            if (useComplex) {
                ExpressionParserT<std::complex<double>> parser(exprStr);
                auto expr = parser.parse();

                std::map<std::string, std::complex<double>> vars;
                for (int i = 3; i < argc; ++i) {
                    std::string assignment = argv[i];
                    size_t pos = assignment.find('=');
                    if (pos == std::string::npos) {
                        std::cerr << "ERR Variable: " << assignment << std::endl;
                        return 1;
                    }
                    std::string var = assignment.substr(0, pos);
                    std::string valStr = assignment.substr(pos + 1);
                    if (valStr.find('i') != std::string::npos)
                        vars[var] = parseComplex(valStr);
                    else {
                        double value = std::stod(valStr);
                        vars[var] = std::complex<double>(value, 0);
                    }
                }
                auto result = expr.evaluate(vars);
                std::cout << result << std::endl;
            } else {
                ExpressionParserT<double> parser(exprStr);
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
            }
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

            ExpressionParserT<double> parser(exprStr);
            auto expr = parser.parse();
            auto deriv = expr.differentiate(diffVar);
            std::cout << deriv.to_string() << std::endl;
        } else {
            std::cerr << "Unknown method: " << mode << std::endl;
            return 1;
        }
    } catch (const std::exception &ex) {
        std::cerr << "ERR: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
