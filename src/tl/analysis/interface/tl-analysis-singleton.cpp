/*--------------------------------------------------------------------
 (C) Copyright 2006-2013 Barcelona Supercomputing Center             *
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


#include "cxx-cexpr.h"
#include "cxx-process.h"

#include "tl-analysis-singleton.hpp"
#include "tl-analysis-utils.hpp"
#include "tl-auto-scope.hpp"
#include "tl-cyclomatic-complexity.hpp"
#include "tl-iv-analysis.hpp"
#include "tl-liveness.hpp"
#include "tl-loop-analysis.hpp"
#include "tl-pcfg-visitor.hpp"
#include "tl-pointer-size.hpp"
#include "tl-range-analysis.hpp"
#include "tl-reaching-definitions.hpp"
#include "tl-task-sync.hpp"
#include "tl-task-syncs-tune.hpp"
#include "tl-use-def.hpp"

namespace TL {
namespace Analysis {

    // ************************************************************************************ //
    // *************** Class containing all analysis related to a given AST *************** //

    PCFGAnalysis_memento::PCFGAnalysis_memento()
            : _pcfgs(), _tdgs(),
              _pcfg(false), /*_constants_propagation(false),*/ _canonical(false),
              _use_def(false), _liveness(false), _loops(false),
              _reaching_definitions(false), _induction_variables(false),
              _tune_task_syncs(false), _range(false), _cyclomatic_complexity(false),
              _auto_scoping(false), _auto_deps(false), _tdg(false)
    {}

    ExtensibleGraph* PCFGAnalysis_memento::get_pcfg(std::string name) const
    {
        ExtensibleGraph* pcfg = NULL;
        Name_to_pcfg_map::const_iterator pcfgs_it = _pcfgs.find(name);
        if (pcfgs_it != _pcfgs.end())
            pcfg = pcfgs_it->second;
        return pcfg;
    }

    void PCFGAnalysis_memento::add_pcfg(std::string name, ExtensibleGraph* pcfg)
    {
        _pcfgs[name] = pcfg;
    }

    ObjectList<ExtensibleGraph*> PCFGAnalysis_memento::get_pcfgs() const
    {
        ObjectList<ExtensibleGraph*> result;
        for (Name_to_pcfg_map::const_iterator it = _pcfgs.begin(); it != _pcfgs.end(); ++it)
            result.insert(it->second);
        return result;
    }

    TaskDependencyGraph* PCFGAnalysis_memento::get_tdg(std::string name) const
    {
        TaskDependencyGraph* tdg = NULL;
        Name_to_tdg_map::const_iterator tdgs_it = _tdgs.find(name);
        if (tdgs_it != _tdgs.end())
            tdg = tdgs_it->second;
        return tdg;
    }

    void PCFGAnalysis_memento::set_tdg(std::string name, TaskDependencyGraph* tdg)
    {
        _tdgs[name] = tdg;
    }

    bool PCFGAnalysis_memento::is_pcfg_computed() const
    {
        return _pcfg;
    }

    void PCFGAnalysis_memento::set_pcfg_computed()
    {
        _pcfg = true;
    }

