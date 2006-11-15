#include "tl-omp.hpp"
#include "tl-omptransform.hpp"
#include "tl-predicateutils.hpp"
#include "tl-source.hpp"
#include <iostream>
#include <utility>

namespace TL
{
    class OpenMPTransform : public OpenMP::OpenMPPhase
    {
        private:
            int num_parallels;
            int parallel_nesting;
        public:
            OpenMPTransform()
            {
            }

            virtual void init()
            {
                // Register the handlers (callbacks) for every construction
                // for now only '#pragma omp parallel'
                on_parallel_pre.connect(&OpenMPTransform::parallel_pre, *this);
                on_parallel_post.connect(&OpenMPTransform::parallel_post, *this);
            }

            // Parallel preorder
            void parallel_pre(OpenMP::ParallelConstruct parallel_construct)
            {
                parallel_nesting++;
            }

            // Parallel postorder
            void parallel_post(OpenMP::ParallelConstruct parallel_construct)
            {
                parallel_nesting--;
                num_parallels++;

                OpenMP::Directive directive = parallel_construct.directive();

                ObjectList<Symbol> shared_symbols;
                ObjectList<Symbol> private_symbols;

                // Get the enclosing function definition
                FunctionDefinition f = parallel_construct.get_enclosing_function();

                Source outline_function_name; 
                outline_function_name << "outline_" << f.get_function_name() << "_" << num_parallels;

                // Get the body of the statement
                Statement body = parallel_construct.body();

                // Construct the set of shared and privatized symbols
                get_data_attributes(directive, body, shared_symbols, private_symbols);

                // Create the outline function
                Source outline_code;
                outline_code << create_outline(shared_symbols, private_symbols, body, outline_function_name.get_source());

				std::cerr << "About to compile outline code '" << outline_code.get_source() << "'" << std::endl;

                AST_t outline_tree = outline_code.parse_global(f.get_scope(), f.get_scope_link());

				std::cerr << "Outline code parsed" << std::endl;

                f.prepend_sibling(outline_tree);

                // Create the spawning code
                AST_t spawn_tree = create_spawn_code(parallel_construct.get_scope(), 
                        parallel_construct.get_scope_link(),
                        outline_function_name, shared_symbols);
                parallel_construct.get_ast().replace_with(spawn_tree);
            }

            AST_t create_spawn_code(Scope spawn_scope,
                    ScopeLink spawn_scope_link,
                    Source& outline_function_name,
                    ObjectList<Symbol>& shared_symbols)
            {
                Source spawning_code;
                Source shared_references;

                // Spawn skeleton
                spawning_code 
                    << "{"
                    << "  extern int nth_num_threads();"
                    << "  extern int nth_create(...);"
                    << "  for (int i = 0; i < nth_num_threads(); i++)"
                    << "  {"
                    << "    nth_create(" << outline_function_name << ", " << shared_symbols.size() << shared_references << ");"
                    << "  }"
                    << "}"
                    ;

                // Transform every symbol 'name' to '&name'
                ObjectList<std::string> shared_references_names = shared_symbols.map(
                        functor(&OpenMPTransform::reference_to_name, *this)
                        );

                if (!shared_references_names.empty())
                {
                    shared_references << ", " << concat_strings(shared_references_names, ",");
                }

				std::cerr << "--SPAWN CODE--" << std::endl;
				std::cerr << spawning_code.get_source() << std::endl;
				std::cerr << "--END SPAWN CODE--" << std::endl;

                AST_t spawn_tree = spawning_code.parse_statement(spawn_scope, spawn_scope_link);
                return spawn_tree;
            }

            // The data enviroment
            void get_data_attributes(OpenMP::Directive& directive,
                    Statement& body,
                    ObjectList<Symbol>& shared_symbols,
                    ObjectList<Symbol>& private_symbols)
            {
                // Get symbols in shared clause
                OpenMP::Clause shared_clause = directive.shared_clause();
                shared_symbols = shared_clause.symbols();

                // Get symbols in private_clause
                OpenMP::Clause private_clause = directive.private_clause();
                private_symbols = private_clause.symbols();

                // default(none|shared) clause
                OpenMP::DefaultClause default_clause = directive.default_clause();

                // Recall there is no is_private() in C/C++
                if (!default_clause.is_none())
                {
                    ObjectList<Symbol> symbols = body.non_local_symbols();

                    // We only want variables
                    symbols = symbols.filter(&Symbol::is_variable);

                    // that are not already set private
                    symbols = symbols.filter(not_in_set(private_symbols));

                    // and not already set shared
                    symbols = symbols.filter(not_in_set(shared_symbols));

                    // we should also ignore qualified symbols since they are
                    // not needed to be shared
                    // (currently not done)

                    // add to the shared symbols
                    shared_symbols.insert(shared_symbols.end(), symbols.begin(), symbols.end());
                }
            }

