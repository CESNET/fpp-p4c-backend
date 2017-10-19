/*
Modifications copyright 2017 CESNET

Copyright 2013-present Barefoot Networks, Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "fppType.h"

namespace FPP {

FPPTypeFactory* FPPTypeFactory::instance;

FPPType* FPPTypeFactory::create(const IR::Type* type) {
    CHECK_NULL(type);
    CHECK_NULL(typeMap);
    FPPType* result = nullptr;
    if (type->is<IR::Type_Boolean>()) {
        result = new FPPBoolType();
    } else if (type->is<IR::Type_Bits>()) {
        result = new FPPScalarType(type->to<IR::Type_Bits>());
    } else if (type->is<IR::Type_StructLike>()) {
        result = new FPPStructType(type->to<IR::Type_StructLike>());
    } else if (type->is<IR::Type_Typedef>()) {
        auto canon = typeMap->getTypeType(type, true);
        result = create(canon);
        auto path = new IR::Path(type->to<IR::Type_Typedef>()->name);
        result = new FPPTypeName(new IR::Type_Name(path), result);
    } else if (type->is<IR::Type_Name>()) {
        auto canon = typeMap->getTypeType(type, true);
        result = create(canon);
        result = new FPPTypeName(type->to<IR::Type_Name>(), result);
    } else if (type->is<IR::Type_Enum>()) {
        return new FPPEnumType(type->to<IR::Type_Enum>());
    } else {
        ::error("Type %1% not supported", type);
    }

    return result;
}

void
FPPBoolType::declare(CodeBuilder* builder, cstring id, bool asPointer) {
    emit(builder);
    if (asPointer)
        builder->append("*");
    builder->appendFormat(" %s", id.c_str());
}

/////////////////////////////////////////////////////////////

unsigned FPPScalarType::alignment() const {
    if (width <= 8)
        return 1;
    else if (width <= 16)
        return 2;
    else if (width <= 32)
        return 4;
    else
        return 1;
}

void FPPScalarType::emit(CodeBuilder* builder) {
    auto prefix = isSigned ? "" : "u";

    if (width <= 8)
        builder->appendFormat("%sint8_t", prefix);
    else if (width <= 16)
        builder->appendFormat("%sint16_t", prefix);
    else if (width <= 32)
        builder->appendFormat("%sint32_t", prefix);
    else
        builder->appendFormat("uint8_t*");
}

void FPPScalarType::emitType(CodeBuilder* builder) {
    auto prefix = isSigned ? "" : "u";

    builder->emitIndent();
    if (width <= 8)
        builder->appendFormat("%sint8_t", prefix);
    else if (width <= 16)
        builder->appendFormat("%sint16_t", prefix);
    else if (width <= 32)
        builder->appendFormat("%sint32_t", prefix);
    else
        builder->appendFormat("uint8_t*");
}

void
FPPScalarType::declare(CodeBuilder* builder, cstring id, bool asPointer) {
    if (width <= 32) {
        emit(builder);
        if (asPointer)
            builder->append("*");
        builder->spc();
        builder->append(id);
    } else {
        if (asPointer)
            builder->append("uint8_t*");
        else
            builder->appendFormat("uint8_t %s[%d]", id.c_str(), bytesRequired());
    }
}

//////////////////////////////////////////////////////////

FPPStructType::FPPStructType(const IR::Type_StructLike* strct) :
        FPPType(strct) {
    if (strct->is<IR::Type_Struct>())
        kind = "struct";
    else if (strct->is<IR::Type_Header>())
        kind = "struct";
    else if (strct->is<IR::Type_HeaderUnion>())
        kind = "union";
    else
        BUG("Unexpected struct type %1%", strct);
    name = strct->name.name;
    width = 0;
    implWidth = 0;

    for (auto f : strct->fields) {
        auto type = FPPTypeFactory::instance->create(f->type);
        auto wt = dynamic_cast<IHasWidth*>(type);
        if (wt == nullptr) {
            ::error("FPP: Unsupported type in struct: %s", f->type);
        } else {
            width += wt->widthInBits();
            implWidth += wt->implementationWidthInBits();
        }
        fields.push_back(new FPPField(type, f));
    }
}

void
FPPStructType::declare(CodeBuilder* builder, cstring id, bool asPointer) {
    builder->append(kind);
    const char* n = name.c_str();
    builder->appendFormat(" %s", n);
    if (asPointer)
        builder->append("*");
    builder->appendFormat(" %s", id.c_str());
}

void FPPStructType::emitInitializer(CodeBuilder* builder) {
    builder->blockStart();
    if (type->is<IR::Type_Struct>() || type->is<IR::Type_HeaderUnion>()) {
        for (auto f : fields) {
            builder->emitIndent();
            builder->appendFormat(".%s = ", f->field->name.name);
            f->type->emitInitializer(builder);
            builder->append(",");
            builder->newline();
        }
    } else if (type->is<IR::Type_Header>()) {
        builder->emitIndent();
        builder->appendLine(".header_valid = 0");
    } else {
        BUG("Unexpected type %1%", type);
    }
    builder->blockEnd(false);
}

void FPPStructType::emit(CodeBuilder* builder) {
    builder->emitIndent();
    builder->append(kind);
    builder->spc();
    builder->append(name);
    builder->spc();
    builder->blockStart();

    for (auto f : fields) {
        auto type = f->type;
        builder->emitIndent();

        type->declare(builder, f->field->name, false);
        builder->append("; ");
        builder->append("/* ");
        builder->append(type->type->toString());
        if (f->comment != nullptr) {
            builder->append(" ");
            builder->append(f->comment);
        }
        builder->append(" */");
        builder->newline();
    }

    if (type->is<IR::Type_Header>()) {
        auto type_offset = FPPTypeFactory::instance->create(IR::Type_Bits::get(32));
        builder->newline();
        builder->emitIndent();
        type_offset->declare(builder, "header_offset", false);
        builder->endOfStatement(true);

        auto type = FPPTypeFactory::instance->create(IR::Type_Boolean::get());
        if (type != nullptr) {
            builder->emitIndent();
            type->declare(builder, "header_valid", false);
            builder->endOfStatement(true);
        }
    }

    builder->blockEnd(false);
    builder->endOfStatement(true);
}

