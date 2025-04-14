// Copyright 2025 solar-mist

#include "parser/ast/global/StructDeclaration.h"

#include "type/StructType.h"

#include <vipir/IR/Function.h>
#include <vipir/Type/FunctionType.h>

namespace parser
{
    StructField::StructField(Type* type, std::string name)
        : type(type)
        , name(std::move(name))
    {
    }
    
    
    StructDeclaration::StructDeclaration(Scope* scope, std::string name, std::vector<StructField> fields, SourcePair source)
        : ASTNode(scope, std::move(source))
        , mName(std::move(name))
        , mFields(std::move(fields))
    {
        std::vector<StructType::Field> structTypeFields;
        for (auto& field : mFields)
        {
            structTypeFields.push_back(StructType::Field{field.name, field.type});
        }
        
        mType = StructType::Create(mName, std::move(structTypeFields));
    }

    vipir::Value* StructDeclaration::codegen(vipir::IRBuilder& builder, vipir::Module& module, diagnostic::Diagnostics& diag)
    {
        return nullptr;
    }
    
    void StructDeclaration::typeCheck(diagnostic::Diagnostics& diag, bool& exit)
    {
    }
}