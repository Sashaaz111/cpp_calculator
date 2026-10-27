#pragma once
#include "PluginManager.h"
#include <string>
#include <vector>

class ExpressionParser {
public:
    explicit ExpressionParser(const PluginManager& plugin_manager);
    double evaluate(const std::string& expression);

private:
    enum class TokenType { NUMBER, IDENTIFIER, OPERATOR, LPAREN, RPAREN, END };

    struct Token {
        TokenType type;
        std::string text;
        double number_value = 0.0;
    };

    const PluginManager& plugin_mgr_;
    std::vector<Token> tokens_;
    std::size_t pos_ = 0;

    void tokenize(const std::string& expr);
    const Token& peek() const;
    Token consume();

    double parseExpression(int min_precedence = 1);
    double parsePrimary();

    int getOperatorPrecedence(const std::string& op) const;
    bool isRightAssociative(const std::string& op) const;
    double applyBinaryOperator(const std::string& op, double left, double right) const;
    double applyUnaryFunction(const std::string& func_name, double arg) const;
};