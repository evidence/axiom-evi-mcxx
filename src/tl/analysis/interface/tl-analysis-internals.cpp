/*--------------------------------------------------------------------
  (C) Copyright 2006-2013 Barcelona Supercomputing Center
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
 
#include "tl-analysis-internals.hpp"

#include "tl-tribool.hpp"
 
namespace TL  {
namespace Analysis {

    /*
     *  PROPERTIES
     */

    TL::tribool uniform_property(
            Node* const scope_node,
            Node* const stmt_node,
            const Nodecl::NodeclBase& n,
            const Nodecl::NodeclBase& prev_n,
            ExtensibleGraph* const pcfg,
            std::set<Nodecl::NodeclBase> visited_nodes)
    {
        // Start of base cases for Uniform.
        if(Nodecl::Utils::nodecl_is_literal(n))
            return true;

        if (n.is<Nodecl::Symbol>())
        {
            TL::Symbol sym = n.get_symbol();

            // If it's in the linear clause -> false
            ObjectList<Utils::LinearVars> linear_vars = scope_node->get_linear_symbols();
            for (ObjectList<Utils::LinearVars>::iterator it = 
                    linear_vars.begin(); 
                    it != linear_vars.end(); 
                    ++it)
            {
                ObjectList<TL::Symbol> linear_syms = it->get_symbols();

                if (linear_syms.contains(sym))
                    return false;
            }

            // If it's in the uniform clause -> true
            ObjectList<TL::Symbol> uniform_syms = scope_node->get_uniform_symbols();
            if(uniform_syms.contains(sym))
                return true;
        }


        // Unknown RD
        if(n.is<Nodecl::Unknown>())
        {
            // Unknown parameter must be taken into account only
            // if is inside the scope_node, i.e. is a scope_node
            // is a function code
            if (scope_node->is_function_code_node())
            {
                if (prev_n.is<Nodecl::Symbol>() && 
                        prev_n.get_symbol().is_parameter())
                {
                    // TODO: if 'n' is uniform parameter: true
                    // TODO: else false. (if 'n' is non uniform or linear (IV) parameter)

                    // So far, assume not uniform by default
                    return false;
                }
                else
                {
                    internal_error("UniformProperty: Unknown RD from a non parameter node '%s'",
                            n.prettyprint().c_str());
                }
            }
            else
            {
                // Unknown parameter is not inside the scope
                // E.g. it's uniform
                return true;
            }
        }
        // TODO: n instead of stmt_node.
        // If the 'n' is not contained in the scope node,
        // then n is uniform in the scope
        if(!ExtensibleGraph::node_contains_node(
                    scope_node, stmt_node))
            return true;

        if(Nodecl::Utils::nodecl_contains_nodecl_of_kind
                <Nodecl::FunctionCall>(n)) 
            return false;

        // Check if n contains an IV of the scope
        // TODO: if it's not loop, return empty list?
        if(scope_node->is_loop_node())
        {
            Utils::InductionVarList scope_ivs =
                scope_node->get_induction_variables();

            for(Utils::InductionVarList::iterator it = scope_ivs.begin();
                    it != scope_ivs.end();
                    it++)
            {
                if(Nodecl::Utils::nodecl_contains_nodecl_by_structure(
                            n, (*it)->get_variable()))
                    return false;
            }
        }

        return TL::tribool::unknown;
    }

    /*
     *  QUERIES
     */
    bool is_uniform_internal(
            Node* const scope_node,
            Node* const stmt_node,
            const Nodecl::NodeclBase& n,
            ExtensibleGraph* const pcfg,
            std::set<Nodecl::NodeclBase> visited_nodes)
    {
        TL::tribool result = nodecl_has_property_in_scope(scope_node,
                stmt_node, stmt_node, n, Nodecl::NodeclBase::null(), pcfg,
                uniform_property, visited_nodes);

        ERROR_CONDITION(result.is_unknown(),
                "is_uniform_internal returns unknown!", 0);

        return result.is_true();
    }

    bool is_linear_internal(
            Node* const scope_node, 
            const Nodecl::NodeclBase& n)
    {
        if (n.is<Nodecl::Symbol>())
        {
            Symbol s(n.get_symbol());
            if(!s.is_valid())
            {
                WARNING_MESSAGE("Object %s is not linear because it is not a valid symbol.\n", 
                        n.prettyprint().c_str());
                return false;
            }

            Node* new_scope = scope_node;
            if(scope_node->is_loop_node())
            {
                new_scope = scope_node->get_outer_node();
                if(!new_scope->is_omp_simd_node())
                    goto iv_as_linear;
            }
            else if(scope_node->is_function_code_node())
            {
                new_scope = scope_node->get_outer_node();
                // If the scope is a function and it is not enclosed in a simd pragma
                // then no IVs will be attached to this node
                if(!new_scope->is_omp_simd_function_node())
                    goto final_linear;
            }
            // If, after checking the possibility of being enclosed within a simd node
            // then, only induction variables calculated during analysis may be reported
            if(!new_scope->is_omp_simd_node())
                goto iv_as_linear;

            // First try to get the information form the user clauses
            {
                ObjectList<Utils::LinearVars> linear_syms = new_scope->get_linear_symbols();
                for (ObjectList<Utils::LinearVars>::iterator it = linear_syms.begin(); it != linear_syms.end(); ++it)
                {
                    ObjectList<Symbol> syms = it->get_symbols();
                    for(ObjectList<Symbol>::iterator itt = syms.begin(); itt != syms.end(); ++itt)
                    {
                        if (*itt == s)
                            return true;
                    }
                }
            }
        }
        
        // Second get the information form the analysis: IV
iv_as_linear:
        if(scope_node->is_loop_node())
            return is_iv_internal(scope_node, n);
        
final_linear:
        return false;
    }
    
    bool has_been_defined_internal(Node* const n_node,
            const Nodecl::NodeclBase& n,
            const NodeclSet& global_variables)
    {
        bool result = false;

        if( n.is<Nodecl::Symbol>( ) || n.is<Nodecl::ArraySubscript>( )
                || n.is<Nodecl::ClassMemberAccess>( ) )
        {
            NodeclMap rd_in = n_node->get_reaching_definitions_in();
            std::pair<NodeclMap::iterator, NodeclMap::iterator> n_rds =
                rd_in.equal_range(n);
    
            if(n_rds.first != n_rds.second) // n has RDs
            {
                return true;
            }
            else // n doesn't have RDs
            {
                Nodecl::NodeclBase nodecl_base = Utils::get_nodecl_base(n);
                if (!nodecl_base.is_null())
                {
                    if(global_variables.find(nodecl_base) != 
                                global_variables.end()) // n is a global var
                        result = true;
                }
            }
        }
        else
        {
            WARNING_MESSAGE( "Nodecl '%s' is neither symbol, ArraySubscript or ClassMemberAccess. " \
                    "One of these types required as defined option. Returning false.\n", n.prettyprint( ).c_str( ) );
        }

        return result;
    }

    bool is_iv_internal(Node* const scope_node, const Nodecl::NodeclBase& n)
    { 
        bool result = false;

        Utils::InductionVarList ivs = scope_node->get_induction_variables();

        for( Utils::InductionVarList::const_iterator it = ivs.begin( );
                it != ivs.end( ); ++it )
        {
            if ( Nodecl::Utils::structurally_equal_nodecls(
                        ( *it )->get_variable( ), n,
                        /* skip conversion nodes */ true ) )
            {
                result = ( *it )->is_basic( );
                break;
            }
        }

        return result;
    }

    bool is_non_reduction_basic_iv_internal(Node* const scope_node,
            const Nodecl::NodeclBase& n)
    {
        bool result = false;
        
        if (scope_node->is_loop_node() || scope_node->is_function_code_node())
        {
            Utils::InductionVarList ivs = scope_node->get_induction_variables();
            ObjectList<TL::Symbol> reductions =
                scope_node->get_reductions();

            for( Utils::InductionVarList::const_iterator it = ivs.begin( );
                    it != ivs.end( ); ++it )
            {
                if( !reductions.contains( ( *it )->get_variable( ).get_symbol( ) ) )
                {
                    if ( Nodecl::Utils::structurally_equal_nodecls(
                                ( *it )->get_variable( ), n,
                                /* skip conversion nodes */ true ) )
                    {
                        result = ( *it )->is_basic( );
                        break;
                    }
                }
            }
        }

        return result;
    }

    NodeclSet get_iv_lower_bound_internal(Node* const scope_node,
            const Nodecl::NodeclBase& n)
    {
        NodeclSet result;

        const Utils::InductionVarList& ivs = scope_node->get_induction_variables();
        Utils::InductionVarList::const_iterator it;
        for (it = ivs.begin(); it != ivs.end(); ++it)
        {
            if (Nodecl::Utils::structurally_equal_nodecls((*it)->get_variable(), n, /*skip_conversions*/ true))
                return (*it)->get_lb();
        }

        WARNING_MESSAGE("You are asking for the lower bound of an Object (%s) "
                        "which is not an Induction Variable\n",
                        n.prettyprint().c_str());
        return result;
    }

    Nodecl::NodeclBase get_iv_increment_internal(Node* const scope_node,
            const Nodecl::NodeclBase& n)
    {
        Nodecl::NodeclBase result;

        Utils::InductionVarList ivs =
            scope_node->get_induction_variables();

        Utils::InductionVarList::const_iterator it;
        for( it = ivs.begin( );
             it != ivs.end( ); ++it )
        {
            if ( Nodecl::Utils::structurally_equal_nodecls(
                        ( *it )->get_variable( ), n,
                        /* skip conversion nodes */ true ) )
            {
                result = ( *it )->get_increment( );
                break;
            }
        }

        if( it == ivs.end( ) )
        {
            WARNING_MESSAGE( "You are asking for the increment bound of an Object ( %s ) "\
                             "which is not an Induction Variable\n", n.prettyprint( ).c_str( ) );
        }

        return result;
    }
    
    Utils::InductionVarList get_linear_variables_internal(Node* const scope_node)
    {
        ObjectList<Utils::LinearVars> linear_syms;
        Utils::InductionVarList result;
        
        Node* new_scope = scope_node;
        if(scope_node->is_loop_node())
        {
            new_scope = scope_node->get_outer_node();
            if(!new_scope->is_omp_simd_node())
                goto get_ivs;
        }
        else if(scope_node->is_function_code_node())
        {
            new_scope = scope_node->get_outer_node();
            // If the scope is a function and it is not enclosed in a simd pragma
            // then no linear variables can be related to this node
            if(!new_scope->is_omp_simd_function_node())
                goto final_get_linear;
        }
        // If, after checking the possibility of being enclosed within a simd node
        // then, only induction variables calculated during analysis may be reported
        if(!new_scope->is_omp_simd_node())
            goto get_ivs;
        
        linear_syms = new_scope->get_linear_symbols();
        
get_ivs:
        // Get the induction variables if the scope is a loop
        if(scope_node->is_loop_node())
        {
            result = scope_node->get_induction_variables();
        }
        // Enrich the induction variables list with the linear symbols of the clause, if there are
        for(ObjectList<Utils::LinearVars>::iterator it = linear_syms.begin(); it != linear_syms.end(); ++it)
        {            
            Symbol s;
            ObjectList<Symbol> syms = it->get_symbols();
            for(ObjectList<Symbol>::iterator itt = syms.begin(); itt != syms.end(); ++itt)
            {
                bool found = false;
                Utils::InductionVarList::iterator ittt = result.begin();
                for( ; ittt != result.end(); ++ittt)
                {
                    found = false;
                    s = (*ittt)->get_variable().get_symbol();
                    if(s.is_valid() && (s==*itt))
                    {
                        NBase step = (*ittt)->get_increment();
                        if(!Nodecl::Utils::structurally_equal_nodecls(step, it->get_step()))
                        {
                            WARNING_MESSAGE("Step set by the user for linear variable %s "\
                                    "is different from step computed during analysis phase.\n", 
                                    s.get_name().c_str());
                            (*ittt)->set_increment(it->get_step());
                        }
                        found = true;
                        break;
                    }
                }
                if(!found)
                {
                    // The variable in the linear clause has not been recognized during analysis as an IV
                    // We add it here to the list
                    Utils::InductionVar* iv = new Utils::InductionVar(itt->make_nodecl(/*set_ref_type*/ false));
                    iv->set_increment(it->get_step());
                    result.append(iv);
                }
            }
        }
        
final_get_linear:
        return result;
    }
    
    NodeclSet get_linear_variable_lower_bound_internal(Node* const scope_node, const Nodecl::NodeclBase& n)
    {
        Utils::InductionVarList scope_ivs = get_linear_variables_internal(scope_node);
        for(Utils::InductionVarList::iterator it = scope_ivs.begin(); it != scope_ivs.end(); it++)
        {
            NBase v = (*it)->get_variable();
            if(Nodecl::Utils::structurally_equal_nodecls(n, v, /*skip_conversions*/true))
                return (*it)->get_lb();
        }
        
        return NodeclSet();
    }
    
    NBase get_linear_variable_increment_internal(Node* const scope_node, const Nodecl::NodeclBase& n)
    {
        Utils::InductionVarList scope_ivs = get_linear_variables_internal(scope_node);
        for(Utils::InductionVarList::iterator it = scope_ivs.begin(); it != scope_ivs.end(); it++)
        {
            NBase v = (*it)->get_variable();
            if(Nodecl::Utils::structurally_equal_nodecls(n, v, /*skip_conversions*/true))
                return (*it)->get_increment();
        }
        
        internal_error( "You are asking for the increment of '%s' "\
                        "which is neither induction variable nor linear\n",
                        n.prettyprint( ).c_str( ) );
    }
}
}
