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
#include <type_traits>

// Вспомогательные функции для создания комплексной единицы
template<typename U>
typename std::enable_if<std::is_same<U, std::complex<double>>::value, expression<U>>::type
make_complex_unit_helper() {
    return expression<U>(std::complex<double>(0, 1));
}

template<typename U>
typename std::enable_if<!std::is_same<U, std::complex<double>>::value, expression<U>>::type
make_complex_unit_helper() {
    throw std::runtime_error("Complex unit 'i' encountered for non-complex type");
}

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

// --- Реализация узла constant_node ---
template<typename T>
constant_node<T>::constant_node(T val) : value(val) {}

template<typename T>
T constant_node<T>::evaluate(const std::map<std::string, T>&) const {
    return value;
}

template<typename T>
std::string constant_node<T>::to_string() const {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

template<typename T>
std::shared_ptr<typename expression<T>::node_base> constant_node<T>::differentiate(const std::string &) const {
    return std::make_shared<constant_node<T>>(T(0));
}

template<typename T>
std::shared_ptr<typename expression<T>::node_base> constant_node<T>::substitute(const std::string &, const std::shared_ptr<typename expression<T>::node_base>&) const {
    return clone();
}

template<typename T>
std::shared_ptr<typename expression<T>::node_base> constant_node<T>::clone() const {
    return std::make_shared<constant_node<T>>(value);
}

// --- Реализация узла variable_node ---
template<typename T>
variable_node<T>::variable_node(const std::string &n) : name(n) {}

template<typename T>
T variable_node<T>::evaluate(const std::map<std::string, T>& vars) const {
    auto it = vars.find(name);
    if(it == vars.end()) throw std::runtime_error("Variable " + name + " not found");
    return it->second;
}

template<typename T>
std::string variable_node<T>::to_string() const {
    return name;
}

template<typename T>
std::shared_ptr<typename expression<T>::node_base> variable_node<T>::differentiate(const std::string &var) const {
    return std::make_shared<constant_node<T>>(name == var ? T(1) : T(0));
}

template<typename T>
std::shared_ptr<typename expression<T>::node_base> variable_node<T>::substitute(const std::string &var, const std::shared_ptr<typename expression<T>::node_base>& val) const {
    if(name == var) return val->clone();
    return clone();
}

template<typename T>
std::shared_ptr<typename expression<T>::node_base> variable_node<T>::clone() const {
    return std::make_shared<variable_node<T>>(name);
}

// --- Реализация узла binary_op_node ---
template<typename T>
binary_op_node<T>::binary_op_node(const std::string &o, std::shared_ptr<typename expression<T>::node_base> l, std::shared_ptr<typename expression<T>::node_base> r)
    : op(o), left(l), right(r) {}

template<typename T>
T binary_op_node<T>::evaluate(const std::map<std::string, T>& vars) const {
    T l_val = left->evaluate(vars), r_val = right->evaluate(vars);
    if(op == "+") return l_val + r_val;
    if(op == "-") return l_val - r_val;
    if(op == "*") return l_val * r_val;
    if(op == "/") return l_val / r_val;
    if(op == "^") return std::pow(l_val, r_val);
    throw std::runtime_error("Unknown operator " + op);
}

template<typename T>
std::string binary_op_node<T>::to_string() const {
    return "(" + left->to_string() + " " + op + " " + right->to_string() + ")";
}

template<typename T>
std::shared_ptr<typename expression<T>::node_base> binary_op_node<T>::differentiate(const std::string &var) const {
    if(op == "+")
        return std::make_shared<binary_op_node<T>>("+", left->differentiate(var), right->differentiate(var));
    if(op == "-")
        return std::make_shared<binary_op_node<T>>("-", left->differentiate(var), right->differentiate(var));
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

template<typename T>
std::shared_ptr<typename expression<T>::node_base> binary_op_node<T>::substitute(const std::string &var, const std::shared_ptr<typename expression<T>::node_base>& val) const {
    auto new_left = left->substitute(var, val);
    auto new_right = right->substitute(var, val);
    return std::make_shared<binary_op_node<T>>(op, new_left, new_right);
}

template<typename T>
std::shared_ptr<typename expression<T>::node_base> binary_op_node<T>::clone() const {
    return std::make_shared<binary_op_node<T>>(op, left->clone(), right->clone());
}

template<typename T>
unary_op_node<T>::unary_op_node(const std::string &o, std::shared_ptr<typename expression<T>::node_base> c)
    : op(o), child(c) {}

template<typename T>
T unary_op_node<T>::evaluate(const std::map<std::string, T>& vars) const {
    T val = child->evaluate(vars);
    if(op == "sin") return std::sin(val);
    if(op == "cos") return std::cos(val);
    if(op == "ln")  return std::log(val);
    if(op == "exp") return std::exp(val);
    throw std::runtime_error("Unknown function " + op);
}

template<typename T>
std::string unary_op_node<T>::to_string() const {
    return op + "(" + child->to_string() + ")";
}

template<typename T>
std::shared_ptr<typename expression<T>::node_base> unary_op_node<T>::differentiate(const std::string &var) const {
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

template<typename T>
std::shared_ptr<typename expression<T>::node_base> unary_op_node<T>::substitute(const std::string &var, const std::shared_ptr<typename expression<T>::node_base>& val) const {
    auto new_child = child->substitute(var, val);
    return std::make_shared<unary_op_node<T>>(op, new_child);
}

template<typename T>
std::shared_ptr<typename expression<T>::node_base> unary_op_node<T>::clone() const {
    return std::make_shared<unary_op_node<T>>(op, child->clone());
}
template<typename T>
ExpressionParserT<T>::ExpressionParserT(const std::string &s) : str(s), pos(0) {}

template<typename T>
void ExpressionParserT<T>::skipWhitespace() {
    while (pos < str.size() && isspace(str[pos])) ++pos;
}

template<typename T>
expression<T> ExpressionParserT<T>::parsePrimary() {
    skipWhitespace();
    if (pos >= str.size())
        throw std::runtime_error("Unexpected end of input");

    // Обработка комплексной единицы 'i'
    if (str[pos] == 'i') {
        ++pos;
        return make_complex_unit_helper<T>();
    }

    if (str[pos] == '(') {
        ++pos;
        auto expr = parseExpression();
        skipWhitespace();
        if (pos >= str.size() || str[pos] != ')')
            throw std::runtime_error("Missing closing parenthesis");
        ++pos;
        return expr;
    }

    if (isalpha(str[pos])) {
        std::string id;
        while (pos < str.size() && isalpha(str[pos])) {
            id.push_back(str[pos]);
            ++pos;
        }
        skipWhitespace();
        if (pos < str.size() && str[pos] == '(') {
            ++pos;
            auto arg = parseExpression();
            skipWhitespace();
            if (pos >= str.size() || str[pos] != ')')
                throw std::runtime_error("Missing closing parenthesis for function");
            ++pos;
            return expression<T>::make_unary(id, arg);
        } else {
            return expression<T>(id);
        }
    }

    if (isdigit(str[pos]) || str[pos] == '.') {
        std::string numStr;
        while (pos < str.size() && (isdigit(str[pos]) || str[pos] == '.')) {
            numStr.push_back(str[pos]);
            ++pos;
        }
        double value = std::stod(numStr);
        return expression<T>(T(value));
    }

    throw std::runtime_error("Unexpected character: " + std::string(1, str[pos]));
}

template<typename T>
expression<T> ExpressionParserT<T>::parseFactor() {
    auto left = parsePrimary();
    skipWhitespace();
    while (pos < str.size() && str[pos] == '^') {
        ++pos;
        auto right = parsePrimary();
        left = left ^ right;
        skipWhitespace();
    }
    return left;
}

template<typename T>
expression<T> ExpressionParserT<T>::parseTerm() {
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

template<typename T>
expression<T> ExpressionParserT<T>::parseExpression() {
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

template<typename T>
expression<T> ExpressionParserT<T>::parse() {
    auto expr = parseExpression();
    skipWhitespace();
    if (pos != str.size())
        throw std::runtime_error("Unexpected characters at end of expression");
    return expr;
}

template class expression<double>;
template class expression<std::complex<double>>;
template class ExpressionParserT<std::complex<double>>;
template class ExpressionParserT<double>;
