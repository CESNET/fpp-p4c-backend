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

#ifndef _BACKENDS_FPP_FPPTYPE_H_
#define _BACKENDS_FPP_FPPTYPE_H_

#include "lib/algorithm.h"
#include "lib/sourceCodeBuilder.h"
#include "fppObject.h"
#include "ir/ir.h"

namespace FPP {

// Base class for FPP types
class FPPType : public FPPObject {
 protected:
    explicit FPPType(const IR::Type* type) : type(type) {}
 public:
    const IR::Type* type;
    virtual void emit(CodeBuilder* builder) = 0;
    virtual void emitType(CodeBuilder* builder) = 0;
    virtual void declare(CodeBuilder* builder, cstring id, bool asPointer) = 0;
    virtual void emitInitializer(CodeBuilder* builder) = 0;
    virtual void declareArray(CodeBuilder* /*builder*/, const char* /*id*/, unsigned /*size*/)
    { BUG("Arrays of %1% not supported", type); }
    template<typename T> bool is() const { return dynamic_cast<const T*>(this) != nullptr; }
    template<typename T> T *to() { return dynamic_cast<T*>(this); }
};

class IHasWidth {
 public:
    virtual ~IHasWidth() {}
    // P4 width
    virtual unsigned widthInBits() = 0;
    // Width in the target implementation.
    // Currently a multiple of 8.
    virtual unsigned implementationWidthInBits() = 0;
};

class FPPTypeFactory {
 protected:
    const P4::TypeMap* typeMap;
    explicit FPPTypeFactory(const P4::TypeMap* typeMap) :
            typeMap(typeMap) { CHECK_NULL(typeMap); }
 public:
    static FPPTypeFactory* instance;
    static void createFactory(const P4::TypeMap* typeMap)
    { FPPTypeFactory::instance = new FPPTypeFactory(typeMap); }
    virtual FPPType* create(const IR::Type* type);
};

class FPPBoolType : public FPPType, public IHasWidth {
 public:
    FPPBoolType() : FPPType(IR::Type_Boolean::get()) {}
    void emit(CodeBuilder* builder) override
    { builder->append("uint8_t"); }
    void emitType(CodeBuilder* builder) override
    { builder->append("uint8_t"); }
    void declare(CodeBuilder* builder, cstring id, bool asPointer) override;
    void emitInitializer(CodeBuilder* builder) override
    { builder->append("0"); }
    unsigned widthInBits() override { return 1; }
    unsigned implementationWidthInBits() override { return 8; }
};

class FPPScalarType : public FPPType, public IHasWidth {
 public:
    const unsigned width;
    const bool     isSigned;
    explicit FPPScalarType(const IR::Type_Bits* bits) :
            FPPType(bits), width(bits->size), isSigned(bits->isSigned) {}
    unsigned bytesRequired() const { return ROUNDUP(width, 8); }
    unsigned alignment() const;
    void emit(CodeBuilder* builder) override;
    void emitType(CodeBuilder* builder) override;
    void declare(CodeBuilder* builder, cstring id, bool asPointer) override;
    void emitInitializer(CodeBuilder* builder) override
    { builder->append("0"); }
    unsigned widthInBits() override { return width; }
    unsigned implementationWidthInBits() override { return bytesRequired() * 8; }
    // True if this width is small enough to store in a machine scalar
    static bool generatesScalar(unsigned width)
    { return width <= 32; }
};

// This should not always implement IHasWidth, but it may...
class FPPTypeName : public FPPType, public IHasWidth {
    const IR::Type_Name* type;
    FPPType* canonical;
 public:
    FPPTypeName(const IR::Type_Name* type, FPPType* canonical) :
            FPPType(type), type(type), canonical(canonical) {}
    void emit(CodeBuilder* builder) override { canonical->emit(builder); }
    void emitType(CodeBuilder* builder) override { canonical->emitType(builder); }
    void declare(CodeBuilder* builder, cstring id, bool asPointer) override;
    void emitInitializer(CodeBuilder* builder) override;
    unsigned widthInBits() override;
    unsigned implementationWidthInBits() override;
};

// Also represents headers and unions
class FPPStructType : public FPPType, public IHasWidth {
    class FPPField {
     public:
        cstring comment;
        FPPType* type;
        const IR::StructField* field;

        FPPField(FPPType* type, const IR::StructField* field, cstring comment = nullptr) :
            comment(comment), type(type), field(field) {}
    };

 public:
    cstring  kind;
    cstring  name;
    std::vector<FPPField*>  fields;
    unsigned width;
    unsigned implWidth;

    explicit FPPStructType(const IR::Type_StructLike* strct);
    void declare(CodeBuilder* builder, cstring id, bool asPointer) override;
    void emitInitializer(CodeBuilder* builder) override;
    unsigned widthInBits() override { return width; }
    unsigned implementationWidthInBits() override { return implWidth; }
    void emit(CodeBuilder* builder) override;
    void emitType(CodeBuilder* builder) override;
};

class FPPEnumType : public FPPType, public FPP::IHasWidth {
 public:
    explicit FPPEnumType(const IR::Type_Enum* type) : FPPType(type) {}
    void emit(CodeBuilder* builder) override;
    void emitType(CodeBuilder* builder) override;
    void declare(CodeBuilder* builder, cstring id, bool asPointer) override;
    void emitInitializer(CodeBuilder* builder) override
    { builder->append("0"); }
    unsigned widthInBits() override { return 32; }
    unsigned implementationWidthInBits() override { return 32; }

    const IR::Type_Enum* getType() const { return type->to<IR::Type_Enum>(); }
};

}  // namespace FPP

#endif /* _BACKENDS_FPP_FPPTYPE_H_ */
