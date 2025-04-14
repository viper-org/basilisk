// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_PARSER_AST_EXPRESSION_MEMBER_ACCESS_H
#define BASILISK_FRAMEWORK_PARSER_AST_EXPRESSION_MEMBER_ACCESS_H 1

#include "parser/ast/ASTNode.h"

#include "type/StructType.h"

#include <memory>

namespace parser
{
    class MemberAccess : public ASTNode
    {
    public:
        MemberAccess(Scope* scope, ASTNodePtr struc, std::string id, bool pointer, SourcePair source);

        virtual vipir::Value* codegen(vipir::IRBuilder& builder, vipir::Module& module, diagnostic::Diagnostics& diag) override;

        virtual void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;

    private:
        ASTNodePtr mStruct;
        std::string mId;
        bool mPointer;

        StructType* mStructType;
    };
    using MemberAccessPtr = std::unique_ptr<MemberAccess>;
}

#endif // BASILISK_FRAMEWORK_PARSER_AST_EXPRESSION_MEMBER_ACCESS_H