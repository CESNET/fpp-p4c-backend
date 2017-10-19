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

#include "target.h"
#include "fppType.h"

namespace FPP {

void CTarget::emitIncludes(Util::SourceCodeBuilder* builder) const {
    builder->append("#include <stdint.h>\n");
    builder->append("#include <stdlib.h>\n");
    builder->append("#include <arpa/inet.h>\n");
}

void CTarget::emitLicense(Util::SourceCodeBuilder*, cstring) const {}

void CTarget::emitMain(Util::SourceCodeBuilder* builder,
                                   cstring functionName) const {
    builder->appendFormat("int %s(const uint8_t *packet, uint32_t packet_len, packet_hdr_t **out)", functionName);
}

}  // namespace FPP
