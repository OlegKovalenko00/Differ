#include <iostream>
#include <stdexcept>
#include <cmath>
#include "head.hpp"

bool nearlyEqual(double a, double b, double epsilon = 1e-9) {
    return std::fabs(a - b) < epsilon;
}

template<typename TestFunc>
void run_test(const std::string &test_name, TestFunc func) {
    try {
        func();
        std::cout << test_name << ": OK" << std::endl;
    } catch (const std::exception &e) {
        std::cout << test_name << ": FAIL (" << e.what() << ")" << std::endl;
    } catch (...) {
        std::cout << test_name << ": FAIL (неизвестная ошибка)" << std::endl;
    }
}

int main() {
    run_test("Test Constant Evaluation", [](){
        expression<double> expr(3.14);
        double val = expr.evaluate({});
        if (!nearlyEqual(val, 3.14))
            throw std::runtime_error("Ожидалось 3.14, получено " + std::to_string(val));
    });

    run_test("Test Variable Evaluation", [](){
        expression<double> expr("x");
        double val = expr.evaluate({{"x", 2.718}});
        if (!nearlyEqual(val, 2.718))
            throw std::runtime_error("Ожидалось 2.718, получено " + std::to_string(val));
    });

    run_test("Test Binary Operations", [](){
        expression<double> expr1(5);
        expression<double> expr2(3);
        expression<double> expr_add = expr1 + expr2;
        if (!nearlyEqual(expr_add.evaluate({}), 8))
            throw std::runtime_error("Addition failed");
        expression<double> expr_sub = expr1 - expr2;
        if (!nearlyEqual(expr_sub.evaluate({}), 2))
            throw std::runtime_error("Subtraction failed");
        expression<double> expr_mul = expr1 * expr2;
        if (!nearlyEqual(expr_mul.evaluate({}), 15))
            throw std::runtime_error("Multiplication failed");
        expression<double> expr_div = expr1 / expr2;
        if (!nearlyEqual(expr_div.evaluate({}), 5.0 / 3.0))
            throw std::runtime_error("Division failed");
        expression<double> expr_pow = expr1 ^ expr2;
        if (!nearlyEqual(expr_pow.evaluate({}), 125))
            throw std::runtime_error("Exponentiation failed");
    });

    run_test("Test Unary Operations", [](){
        auto expr_sin = expression<double>::make_unary("sin", expression<double>(0));
        if (!nearlyEqual(expr_sin.evaluate({}), 0))
            throw std::runtime_error("sin(0) failed");
        auto expr_cos = expression<double>::make_unary("cos", expression<double>(0));
        if (!nearlyEqual(expr_cos.evaluate({}), 1))
            throw std::runtime_error("cos(0) failed");
        auto expr_ln = expression<double>::make_unary("ln", expression<double>(1));
        if (!nearlyEqual(expr_ln.evaluate({}), 0))
            throw std::runtime_error("ln(1) failed");
        auto expr_exp = expression<double>::make_unary("exp", expression<double>(0));
        if (!nearlyEqual(expr_exp.evaluate({}), 1))
            throw std::runtime_error("exp(0) failed");
    });

    run_test("Test Differentiation", [](){
        expression<double> expr("x");
        expression<double> expr_squared = expr * expr;
        expression<double> deriv = expr_squared.differentiate("x");
        double result = deriv.evaluate({{"x", 3}});
        if (!nearlyEqual(result, 6))
            throw std::runtime_error("Differentiation failed, получено " + std::to_string(result));
    });

    run_test("Test Precedence with Parentheses", [](){
        ExpressionParserT<double> parser("2 * (3 + 4)");
        auto expr = parser.parse();
        double result = expr.evaluate({});
        if (!nearlyEqual(result, 14))
            throw std::runtime_error("Ожидалось 14, получено " + std::to_string(result));
    });

    run_test("Test Function Composition", [](){
        ExpressionParserT<double> parser("sin(cos(0))");
        auto expr = parser.parse();
        double result = expr.evaluate({});
        double expected = std::sin(std::cos(0));
        if (!nearlyEqual(result, expected))
            throw std::runtime_error("Ожидалось sin(cos(0)) = " + std::to_string(expected) + ", получено " + std::to_string(result));
    });

    run_test("Test Parser Unmatched Parenthesis", [](){
        try {
            ExpressionParserT<double> parser("3 + (4 * 2");
            auto expr = parser.parse();
            throw std::runtime_error("Ожидалась ошибка из-за незакрытой скобки");
        } catch (const std::runtime_error &e) {
        }
    });

    run_test("Test Parser Unknown Token", [](){
        try {
            ExpressionParserT<double> parser("3 + $");
            auto expr = parser.parse();
            throw std::runtime_error("Ожидалась ошибка из-за неизвестного символа");
        } catch (const std::runtime_error &e) {
        }
    });

    run_test("Test Substitution", [](){
        ExpressionParserT<double> parser("x^2");
        auto expr = parser.parse();
        auto substituted = expr.substitute("x", expression<double>(3));
        double result = substituted.evaluate({});
        if (!nearlyEqual(result, 9))
            throw std::runtime_error("Ожидалось 9, получено " + std::to_string(result));
    });

    run_test("Test Chained Operations", [](){
        ExpressionParserT<double> parser("1 + 2 * 3 - 4 / 2");
        auto expr = parser.parse();
        double result = expr.evaluate({});
        if (!nearlyEqual(result, 5))
            throw std::runtime_error("Ожидалось 5, получено " + std::to_string(result));
    });

    run_test("Test Exponentiation Associativity", [](){
        ExpressionParserT<double> parser("2^3^2");
        auto expr = parser.parse();
        double result = expr.evaluate({});
        if (!nearlyEqual(result, 64))
            throw std::runtime_error("Ожидалось 64 (left-associative), получено " + std::to_string(result));
    });

    run_test("Test Complex Expression Evaluation", [](){
        ExpressionParserT<double> parser("3 + sin(0) * (2 + x) - ln(exp(1))");
        auto expr = parser.parse();
        double result = expr.evaluate({{"x", 5}});
        if (!nearlyEqual(result, 2))
            throw std::runtime_error("Ожидалось 2, получено " + std::to_string(result));
    });

    run_test("Test Differentiation Output sin(x)", [](){
        ExpressionParserT<double> parser("sin(x)");
        auto expr = parser.parse();
        auto deriv = expr.differentiate("x");
        std::cout << "Derivative of sin(x): " << deriv.to_string() << std::endl;
    });

    run_test("Test Differentiation Output x^2", [](){
        ExpressionParserT<double> parser("x^2");
        auto expr = parser.parse();
        auto deriv = expr.differentiate("x");
        std::cout << "Derivative of x^2: " << deriv.to_string() << std::endl;
    });

    run_test("Test Differentiation Output exp(x)", [](){
        ExpressionParserT<double> parser("exp(x)");
        auto expr = parser.parse();
        auto deriv = expr.differentiate("x");
        std::cout << "Derivative of exp(x): " << deriv.to_string() << std::endl;
    });

    run_test("Test Differentiation Output ln(x)", [](){
        ExpressionParserT<double> parser("ln(x)");
        auto expr = parser.parse();
        auto deriv = expr.differentiate("x");
        std::cout << "Derivative of ln(x): " << deriv.to_string() << std::endl;
    });

    run_test("Test Differentiation Output x^3 + 2*x", [](){
        ExpressionParserT<double> parser("x^3 + 2*x");
        auto expr = parser.parse();
        auto deriv = expr.differentiate("x");
        std::cout << "Derivative of x^3 + 2*x: " << deriv.to_string() << std::endl;
    });

    return 0;
}
