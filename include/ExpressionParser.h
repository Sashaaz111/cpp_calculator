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
    static bool isEnDash(const std::string& expr, std::size_t index);
    static bool isNumberStart(char c);
    static bool isIdentifierStart(char c);
    static bool isIdentifierChar(char c);
    static Token readNumberToken(const std::string& expr, std::size_t& index);
    static Token readIdentifierToken(const std::string& expr, std::size_t& index);
    bool isSingleCharOperator(char c) const;

    const Token& peek() const;
    Token consume();

    double parseExpression(int min_precedence = 1);
    double parsePrimary();
    double parseFunctionCall(const std::string& func_name);
    double parseParenthesized();

    int getOperatorPrecedence(const std::string& op) const;
    bool isRightAssociative(const std::string& op) const;
    double applyBinaryOperator(const std::string& op, double left, double right) const;
    double applyUnaryFunction(const std::string& func_name, double arg) const;
};
