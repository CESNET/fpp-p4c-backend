# Modifications copyright 2017 CESNET
#
# Copyright 2013-present Barefoot Networks, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

bin_PROGRAMS += p4c-fpp
p4c_fpp_LDADD = libfrontend.la

#p4c_fpp_CXXFLAGS= -g -O0
p4c_fpp_SOURCES = \
	extensions/fpp/p4c-fpp.cpp \
	extensions/fpp/fppBackend.cpp \
	extensions/fpp/fppProgram.cpp \
	extensions/fpp/fppParser.cpp \
	extensions/fpp/fppType.cpp \
	extensions/fpp/fppModel.cpp \
	extensions/fpp/target.cpp \
	extensions/fpp/codeGen.cpp \
	extensions/fpp/midend.cpp

noinst_HEADERS += \
	extensions/fpp/codeGen.h \
	extensions/fpp/fppBackend.h \
	extensions/fpp/fppModel.h \
	extensions/fpp/fppObject.h \
	extensions/fpp/fppProgram.h \
	extensions/fpp/fppOptions.h \
	extensions/fpp/fppParser.h \
	extensions/fpp/fppType.h \
	extensions/fpp/midend.h \
	extensions/fpp/target.h

cpplint_FILES += $(p4c_fpp_SOURCES) $(p4c_fpp_UNIFIED) $(p4c_fpp_NONUNIFIED)

