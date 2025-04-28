// Copyright 2025 solar-mist

#include "parser/Parser.h"

#include "parser/ast/expression/BinaryExpression.h"
#include "parser/ast/expression/UnaryExpression.h"
#include "parser/ast/expression/MemberAccess.h"

#include "type/StructType.h"
#include "type/PointerType.h"
#include "type/ArrayType.h"
#include "type/PendingType.h"

#include <algorithm>

namespace parser
{
    Parser::Parser(std::vector<lexer::Token>& tokens, diagnostic::Diagnostics& diag, ImportManager& importManager, bool imported)
        : mTokens(tokens)
        , mPosition(0)
        , mDiag(diag)
        , mImportManager(importManager)
        , mImported(imported)
        , mDoneImports(false)
        , mActiveScope(Scope::GetGlobalScope())
    {
    }

    std::vector<std::filesystem::path> Parser::findImports()
    {
        std::vector<std::filesystem::path> ret;

        while (current().getTokenType() == lexer::TokenType::ImportKeyword ||
              (current().getTokenType() == lexer::TokenType::ExportKeyword && peek(1).getTokenType() == lexer::TokenType::ImportKeyword))
        {
            if (current().getTokenType() == lexer::TokenType::ExportKeyword) consume();
            consume();
            std::filesystem::path path;
            while (current().getTokenType() != lexer::TokenType::Semicolon)
            {
                expectToken(lexer::TokenType::Identifier);
                path /= consume().getText();

                if (current().getTokenType() != lexer::TokenType::Semicolon)
                {
                    expectToken(lexer::TokenType::Dot);
                    consume();
                }
            }
            consume();
            ret.push_back(std::move(path));
        }

        return ret;
    }

