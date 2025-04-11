// Copyright 2025 solar-mist

#include "parser/Parser.h"

namespace parser
{
    Parser::Parser(std::vector<lexer::Token>& tokens)
        : mTokens(tokens)
        , mPosition(0)
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
            // TODO: Print error and exit
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
                // TODO: Print error and exit
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
            
            case lexer::TokenType::IntegerLiteral:
                return parseIntegerLiteral();

            default:
                // TODO: Print error and exit
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
        // TODO: Create scope
        std::vector<ASTNodePtr> body;
        while (current().getTokenType() != lexer::TokenType::RightBrace)
        {
            body.push_back(parseExpression());
            expectToken(lexer::TokenType::Semicolon);
            consume();
        }
        consume();

        return std::make_unique<Function>(std::move(name), std::move(body));
    }


    ReturnStatementPtr Parser::parseReturnStatement()
    {
        consume(); // return

        auto returnValue = parseExpression();
        
        return std::make_unique<ReturnStatement>(std::move(returnValue));
    }


    IntegerLiteralPtr Parser::parseIntegerLiteral()
    {
        std::string text(consume().getText());
        auto value = std::stoull(text, nullptr, 0);
        return std::make_unique<IntegerLiteral>(value);
    }
}