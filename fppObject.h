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

#ifndef _BACKENDS_FPP_FPPOBJECT_H_
#define _BACKENDS_FPP_FPPOBJECT_H_

#include "target.h"
#include "fppModel.h"
#include "ir/ir.h"
#include "frontends/p4/typeMap.h"
#include "frontends/p4/evaluator/evaluator.h"
#include "codeGen.h"

namespace FPP {

// Base class for FPP objects
class FPPObject {
 public:
    virtual ~FPPObject() {}
    template<typename T> bool is() const { return to<T>() != nullptr; }
    template<typename T> const T* to() const {
        return dynamic_cast<const T*>(this); }
    template<typename T> T* to() {
        return dynamic_cast<T*>(this); }
};

}  // namespace FPP

#endif /* _BACKENDS_FPP_FPPOBJECT_H_ */
