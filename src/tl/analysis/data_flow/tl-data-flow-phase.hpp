/*--------------------------------------------------------------------
  (C) Copyright 2006-2012 Barcelona Supercomputing Center
                          Centro Nacional de Supercomputacion
  
  This file is part of Mercurium C/C++ source-to-source compiler.
  
  See AUTHORS file in the top level directory for information 
  regarding developers and contributors.
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 3 of the License, or (at your option) any later version.
  
  Mercurium C/C++ source-to-source compiler is distributed in the hope
  that it will be useful, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the GNU Lesser General Public License for more
  details.
  
  You should have received a copy of the GNU Lesser General Public
  License along with Mercurium C/C++ source-to-source compiler; if
  not, write to the Free Software Foundation, Inc., 675 Mass Ave,
  Cambridge, MA 02139, USA.
--------------------------------------------------------------------*/

#ifndef TL_DATA_FLOW_PHASE_HPP
#define TL_DATA_FLOW_PHASE_HPP

#include "tl-objectlist.hpp"
#include "tl-compilerphase.hpp"

namespace TL
{
    namespace Analysis
    {
        //!This phase performs different data-flow analysis
        /*!Further analysis and optimizations requiere this phase to be executed previously
        * The analysis and optimizations implemented in this phase are:
        * - Parallel Control Flow Analysis: This generates a PCFG
        * - Use-Definition analysis
        * - Constant propagation and folding
        * - Unreachable code elimination
        * - Common subexpression elimination
        */ 
        class LIBTL_CLASS DataFlowPhase : public CompilerPhase
        {
            public:
                //!Constructor of this phase
                DataFlowPhase();
                
                //!Entry point of the phase
                /*!This function gets the different FunctionDefinitions / ProgramUnits of the DTO,
                 * depending on the language of the code
                 */
                virtual void run(TL::DTO& dto);
        };
    }
}

#endif  // TL_FLOW_PHASE_HPP