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

#ifndef _BACKENDS_FPP_FPPMODEL_H_
#define _BACKENDS_FPP_FPPMODEL_H_

#include "frontends/common/model.h"
#include "frontends/p4/coreLibrary.h"
#include "ir/ir.h"
#include "lib/cstring.h"

namespace FPP {

struct Parser_Model : public ::Model::Elem {
    Parser_Model() : Elem("fpp"),
                     parser("prs") {}
    ::Model::Elem parser;
};

class FPPModel : public ::Model::Model {
 protected:
    FPPModel() : Model("0.1"),
                  packet("packet", P4::P4CoreLibrary::instance.packetIn, 0),
                  parser()
    {}

 public:
    static FPPModel instance;
    static cstring reservedPrefix;

    ::Model::Param_Model   packet;
    Parser_Model           parser;

    static cstring reserved(cstring name)
    { return reservedPrefix + name; }
};

}  // namespace FPP

#endif /* _BACKENDS_FPP_FPPMODEL_H_ */
