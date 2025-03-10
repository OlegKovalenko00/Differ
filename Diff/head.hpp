#ifndef EXPRESSION_HPP
#define EXPRESSION_HPP

#include <string>
#include <memory>
#include <map>
#include <complex>
#include <stdexcept>

template<typename T>
class expression {
public:
    struct node_base {
        virtual ~node_base() {}
        virtual T evaluate(const std::map<std::string, T>& vars) const = 0;
        virtual std::string to_string() const = 0;
        virtual std::shared_ptr<node_base> differentiate(const std::string &var) const = 0;
        virtual std::shared_ptr<node_base> substitute(const std::string &var, const std::shared_ptr<node_base>& val) const = 0;
        virtual std::shared_ptr<node_base> clone() const = 0;
    };

    expression(T value);
    expression(const std::string &var_name);
    expression(const expression &other);
    expression(expression &&other) noexcept;
    expression &operator=(const expression &other);
    expression &operator=(expression &&other) noexcept;
    ~expression();

    std::string to_string() const;
    T evaluate(const std::map<std::string, T> &variables) const;
    expression differentiate(const std::string &var) const;
    expression substitute(const std::string &var, const expression &value) const;

    expression operator+(const expression &other) const;
    expression operator-(const expression &other) const;
    expression operator*(const expression &other) const;
    expression operator/(const expression &other) const;
    expression operator^(const expression &other) const;

    friend expression sin(const expression &expr) {
        return expression::make_unary("sin", expr);
    }
    friend expression cos(const expression &expr) {
        return expression::make_unary("cos", expr);
    }
    friend expression ln(const expression &expr) {
        return expression::make_unary("ln", expr);
    }
    friend expression exp(const expression &expr) {
        return expression::make_unary("exp", expr);
    }
    
    static expression make_unary(const std::string &op, const expression &operand);
    
private:
    std::shared_ptr<node_base> root_;
    explicit expression(std::shared_ptr<node_base> node);
};

// Полная декларация класса ExpressionParser
class ExpressionParser {
public:
    explicit ExpressionParser(const std::string &s);
    expression<double> parse();
    
private:
    std::string str;
    size_t pos;
    
    void skipWhitespace();
    expression<double> parsePrimary();
    expression<double> parseFactor();
    expression<double> parseTerm();
    expression<double> parseExpression();
};

#endif // EXPRESSION_HPP
