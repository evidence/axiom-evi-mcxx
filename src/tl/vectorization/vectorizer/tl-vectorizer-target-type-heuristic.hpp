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

#ifndef TL_VECTORIZER_TARGET_TYPE_HEURISTIC_HPP
#define TL_VECTORIZER_TARGET_TYPE_HEURISTIC_HPP

#include "tl-nodecl-visitor.hpp"
#include "tl-nodecl-base.hpp"


namespace TL
{
namespace Vectorization
{
    class VectorizerTargetTypeHeuristic : public Nodecl::ExhaustiveVisitor<void>
    {
        private:
            int _1b, _2b, _4b, _8b;

            void count_type(const TL::Type& type);

        public:
            VectorizerTargetTypeHeuristic();
            TL::Type get_target_type(const Nodecl::NodeclBase& n);

            void visit(const Nodecl::ArraySubscript& n);
            void visit(const Nodecl::ObjectInit& n);
    };
}
}

#endif //TL_VECTORIZER_TARGET_TYPE_HEURISTIC_HPP

