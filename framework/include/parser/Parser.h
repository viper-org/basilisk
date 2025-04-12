// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_PARSER_PARSER_H
#define BASILISK_FRAMEWORK_PARSER_PARSER_H 1

#include "parser/ast/ASTNode.h"

#include "parser/ast/global/Function.h"

#include "parser/ast/statement/ReturnStatement.h"
#include "parser/ast/statement/VariableDeclaration.h"
#include "parser/ast/statement/IfStatement.h"

#include "parser/ast/expression/IntegerLiteral.h"
#include "parser/ast/expression/VariableExpression.h"
#include "parser/ast/expression/BinaryExpression.h"

#include "diagnostic/Diagnostic.h"

#include "lexer/Token.h"

namespace parser
{
    class Parser
    {
    public:
        Parser(std::vector<lexer::Token>& tokens, diagnostic::Diagnostics& diag);

        std::vector<ASTNodePtr> parse();

    private:
        std::vector<lexer::Token>& mTokens;
        unsigned int mPosition;

        diagnostic::Diagnostics& mDiag;

        Scope* mActiveScope;


        lexer::Token current() const;
        lexer::Token consume();
        lexer::Token peek(int offset) const;

        void expectToken(lexer::TokenType tokenType);

        int getBinaryOperatorPrecedence(lexer::TokenType tokenType);

        Type* parseType();

        ASTNodePtr parseGlobal();
        ASTNodePtr parseExpression(int precedence = 1);
        ASTNodePtr parsePrimary();

        FunctionPtr parseFunction();

        ReturnStatementPtr parseReturnStatement();
        VariableDeclarationPtr parseVariableDeclaration();
        IfStatementPtr parseIfStatement();

        IntegerLiteralPtr parseIntegerLiteral();
        VariableExpressionPtr parseVariableExpression();
    };
}

#endif // BASILISK_FRAMEWORK_PARSER_PARSER_H