    std::vector<ASTNodePtr> Parser::parse()
    {
        std::vector<ASTNodePtr> ast;
        mInsertNodeFn = [&ast](ASTNodePtr& node) {
            ast.push_back(std::move(node));
        };

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
            if (auto structType = Type::Get(name))
            {
                type = structType;
            }
            else if (auto structType = StructType::Get(name))
            {
                type = structType;
            }
            else
            {
                SourcePair source{peek(-2).getStartLocation(), peek(-1).getEndLocation()};
                type = PendingType::Create(std::move(source), std::move(name), {});
            }
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


    ASTNodePtr Parser::parseGlobal(bool exported)
    {
        switch (current().getTokenType())
        {
            case lexer::TokenType::ExportKeyword:
                consume();
                return parseGlobal(true);

            case lexer::TokenType::ImportKeyword:
                parseImport();
                return nullptr;

            case lexer::TokenType::FuncKeyword:
                return parseFunction(exported);

            case lexer::TokenType::StructKeyword:
                return parseStructDeclaration(exported);
            
            case lexer::TokenType::GlobalKeyword:
                return parseGlobalVariableDeclaration(exported, false, true);
            case lexer::TokenType::ConstKeyword:
                return parseGlobalVariableDeclaration(exported, true, true);

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
            case lexer::TokenType::GlobalKeyword:
                return parseGlobalVariableDeclaration(false, false, false);
            case lexer::TokenType::ConstKeyword:
                return parseGlobalVariableDeclaration(false, true, false);

            case lexer::TokenType::IfKeyword:
                return parseIfStatement();
            
            case lexer::TokenType::LeftBrace:
                return parseCompoundStatement();

            case lexer::TokenType::WhileKeyword:
                return parseWhileStatement();

            case lexer::TokenType::ForKeyword:
                return parseForStatement();

            case lexer::TokenType::ContinueKeyword:
                return parseContinueStatement();

            case lexer::TokenType::BreakKeyword:
                return parseBreakStatement();
            

            case lexer::TokenType::IntegerLiteral:
                return parseIntegerLiteral();

            case lexer::TokenType::Identifier:
                return parseVariableExpression();

            case lexer::TokenType::LeftParen:
                return parseParenthesizedExpression();

            case lexer::TokenType::StringLiteral:
                return parseStringLiteral();

            case lexer::TokenType::TrueKeyword:
            case lexer::TokenType::FalseKeyword:
                return parseBooleanLiteral();

            case lexer::TokenType::NullptrKeyword:
                return parseNullptrLiteral();
            
            case lexer::TokenType::SizeofKeyword:
                return parseSizeofExpression();

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


    FunctionPtr Parser::parseFunction(bool exported)
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
            SourcePair blockEnd {current().getStartLocation(), current().getEndLocation()};
            consume();
            ScopePtr scope = std::make_unique<Scope>(mActiveScope);
            return std::make_unique<Function>(exported, std::move(name), functionType, std::move(arguments), std::move(scope), true, std::move(body), std::move(source), std::move(blockEnd));
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
        SourcePair blockEnd {current().getStartLocation(), current().getEndLocation()};
        consume();

        mActiveScope = scope->parent;

        bool external = false;
        if (mImported)
        {
            external = true;
            body.clear();
        }

        return std::make_unique<Function>(
            exported,
            std::move(name),
            functionType,
            std::move(arguments),
            std::move(scope),
            external,
            std::move(body),
            std::move(source),
            std::move(blockEnd)
        );
    }

    StructDeclarationPtr Parser::parseStructDeclaration(bool exported)
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

        PendingType* pendingType = nullptr;
        if (auto type = Type::Get(name))
        {
            pendingType = static_cast<PendingType*>(type);
            auto& pendings = PendingType::GetPending();
            std::erase(pendings, pendingType);
        }

        if (mImported)
        {
            auto structDef = std::make_unique<StructDeclaration>(mActiveScope, exported, true, name, std::move(fields), std::move(source));
            mImportManager.addPendingType(std::move(name));
            return structDef;
        }

        auto structDef = std::make_unique<StructDeclaration>(mActiveScope, exported, false, name, std::move(fields), std::move(source));
        if (pendingType) pendingType->initComplete();
        return structDef;
    }

    GlobalVariableDeclarationPtr Parser::parseGlobalVariableDeclaration(bool exported, bool constant, bool globalScope)
    {
        SourcePair source;
        source.start = current().getStartLocation();
        consume(); // global

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
            
            if (globalScope)
            {
                expectToken(lexer::TokenType::Semicolon);
                consume();
            }

            return std::make_unique<GlobalVariableDeclaration>(mActiveScope, std::move(name), nullptr, std::move(initialValue), exported, constant, std::move(source));
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

        if (globalScope)
        {
            expectToken(lexer::TokenType::Semicolon);
            consume();
        }

        return std::make_unique<GlobalVariableDeclaration>(mActiveScope, std::move(name), type, std::move(initialValue), exported, constant, std::move(source));
    }

    void Parser::parseImport()
    {
        consume(); // import

        std::filesystem::path path;
        while (current().getTokenType() != lexer::TokenType::Semicolon)
        {
            expectToken(lexer::TokenType::Identifier);
            path /= consume().getText();

            if (current().getTokenType() != lexer::TokenType::Semicolon)
            {
                expectToken(lexer::TokenType::Dot);
                consume();
            }
        }
        consume();
        if (mImported || mDoneImports) return;

        std::vector<Import> allImports;
        mImportManager.collectAllImports(path, mTokens[0].getStartLocation().file, allImports);
        for (auto it = allImports.begin(); it != allImports.end(); ++it)
        {
            auto& import = *it;

            // Don't import the current file
            if (import.from == mTokens[0].getStartLocation().file) continue;
            // We only want to import each file once
            auto lookIt = std::find_if(allImports.begin(), allImports.end(), [import](const auto& imp){
                return imp.from == import.from;
            });
            if (lookIt != it) continue;

            //ScopePtr scope = std::make_unique<Scope>(nullptr);

            auto nodes = mImportManager.resolveImports(import.from, import.to, mActiveScope, true);
            for (auto& node : nodes)
            {
                mInsertNodeFn(node);
            }

            auto exports = mImportManager.getExports();
            std::vector<Export> invalid;
            for (auto& exp : exports)
            {
                bool importedHere = mImportManager.wasExportedTo(std::string(mTokens[0].getStartLocation().file), allImports, exp);
                if (!exp.symbol || !importedHere)
                {
                    invalid.push_back(exp);
                }
            }
            for (auto& symbol : invalid)
            {
                if (symbol.symbol)
                {
                    symbol.symbol->removed = true;
                }
            }

            //mActiveScope->children.push_back(scope.get());
            for (auto& pendingType : mImportManager.getPendingTypeNames())
            {
                auto type = static_cast<PendingType*>(Type::Get(pendingType));
                auto sym = mActiveScope->resolveSymbol(pendingType);
                if (sym && !sym->removed) type->initComplete();
                else type->initIncomplete();
            }

            //mImportManager.seizeScope(std::move(scope));
            mImportManager.clearExports();
        }
        mDoneImports = true;
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

    ContinueStatementPtr Parser::parseContinueStatement()
    {
        SourcePair source { current().getStartLocation(), current().getEndLocation() };
        consume();

        return std::make_unique<ContinueStatement>(mActiveScope, std::move(source));
    }

    BreakStatementPtr Parser::parseBreakStatement()
    {
        SourcePair source { current().getStartLocation(), current().getEndLocation() };
        consume();

        return std::make_unique<BreakStatement>(mActiveScope, std::move(source));
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

    BooleanLiteralPtr Parser::parseBooleanLiteral()
    {
        SourcePair source{ current().getStartLocation(), current().getEndLocation() };
        bool value = consume().getTokenType() == lexer::TokenType::TrueKeyword;

        return std::make_unique<BooleanLiteral>(mActiveScope, value, std::move(source));
    }

    NullptrLiteralPtr Parser::parseNullptrLiteral()
    {
        SourcePair source{ current().getStartLocation(), current().getEndLocation() };
        consume(); // nullptr
        return std::make_unique<NullptrLiteral>(mActiveScope, std::move(source));
    }

    SizeofExpressionPtr Parser::parseSizeofExpression()
    {
        SourcePair source{ current().getStartLocation(), current().getEndLocation() };
        consume(); // sizeof

        expectToken(lexer::TokenType::LeftParen);
        consume();
        
        if (current().getTokenType() == lexer::TokenType::StructKeyword || current().getTokenType() == lexer::TokenType::Type)
        {
            auto type = parseType();

            expectToken(lexer::TokenType::RightParen);
            source.end = consume().getEndLocation();

            return std::make_unique<SizeofExpression>(mActiveScope, type, std::move(source));
        }
        
        auto operand = parseExpression();

        expectToken(lexer::TokenType::RightParen);
        source.end = consume().getEndLocation();

        return std::make_unique<SizeofExpression>(mActiveScope, std::move(operand), std::move(source));
    }
}