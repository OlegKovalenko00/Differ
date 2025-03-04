#ifndef EXPRESSION_HPP            // Если макрос EXPRESSION_HPP не определён, продолжаем обработку файла
#define EXPRESSION_HPP            // Определяем макрос для предотвращения повторного включения файла

#include <string>               // Подключение библиотеки для работы со строками (std::string)
#include <memory>               // Подключение библиотеки для работы с умными указателями (std::shared_ptr)
#include <map>                  // Подключение библиотеки для работы с отображениями (std::map)
#include <complex>              // Подключение библиотеки для работы с комплексными числами (при необходимости)
#include <stdexcept>            // Подключение библиотеки для обработки исключений (std::runtime_error и др.)

// Шаблонный класс expression для представления математических выражений с типом значений T
template<typename T>
class expression {
public:
    // Абстрактная базовая структура для узлов дерева выражения.
    // Каждый узел реализует методы для вычисления, представления в виде строки,
    // взятия производной, подстановки значений и клонирования.
    struct node_base {
        // Виртуальный деструктор для корректного уничтожения объектов-наследников
        virtual ~node_base() {}
        
        // Чисто виртуальная функция для вычисления значения выражения с заданными переменными.
        // Параметр vars - отображение имени переменной в значение типа T.
        virtual T evaluate(const std::map<std::string, T>& vars) const = 0;
        
        // Чисто виртуальная функция для получения строкового представления выражения.
        virtual std::string to_string() const = 0;
        
        // Чисто виртуальная функция для взятия производной выражения по переменной var.
        virtual std::shared_ptr<node_base> differentiate(const std::string &var) const = 0;
        
        // Чисто виртуальная функция для подстановки подвыражения вместо переменной.
        // Параметр val задаёт новое подвыражение, которое заменит переменную var.
        virtual std::shared_ptr<node_base> substitute(const std::string &var, const std::shared_ptr<node_base>& val) const = 0;
        
        // Чисто виртуальная функция для создания глубокого копирования узла.
        virtual std::shared_ptr<node_base> clone() const = 0;
    };

    // Конструктор, создающий выражение-константу из значения типа T.
    expression(T value);
    
    // Конструктор, создающий выражение-переменную по имени переменной.
    expression(const std::string &var_name);
    
    // Конструктор копирования.
    expression(const expression &other);
    
    // Конструктор перемещения, объявленный с noexcept для повышения эффективности.
    expression(expression &&other) noexcept;
    
    // Оператор присваивания копированием.
    expression &operator=(const expression &other);
    
    // Оператор присваивания перемещением, объявленный с noexcept.
    expression &operator=(expression &&other) noexcept;
    
    // Деструктор для освобождения ресурсов.
    ~expression();

    // Метод для получения строкового представления выражения.
    std::string to_string() const;
    
    // Метод для вычисления значения выражения с учетом значений переменных, заданных в variables.
    T evaluate(const std::map<std::string, T> &variables) const;
    
    // Метод для вычисления производной выражения по заданной переменной var.
    expression differentiate(const std::string &var) const;
    
    // Метод для подстановки подвыражения value вместо переменной var в выражении.
    expression substitute(const std::string &var, const expression &value) const;

    // Перегрузка оператора сложения, возвращает новое выражение как сумму текущего и other.
    expression operator+(const expression &other) const;
    
    // Перегрузка оператора вычитания, возвращает новое выражение как разность текущего и other.
    expression operator-(const expression &other) const;
    
    // Перегрузка оператора умножения, возвращает новое выражение как произведение текущего и other.
    expression operator*(const expression &other) const;
    
    // Перегрузка оператора деления, возвращает новое выражение как частное текущего и other.
    expression operator/(const expression &other) const;
    
    // Перегрузка оператора возведения в степень, возвращает новое выражение как результат возведения
    // текущего выражения в степень other.
    expression operator^(const expression &other) const;

    // Дружественная функция для создания выражения синуса от expr.
    friend expression sin(const expression &expr) {
        return expression::make_unary("sin", expr);
    }
    
    // Дружественная функция для создания выражения косинуса от expr.
    friend expression cos(const expression &expr) {
        return expression::make_unary("cos", expr);
    }
    
    // Дружественная функция для создания выражения натурального логарифма от expr.
    friend expression ln(const expression &expr) {
        return expression::make_unary("ln", expr);
    }
    
    // Дружественная функция для создания выражения экспоненты от expr.
    friend expression exp(const expression &expr) {
        return expression::make_unary("exp", expr);
    }
    
private:
    // Указатель на корневой узел синтаксического дерева выражения.
    std::shared_ptr<node_base> root_;
    
    // Приватный конструктор, позволяющий создать выражение на основе существующего узла.
    explicit expression(std::shared_ptr<node_base> node);
    
    // Вспомогательная статическая функция для создания выражения с унарной операцией.
    // Параметр op задаёт тип операции (например, "sin", "cos", "ln", "exp"),
    // а operand - подвыражение, к которому применяется операция.
    static expression make_unary(const std::string &op, const expression &operand);
};

#endif // EXPRESSION_HPP   // Конец условной компиляции: если файл уже был включён, повторно не подключаем его
