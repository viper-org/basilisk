// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_PARSER_PARSER_H
#define BASILISK_FRAMEWORK_PARSER_PARSER_H 1

#include "parser/ast/ASTNode.h"

#include "parser/ast/global/Function.h"

#include "parser/ast/statement/ReturnStatement.h"

#include "parser/ast/expression/IntegerLiteral.h"

#include "lexer/Token.h"

namespace parser
{
    class Parser
    {
    public:
        Parser(std::vector<lexer::Token>& tokens);

        std::vector<ASTNodePtr> parse();

    private:
        std::vector<lexer::Token>& mTokens;
        unsigned int mPosition;


        lexer::Token current() const;
        lexer::Token consume();
        lexer::Token peek(int offset) const;

        void expectToken(lexer::TokenType tokenType);

        ASTNodePtr parseGlobal();
        ASTNodePtr parseExpression(int precedence = 1);
        ASTNodePtr parsePrimary();

        FunctionPtr parseFunction();

        ReturnStatementPtr parseReturnStatement();

        IntegerLiteralPtr parseIntegerLiteral();
    };
}

#endif // BASILISK_FRAMEWORK_PARSER_PARSER_H