//     bool PCFGAnalysis_memento::is_constants_propagation_computed() const
//     {
//         return _constants_propagation;
//     }
// 
//     void PCFGAnalysis_memento::set_constants_propagation_computed()
//     {
//         _constants_propagation = true;
//     }

    bool PCFGAnalysis_memento::is_canonical_computed() const
    {
        return _canonical;
    }

    void PCFGAnalysis_memento::set_canonical_computed()
    {
        _canonical = true;
    }

    bool PCFGAnalysis_memento::is_usage_computed() const
    {
        return _use_def;
    }

    void PCFGAnalysis_memento::set_usage_computed()
    {
        _use_def = true;
    }

    bool PCFGAnalysis_memento::is_liveness_computed() const
    {
        return _liveness;
    }

    void PCFGAnalysis_memento::set_liveness_computed()
    {
        _liveness = true;
    }

    bool PCFGAnalysis_memento::is_loops_computed() const
    {
        return _loops;
    }

    void PCFGAnalysis_memento::set_loops_computed()
    {
        _loops = true;
    }

    bool PCFGAnalysis_memento::is_reaching_definitions_computed() const
    {
        return _reaching_definitions;
    }

    void PCFGAnalysis_memento::set_reaching_definitions_computed()
    {
        _reaching_definitions = true;
    }

    bool PCFGAnalysis_memento::is_induction_variables_computed() const
    {
        return _induction_variables;
    }

    void PCFGAnalysis_memento::set_induction_variables_computed()
    {
        _induction_variables = true;
    }

    bool PCFGAnalysis_memento::is_task_synchronizations_tuned() const
    {
        return _tune_task_syncs;
    }

    void PCFGAnalysis_memento::set_tune_task_synchronizations()
    {
        _tune_task_syncs = true;
    }

    bool PCFGAnalysis_memento::is_range_analysis_computed() const
    {
        return _range;
    }

    void PCFGAnalysis_memento::set_range_analysis_computed()
    {
        _range = true;
    }

    bool PCFGAnalysis_memento::is_cyclomatic_complexity_computed() const
    {
        return _cyclomatic_complexity;
    }
    
    void PCFGAnalysis_memento::set_cyclomatic_complexity_computed()
    {
        _cyclomatic_complexity = true;
    }
    
    bool PCFGAnalysis_memento::is_auto_scoping_computed() const
    {
        return _auto_scoping;
    }

    void PCFGAnalysis_memento::set_auto_scoping_computed()
    {
        _auto_scoping = true;
    }

    bool PCFGAnalysis_memento::is_auto_deps_computed() const
    {
        return _auto_deps;
    }

    void PCFGAnalysis_memento::set_auto_deps_computed()
    {
        _auto_deps = true;
    }

    bool PCFGAnalysis_memento::is_tdg_computed() const
    {
        return _tdg;
    }

    void PCFGAnalysis_memento::set_tdg_computed()
    {
        _tdg = true;
    }

    Node* PCFGAnalysis_memento::node_enclosing_nodecl_rec(Node* current, const NBase& n)
    {
        Node* result = NULL;
        if (!current->is_visited())
        {
            current->set_visited(true);
            if (current->is_exit_node())
            {
                return NULL;
            }
            else if (current->is_graph_node())
            {
                // We a node is found as label of a PCFGNode, i might be confusing:
                // When calling to analysis with a non-TopLevel node, both the EXTENSIBLE_GRAPH and
                // the non-TopLevel node, have the same label, so we have to look for the one that
                // is not the EXTENSIBLE_GRAPH node.
                if (Nodecl::Utils::structurally_equal_nodecls(current->get_graph_related_ast(), n, /* skip conversion nodes */ true)
                    && (n.is<Nodecl::FunctionCode>()
                         || n.is<Nodecl::OpenMP::SimdFunction>()
                         || (!n.is<Nodecl::FunctionCode>() && !current->is_extended_graph_node())))
                {
                    result = current;
                }
                else
                {
                    result = node_enclosing_nodecl_rec(current->get_graph_entry_node(), n);
                }
            }
            else if (!current->is_entry_node())
            {
                ObjectList<NBase> stmts = current->get_statements();
                for (ObjectList<NBase>::iterator it = stmts.begin(); it != stmts.end(); ++it)
                {
                    if (Nodecl::Utils::structurally_equal_nodecls(*it, n))
                    {
                        result = current;
                        break;
                    }
                }
            }

            if (result == NULL)
            {
                ObjectList<Node*> children = current->get_children();
                for (ObjectList<Node*>::iterator it = children.begin(); it != children.end(); ++it)
                {
                    result = node_enclosing_nodecl_rec(*it, n);
                    if (result != NULL)
                    {
                        break;
                    }
                }
            }
        }

        return result;
    }

    Node* PCFGAnalysis_memento::node_enclosing_nodecl(const NBase& n)
    {
        Node* result = NULL;
        for (Name_to_pcfg_map::iterator it = _pcfgs.begin(); it != _pcfgs.end(); ++it)
        {
            Node* current = it->second->get_graph();
            result = node_enclosing_nodecl_rec(current, n);
            ExtensibleGraph::clear_visits(current);

            if (result != NULL)
            {
                break;
            }
        }

        if (result == NULL)
        {
            nodecl_t internal_n = n.get_internal_nodecl();
            WARNING_MESSAGE("Nodecl '%s' do not found in current analysis state. "\
                             "You might have modified the code you used to compute the analyses and "\
                             "the nodecl you are asking for now did not exist before.",
                             codegen_to_str(internal_n, nodecl_retrieve_context(internal_n)));
        }

        return result;
    }


    Utils::InductionVarList PCFGAnalysis_memento::get_induction_variables(const NBase& n)
    {
        Utils::InductionVarList result;
        if (_induction_variables)
        {
            Node* pcfg_node = node_enclosing_nodecl(n);
            if ((pcfg_node != NULL) && pcfg_node->is_loop_node())
                result =  pcfg_node->get_induction_variables();
        }
        return result;
    }

    ObjectList<Symbol> PCFGAnalysis_memento::get_reductions(const NBase& n)
    {
        ObjectList<Symbol> result;
        if (_induction_variables)
        {
            Node* pcfg_node = node_enclosing_nodecl(n);
            if (pcfg_node != NULL)
            {
                Node* pcfg_omp_node = ExtensibleGraph::get_omp_enclosing_node(pcfg_node);
                while(pcfg_omp_node != NULL &&
                      !pcfg_omp_node->is_omp_parallel_node() && !pcfg_omp_node->is_omp_loop_node() &&
                      !pcfg_omp_node->is_omp_sections_node() && !pcfg_omp_node->is_omp_simd_node())
                {
                    pcfg_omp_node = ExtensibleGraph::get_omp_enclosing_node(pcfg_omp_node);
                }

                if (pcfg_omp_node != NULL)
                    result = pcfg_omp_node->get_reductions();
            }
        }
        return result;
    }

    NodeclSet PCFGAnalysis_memento::get_killed(const NBase& n)
    {
        NodeclSet result;

        Node* n_pcfg_node = node_enclosing_nodecl(n);
        if (n_pcfg_node != NULL)
        {
            result = n_pcfg_node->get_killed_vars();
        }

        return result;
    }

    Node* PCFGAnalysis_memento::get_autoscoped_task(const NBase& n)
    {
        Node* result = NULL;

        Node* n_pcfg_node = node_enclosing_nodecl(n);
        if (n_pcfg_node != NULL)
        {
            if (n_pcfg_node->is_auto_scoping_enabled())
                result = n_pcfg_node;
        }

        return result;
    }

    // ************* END class containing all analysis related to a given AST ************* //
    // ************************************************************************************ //



    // ************************************************************************ //
    // *********** Analysis Singleton class (Memento originator) ************ //

    static bool IsOmpssEnabled = false;
    
    // Private constructor
    AnalysisSingleton::AnalysisSingleton()
    {}

    // Single instance constructor
    AnalysisSingleton& AnalysisSingleton::get_analysis(bool is_ompss_enabled)
    {
        static AnalysisSingleton analysis;

        IsOmpssEnabled = is_ompss_enabled;
        
        return analysis;
    }

    void AnalysisSingleton::parallel_control_flow_graph(
            PCFGAnalysis_memento& memento,
            const NBase& ast)
    {
        if (!memento.is_pcfg_computed())
        {
            memento.set_pcfg_computed();

            ObjectList<NBase> unique_asts;
            std::map<Symbol, NBase> asserted_funcs;

            // Get all unique ASTs embedded in 'ast'
            if (!ast.is<Nodecl::TopLevel>())
            {
                unique_asts.append(ast);
            }
            else
            {
                // Get all functions in \ast
                Utils::TopLevelVisitor tlv;
                tlv.walk_functions(ast);
                unique_asts = tlv.get_functions();
                asserted_funcs = tlv.get_asserted_funcs();
            }

            // Compute the PCFG corresponding to each AST
            for (ObjectList<NBase>::iterator it = unique_asts.begin(); it != unique_asts.end(); ++it)
            {
                // Generate the hashed name corresponding to the AST of the function
                std::string pcfg_name = Utils::generate_hashed_name(*it);

                // Create the PCFG only if it has not been created previously
                if (memento.get_pcfg(pcfg_name) == NULL)
                {
                    // Create the PCFG
                    if (VERBOSE)
                        std::cerr << "Parallel Control Flow Graph (PCFG) '" << pcfg_name << "'" << std::endl;
                    PCFGVisitor v(pcfg_name, *it);
                    ExtensibleGraph* pcfg = v.parallel_control_flow_graph(*it, asserted_funcs);

                    // Synchronize the tasks, if applies
                    if (VERBOSE)
                        std::cerr << "Task Synchronization of PCFG '" << pcfg_name << "'" << std::endl;
                    TaskAnalysis::TaskSynchronizations task_sync_analysis(pcfg, IsOmpssEnabled);
                    task_sync_analysis.compute_task_synchronizations();

                    // Store the pcfg in the singleton
                    memento.add_pcfg(pcfg_name, pcfg);
                }
            }
        }
    }

    // TODO
