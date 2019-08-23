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

#include "lib/error.h"
#include "lib/nullstream.h"
#include "frontends/p4/evaluator/evaluator.h"

#include "fppBackend.h"
#include "fppOptions.h"
#include "target.h"
#include "fppType.h"
#include "fppProgram.h"

namespace FPP {

void run_fpp_backend(const FPPOptions& options, const IR::ToplevelBlock* toplevel,
                      P4::ReferenceMap* refMap, P4::TypeMap* typeMap) {
    if (toplevel == nullptr)
        return;

    auto main = toplevel->getMain();
    if (main == nullptr) {
        ::error("Could not locate top-level block; is there a %1% module?", IR::P4Program::main);
        return;
    }

    Target* target;
    if (options.target.isNullOrEmpty()) {
        target = new CTarget();
    } else {
        ::error("Unknown target %s", options.target);
        return;
    }

    CodeBuilder c(target);
    CodeBuilder h(target);

    FPPTypeFactory::createFactory(typeMap);
    auto fppprog = new FPPProgram(options, toplevel->getProgram(), refMap, typeMap, toplevel);
    if (!fppprog->build())
        return;

    if (options.file.isNullOrEmpty())
        return;

    cstring cfile = options.file;
    const char* dot = cfile.findlast('.');
    if (dot == nullptr)
        cfile = cfile + ".c";
    else
        cfile = cfile.before(dot) + ".c";
    auto cstream = openFile(cfile, false);
    if (cstream == nullptr)
        return;

    cstring hfile;
    dot = cfile.findlast('.');
    if (dot == nullptr)
        hfile = cfile + ".h";
    else
        hfile = cfile.before(dot) + ".h";
    auto hstream = openFile(hfile, false);
    if (hstream == nullptr)
        return;

    fppprog->emitH(&h, hfile);
    fppprog->emitC(&c, hfile);
    *cstream << c.toString();
    *hstream << h.toString();
    cstream->flush();
    hstream->flush();
}

}  // namespace FPP
