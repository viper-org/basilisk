// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_PARSER_AST_GLOBAL_FUNCTION_H
#define BASILISK_FRAMEWORK_PARSER_AST_GLOBAL_FUNCTION_H 1

#include "parser/ast/ASTNode.h"

#include "type/FunctionType.h"

#include <memory>
#include <string>
#include <vector>

namespace parser
{
    enum class CallingConvention
    {
        Default,
        StdCall
    };

    struct FunctionArgument
    {
        FunctionArgument(Type* type, std::string name);
        Type* type;
        std::string name;

        Symbol* symbol;
    };

    class Function : public ASTNode
    {
    public:
        Function(
            bool exported,
            Type* implType,
            std::string name,
            FunctionType* functionType,
            std::vector<FunctionArgument> arguments,
            ScopePtr ownScope,
            bool external,
            std::vector<ASTNodePtr> body,
            SourcePair source,
            SourcePair blockEnd,
			CallingConvention callingConvention
        );

        virtual vipir::Value* codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag) override;
        virtual void setEmittedValue(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag) override;

        virtual std::vector<ASTNode*> getChildren() override;

        virtual void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;

        virtual ASTNodePtr cloneExternal(Scope* in) override;

        std::string getName() const;

    private:
        Type* mImplType;
        std::string mName;
        std::vector<FunctionArgument> mArguments;
        bool mExternal;
        std::vector<ASTNodePtr> mBody;
        SourcePair mBlockEnd;
        CallingConvention mCallingConvention;

        ScopePtr mOwnScope;
        Symbol* mSymbol;


        static std::string MangleName(std::string name, Type* implType, FunctionType* functionType, CallingConvention callingConvention);
    };
    using FunctionPtr = std::unique_ptr<Function>;
}

#endif // BASILISK_FRAMEWORK_PARSER_AST_GLOBAL_FUNCTION_H