//     void AnalysisSingleton::conditional_constant_propagation(PCFGAnalysis_memento& memento,
//                                                               const NBase& ast)
//     {
//         if (!memento.is_constants_propagation_computed())
//         {
//             memento.set_constants_propagation_computed();
// 
//             ObjectList<ExtensibleGraph*> pcfgs = parallel_control_flow_graph(memento, ast);
// 
//             for (ObjectList<ExtensibleGraph*>::iterator it = pcfgs.begin(); it != pcfgs.end(); ++it)
//             {
//                 if (VERBOSE)
//                     printf("Constants Propagation of PCFG '%s'\n", (*it)->get_name().c_str());
//                 std::cerr << "Constants Propagation is not yet implemented" << std::endl;
//                 // ConditionalConstantAnalysis ca(ipa);
//                 // ca.conditional_constant_propagation(pcfg);
//             }
//         }
//     }

    static void use_def_rec(
            Symbol func_sym, 
            std::set<Symbol>& visited_funcs, 
            ObjectList<ExtensibleGraph*>& pcfgs)
    {
        // Nothing to do if the we are analyzing something that:
        // - is not a function
        // - has already been analyzed
        if (!func_sym.is_valid() || (visited_funcs.find(func_sym) != visited_funcs.end()))
            return;

        for (ObjectList<ExtensibleGraph*>::const_iterator it = pcfgs.begin(); it != pcfgs.end(); ++it)
        {
            Symbol it_func_sym((*it)->get_function_symbol());
            if (it_func_sym.is_valid() && it_func_sym == func_sym)
            {
                visited_funcs.insert(func_sym);
                if (!(*it)->usage_is_computed())
                {
                    // Recursively analyze the functions called from the current graph
                    ObjectList<Symbol> called_funcs = (*it)->get_function_calls();
                    for (ObjectList<Symbol>::iterator itf = called_funcs.begin(); itf != called_funcs.end(); ++itf)
                        use_def_rec(*itf, visited_funcs, pcfgs);

                    // Analyze the current graph
                    if (VERBOSE)
                        std::cerr << "Use-Definition of PCFG '" << (*it)->get_name() << "'" << std::endl;
                    UseDef ud(*it, pcfgs);
                    ud.compute_usage();
                }
            }
        }
    }

    void AnalysisSingleton::use_def(
            PCFGAnalysis_memento& memento,
            const NBase& ast)
    {
        // Required previous analysis
        parallel_control_flow_graph(memento, ast);

        if (!memento.is_usage_computed())
        {
            memento.set_usage_computed();

            std::set<Symbol> visited_funcs;
            ObjectList<ExtensibleGraph*> pcfgs = memento.get_pcfgs();
            for (ObjectList<ExtensibleGraph*>::iterator it = pcfgs.begin(); it != pcfgs.end(); ++it)
            {
                if (!(*it)->usage_is_computed())
                {
                    PointerSize ps(*it);
                    ps.compute_pointer_vars_size();
                    use_def_rec((*it)->get_function_symbol(), visited_funcs, pcfgs);
                }
            }
        }
    }

    void AnalysisSingleton::liveness(
            PCFGAnalysis_memento& memento,
            const NBase& ast)
    {
        // Required previous analysis
        use_def(memento, ast);

        if (!memento.is_liveness_computed())
        {
            memento.set_liveness_computed();

            ObjectList<ExtensibleGraph*> pcfgs = memento.get_pcfgs();
            for (ObjectList<ExtensibleGraph*>::iterator it = pcfgs.begin(); it != pcfgs.end(); ++it)
            {
                if (VERBOSE)
                    std::cerr << "Liveness of PCFG '" << (*it)->get_name() << "'" << std::endl;
                Liveness l(*it);
                l.compute_liveness();
            }
        }
    }

    void AnalysisSingleton::reaching_definitions(
            PCFGAnalysis_memento& memento,
            const NBase& ast)
    {
        // Required previous analysis
        liveness(memento, ast);

        if (!memento.is_reaching_definitions_computed())
        {
            memento.set_reaching_definitions_computed();

            const ObjectList<ExtensibleGraph*>& pcfgs = memento.get_pcfgs();
            for (ObjectList<ExtensibleGraph*>::const_iterator it = pcfgs.begin(); it != pcfgs.end(); ++it)
            {
                if (VERBOSE)
                    std::cerr << "Reaching Definitions of PCFG '" << (*it)->get_name() << "'" << std::endl;
                ReachingDefinitions rd(*it);
                rd.compute_reaching_definitions();
            }
        }
    }

    void AnalysisSingleton::induction_variables(
            PCFGAnalysis_memento& memento,
            const NBase& ast)
    {
        // Required previous analysis
        reaching_definitions(memento, ast);

        if (!memento.is_induction_variables_computed())
        {
            memento.set_induction_variables_computed();

            ObjectList<ExtensibleGraph*> pcfgs = memento.get_pcfgs();
            for (ObjectList<ExtensibleGraph*>::iterator it = pcfgs.begin(); it != pcfgs.end(); ++it)
            {
                if (VERBOSE)
                    std::cerr << "Induction Variables of PCFG '" << (*it)->get_name() << "'" << std::endl;

                // Compute the induction variables of all loops of each PCFG
                InductionVariableAnalysis iva(*it);
                iva.compute_induction_variables();

                // Compute the limits of the induction variables
                Utils::InductionVarsPerNode ivs = iva.get_all_induction_vars();
                LoopAnalysis la(*it, ivs);
                la.compute_loop_ranges();

                if (VERBOSE)
                    Utils::print_induction_vars(ivs);
            }
        }
    }

    void AnalysisSingleton::tune_task_synchronizations(
            PCFGAnalysis_memento& memento,
            const NBase& ast)
    {
        // Required previous analysis
        reaching_definitions(memento, ast);

        if (!memento.is_task_synchronizations_tuned())
        {
            memento.set_tune_task_synchronizations();

            ObjectList<ExtensibleGraph*> pcfgs = memento.get_pcfgs();
            for (ObjectList<ExtensibleGraph*>::iterator it = pcfgs.begin(); it != pcfgs.end(); ++it)
            {
                if (VERBOSE)
                    std::cerr << "Task Synchronizations Tunning of PCFG '" << (*it)->get_name() << "'" << std::endl;

                TaskAnalysis::TaskSyncTunning tst(*it);
                tst.tune_task_synchronizations();
            }
        }
    }

    void AnalysisSingleton::range_analysis(
            PCFGAnalysis_memento& memento,
            const NBase& ast)
    {
        // Required previous analysis
        parallel_control_flow_graph(memento, ast);
        
        if (!memento.is_range_analysis_computed())
        {
            memento.set_range_analysis_computed();

            ObjectList<ExtensibleGraph*> pcfgs = memento.get_pcfgs();
            for (ObjectList<ExtensibleGraph*>::iterator it = pcfgs.begin(); it != pcfgs.end(); ++it)
            {
                if (VERBOSE)
                    std::cerr << "Range Analysis of PCFG '" << (*it)->get_name() << "'" << std::endl;

                // Compute the induction variables of all loops of each PCFG
                RangeAnalysis ra(*it);
                ra.compute_range_analysis();
            }
        }
    }

    void AnalysisSingleton::cyclomatic_complexity(
            PCFGAnalysis_memento& memento, 
            const NBase& ast)
    {
        // Required previous analysis
        parallel_control_flow_graph(memento, ast);
        
        if (!memento.is_cyclomatic_complexity_computed())
        {
            memento.set_cyclomatic_complexity_computed();
            
            ObjectList<ExtensibleGraph*> pcfgs = memento.get_pcfgs();
            for (ObjectList<ExtensibleGraph*>::iterator it = pcfgs.begin(); it != pcfgs.end(); ++it)
            {
                if (VERBOSE)
                    std::cerr << "Cyclomatic Complexity of PCFG '" << (*it)->get_name() << "'" << std::endl;
                
                // Compute the cyclomatic complexity of each PCFG
                CyclomaticComplexity cc(*it);
                unsigned int res = cc.compute_cyclomatic_complexity();
                if (VERBOSE)
                    printf(" = %d\n", res);
            }
        }
    }
    
    void AnalysisSingleton::auto_scoping(
            PCFGAnalysis_memento& memento,
            const NBase& ast)
    {
        // Required previous analysis
        tune_task_synchronizations(memento, ast);

        if (!memento.is_auto_scoping_computed())
        {
            memento.set_auto_scoping_computed();

            ObjectList<ExtensibleGraph*> pcfgs = memento.get_pcfgs();
            for (ObjectList<ExtensibleGraph*>::iterator it = pcfgs.begin(); it != pcfgs.end(); ++it)
            {
                if (VERBOSE)
                    std::cerr << "Auto-Scoping of PCFG '" << (*it)->get_name() << "'" << std::endl;

                AutoScoping as(*it);
                as.compute_auto_scoping();
            }
        }
    }

    ObjectList<TaskDependencyGraph*> AnalysisSingleton::task_dependency_graph(
            PCFGAnalysis_memento& memento,
            const NBase& ast)
    {
        ObjectList<TaskDependencyGraph*> tdgs;
        
        // Required previous analyses
        induction_variables(memento, ast);
        range_analysis(memento, ast);
        tune_task_synchronizations(memento, ast);
        
        if (!memento.is_tdg_computed())
        {
            memento.set_tdg_computed();

            ObjectList<ExtensibleGraph*> pcfgs = memento.get_pcfgs();
            for (ObjectList<ExtensibleGraph*>::iterator it = pcfgs.begin(); it != pcfgs.end(); ++it)
            {
                if (VERBOSE)
                    std::cerr << "Task Dependency Graph (TDG) of PCFG '" << (*it)->get_name() << "'" << std::endl;

                TaskDependencyGraph* tdg = new TaskDependencyGraph(*it);
                tdgs.insert(tdg);
                memento.set_tdg((*it)->get_name(), tdg);
            }
        }

        return tdgs;
    }

    void AnalysisSingleton::all_analyses(
            PCFGAnalysis_memento& memento,
            const NBase& ast)
    {
        // This launches PCFG, UseDef, Liveness, ReachingDefs and InductionVars analysis
        induction_variables(memento, ast);
        // This launches Auto-Scoping
        auto_scoping(memento, ast);
    }

    void AnalysisSingleton::print_pcfg(PCFGAnalysis_memento& memento, std::string pcfg_name)
    {
        ExtensibleGraph* pcfg = memento.get_pcfg(pcfg_name);
        if (CURRENT_CONFIGURATION->debug_options.print_pcfg_w_analysis ||
            CURRENT_CONFIGURATION->debug_options.print_pcfg_full)
        {   // Print analysis information
            if (VERBOSE)
                std::cerr << "Printing PCFG '" << pcfg_name << "' to DOT" << std::endl;
            pcfg->print_graph_to_dot(memento.is_usage_computed(), memento.is_liveness_computed(),
                                    memento.is_reaching_definitions_computed(),
                                    memento.is_induction_variables_computed(),
                                    memento.is_range_analysis_computed(),
                                    memento.is_auto_scoping_computed(), memento.is_auto_deps_computed());
        }
        else if (CURRENT_CONFIGURATION->debug_options.print_pcfg ||
            CURRENT_CONFIGURATION->debug_options.print_pcfg_w_context)
        {   // Do not print analysis information
            if (VERBOSE)
                std::cerr << "Printing PCFG '" << pcfg_name << "' to DOT" << std::endl;
            pcfg->print_graph_to_dot();
        }
    }

    void AnalysisSingleton::print_all_pcfg(PCFGAnalysis_memento& memento)
    {
        const ObjectList<ExtensibleGraph*>& pcfgs = memento.get_pcfgs();
        if (CURRENT_CONFIGURATION->debug_options.print_pcfg_w_analysis ||
            CURRENT_CONFIGURATION->debug_options.print_pcfg_full)
        {   // Print analysis information
            for (ObjectList<ExtensibleGraph*>::const_iterator it = pcfgs.begin(); it != pcfgs.end(); ++it)
            {
                if (VERBOSE)
                    std::cerr << "Printing PCFG '" << (*it)->get_name() << "' to DOT" << std::endl;
                (*it)->print_graph_to_dot(memento.is_usage_computed(), memento.is_liveness_computed(),
                                         memento.is_reaching_definitions_computed(),
                                         memento.is_induction_variables_computed(),
                                         memento.is_range_analysis_computed(),
                                         memento.is_auto_scoping_computed(), memento.is_auto_deps_computed());
            }
        }
        else if (CURRENT_CONFIGURATION->debug_options.print_pcfg ||
            CURRENT_CONFIGURATION->debug_options.print_pcfg_w_context)
        {   // Do not print analysis information
            for (ObjectList<ExtensibleGraph*>::const_iterator it = pcfgs.begin(); it != pcfgs.end(); ++it)
            {
                if (VERBOSE)
                    std::cerr << "Printing PCFG '" << (*it)->get_name() << "' to DOT" << std::endl;
                (*it)->print_graph_to_dot();
            }
        }
    }
    
    void AnalysisSingleton::print_tdg(PCFGAnalysis_memento& memento, std::string tdg_name)
    {
        if (VERBOSE)
            std::cerr << "Printing TDG '" << tdg_name << "' to DOT" << std::endl;
        TaskDependencyGraph* tdg = memento.get_tdg(tdg_name);
        tdg->print_tdg_to_dot();
    }

    void AnalysisSingleton::tdg_to_json(PCFGAnalysis_memento& memento, std::string tdg_name)
    {
        if (VERBOSE)
            std::cerr << "Printing TDG '" << tdg_name << "' to JSON" << std::endl;
        TaskDependencyGraph* tdg = memento.get_tdg(tdg_name);
        tdg->print_tdg_to_json();
    }
}
}