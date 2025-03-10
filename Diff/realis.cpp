#include "head.hpp"
#include <sstream>
#include <cmath>
#include <iostream>
#include <map>
#include <complex>
#include <stdexcept>
#include <cctype>
#include <string>
#include <memory>

// Предварительная декларация структуры unary_op_node
template<typename T>
struct unary_op_node;

// constant_node
template<typename T>
struct constant_node : public expression<T>::node_base {
    T value;
    constant_node(T val): value(val) {}
    T evaluate(const std::map<std::string, T>&) const override { return value; }
    std::string to_string() const override {
        std::ostringstream oss;
        oss << value;
        return oss.str();
    }
    std::shared_ptr<typename expression<T>::node_base> differentiate(const std::string &) const override {
        return std::make_shared<constant_node<T>>(T(0));
    }
    std::shared_ptr<typename expression<T>::node_base> substitute(const std::string &, const std::shared_ptr<typename expression<T>::node_base>&) const override {
        return clone();
    }
    std::shared_ptr<typename expression<T>::node_base> clone() const override {
        return std::make_shared<constant_node<T>>(value);
    }
};

// variable_node
template<typename T>
struct variable_node : public expression<T>::node_base {
    std::string name;
    variable_node(const std::string &n): name(n) {}
    T evaluate(const std::map<std::string, T>& vars) const override {
        auto it = vars.find(name);
        if(it == vars.end()) throw std::runtime_error("Variable " + name + " not found");
        return it->second;
    }
    std::string to_string() const override { return name; }
    std::shared_ptr<typename expression<T>::node_base> differentiate(const std::string &var) const override {
        return std::make_shared<constant_node<T>>(name == var ? T(1) : T(0));
    }
    std::shared_ptr<typename expression<T>::node_base> substitute(const std::string &var, const std::shared_ptr<typename expression<T>::node_base>& val) const override {
        if(name == var) return val->clone();
        return clone();
    }
    std::shared_ptr<typename expression<T>::node_base> clone() const override {
        return std::make_shared<variable_node<T>>(name);
    }
};

// binary_op_node
template<typename T>
struct binary_op_node : public expression<T>::node_base {
    std::string op;
    std::shared_ptr<typename expression<T>::node_base> left;
    std::shared_ptr<typename expression<T>::node_base> right;
    binary_op_node(const std::string &o, std::shared_ptr<typename expression<T>::node_base> l, std::shared_ptr<typename expression<T>::node_base> r)
        : op(o), left(l), right(r) {}
    T evaluate(const std::map<std::string, T>& vars) const override {
        T l_val = left->evaluate(vars), r_val = right->evaluate(vars);
        if(op == "+") return l_val + r_val;
        if(op == "-") return l_val - r_val;
        if(op == "*") return l_val * r_val;
        if(op == "/") return l_val / r_val;
        if(op == "^") return std::pow(l_val, r_val);
        throw std::runtime_error("Unknown operator " + op);
    }
    std::string to_string() const override {
        return "(" + left->to_string() + " " + op + " " + right->to_string() + ")";
    }
    std::shared_ptr<typename expression<T>::node_base> differentiate(const std::string &var) const override {
        if(op == "+") {
            return std::make_shared<binary_op_node<T>>("+", left->differentiate(var), right->differentiate(var));
        }
        if(op == "-") {
            return std::make_shared<binary_op_node<T>>("-", left->differentiate(var), right->differentiate(var));
        }
        if(op == "*") {
            auto left_diff = std::make_shared<binary_op_node<T>>("*", left->differentiate(var), right->clone());
            auto right_diff = std::make_shared<binary_op_node<T>>("*", left->clone(), right->differentiate(var));
            return std::make_shared<binary_op_node<T>>("+", left_diff, right_diff);
        }
        if(op == "/") {
            auto num_left = std::make_shared<binary_op_node<T>>("*", left->differentiate(var), right->clone());
            auto num_right = std::make_shared<binary_op_node<T>>("*", left->clone(), right->differentiate(var));
            auto numerator = std::make_shared<binary_op_node<T>>("-", num_left, num_right);
            auto denominator = std::make_shared<binary_op_node<T>>("^", right->clone(), std::make_shared<constant_node<T>>(T(2)));
            return std::make_shared<binary_op_node<T>>("/", numerator, denominator);
        }
        if(op == "^") {
            auto u = left;
            auto v = right;
            auto u_diff = left->differentiate(var);
            auto v_diff = right->differentiate(var);
            auto ln_u = std::make_shared<unary_op_node<T>>("ln", u->clone());
            auto term1 = std::make_shared<binary_op_node<T>>("*", v_diff, ln_u);
            auto term2 = std::make_shared<binary_op_node<T>>("/", std::make_shared<binary_op_node<T>>("*", v->clone(), u_diff), u->clone());
            auto sum_terms = std::make_shared<binary_op_node<T>>("+", term1, term2);
            auto u_pow_v = clone();
            return std::make_shared<binary_op_node<T>>("*", u_pow_v, sum_terms);
        }
        throw std::runtime_error("Differentiation not implemented for operator " + op);
    }
    std::shared_ptr<typename expression<T>::node_base> substitute(const std::string &var, const std::shared_ptr<typename expression<T>::node_base>& val) const override {
        auto new_left = left->substitute(var, val);
        auto new_right = right->substitute(var, val);
        return std::make_shared<binary_op_node<T>>(op, new_left, new_right);
    }
    std::shared_ptr<typename expression<T>::node_base> clone() const override {
        return std::make_shared<binary_op_node<T>>(op, left->clone(), right->clone());
    }
};

