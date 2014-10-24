/*!if GRAMMAR_PROLOGUE*/
%token<token_atrib> XL_BUILTIN_SPEC "_Builtin"

/*!endif*/
/*!if GRAMMAR_RULES*/

nontype_specifier_without_attribute : XL_BUILTIN_SPEC
{
    $$ = ASTLeaf(AST_XL_BUILTIN_SPEC, make_locus($1.token_file, $1.token_line, 0), $1.token_text);
}
;

/*!if C99*/
gcc_extra_bits_init_declarator : unknown_pragma attribute_specifier_seq
{
    if (CURRENT_CONFIGURATION->xl_compatibility)
    {
        $$ = ast_list_concat(ASTListLeaf($1), $2);
    }
    else
    {
        warn_printf("%s: warning: ignoring '#pragma %s' after the declarator\n",
                 ast_location($1),
                 ast_get_text($1));
        $$ = $2;
    }
}
| unknown_pragma
{
    if (CURRENT_CONFIGURATION->xl_compatibility)
    {
        $$ = ASTListLeaf($1);
    }
    else
    {
        warn_printf("%s: warning: ignoring '#pragma %s' after the declarator\n",
                 ast_location($1),
                 ast_get_text($1));
        $$ = NULL;
    }
}
;
/*!endif*/

/*!endif*/
