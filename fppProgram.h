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

#ifndef _BACKENDS_FPP_FPPPROGRAM_H_
#define _BACKENDS_FPP_FPPPROGRAM_H_

#include "target.h"
#include "fppModel.h"
#include "fppObject.h"
#include "ir/ir.h"
#include "frontends/p4/typeMap.h"
#include "frontends/p4/evaluator/evaluator.h"
#include "frontends/common/options.h"
#include "codeGen.h"

namespace FPP {

class FPPProgram;
class FPPParser;
class FPPTable;
class FPPType;

class FPPProgram : public FPPObject {
 public:
    const CompilerOptions& options;
    const IR::P4Program* program;
    const IR::ToplevelBlock*  toplevel;
    P4::ReferenceMap*    refMap;
    P4::TypeMap*         typeMap;
    FPPParser*          parser;
    FPPModel            &model;

    cstring endLabel, offsetVar, lengthVar;
    cstring zeroKey, functionName, errorVar;
    cstring packetStartVar, packetEndVar, byteVar;
    cstring errorEnum;
    cstring license = "GPL";  // TODO: this should be a compiler option probably
    cstring arrayIndexType = "uint32_t";

    virtual bool build();  // return 'true' on success

    FPPProgram(const CompilerOptions &options, const IR::P4Program* program,
                P4::ReferenceMap* refMap, P4::TypeMap* typeMap, const IR::ToplevelBlock* toplevel) :
            options(options), program(program), toplevel(toplevel),
            refMap(refMap), typeMap(typeMap),
            parser(nullptr), model(FPPModel::instance) {
        offsetVar = FPPModel::reserved("packetOffsetInBits");
        zeroKey = FPPModel::reserved("zero");
        functionName = FPPModel::reserved("parse_packet");
        errorVar = FPPModel::reserved("errorCode");
        packetStartVar = FPPModel::reserved("packetStart");
        packetEndVar = FPPModel::reserved("packetEnd");
        byteVar = FPPModel::reserved("byte");
        endLabel = FPPModel::reserved("end");
        errorEnum = FPPModel::reserved("errorCodes");
    }

 protected:
    virtual void emitGeneratedComment(CodeBuilder* builder);
    virtual void emitPreamble(CodeBuilder* builder);
    virtual void emitTypes(CodeBuilder* builder);
    virtual void emitHeaderInstances(CodeBuilder* builder);
    virtual void emitLocalVariables(CodeBuilder* builder);
    virtual void emitAcceptState(CodeBuilder* builder);

 public:
    virtual void emitH(CodeBuilder* builder, cstring headerFile);  // emits C headers
    virtual void emitC(CodeBuilder* builder, cstring headerFile);  // emits C program
};

}  // namespace FPP

#endif /* _BACKENDS_FPP_FPPPROGRAM_H_ */
