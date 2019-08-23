# Author(s): Jiri Havranek <havranek@cesnet.cz>
# Copyright (C) 2019 CESNET
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
# 3. Neither the name of the Company nor the names of its contributors
#    may be used to endorse or promote products derived from this
#    software without specific prior written permission.
#
# This software is provided ``as is'', and any express or implied
# warranties, including, but not limited to, the implied warranties of
# merchantability and fitness for a particular purpose are disclaimed.
# In no event shall the company or contributors be liable for any
# direct, indirect, incidental, special, exemplary, or consequential
# damages (including, but not limited to, procurement of substitute
# goods or services; loss of use, data, or profits; or business
# interruption) however caused and on any theory of liability, whether
# in contract, strict liability, or tort (including negligence or
# otherwise) arising in any way out of the use of this software, even
# if advised of the possibility of such damage.

set (MYBACKEND_SRCS
	p4c-fpp.cpp
	fppBackend.cpp
	fppProgram.cpp
	fppParser.cpp
	fppType.cpp
	fppModel.cpp
	target.cpp
	codeGen.cpp
	midend.cpp
)

set (MYBACKEND_HDRS
	codeGen.h
	fppBackend.h
	fppModel.h
	fppObject.h
	fppProgram.h
	fppOptions.h
	fppParser.h
	fppType.h
	midend.h
	target.h
   )


add_library(fpp_backend ${MYBACKEND_SRCS})
add_dependencies(fpp_backend genIR frontend)

add_executable(p4c-fpp p4c-fpp.cpp)
target_link_libraries (p4c-fpp fpp_backend ${P4C_LIBRARIES} ${P4C_LIB_DEPS})

install (TARGETS p4c-fpp
  RUNTIME DESTINATION ${P4C_RUNTIME_OUTPUT_DIRECTORY})