            // Given a symbol declares a suitable parameter pointer to it
            std::string declare_parameter(Symbol& s)
            {
                // Get the type of the symbol
                Type type = s.get_type();
                // Construct a type that is a pointer to the original type
                Type pointer_type = type.get_pointer_to();

                // And return its declaration
                return pointer_type.get_declaration(s.get_name());
            }

            // Given a symbol declares a full private declaration to it
            std::string declare_privates(Symbol& s)
            {
                // Get the type
                Type type = s.get_type();

                // and return its declaration but the symbol declaration will have "p_" prepended
                return type.get_declaration(std::string("p_") + s.get_name()) + std::string(";");
            }

            // Create the outline
            std::string create_outline(ObjectList<Symbol>& shared_symbols, 
                    ObjectList<Symbol>& private_symbols,
                    Statement& body,
                    const std::string& outline_function_name)
            {
                Source shared_parameters;
                Source privatized_variables;
                Source outlined_body;

                // Define the skeleton
                Source outline_code;

                outline_code
                    << "void " << outline_function_name << "(" << shared_parameters << ")"
                    << "{"
                    <<     privatized_variables
                    <<     outlined_body
                    << "}";

                // For every shared symbol, return a declaration to it
                ObjectList<std::string> parameter_declarations = shared_symbols.map(
                        functor(&OpenMPTransform::declare_parameter, *this)
                        );

                // And concat all declarations with ','
                shared_parameters = concat_strings(parameter_declarations, ", ");

                ObjectList<std::string> private_declarations = private_symbols.map(
                        functor(&OpenMPTransform::declare_privates, *this)
                        );

                privatized_variables << concat_strings(private_declarations);
                
                // Copy the body since we will modify it
                std::pair<AST_t, ScopeLink> new_body = body.get_ast().duplicate_with_scope(body.get_scope_link());
                Statement modified_body(new_body.first, new_body.second);
                ScopeLink modified_body_scope_link = modified_body.get_scope_link();
                
                // Derreference all shared references
                ObjectList<std::pair<Symbol, AST_t> > non_local_symbols = modified_body.non_local_symbol_trees();

                for (ObjectList<std::pair<Symbol, AST_t> >::iterator it = non_local_symbols.begin();
                        it != non_local_symbols.end();
                        it++)
                {
                    if (find(shared_symbols.begin(), shared_symbols.end(), it->first) != shared_symbols.end())
                    {
                        AST_t ref = it->second;

                        Source derref_source;
                        derref_source << "(*" << ref.prettyprint() << ")";

                        // Get the scope of this expression
                        Scope expr_scope = modified_body_scope_link.get_scope(ref);

                        // Parse this new expression
                        AST_t derref_expr = derref_source.parse_expression(expr_scope);

                        // And replace it in the tree
                        ref.replace_with(derref_expr);
                    }
                }
                
                // Rename all private references
                for (ObjectList<std::pair<Symbol, AST_t> >::iterator it = non_local_symbols.begin();
                        it != non_local_symbols.end();
                        it++)
                {
                    if (find(private_symbols.begin(), private_symbols.end(), it->first) != private_symbols.end())
                    {
                        AST_t ref = it->second;

                        std::string name = it->second.prettyprint();
                        
                        // Here we would have to mangle privatized qualified
                        // names (currently not done)
                        //
                        //   q       -> p_q
                        //   A::q    -> p_A__q
                        //   A<0>::q -> p_A_0___q 
                        //
                        name = "p_" + name;

                        ref.replace_text(name);
                    }
                }
                
                outlined_body << modified_body.prettyprint();

				std::cerr << "--OUTLINE CODE--" << std::endl;
				std::cerr << outline_code.get_source() << std::endl;
				std::cerr << "--END OUTLINE CODE--" << std::endl;

                return outline_code.get_source();
            }

            std::string reference_to_name(Symbol& sym)
            {
                return std::string("&") + sym.get_name();
            }

    };
}

EXPORT_PHASE(TL::OpenMPTransform);
