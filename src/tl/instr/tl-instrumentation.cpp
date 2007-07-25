/*
    Mercurium C/C++ Compiler
    Copyright (C) 2006-2007 - Roger Ferrer Ibanez <roger.ferrer@bsc.es>
    Barcelona Supercomputing Center - Centro Nacional de Supercomputacion
    Universitat Politecnica de Catalunya

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "cxx-utils.h"
#include "tl-instrumentation.hpp"
#include "tl-instrumentcalls.hpp"
#include "tl-compilerphase.hpp"
#include "tl-predicateutils.hpp"
#include "tl-langconstruct.hpp"
#include "tl-traverse.hpp"
#include "tl-scopelink.hpp"

#include <iostream>
#include <fstream>
#include <set>

namespace TL
{
    class Instrumentation : public CompilerPhase
    {
        private:
            std::string instrument_enabled_str;
            std::string instrument_mode;
            std::string instrument_file_name;
            std::string instrument_filter_mode;
        public:
            Instrumentation()
            {
                // Phase description"
                set_phase_name("Mintaka instrumentation");
                set_phase_description("This phase adds instrumentation to either "
                        "function calls or function definitions using the mintaka runtime");

                // Phase parameters
                register_parameter("instrument", 
                        "If set to '1' enables instrumentation, otherwise it is disabled",
                        instrument_enabled_str, 
                        "0");

                register_parameter("instrument_mode", 
                        "It sets the kind of instrumentation done. Valid values are 'calls' or 'functions'. Currently only 'calls' is valid",
                        instrument_mode,
                        "calls");

                register_parameter("instrument_file_name", 
                        "Sets the filtering file for instrumentation",
                        instrument_file_name,
                        "./filter_instrument");

                register_parameter("instrument_filter_mode",
                        "Sets the filtering mode. It can be either 'normal' or 'inverted'",
                        instrument_filter_mode,
                        "normal");
            }

            void virtual run(DTO& dto)
            {
                CompilerPhase* instrument_phase = NULL;

                if (instrument_mode == "calls")
                {
                    instrument_phase = new InstrumentCalls(instrument_file_name, instrument_filter_mode);
                }
                else if (instrument_mode == "functions")
                {
                    std::cerr << "Instrumentation of functions disabled. Only 'calls' can be instrumented at the moment" << std::endl;
                }
                else
                {
                    std::cerr << "Invalid instrument_mode. It can be 'calls' or 'functions'" << std::endl;
                }

                if (instrument_phase == NULL)
                {
                    std::cerr << "Skipping instrumentation. No valid mode defined" << std::endl;
                    return;
                }

                if (instrument_enabled_str == "1"
                        || instrument_enabled_str == "yes"
                        || instrument_enabled_str == "true")
                {
                    instrument_phase->run(dto);
                }
                else
                {
                    std::cerr << "Instrumentation disabled. Enable it with '--variable=instrument:1'" << std::endl;
                }
            }
    };
}

EXPORT_PHASE(TL::Instrumentation);
