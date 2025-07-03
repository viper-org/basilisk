// Copyright 2025 solar-mist

#include "parser/ast/global/StructDeclaration.h"

#include "type/StructType.h"
#include "type/PendingType.h"

#include <vipir/IR/Function.h>
#include <vipir/Type/FunctionType.h>

namespace parser
{
    StructField::StructField(Type* type, std::string name)
        : type(type)
        , name(std::move(name))
    {
    }
    
    
    StructDeclaration::StructDeclaration(Scope* scope, bool exported, bool pending, std::string name, std::vector<StructField> fields, SourcePair source)
        : ASTNode(scope, std::move(source))
        , mName(std::move(name))
        , mFields(std::move(fields))
    {
        mScope->symbols.push_back(std::make_unique<Symbol>(mName, nullptr));
        mSymbol = mScope->symbols.back().get();
        mSymbol->exported = exported;

        std::vector<StructType::Field> structTypeFields;
        for (auto& field : mFields)
        {
            // TODO: Line and column numbers for fields
            structTypeFields.push_back(StructType::Field{field.name, field.type, 0, 0});
        }

        if (auto type = Type::Get(mName))
        {
            auto pending = dynamic_cast<PendingType*>(type);
            pending->set(structTypeFields);
            mType = type;
        }
        else
        {
            if (pending)
            {
                mType = PendingType::Create(mSource, mName, std::move(structTypeFields));
            }
            else
            {
                mType = StructType::Create(mName, std::move(structTypeFields), mSource.start.line, mSource.start.col);
                static_cast<StructType*>(mType)->setDIType(); // TODO: nicer solution, but i am lazy
            }
        }
    }

    vipir::Value* StructDeclaration::codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag)
    {
        return nullptr;
    }
    
    void StructDeclaration::typeCheck(diagnostic::Diagnostics& diag, bool& exit)
    {
    }
}
