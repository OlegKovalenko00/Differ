#include "head.hpp"
#include <sstream>
#include <cmath>

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

// Методы класса expression

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
expression<T> &expression<T>::operator=(const expression &other) {
    if(this != &other) {
        root_ = other.root_->clone();
    }
    return *this;
}

template<typename T>
expression<T> &expression<T>::operator=(expression &&other) noexcept {
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

#include <iostream>
#include <map>
#include <complex>

int main() {
    // Тест 1: f(x) = ln(x^2) + 3, производная f'(x) = (2*x)/(x^2)
    expression<double> x("x");
    expression<double> expr1 = ln(x * x) + expression<double>(3.0);
    auto dexpr1 = expr1.differentiate("x");
    std::map<std::string, double> vars1 = {{"x", 2.0}};
    std::cout << "Test 1:\n";
    std::cout << "f(x) = " << expr1.to_string() << "\n";
    std::cout << "f'(x) = " << dexpr1.to_string() << "\n";
    std::cout << "f(2) = " << expr1.evaluate(vars1) << "\n\n";

    // Тест 2: g(x) = sin(x) + cos(x), производная g'(x) = cos(x) - sin(x)
    expression<double> expr2 = sin(x) + cos(x);
    auto dexpr2 = expr2.differentiate("x");
    std::map<std::string, double> vars2 = {{"x", 3.0}};
    std::cout << "Test 2:\n";
    std::cout << "g(x) = " << expr2.to_string() << "\n";
    std::cout << "g'(x) = " << dexpr2.to_string() << "\n";
    std::cout << "g(3) = " << expr2.evaluate(vars2) << "\n\n";

    // Тест 3: h(x,y) = x^2 + y, подстановка y = 5, h(2,5) = 4+5 = 9
    expression<double> y("y");
    expression<double> expr3 = (x * x) + y;
    auto expr3_sub = expr3.substitute("y", expression<double>(5.0));
    std::map<std::string, double> vars3 = {{"x", 2.0}};
    std::cout << "Test 3:\n";
    std::cout << "h(x,y) = " << expr3.to_string() << "\n";
    std::cout << "h(x,5) = " << expr3_sub.to_string() << "\n";
    std::cout << "h(2,5) = " << expr3_sub.evaluate(vars3) << "\n\n";

    // Тест 4: Константа: f = 5, производная должна быть 0
    expression<double> expr4(5.0);
    auto dexpr4 = expr4.differentiate("x");
    std::cout << "Test 4:\n";
    std::cout << "f = " << expr4.to_string() << "\n";
    std::cout << "f' = " << dexpr4.to_string() << "\n\n";

    // Тест 5: Переменная: f(x) = x, производная должна быть 1
    expression<double> expr5("x");
    auto dexpr5 = expr5.differentiate("x");
    std::map<std::string, double> vars5 = {{"x", 10.0}};
    std::cout << "Test 5:\n";
    std::cout << "f(x) = " << expr5.to_string() << "\n";
    std::cout << "f'(x) = " << dexpr5.to_string() << "\n";
    std::cout << "f(10) = " << expr5.evaluate(vars5) << "\n\n";

    // Тест 6: f(x) = x * ln(x), производная по правилу произведения
    expression<double> expr6 = x * ln(x);
    auto dexpr6 = expr6.differentiate("x");
    std::map<std::string, double> vars6 = {{"x", 2.0}};
    std::cout << "Test 6:\n";
    std::cout << "f(x) = " << expr6.to_string() << "\n";
    std::cout << "f'(x) = " << dexpr6.to_string() << "\n";
    std::cout << "f(2) = " << expr6.evaluate(vars6) << "\n\n";

    // Тест 7: f(x) = exp(x) / x, производная по правилу частного
    expression<double> expr7 = exp(x) / x;
    auto dexpr7 = expr7.differentiate("x");
    std::map<std::string, double> vars7 = {{"x", 1.0}};
    std::cout << "Test 7:\n";
    std::cout << "f(x) = " << expr7.to_string() << "\n";
    std::cout << "f'(x) = " << dexpr7.to_string() << "\n";
    std::cout << "f(1) = " << expr7.evaluate(vars7) << "\n\n";

    // Тест 8: f(x) = x^x, производная с использованием правила возведения в степень
    expression<double> expr8 = x ^ x;
    auto dexpr8 = expr8.differentiate("x");
    std::map<std::string, double> vars8 = {{"x", 2.0}};
    std::cout << "Test 8:\n";
    std::cout << "f(x) = " << expr8.to_string() << "\n";
    std::cout << "f'(x) = " << dexpr8.to_string() << "\n";
    std::cout << "f(2) = " << expr8.evaluate(vars8) << "\n\n";

    // Тест 9: f(x) = sin(ln(x)), производная по правилу композиции
    expression<double> expr9 = sin(ln(x));
    auto dexpr9 = expr9.differentiate("x");
    std::map<std::string, double> vars9 = {{"x", 2.0}};
    std::cout << "Test 9:\n";
    std::cout << "f(x) = " << expr9.to_string() << "\n";
    std::cout << "f'(x) = " << dexpr9.to_string() << "\n";
    std::cout << "f(2) = " << expr9.evaluate(vars9) << "\n\n";

    // Тест 10: f(x) = 2 + 3, результат должен быть 5, производная 0
    expression<double> expr10 = expression<double>(2.0) + expression<double>(3.0);
    auto dexpr10 = expr10.differentiate("x");
    std::cout << "Test 10:\n";
    std::cout << "f(x) = " << expr10.to_string() << "\n";
    std::cout << "f'(x) = " << dexpr10.to_string() << "\n";
    std::cout << "f() = " << expr10.evaluate({}) << "\n\n";

    // Тест 11: f(x) = (x + 2)^2, производная с возведением в степень
    expression<double> expr11 = (x + expression<double>(2.0)) ^ expression<double>(2.0);
    auto dexpr11 = expr11.differentiate("x");
    std::map<std::string, double> vars11 = {{"x", 3.0}};
    std::cout << "Test 11:\n";
    std::cout << "f(x) = " << expr11.to_string() << "\n";
    std::cout << "f'(x) = " << dexpr11.to_string() << "\n";
    std::cout << "f(3) = " << expr11.evaluate(vars11) << "\n\n";

    // Тест 12: f(x,y) = x + y, подстановка x = 4, y = 7
    expression<double> expr12 = x + y;
    auto expr12_sub = expr12.substitute("x", expression<double>(4.0))
                               .substitute("y", expression<double>(7.0));
    std::cout << "Test 12:\n";
    std::cout << "f(x,y) = " << expr12.to_string() << "\n";
    std::cout << "f(4,7) = " << expr12_sub.to_string() << "\n";
    std::cout << "f(4,7) = " << expr12_sub.evaluate({}) << "\n\n";

    // Тест 13: f(x) = exp(ln(x)) = x, производная должна быть 1
    expression<double> expr13 = exp(ln(x));
    auto dexpr13 = expr13.differentiate("x");
    std::map<std::string, double> vars13 = {{"x", 5.0}};
    std::cout << "Test 13:\n";
    std::cout << "f(x) = " << expr13.to_string() << "\n";
    std::cout << "f'(x) = " << dexpr13.to_string() << "\n";
    std::cout << "f(5) = " << expr13.evaluate(vars13) << "\n\n";

    // Тест 14: f(x) = cos(x) * sin(x), производная по правилу произведения
    expression<double> expr14 = cos(x) * sin(x);
    auto dexpr14 = expr14.differentiate("x");
    std::map<std::string, double> vars14 = {{"x", 3.0}};
    std::cout << "Test 14:\n";
    std::cout << "f(x) = " << expr14.to_string() << "\n";
    std::cout << "f'(x) = " << dexpr14.to_string() << "\n";
    std::cout << "f(3) = " << expr14.evaluate(vars14) << "\n\n";

    return 0;
}

