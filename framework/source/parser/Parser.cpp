// Copyright 2025 solar-mist

#include "parser/Parser.h"

namespace parser
{
    Parser::Parser(std::vector<lexer::Token>& tokens, diagnostic::Diagnostics& diag)
        : mTokens(tokens)
        , mPosition(0)
        , mDiag(diag)
        , mActiveScope(Scope::GetGlobalScope())
    {
    }

    std::vector<ASTNodePtr> Parser::parse()
    {
        std::vector<ASTNodePtr> ast;
        while (mPosition < mTokens.size())
        {
            auto node = parseGlobal();
            if (node)
            {
                ast.emplace_back(std::move(node));
            }
        }

        return ast;
    }


    lexer::Token Parser::current() const
    {
        return mTokens[mPosition];
    }

    lexer::Token Parser::consume()
    {
        return mTokens[mPosition++];
    }

    lexer::Token Parser::peek(int offset) const
    {
        return mTokens[mPosition + offset];
    }


    void Parser::expectToken(lexer::TokenType tokenType)
    {
        if (current().getTokenType() != tokenType)
        {
            lexer::Token temp("", tokenType, lexer::SourceLocation(), lexer::SourceLocation());
            mDiag.reportCompilerError(
                current().getStartLocation(),
                current().getEndLocation(),
                std::format("Expected '{}{}{}', found '{}{}{}'",
                    fmt::bold, temp.getName(), fmt::defaults,
                    fmt::bold, current().getText(), fmt::defaults)
            );
            std::exit(1);
        }
    }


    ASTNodePtr Parser::parseGlobal()
    {
        switch (current().getTokenType())
        {
            case lexer::TokenType::FuncKeyword:
                return parseFunction();

            case lexer::TokenType::EndOfFile:
                consume();
                return nullptr;
            
            default:
                mDiag.reportCompilerError(
                    current().getStartLocation(),
                    current().getEndLocation(),
                    std::format("Expected global expression. Found '{}{}{}'", fmt::bold, current().getText(), fmt::defaults)
                );
                std::exit(1);
        }
    }

    ASTNodePtr Parser::parseExpression(int precedence)
    {
        return parsePrimary();
    }

    ASTNodePtr Parser::parsePrimary()
    {
        switch (current().getTokenType())
        {
            case lexer::TokenType::ReturnKeyword:
                return parseReturnStatement();

            case lexer::TokenType::LetKeyword:
                return parseVariableDeclaration();
            
            case lexer::TokenType::IntegerLiteral:
                return parseIntegerLiteral();

            case lexer::TokenType::Identifier:
                return parseVariableExpression();

            default:
                mDiag.reportCompilerError(
                    current().getStartLocation(),
                    current().getEndLocation(),
                    std::format("Expected an expression. Found '{}{}{}'", fmt::bold, current().getText(), fmt::defaults)
                );
                std::exit(1);
        }
    }


    FunctionPtr Parser::parseFunction()
    {
        consume(); // func

        expectToken(lexer::TokenType::Identifier);
        std::string name(consume().getText());

        expectToken(lexer::TokenType::LeftParen);
        consume();
        // TODO: Parse arguments
        expectToken(lexer::TokenType::RightParen);
        consume();

        expectToken(lexer::TokenType::RightArrow);
        consume();

        // TODO: Parse type
        expectToken(lexer::TokenType::Type);
        consume();

        expectToken(lexer::TokenType::LeftBrace);
        consume();
        
        ScopePtr scope = std::make_unique<Scope>(mActiveScope);
        mActiveScope = scope.get();

        std::vector<ASTNodePtr> body;
        while (current().getTokenType() != lexer::TokenType::RightBrace)
        {
            body.push_back(parseExpression());
            expectToken(lexer::TokenType::Semicolon);
            consume();
        }
        consume();

        mActiveScope = scope->parent;

        return std::make_unique<Function>(std::move(name), std::move(scope), std::move(body));
    }


    ReturnStatementPtr Parser::parseReturnStatement()
    {
        consume(); // return

        auto returnValue = parseExpression();
        
        return std::make_unique<ReturnStatement>(mActiveScope, std::move(returnValue));
    }

    VariableDeclarationPtr Parser::parseVariableDeclaration()
    {
        consume(); // let

        expectToken(lexer::TokenType::Identifier);
        std::string name(consume().getText());

        // TODO: Implicit typing

        expectToken(lexer::TokenType::Colon);
        consume();

        // TODO: Parse type
        expectToken(lexer::TokenType::Type);
        consume();

        ASTNodePtr initialValue = nullptr;
        if (current().getTokenType() == lexer::TokenType::Equal)
        {
            consume();
            initialValue = parseExpression();
        }

        return std::make_unique<VariableDeclaration>(mActiveScope, std::move(name), std::move(initialValue));
    }


    IntegerLiteralPtr Parser::parseIntegerLiteral()
    {
        std::string text(consume().getText());
        auto value = std::stoull(text, nullptr, 0);
        return std::make_unique<IntegerLiteral>(mActiveScope, value);
    }

    VariableExpressionPtr Parser::parseVariableExpression()
    {
        std::string text(consume().getText());
        return std::make_unique<VariableExpression>(mActiveScope, std::move(text));
    }
}