// unary_op_node
template<typename T>
struct unary_op_node : public expression<T>::node_base {
    std::string op;
    std::shared_ptr<typename expression<T>::node_base> child;
    unary_op_node(const std::string &o, std::shared_ptr<typename expression<T>::node_base> c)
        : op(o), child(c) {}
    T evaluate(const std::map<std::string, T>& vars) const override {
        T val = child->evaluate(vars);
        if(op == "sin") return std::sin(val);
        if(op == "cos") return std::cos(val);
        if(op == "ln") return std::log(val);
        if(op == "exp") return std::exp(val);
        throw std::runtime_error("Unknown function " + op);
    }
    std::string to_string() const override {
        return op + "(" + child->to_string() + ")";
    }
    std::shared_ptr<typename expression<T>::node_base> differentiate(const std::string &var) const override {
        if(op == "sin") {
            auto deriv = child->differentiate(var);
            return std::make_shared<binary_op_node<T>>("*", std::make_shared<unary_op_node<T>>("cos", child->clone()), deriv);
        }
        if(op == "cos") {
            auto deriv = child->differentiate(var);
            auto sin_node = std::make_shared<unary_op_node<T>>("sin", child->clone());
            auto neg_sin = std::make_shared<binary_op_node<T>>("*", std::make_shared<constant_node<T>>(T(-1)), sin_node);
            return std::make_shared<binary_op_node<T>>("*", neg_sin, deriv);
        }
        if(op == "ln") {
            auto deriv = child->differentiate(var);
            return std::make_shared<binary_op_node<T>>("/", deriv, child->clone());
        }
        if(op == "exp") {
            auto deriv = child->differentiate(var);
            return std::make_shared<binary_op_node<T>>("*", clone(), deriv);
        }
        throw std::runtime_error("Differentiation not implemented for function " + op);
    }
    std::shared_ptr<typename expression<T>::node_base> substitute(const std::string &var, const std::shared_ptr<typename expression<T>::node_base>& val) const override {
        auto new_child = child->substitute(var, val);
        return std::make_shared<unary_op_node<T>>(op, new_child);
    }
    std::shared_ptr<typename expression<T>::node_base> clone() const override {
        return std::make_shared<unary_op_node<T>>(op, child->clone());
    }
};

// Реализация методов класса expression

template<typename T>
expression<T>::expression(T value)
    : root_(std::make_shared<constant_node<T>>(value)) {}

template<typename T>
expression<T>::expression(const std::string &var_name)
    : root_(std::make_shared<variable_node<T>>(var_name)) {}

template<typename T>
expression<T>::expression(const expression &other)
    : root_(other.root_->clone()) {}

template<typename T>
expression<T>::expression(expression &&other) noexcept
    : root_(std::move(other.root_)) {}

template<typename T>
expression<T>& expression<T>::operator=(const expression &other) {
    if(this != &other) {
        root_ = other.root_->clone();
    }
    return *this;
}

template<typename T>
expression<T>& expression<T>::operator=(expression &&other) noexcept {
    if(this != &other) {
        root_ = std::move(other.root_);
    }
    return *this;
}

template<typename T>
expression<T>::~expression() {}

template<typename T>
std::string expression<T>::to_string() const {
    return root_->to_string();
}

template<typename T>
T expression<T>::evaluate(const std::map<std::string, T> &variables) const {
    return root_->evaluate(variables);
}

template<typename T>
expression<T> expression<T>::differentiate(const std::string &var) const {
    return expression(root_->differentiate(var));
}

template<typename T>
expression<T> expression<T>::substitute(const std::string &var, const expression &value) const {
    return expression(root_->substitute(var, value.root_));
}

template<typename T>
expression<T> expression<T>::operator+(const expression &other) const {
    return expression(std::make_shared<binary_op_node<T>>("+", root_->clone(), other.root_->clone()));
}

