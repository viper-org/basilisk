// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_TYPE_STRUCT_TYPE_H
#define BASILISK_FRAMEWORK_TYPE_STRUCT_TYPE_H 1

#include "type/Type.h"

class StructType : public Type
{
public:
    struct Field
    {
        std::string name;
        Type* type;

        int line;
        int col;
    };

    StructType(std::string name, std::vector<Field> fields);

    std::string_view getName() const;
    std::vector<Field>& getFields();
    bool hasField(std::string_view fieldName);
    Field* getField(std::string_view fieldName);
    int getFieldOffset(std::string fieldName);

    virtual int getSize() const override;
    virtual vipir::Type* getVipirType() const override;
    virtual CastLevel castTo(Type* destType) const override;

    bool isStructType() const override;

    static StructType* Get(std::string name);
    static StructType* Create(std::string name, std::vector<Field> fields, int line, int col);

private:
    std::string mName;
    std::vector<Field> mFields;
};

#endif // BASILISK_FRAMEWORK_TYPE_STRUCT_TYPE_H