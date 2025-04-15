// Copyright 2025 solar-mist

#include "parser/Parser.h"

#include "parser/ast/expression/BinaryExpression.h"
#include "parser/ast/expression/UnaryExpression.h"
#include "parser/ast/expression/MemberAccess.h"

#include "type/StructType.h"
#include "type/PointerType.h"
#include "type/ArrayType.h"

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

    int Parser::getBinaryOperatorPrecedence(lexer::TokenType tokenType)
    {
        switch (tokenType) 
        {
            case lexer::TokenType::LeftParen:
            case lexer::TokenType::LeftBracket:
            case lexer::TokenType::Dot:
            case lexer::TokenType::RightArrow:
                return 90;

            case lexer::TokenType::Star:
            case lexer::TokenType::Slash:
                return 75;
            case lexer::TokenType::Plus:
            case lexer::TokenType::Minus:
                return 70;

            case lexer::TokenType::LessThan:
            case lexer::TokenType::GreaterThan:
            case lexer::TokenType::LessEqual:
            case lexer::TokenType::GreaterEqual:
                return 55;

            case lexer::TokenType::DoubleEqual:
            case lexer::TokenType::BangEqual:
                return 50;

            case lexer::TokenType::Equal:
                return 20;

            default:
                return 0;
        }
    }

    int Parser::getUnaryOperatorPrecedence(lexer::TokenType tokenType)
    {
        switch (tokenType) 
        {
            case lexer::TokenType::Ampersand:
            case lexer::TokenType::Star:
                return 85;

            default:
                return 0;
        }
    }


    Type* Parser::parseType()
    {
        // TODO: Function pointers

        Type* type;
        if (current().getTokenType() == lexer::TokenType::StructKeyword)
        {
            consume();
            expectToken(lexer::TokenType::Identifier);
            std::string name(consume().getText());
            type = StructType::Get(name);
        }
        else
        {
            expectToken(lexer::TokenType::Type);
            type = Type::Get(std::string(consume().getText()));
        }

        while (current().getTokenType() == lexer::TokenType::Star ||
               current().getTokenType() == lexer::TokenType::LeftBracket)
        {
            if (current().getTokenType() == lexer::TokenType::Star)
            {
                type = PointerType::Get(type);
                consume();
            }
            else // [
            {
                consume();

                expectToken(lexer::TokenType::IntegerLiteral);
                std::string text(consume().getText());
                auto value = std::stoull(text, nullptr, 0);

                expectToken(lexer::TokenType::RightBracket);
                consume();

                type = ArrayType::Get(type, value);
            }
        }

        return type;
    }


    ASTNodePtr Parser::parseGlobal()
    {
        switch (current().getTokenType())
        {
            case lexer::TokenType::FuncKeyword:
                return parseFunction();

            case lexer::TokenType::StructKeyword:
                return parseStructDeclaration();

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
        SourcePair source;
        ASTNodePtr left;
        source.start = current().getStartLocation();
        int unaryOperatorPrecedence = getUnaryOperatorPrecedence(current().getTokenType());
        if (unaryOperatorPrecedence >= precedence)
        {
            lexer::Token operatorToken = consume();
            auto operand = parseExpression(unaryOperatorPrecedence);
            source.end = peek(-1).getEndLocation();
            left = std::make_unique<UnaryExpression>(mActiveScope, std::move(operand), std::move(operatorToken), std::move(source));
        }
        else
        {
            left = parsePrimary();
        }

        while (true)
        {
            int binaryOperatorPrecedence = getBinaryOperatorPrecedence(current().getTokenType());
            if (binaryOperatorPrecedence < precedence)
            {
                break;
            }

            lexer::Token operatorToken = consume();
            if (operatorToken.getTokenType() == lexer::TokenType::LeftParen)
            {
                left = parseCallExpression(std::move(left));
            }
            else if (operatorToken.getTokenType() == lexer::TokenType::LeftBracket)
            {
                ASTNodePtr index = parseExpression(binaryOperatorPrecedence);
                expectToken(lexer::TokenType::RightBracket);
                source.end = consume().getEndLocation();

                left = std::make_unique<BinaryExpression>(mActiveScope, std::move(left), std::move(operatorToken), std::move(index), std::move(source));
            }
            else if (operatorToken.getTokenType() == lexer::TokenType::Dot)
            {
                expectToken(lexer::TokenType::Identifier);
                std::string id(consume().getText());
                source.end = peek(-1).getEndLocation();
                left = std::make_unique<MemberAccess>(mActiveScope, std::move(left), std::move(id), false, std::move(source));
            }
            else if (operatorToken.getTokenType() == lexer::TokenType::RightArrow)
            {
                expectToken(lexer::TokenType::Identifier);
                std::string id(consume().getText());
                source.end = peek(-1).getEndLocation();
                left = std::make_unique<MemberAccess>(mActiveScope, std::move(left), std::move(id), true, std::move(source));
            }
            else
            {
                ASTNodePtr right = parseExpression(binaryOperatorPrecedence);
                source.end = peek(-1).getEndLocation();
                left = std::make_unique<BinaryExpression>(mActiveScope, std::move(left), std::move(operatorToken), std::move(right), std::move(source));
            }
        }

        return left;
    }

    ASTNodePtr Parser::parsePrimary()
    {
        switch (current().getTokenType())
        {
            case lexer::TokenType::ReturnKeyword:
                return parseReturnStatement();

            case lexer::TokenType::LetKeyword:
                return parseVariableDeclaration();

            case lexer::TokenType::IfKeyword:
                return parseIfStatement();
            
            case lexer::TokenType::LeftBrace:
                return parseCompoundStatement();

            case lexer::TokenType::WhileKeyword:
                return parseWhileStatement();

            case lexer::TokenType::ForKeyword:
                return parseForStatement();
            

            case lexer::TokenType::IntegerLiteral:
                return parseIntegerLiteral();

            case lexer::TokenType::Identifier:
                return parseVariableExpression();

            case lexer::TokenType::LeftParen:
                return parseParenthesizedExpression();

            case lexer::TokenType::StringLiteral:
                return parseStringLiteral();

            default:
                mDiag.reportCompilerError(
                    current().getStartLocation(),
                    current().getEndLocation(),
                    std::format("Expected an expression. Found '{}{}{}'", fmt::bold, current().getText(), fmt::defaults)
                );
                std::exit(1);
        }
    }

    ASTNodePtr Parser::parseParenthesizedExpression()
    {
        consume(); // (
        auto expression = parseExpression();
        expectToken(lexer::TokenType::RightParen);
        consume();

        return expression;
    }


    FunctionPtr Parser::parseFunction()
    {
        SourcePair source;
        source.start = current().getStartLocation();

        consume(); // func
        
        source.end = current().getEndLocation();

        expectToken(lexer::TokenType::Identifier);
        std::string name(consume().getText());

        expectToken(lexer::TokenType::LeftParen);
        consume();
        std::vector<FunctionArgument> arguments;
        std::vector<Type*> argumentTypes;
        while (current().getTokenType() != lexer::TokenType::RightParen)
        {
            expectToken(lexer::TokenType::Identifier);
            std::string name(consume().getText());

            expectToken(lexer::TokenType::Colon);
            consume();

            auto type = parseType();
            argumentTypes.push_back(type);
            arguments.emplace_back(type, std::move(name));

            if (current().getTokenType() != lexer::TokenType::RightParen)
            {
                expectToken(lexer::TokenType::Comma);
                consume();
            }
        }
        consume();

        expectToken(lexer::TokenType::RightArrow);
        consume();

        auto returnType = parseType();
        auto functionType = FunctionType::Create(returnType, std::move(argumentTypes));
        std::vector<ASTNodePtr> body;

        if (current().getTokenType() == lexer::TokenType::Semicolon)
        {
            consume();
            ScopePtr scope = std::make_unique<Scope>(mActiveScope);
            return std::make_unique<Function>(std::move(name), functionType, std::move(arguments), std::move(scope), true, std::move(body), std::move(source));
        }

        expectToken(lexer::TokenType::LeftBrace);
        consume();
        
        ScopePtr scope = std::make_unique<Scope>(mActiveScope);
        mActiveScope = scope.get();

        while (current().getTokenType() != lexer::TokenType::RightBrace)
        {
            body.push_back(parseExpression());
            expectToken(lexer::TokenType::Semicolon);
            consume();
        }
        consume();

        mActiveScope = scope->parent;

        return std::make_unique<Function>(std::move(name), functionType, std::move(arguments), std::move(scope), false, std::move(body), std::move(source));
    }

    StructDeclarationPtr Parser::parseStructDeclaration()
    {
        SourcePair source;
        source.start = current().getStartLocation();
        consume(); // struct

        expectToken(lexer::TokenType::Identifier);
        std::string name(consume().getText());

        expectToken(lexer::TokenType::LeftBrace);
        consume();
        std::vector<StructField> fields;
        while (current().getTokenType() != lexer::TokenType::RightBrace)
        {
            expectToken(lexer::TokenType::Identifier);
            std::string name(consume().getText());

            expectToken(lexer::TokenType::Colon);
            consume();
            auto type = parseType();

            fields.emplace_back(type, std::move(name));

            expectToken(lexer::TokenType::Semicolon);
            consume();
        }
        consume();
        source.end = peek(-1).getEndLocation();

        return std::make_unique<StructDeclaration>(mActiveScope, std::move(name), std::move(fields), std::move(source));
    }


    ReturnStatementPtr Parser::parseReturnStatement()
    {
        SourcePair source;
        source.start = current().getStartLocation();
        consume(); // return

        if (current().getTokenType() == lexer::TokenType::Semicolon)
        {
            source.end = peek(-1).getEndLocation();
            return std::make_unique<ReturnStatement>(mActiveScope, nullptr, std::move(source));
        }

        auto returnValue = parseExpression();

        source.end = peek(-1).getEndLocation();
        
        return std::make_unique<ReturnStatement>(mActiveScope, std::move(returnValue), std::move(source));
    }

    VariableDeclarationPtr Parser::parseVariableDeclaration()
    {
        SourcePair source;
        source.start = current().getStartLocation();
        consume(); // let

        expectToken(lexer::TokenType::Identifier);
        std::string name(consume().getText());

        if (current().getTokenType() != lexer::TokenType::Colon)
        {
            if (current().getTokenType() != lexer::TokenType::Equal)
            {
                source.end = peek(-1).getEndLocation();
                mDiag.reportCompilerError(
                    source.start,
                    source.end,
                    std::format("untyped declaration of '{}{}{}' has no initializer",
                        fmt::bold, name, fmt::defaults
                    )
                );
                std::exit(1);
            }
            consume(); // =
            ASTNodePtr initialValue = parseExpression();
            source.end = peek(-1).getEndLocation();

            return std::make_unique<VariableDeclaration>(mActiveScope, std::move(name), nullptr, std::move(initialValue), std::move(source));
        }

        expectToken(lexer::TokenType::Colon);
        consume();

        auto type = parseType();

        ASTNodePtr initialValue = nullptr;
        if (current().getTokenType() == lexer::TokenType::Equal)
        {
            consume();
            initialValue = parseExpression();
        }

        source.end = peek(-1).getEndLocation();

        return std::make_unique<VariableDeclaration>(mActiveScope, std::move(name), type, std::move(initialValue), std::move(source));
    }

    IfStatementPtr Parser::parseIfStatement()
    {
        SourcePair source;
        source.start = current().getStartLocation();
        consume(); // if

        expectToken(lexer::TokenType::LeftParen);
        consume();
        
        auto condition = parseExpression();

        expectToken(lexer::TokenType::RightParen);
        consume();

        source.end = peek(-1).getEndLocation();

        auto body = parseExpression();

        ASTNodePtr elseBody = nullptr;
        if (peek(1).getTokenType() == lexer::TokenType::ElseKeyword)
        {
            expectToken(lexer::TokenType::Semicolon);
            consume();

            consume(); // else
            elseBody = parseExpression();
        }

        return std::make_unique<IfStatement>(std::move(condition), std::move(body), std::move(elseBody), mActiveScope, std::move(source));
    }

    CompoundStatementPtr Parser::parseCompoundStatement()
    {
        SourcePair source;
        source.start = current().getStartLocation();
        source.end = current().getEndLocation(); // Maybe use something else as the end location?
        consume(); // {

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

        mTokens.insert(mTokens.begin() + mPosition, lexer::Token(";", lexer::TokenType::Semicolon, {}, {}));

        mActiveScope = scope->parent;

        return std::make_unique<CompoundStatement>(std::move(scope), std::move(body), std::move(source));
    }

    WhileStatementPtr Parser::parseWhileStatement()
    {
        SourcePair source;
        source.start = current().getStartLocation();
        consume(); // while

        expectToken(lexer::TokenType::LeftParen);
        consume();

        auto condition = parseExpression();

        expectToken(lexer::TokenType::RightParen);
        source.end = consume().getEndLocation();

        auto body = parseExpression();

        return std::make_unique<WhileStatement>(std::move(condition), std::move(body), mActiveScope, std::move(source));
    }

    ForStatementPtr Parser::parseForStatement()
    {
        SourcePair source;
        source.start = current().getStartLocation();
        consume(); // for

        expectToken(lexer::TokenType::LeftParen);
        consume();

        auto init = parseExpression();
        expectToken(lexer::TokenType::Semicolon);
        consume();

        auto condition = parseExpression();
        expectToken(lexer::TokenType::Semicolon);
        consume();

        auto it = parseExpression();

        expectToken(lexer::TokenType::RightParen);
        source.end = consume().getEndLocation();

        auto body = parseExpression();

        return std::make_unique<ForStatement>(std::move(init), std::move(condition), std::move(it), std::move(body), mActiveScope, std::move(source));
    }


    IntegerLiteralPtr Parser::parseIntegerLiteral()
    {
        SourcePair source{current().getStartLocation(), current().getEndLocation()};
        std::string text(consume().getText());
        auto value = std::stoull(text, nullptr, 0);
        return std::make_unique<IntegerLiteral>(mActiveScope, value, std::move(source));
    }

    VariableExpressionPtr Parser::parseVariableExpression()
    {
        SourcePair source{current().getStartLocation(), current().getEndLocation()};
        std::string text(consume().getText());
        return std::make_unique<VariableExpression>(mActiveScope, std::move(text), std::move(source));
    }

    CallExpressionPtr Parser::parseCallExpression(ASTNodePtr callee)
    {
        SourcePair source;
        source.start = callee->getSourcePair().start;
        std::vector<ASTNodePtr> parameters;
        while (current().getTokenType() != lexer::TokenType::RightParen)
        {
            parameters.push_back(parseExpression());

            if (current().getTokenType() != lexer::TokenType::RightParen)
            {
                expectToken(lexer::TokenType::Comma);
                consume();
            }
        }

        consume();
        source.end = peek(-1).getEndLocation();

        return std::make_unique<CallExpression>(mActiveScope, std::move(callee), std::move(parameters), std::move(source));
    }

    StringLiteralPtr Parser::parseStringLiteral()
    {
        SourcePair source{current().getStartLocation(), current().getEndLocation()};
        std::string text(consume().getText());
        return std::make_unique<StringLiteral>(mActiveScope, std::move(text), std::move(source));
    }
}