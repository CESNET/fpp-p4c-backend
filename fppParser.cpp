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

#include "fppModel.h"
#include "fppParser.h"
#include "fppType.h"
#include "frontends/p4/coreLibrary.h"
#include "frontends/p4/methodInstance.h"

namespace FPP {

namespace {
class StateTranslationVisitor : public CodeGenInspector {
    bool hasDefault;
    bool is_headers_type;
    bool headers_path;
    P4::P4CoreLibrary& p4lib;
    const FPPParserState* state;

    void compileExtractField(const IR::Expression* expr, cstring name,
                             unsigned alignment, FPPType* type);
    void compileExtract(const IR::Vector<IR::Argument>* args);
    void compileLookahead(const IR::Type* args);

 public:
    explicit StateTranslationVisitor(const FPPParserState* state) :
            CodeGenInspector(state->parser->program->refMap, state->parser->program->typeMap),
            hasDefault(false), p4lib(P4::P4CoreLibrary::instance), state(state), is_headers_type(false) {}
    bool preorder(const IR::ParserState* state) override;
    bool preorder(const IR::SelectCase* selectCase) override;
    bool preorder(const IR::SelectExpression* expression) override;
    bool preorder(const IR::Member* expression) override;
    bool preorder(const IR::Path* p) override;
    bool preorder(const IR::PathExpression* expression) override;
    bool preorder(const IR::MethodCallExpression* expression) override;
    bool preorder(const IR::MethodCallStatement* stat) override
    { visit(stat->methodCall); return false; }
};
}  // namespace

bool StateTranslationVisitor::preorder(const IR::ParserState* parserState) {
    if (parserState->isBuiltin()) return false;

    builder->emitIndent();
    builder->append(parserState->name.name);
    builder->append(":");
    builder->spc();
    builder->blockStart();

    setVecSep("\n", "\n");
    visit(parserState->components, "components");
    doneVec();

    if (parserState->selectExpression == nullptr) {
        builder->emitIndent();
        builder->append("goto ");
        builder->append(IR::ParserState::reject);
        builder->endOfStatement(true);
    } else if (parserState->selectExpression->is<IR::SelectExpression>()) {
        visit(parserState->selectExpression);
    } else {
        // must be a PathExpression which is a state name
        if (!parserState->selectExpression->is<IR::PathExpression>())
            BUG("Expected a PathExpression, got a %1%", parserState->selectExpression);
        builder->emitIndent();
        builder->append("goto ");
        visit(parserState->selectExpression);
        builder->endOfStatement(true);
    }

    builder->blockEnd(true);
    return false;
}

bool StateTranslationVisitor::preorder(const IR::SelectExpression* expression) {
    hasDefault = false;
    if (expression->select->components.size() != 1) {
        // TODO: this does not handle correctly tuples
        ::error("%1%: only supporting a single argument for select", expression->select);
        return false;
    }
    builder->emitIndent();
    builder->append("switch (");
    visit(expression->select);
    builder->append(") ");
    builder->blockStart();

    for (auto e : expression->selectCases)
        visit(e);

    if (!hasDefault) {
        builder->emitIndent();
        builder->appendFormat("default: goto %s;", IR::ParserState::reject.c_str());
        builder->newline();
    }

    builder->blockEnd(true);
    return false;
}

bool StateTranslationVisitor::preorder(const IR::SelectCase* selectCase) {
    builder->emitIndent();
    if (selectCase->keyset->is<IR::DefaultExpression>()) {
        hasDefault = true;
        builder->append("default: ");
    } else {
        builder->append("case ");
        visit(selectCase->keyset);
        builder->append(": ");
    }
    builder->append("goto ");
    visit(selectCase->state);
    builder->endOfStatement(true);
    return false;
}

void
StateTranslationVisitor::compileExtractField(
    const IR::Expression* expr, cstring field, unsigned alignment, FPPType* type) {
    unsigned widthToExtract = dynamic_cast<IHasWidth*>(type)->widthInBits();
    auto program = state->parser->program;

    if (widthToExtract <= 32) {
        unsigned lastBitIndex = widthToExtract + alignment - 1;
        unsigned lastWordIndex = lastBitIndex / 8;
        unsigned wordsToRead = lastWordIndex + 1;
        unsigned loadSize;

        const char* helper = nullptr;
        const char* switch_func = "";
        if (wordsToRead <= 1) {
            helper = "load_byte";
            loadSize = 8;
        } else if (widthToExtract <= 16)  {
            helper = "load_half";
            switch_func = "ntohs";
            loadSize = 16;
        } else if (widthToExtract <= 32) {
            helper = "load_word";
            switch_func = "ntohl";
            loadSize = 32;
        } else {
            if (widthToExtract > 64) BUG("Unexpected width %d", widthToExtract);
            helper = "load_dword";
            // TODO switch func
            loadSize = 64;
        }

        unsigned shift = loadSize - alignment - widthToExtract;
        builder->emitIndent();
        visit(expr);
        builder->appendFormat(".%s = (%s((", field.c_str(), switch_func);
        type->emit(builder);
        builder->appendFormat(")(%s(%s, BYTES(%s))",
                              helper,
                              program->packetStartVar.c_str(),
                              program->offsetVar.c_str());
        builder->append(")");
        builder->append(")");
        if (shift != 0)
            builder->appendFormat(" >> %d", shift);
        builder->append(")");

        if (widthToExtract != loadSize) {
            builder->append(" & FPP_MASK(");
            type->emit(builder);
            builder->appendFormat(", %d)", widthToExtract);
        }

        builder->endOfStatement(true);
    } else {
        // bigger than 4 bytes; read all bytes one by one.
        unsigned shift;
        if (alignment == 0)
            shift = 0;
        else
            shift = 8 - alignment;

        const char* helper;
        if (shift == 0)
            helper = "load_byte";
        else
            helper = "load_half";
        auto bt = FPPTypeFactory::instance->create(IR::Type_Bits::get(8));
        unsigned bytes = ROUNDUP(widthToExtract, 8);
        for (unsigned i=0; i < bytes; i++) {
            builder->emitIndent();
            visit(expr);
            builder->appendFormat(".%s[%d] = (", field.c_str(), i);
            bt->emit(builder);
            builder->appendFormat(")((%s(%s, BYTES(%s) + %d) >> %d)",
                                  helper,
                                  program->packetStartVar.c_str(),
                                  program->offsetVar.c_str(), i, shift);

            if ((i == bytes - 1) && (widthToExtract % 8 != 0)) {
                builder->append(" & FPP_MASK(");
                bt->emit(builder);
                builder->appendFormat(", %d)", widthToExtract % 8);
            }

            builder->append(")");
            builder->endOfStatement(true);
        }
    }

    builder->emitIndent();
    builder->appendFormat("%s += %d", program->offsetVar.c_str(), widthToExtract);
    builder->endOfStatement(true);
    builder->newline();
}

void
StateTranslationVisitor::compileLookahead(const IR::Type* type) {
   if (type == nullptr) {
        ::error("NULL type for lookahead function");
        return;
   }
   /*
   auto bits = type->to<IR::Type_Bits>();
   if (bits == nullptr) {
      ::error("non-bits type in lookahead functions");
      return;
   }
*/
    auto etype = FPPTypeFactory::instance->create(type);

    unsigned widthToExtract = dynamic_cast<IHasWidth*>(etype)->widthInBits();
    auto program = state->parser->program;

    if (widthToExtract <= 32) {
        unsigned lastBitIndex = widthToExtract - 1;
        unsigned lastWordIndex = lastBitIndex / 8;
        unsigned wordsToRead = lastWordIndex + 1;
        unsigned loadSize;

        const char* helper = nullptr;
        const char* switch_func = "";
        if (wordsToRead <= 1) {
            helper = "load_byte";
            loadSize = 8;
        } else if (widthToExtract <= 16)  {
            helper = "load_half";
            switch_func = "ntohs";
            loadSize = 16;
        } else if (widthToExtract <= 32) {
            helper = "load_word";
            switch_func = "ntohl";
            loadSize = 32;
        } else {
            if (widthToExtract > 64) BUG("Unexpected width %d", widthToExtract);
            helper = "load_dword";
            // TODO switch func
            loadSize = 64;
        }

        unsigned shift = loadSize - widthToExtract;
        builder->emitIndent();
        builder->appendFormat("%s((", switch_func);
        etype->emit(builder);
        builder->appendFormat(")((%s(%s, BYTES(%s))",
                              helper,
                              program->packetStartVar.c_str(),
                              program->offsetVar.c_str());
        if (shift != 0)
            builder->appendFormat(" >> %d", shift);
        builder->append(")");

        if (widthToExtract != loadSize) {
            builder->append(" & FPP_MASK(");
            etype->emit(builder);
            builder->appendFormat(", %d)", widthToExtract);
        }

        builder->append("))");
        builder->endOfStatement(true);
     } else {
         ::error("lookahead bits >32 not supported");
         return;
     }
}

void
StateTranslationVisitor::compileExtract(const IR::Vector<IR::Argument>* args) {
    if (args->size() != 1) {
        ::error("Variable-sized header fields not yet supported %1%", args);
        return;
    }

    bool is_headers_type = false;
    auto expr = args->at(0)->expression;
    auto type = state->parser->typeMap->getType(expr);

    auto membr = expr->to<IR::Member>();
    if (membr != nullptr) {
       if (membr->expr->is<IR::PathExpression>()) {
           auto pe = membr->expr->to<IR::PathExpression>();
           auto decl = state->parser->program->refMap->getDeclaration(pe->path, true);
           if (pe->path->name.name == "headers") {
               is_headers_type = true;
           }
       }
    }

    auto ht = type->to<IR::Type_Header>();
    if (ht == nullptr) {
        ::error("Cannot extract to a non-header type %1%", expr);
        return;
    }

    unsigned width = ht->width_bits();
    auto program = state->parser->program;
    builder->emitIndent();
    builder->appendFormat("if (%s < %s + BYTES(%s + %d)) ",
                          program->packetEndVar.c_str(),
                          program->packetStartVar.c_str(),
                          program->offsetVar.c_str(), width);
    builder->blockStart();

    builder->emitIndent();
    builder->appendFormat("%s = %s;", program->errorVar.c_str(),
                          p4lib.packetTooShort.str());
    builder->newline();

    builder->emitIndent();
    builder->appendFormat("goto %s;", IR::ParserState::reject.c_str());
    builder->newline();
    builder->blockEnd(true);

   if (is_headers_type) {
      cstring hdr_type = type->to<IR::Type_StructLike>()->name.name;
      cstring hdr_name = membr->member.name;

      builder->emitIndent();
      builder->appendFormat("struct %s *headers = (struct %s *) malloc(sizeof(struct %s));\n", hdr_type, hdr_type, hdr_type);
      builder->emitIndent();
      builder->appendLine("if (headers == NULL) { fpp_errorCode = OutOfMemory; goto fpp_end; }");
      builder->emitIndent();
      builder->appendFormat("hdr = (packet_hdr_t *) malloc(sizeof(packet_hdr_t));\n");
      builder->emitIndent();
      builder->appendLine("if (hdr == NULL) { free(headers); fpp_errorCode = OutOfMemory; goto fpp_end; }");
      builder->emitIndent();
      builder->appendLine("");
      builder->emitIndent();
      builder->appendFormat("hdr->type = fpp_%s;\n", hdr_type);
      builder->emitIndent();
      builder->appendLine("hdr->hdr = headers;");
      builder->emitIndent();
      builder->appendLine("hdr->next = NULL;");
      builder->appendLine("");
      builder->emitIndent();
      builder->appendLine("if (*out == NULL) {");
      builder->emitIndent();
      builder->emitIndent();
      builder->appendLine("*out = hdr;");
      builder->emitIndent();
      builder->emitIndent();
      builder->appendLine("last_hdr = hdr;");
      builder->emitIndent();
      builder->appendLine("} else {");
      builder->emitIndent();
      builder->emitIndent();
      builder->appendLine("last_hdr->next = hdr;");
      builder->emitIndent();
      builder->emitIndent();
      builder->appendLine("last_hdr = hdr;");
      builder->emitIndent();
      builder->appendLine("}");
      builder->appendLine("");
      builder->emitIndent();
      builder->appendLine("headers->header_offset = fpp_packetOffsetInBits / 8;");
      builder->appendLine("");
   }

    unsigned alignment = 0;
    for (auto f : ht->fields) {
        auto ftype = state->parser->typeMap->getType(f);
        auto etype = FPPTypeFactory::instance->create(ftype);
        auto et = dynamic_cast<IHasWidth*>(etype);
        if (et == nullptr) {
            ::error("Only headers with fixed widths supported %1%", f);
            return;
        }
        compileExtractField(expr, f->name, alignment, etype);
        alignment += et->widthInBits();
        alignment %= 8;
    }

    builder->emitIndent();
    visit(expr);
    builder->appendLine(".header_valid = 1;");
    return;
}

bool StateTranslationVisitor::preorder(const IR::MethodCallExpression* expression) {
    builder->append("/* ");
    visit(expression->method);
    builder->append("(");
    bool first = true;
    for (auto a  : *expression->arguments) {
        if (!first)
            builder->append(", ");
        first = false;
        visit(a);
    }
    builder->append(")");
    builder->append("*/");
    builder->newline();

    auto mi = P4::MethodInstance::resolve(expression,
                                          state->parser->program->refMap,
                                          state->parser->program->typeMap);
    auto extMethod = mi->to<P4::ExternMethod>();
    if (extMethod != nullptr) {
        auto decl = extMethod->object;
        if (decl == state->parser->packet) {
            if (extMethod->method->name.name == p4lib.packetIn.extract.name) {
                compileExtract(expression->arguments);
                return false;
            } else if (extMethod->method->name.name == p4lib.packetIn.lookahead.name) {
               auto type = (expression->typeArguments[0])[0];
               compileLookahead(type);

               return false;
            } else if (extMethod->method->name.name == p4lib.packetIn.advance.name) {
               auto arg = expression->arguments->at(0);
               builder->emitIndent();
               builder->appendFormat("fpp_packetOffsetInBits += ");

               visit(arg);

               builder->append(";\n");
               return false;
            } else if (extMethod->method->name.name == p4lib.packetIn.length.name) {
               builder->append("packet_len");
               return false;
            }

            BUG("Unhandled packet method %1%", expression->method);
            return false;
        }
    }

    ::error("Unexpected method call in parser %1%", expression);
    return false;
}

bool StateTranslationVisitor::preorder(const IR::Member* expression) {
    if (expression->expr->is<IR::PathExpression>()) {
        auto pe = expression->expr->to<IR::PathExpression>();
        auto decl = state->parser->program->refMap->getDeclaration(pe->path, true);
        if (decl == state->parser->packet) {
           builder->append(expression->member);
            return false;
        }
    }

    headers_path = false;
    visit(expression->expr);

    if (headers_path) {
       builder->append("[0]");
    } else {
       builder->append(".");
       builder->append(expression->member);
    }
    headers_path = false;

    return false;
}

bool StateTranslationVisitor::preorder(const IR::PathExpression* expression) {
    visit(expression->path);
    return false;
}

bool StateTranslationVisitor::preorder(const IR::Path* p) {
    if (p->absolute)
        ::error("%1%: Unexpected absolute path", p);

    builder->append(p->name);
    if (p->name == "headers") {
       headers_path = true;
    }

    return false;
}

//////////////////////////////////////////////////////////////////

void FPPParserState::emit(CodeBuilder* builder) {
    StateTranslationVisitor visitor(this);
    visitor.setBuilder(builder);
    state->apply(visitor);
}

FPPParser::FPPParser(const FPPProgram* program, const IR::ParserBlock* block,
                       const P4::TypeMap* typeMap) :
        program(program), typeMap(typeMap), parserBlock(block),
        packet(nullptr), headers(nullptr), headerType(nullptr) {}

void FPPParser::emit(CodeBuilder* builder) {
    for (auto s : states)
        s->emit(builder);
    builder->newline();

    // Create a synthetic reject state
    builder->emitIndent();
    builder->appendFormat("%s: { return fpp_errorCode; }", IR::ParserState::reject.c_str());
    builder->newline();
    builder->newline();
}

bool FPPParser::build() {
    auto pl = parserBlock->container->type->applyParams;
    if (pl->size() != 2) {
        ::error("Expected parser to have exactly 2 parameters");
        return false;
    }

    auto it = pl->parameters.begin();
    packet = *it; ++it;
    headers = *it;
    for (auto state : parserBlock->container->states) {
        auto ps = new FPPParserState(state, this);
        states.push_back(ps);
    }

    auto ht = typeMap->getType(headers);
    if (ht == nullptr)
        return false;
    headerType = FPPTypeFactory::instance->create(ht);
    return true;
}

}  // namespace FPP
