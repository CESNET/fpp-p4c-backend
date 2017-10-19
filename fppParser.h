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

#ifndef _BACKENDS_FPP_FPPPARSER_H_
#define _BACKENDS_FPP_FPPPARSER_H_

#include "ir/ir.h"
#include "fppObject.h"
#include "fppProgram.h"

namespace FPP {

class FPPParser;

class FPPParserState : public FPPObject {
 public:
    const IR::ParserState* state;
    const FPPParser* parser;

    FPPParserState(const IR::ParserState* state, FPPParser* parser) :
            state(state), parser(parser) {}
    void emit(CodeBuilder* builder);
};

class FPPParser : public FPPObject {
 public:
    const FPPProgram*            program;
    const P4::TypeMap*            typeMap;
    const IR::ParserBlock*        parserBlock;
    std::vector<FPPParserState*> states;
    const IR::Parameter*          packet;
    const IR::Parameter*          headers;
    FPPType*                     headerType;

    explicit FPPParser(const FPPProgram* program, const IR::ParserBlock* block,
                        const P4::TypeMap* typeMap);
    void emit(CodeBuilder* builder);
    bool build();
};

}  // namespace FPP

#endif /* _BACKENDS_FPP_FPPPARSER_H_ */
