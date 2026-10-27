#include "ExpressionParser.h"
#include <cctype>
#include <cmath>
#include <stdexcept>
#include <cstdlib>

ExpressionParser::ExpressionParser(const PluginManager& plugin_manager)
    : plugin_mgr_(plugin_manager) {}

double ExpressionParser::evaluate(const std::string& expression) {
    tokenize(expression);
    pos_ = 0;
    if (peek().type == TokenType::END) {
        throw std::runtime_error("Пустое выражение.");
    }
    double result = parseExpression(1);
    if (peek().type != TokenType::END) {
        throw std::runtime_error("Неожиданный токен в конце выражения: '" + peek().text + "'");
    }
    return result;
}

void ExpressionParser::tokenize(const std::string& expr) {
    tokens_.clear();
    std::size_t i = 0;
    const std::size_t n = expr.length();

    while (i < n) {
        if (std::isspace(static_cast<unsigned char>(expr[i]))) {
            ++i;
            continue;
        }

        if (i + 2 < n && static_cast<unsigned char>(expr[i]) == 0xE2 &&
            static_cast<unsigned char>(expr[i + 1]) == 0x80 &&
            static_cast<unsigned char>(expr[i + 2]) == 0x93) {
            tokens_.push_back({ TokenType::OPERATOR, "-", 0.0 });
            i += 3;
            continue;
        }

        if (std::isdigit(static_cast<unsigned char>(expr[i])) || expr[i] == '.') {
            std::size_t start = i;
            bool has_dot = false;
            while (i < n && (std::isdigit(static_cast<unsigned char>(expr[i])) || expr[i] == '.')) {
                if (expr[i] == '.') {
                    if (has_dot) throw std::runtime_error("Некорректный формат числа (две точки).");
                    has_dot = true;
                }
                ++i;
            }
            std::string num_str = expr.substr(start, i - start);
            double val = std::strtod(num_str.c_str(), nullptr);
            tokens_.push_back({ TokenType::NUMBER, num_str, val });
            continue;
        }

        if (std::isalpha(static_cast<unsigned char>(expr[i])) || expr[i] == '_') {
            std::size_t start = i;
            while (i < n && (std::isalnum(static_cast<unsigned char>(expr[i])) || expr[i] == '_')) {
                ++i;
            }
            tokens_.push_back({ TokenType::IDENTIFIER, expr.substr(start, i - start), 0.0 });
            continue;
        }

        if (expr[i] == '(') {
            tokens_.push_back({ TokenType::LPAREN, "(", 0.0 });
            ++i;
            continue;
        }
        if (expr[i] == ')') {
            tokens_.push_back({ TokenType::RPAREN, ")", 0.0 });
            ++i;
            continue;
        }

        char c = expr[i];
        if (c == '+' || c == '-' || c == '*' || c == '/' || plugin_mgr_.isOperatorChar(c)) {
            tokens_.push_back({ TokenType::OPERATOR, std::string(1, c), 0.0 });
            ++i;
            continue;
        }

        throw std::runtime_error(std::string("Неизвестный символ в выражении: '") + c + "'");
    }

    tokens_.push_back({ TokenType::END, "", 0.0 });
}

const ExpressionParser::Token& ExpressionParser::peek() const {
    return tokens_[pos_];
}

ExpressionParser::Token ExpressionParser::consume() {
    return tokens_[pos_++];
}

int ExpressionParser::getOperatorPrecedence(const std::string& op) const {
    if (op == "+" || op == "-") return 1;
    if (op == "*" || op == "/") return 2;
    if (const auto* info = plugin_mgr_.getBinaryOperator(op)) {
        return info->precedence;
    }
    return -1;
}

bool ExpressionParser::isRightAssociative(const std::string& op) const {
    if (const auto* info = plugin_mgr_.getBinaryOperator(op)) {
        return info->right_associative;
    }
    return false;
}

double ExpressionParser::parseExpression(int min_precedence) {
    double left = parsePrimary();

    while (peek().type == TokenType::OPERATOR) {
        std::string op = peek().text;
        int prec = getOperatorPrecedence(op);
        if (prec < min_precedence) {
            break;
        }
        consume();

        int next_min_prec = isRightAssociative(op) ? prec : (prec + 1);
        double right = parseExpression(next_min_prec);
        left = applyBinaryOperator(op, left, right);
    }

    return left;
}

double ExpressionParser::parsePrimary() {
    const Token& tok = peek();

    if (tok.type == TokenType::OPERATOR && (tok.text == "-" || tok.text == "+")) {
        std::string op = consume().text;
        double operand = parseExpression(3); 
        return (op == "-") ? -operand : operand;
    }

    if (tok.type == TokenType::NUMBER) {
        return consume().number_value;
    }

    if (tok.type == TokenType::IDENTIFIER) {
        std::string func_name = consume().text;
        if (peek().type != TokenType::LPAREN) {
            throw std::runtime_error("Ожидалась '(' после имени функции '" + func_name + "'");
        }
        consume();
        if (peek().type == TokenType::RPAREN) {
            throw std::runtime_error("Функция '" + func_name + "' вызвана без аргументов.");
        }
        double arg = parseExpression(1);
        if (peek().type != TokenType::RPAREN) {
            throw std::runtime_error("Ожидалась закрывающая ')' после аргумента функции '" + func_name + "'");
        }
        consume();
        return applyUnaryFunction(func_name, arg);
    }

    if (tok.type == TokenType::LPAREN) {
        consume(); 
        double val = parseExpression(1);
        if (peek().type != TokenType::RPAREN) {
            throw std::runtime_error("Пропущена закрывающая скобка ')'.");
        }
        consume(); 
        return val;
    }

    throw std::runtime_error("Синтаксическая ошибка рядом с токеном: '" + tok.text + "'");
}

double ExpressionParser::applyBinaryOperator(const std::string& op, double left, double right) const {
    if (op == "+") return left + right;
    if (op == "-") return left - right;
    if (op == "*") return left * right;
    if (op == "/") {
        if (right == 0.0) {
            throw std::domain_error("Ошибка: деление на ноль.");
        }
        return left / right;
    }

    const PluginInfo* info = plugin_mgr_.getBinaryOperator(op);
    if (!info) {
        throw std::runtime_error("Неизвестный бинарный оператор: '" + op + "'");
    }

    double args[2] = { left, right };
    double result = 0.0;
    char err_buf[256] = { 0 };

    if (!info->eval(args, 2, &result, err_buf, sizeof(err_buf))) {
        throw std::domain_error(std::string("Ошибка в плагине оператора '") + op + "': " + err_buf);
    }
    return result;
}

double ExpressionParser::applyUnaryFunction(const std::string& func_name, double arg) const {
    const PluginInfo* info = plugin_mgr_.getUnaryFunction(func_name);
    if (!info) {
        throw std::runtime_error("Неизвестная функция '" + func_name + "' (соответствующий плагин не загружен)");
    }

    double result = 0.0;
    char err_buf[256] = { 0 };

    if (!info->eval(&arg, 1, &result, err_buf, sizeof(err_buf))) {
        throw std::domain_error(std::string("Ошибка при вычислении ") + func_name + ": " + err_buf);
    }
    return result;
}