template<typename T>
expression<T> expression<T>::operator-(const expression &other) const {
    return expression(std::make_shared<binary_op_node<T>>("-", root_->clone(), other.root_->clone()));
}

template<typename T>
expression<T> expression<T>::operator*(const expression &other) const {
    return expression(std::make_shared<binary_op_node<T>>("*", root_->clone(), other.root_->clone()));
}

template<typename T>
expression<T> expression<T>::operator/(const expression &other) const {
    return expression(std::make_shared<binary_op_node<T>>("/", root_->clone(), other.root_->clone()));
}

template<typename T>
expression<T> expression<T>::operator^(const expression &other) const {
    return expression(std::make_shared<binary_op_node<T>>("^", root_->clone(), other.root_->clone()));
}

template<typename T>
expression<T>::expression(std::shared_ptr<node_base> node)
    : root_(node) {}

template<typename T>
expression<T> expression<T>::make_unary(const std::string &op, const expression &operand) {
    return expression(std::make_shared<unary_op_node<T>>(op, operand.root_->clone()));
}

template class expression<double>;
template class expression<std::complex<double>>;

// ==================================================
// Реализация методов класса ExpressionParser
// ==================================================

// Определения методов вынесены из объявления (head.hpp)

ExpressionParser::ExpressionParser(const std::string &s)
    : str(s), pos(0) {}

void ExpressionParser::skipWhitespace() {
    while (pos < str.size() && isspace(str[pos])) ++pos;
}

expression<double> ExpressionParser::parsePrimary() {
    skipWhitespace();
    if (pos >= str.size())
        throw std::runtime_error("Unexpected end of input");

    // Обработка скобок: (expr)
    if (str[pos] == '(') {
        ++pos; // пропускаем '('
        auto expr = parseExpression();
        skipWhitespace();
        if (pos >= str.size() || str[pos] != ')')
            throw std::runtime_error("Missing closing parenthesis");
        ++pos; // пропускаем ')'
        return expr;
    }

    // Если идентификатор начинается с буквы: переменная или функция
    if (isalpha(str[pos])) {
        std::string id;
        while (pos < str.size() && isalpha(str[pos])) {
            id.push_back(str[pos]);
            ++pos;
        }
        skipWhitespace();
        // Если после идентификатора идёт '(' — функция
        if (pos < str.size() && str[pos] == '(') {
            ++pos; // пропускаем '('
            auto arg = parseExpression();
            skipWhitespace();
            if (pos >= str.size() || str[pos] != ')')
                throw std::runtime_error("Missing closing parenthesis for function");
            ++pos; // пропускаем ')'
            return expression<double>::make_unary(id, arg);
        } else {
            // иначе переменная
            return expression<double>(id);
        }
    }

    // Обработка чисел
    if (isdigit(str[pos]) || str[pos] == '.') {
        std::string numStr;
        while (pos < str.size() && (isdigit(str[pos]) || str[pos] == '.')) {
            numStr.push_back(str[pos]);
            ++pos;
        }
        double value = std::stod(numStr);
        return expression<double>(value);
    }

    throw std::runtime_error("Unexpected character: " + std::string(1, str[pos]));
}

expression<double> ExpressionParser::parseFactor() {
    auto left = parsePrimary();
    skipWhitespace();
    while (pos < str.size() && str[pos] == '^') {
        ++pos; // пропускаем '^'
        auto right = parsePrimary();
        left = left ^ right;
        skipWhitespace();
    }
    return left;
}

expression<double> ExpressionParser::parseTerm() {
    auto left = parseFactor();
    skipWhitespace();
    while (pos < str.size() && (str[pos] == '*' || str[pos] == '/')) {
        char op = str[pos];
        ++pos;
        auto right = parseFactor();
        if (op == '*')
            left = left * right;
        else
            left = left / right;
        skipWhitespace();
    }
    return left;
}

expression<double> ExpressionParser::parseExpression() {
    auto left = parseTerm();
    skipWhitespace();
    while (pos < str.size() && (str[pos] == '+' || str[pos] == '-')) {
        char op = str[pos];
        ++pos;
        auto right = parseTerm();
        if (op == '+')
            left = left + right;
        else
            left = left - right;
        skipWhitespace();
    }
    return left;
}

expression<double> ExpressionParser::parse() {
    auto expr = parseExpression();
    skipWhitespace();
    if (pos != str.size())
        throw std::runtime_error("Unexpected characters at end of expression");
    return expr;
}

/*
int main() {
    // Пример использования:
    try {
        ExpressionParser parser("sin(3.14) + 2*(x - 1)");
        auto expr = parser.parse();
        std::cout << "Parsed expression: " << expr.to_string() << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}
*/
