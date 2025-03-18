#ifndef HEAD_HPP
#define HEAD_HPP

#include <sstream>
#include <cmath>
#include <iostream>
#include <map>
#include <complex>
#include <stdexcept>
#include <cctype>
#include <string>
#include <memory>

// ============================================================================
// Объявление класса expression (шаблонный класс)
// ============================================================================
template<typename T>
class expression {
public:
    // Абстрактный базовый класс для узлов дерева выражения
    struct node_base {
        virtual T evaluate(const std::map<std::string, T>&) const = 0;
        virtual std::string to_string() const = 0;
        virtual std::shared_ptr<node_base> differentiate(const std::string&) const = 0;
        virtual std::shared_ptr<node_base> substitute(const std::string&, const std::shared_ptr<node_base>&) const = 0;
        virtual std::shared_ptr<node_base> clone() const = 0;
        virtual ~node_base() {}
    };

    // Конструкторы
    expression(T value);
    expression(const std::string &var_name);
    expression(const expression &other);
    expression(expression &&other) noexcept;
    expression& operator=(const expression &other);
    expression& operator=(expression &&other) noexcept;
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

    // Фабрика для унарных операций
    static expression make_unary(const std::string &op, const expression &operand);

private:
    // Конструктор от указателя на узел (используется внутри реализации)
    expression(std::shared_ptr<node_base> node);
    std::shared_ptr<node_base> root_;
};

// ============================================================================
// Объявления узлов дерева выражения
// ============================================================================

// Узел константы
template<typename T>
struct constant_node : public expression<T>::node_base {
    T value;
    constant_node(T val);
    T evaluate(const std::map<std::string, T>&) const override;
    std::string to_string() const override;
    std::shared_ptr<typename expression<T>::node_base> differentiate(const std::string &) const override;
    std::shared_ptr<typename expression<T>::node_base> substitute(const std::string &, const std::shared_ptr<typename expression<T>::node_base>&) const override;
    std::shared_ptr<typename expression<T>::node_base> clone() const override;
};

// Узел переменной
template<typename T>
struct variable_node : public expression<T>::node_base {
    std::string name;
    variable_node(const std::string &n);
    T evaluate(const std::map<std::string, T>& vars) const override;
    std::string to_string() const override;
    std::shared_ptr<typename expression<T>::node_base> differentiate(const std::string &var) const override;
    std::shared_ptr<typename expression<T>::node_base> substitute(const std::string &var, const std::shared_ptr<typename expression<T>::node_base>& val) const override;
    std::shared_ptr<typename expression<T>::node_base> clone() const override;
};

// Узел бинарной операции
template<typename T>
struct binary_op_node : public expression<T>::node_base {
    std::string op;
    std::shared_ptr<typename expression<T>::node_base> left;
    std::shared_ptr<typename expression<T>::node_base> right;
    binary_op_node(const std::string &o, std::shared_ptr<typename expression<T>::node_base> l, std::shared_ptr<typename expression<T>::node_base> r);
    T evaluate(const std::map<std::string, T>& vars) const override;
    std::string to_string() const override;
    std::shared_ptr<typename expression<T>::node_base> differentiate(const std::string &var) const override;
    std::shared_ptr<typename expression<T>::node_base> substitute(const std::string &var, const std::shared_ptr<typename expression<T>::node_base>& val) const override;
    std::shared_ptr<typename expression<T>::node_base> clone() const override;
};

// Узел унарной операции
template<typename T>
struct unary_op_node : public expression<T>::node_base {
    std::string op;
    std::shared_ptr<typename expression<T>::node_base> child;
    unary_op_node(const std::string &o, std::shared_ptr<typename expression<T>::node_base> c);
    T evaluate(const std::map<std::string, T>& vars) const override;
    std::string to_string() const override;
    std::shared_ptr<typename expression<T>::node_base> differentiate(const std::string &var) const override;
    std::shared_ptr<typename expression<T>::node_base> substitute(const std::string &var, const std::shared_ptr<typename expression<T>::node_base>& val) const override;
    std::shared_ptr<typename expression<T>::node_base> clone() const override;
};

// ============================================================================
// Объявление шаблонного класса парсера выражений
// ============================================================================

template<typename T>
class ExpressionParserT {
public:
    ExpressionParserT(const std::string &s);
    expression<T> parse();
private:
    std::string str;
    size_t pos;
    void skipWhitespace();
    expression<T> parsePrimary();
    expression<T> parseFactor();
    expression<T> parseTerm();
    expression<T> parseExpression();
};

#endif // HEAD_HPP