void FPPStructType::emitType(CodeBuilder* builder) {
    builder->emitIndent();
    builder->append(kind);
    builder->spc();
    builder->append(name);
}

///////////////////////////////////////////////////////////////

void FPPTypeName::declare(CodeBuilder* builder, cstring id, bool asPointer) {
    if (canonical != nullptr)
        canonical->declare(builder, id, asPointer);
}

void FPPTypeName::emitInitializer(CodeBuilder* builder) {
    if (canonical != nullptr)
        canonical->emitInitializer(builder);
}

unsigned FPPTypeName::widthInBits() {
    auto wt = dynamic_cast<IHasWidth*>(canonical);
    if (wt == nullptr) {
        ::error("Type %1% does not have a fixed witdh", type);
        return 0;
    }
    return wt->widthInBits();
}

unsigned FPPTypeName::implementationWidthInBits() {
    auto wt = dynamic_cast<IHasWidth*>(canonical);
    if (wt == nullptr) {
        ::error("Type %1% does not have a fixed witdh", type);
        return 0;
    }
    return wt->implementationWidthInBits();
}

////////////////////////////////////////////////////////////////

void FPPEnumType::declare(FPP::CodeBuilder* builder, cstring id, bool asPointer) {
    builder->append("enum ");
    builder->append(getType()->name);
    if (asPointer)
        builder->append("*");
    builder->append(" ");
    builder->append(id);
}

void FPPEnumType::emit(FPP::CodeBuilder* builder) {
    builder->append("enum ");
    auto et = getType();
    builder->append(et->name);
    builder->blockStart();
    for (auto m : et->members) {
        builder->append(m->name);
        builder->appendLine(",");
    }
    builder->blockEnd(true);
}

void FPPEnumType::emitType(FPP::CodeBuilder* builder) {
    builder->append("enum ");
    auto et = getType();
    builder->append(et->name);
}

}  // namespace FPP
