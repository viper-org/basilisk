// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_PARSER_PARSER_H
#define BASILISK_FRAMEWORK_PARSER_PARSER_H 1

#include "parser/ast/ASTNode.h"

#include "parser/ast/global/Function.h"
#include "parser/ast/global/StructDeclaration.h"

#include "parser/ast/statement/ReturnStatement.h"
#include "parser/ast/statement/VariableDeclaration.h"
#include "parser/ast/statement/IfStatement.h"
#include "parser/ast/statement/CompoundStatement.h"
#include "parser/ast/statement/WhileStatement.h"
#include "parser/ast/statement/ForStatement.h"

#include "parser/ast/expression/IntegerLiteral.h"
#include "parser/ast/expression/VariableExpression.h"
#include "parser/ast/expression/BinaryExpression.h"
#include "parser/ast/expression/CallExpression.h"
#include "parser/ast/expression/StringLiteral.h"

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
        int getUnaryOperatorPrecedence(lexer::TokenType tokenType);

        Type* parseType();

        ASTNodePtr parseGlobal();
        ASTNodePtr parseExpression(int precedence = 1);
        ASTNodePtr parsePrimary();
        ASTNodePtr parseParenthesizedExpression();

        FunctionPtr parseFunction();
        StructDeclarationPtr parseStructDeclaration();

        ReturnStatementPtr parseReturnStatement();
        VariableDeclarationPtr parseVariableDeclaration();
        IfStatementPtr parseIfStatement();
        CompoundStatementPtr parseCompoundStatement();
        WhileStatementPtr parseWhileStatement();
        ForStatementPtr parseForStatement();

        IntegerLiteralPtr parseIntegerLiteral();
        VariableExpressionPtr parseVariableExpression();
        CallExpressionPtr parseCallExpression(ASTNodePtr callee);
        StringLiteralPtr parseStringLiteral();
    };
}

#endif // BASILISK_FRAMEWORK_PARSER_PARSER_H