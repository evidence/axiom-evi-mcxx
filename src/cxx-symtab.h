#ifndef CXX_SYMTAB_H
#define CXX_SYMTAB_H

#include "cxx-ast.h"
#include "hash.h"

#define BITMAP(x) (1 << x)

/*
 * A symbol table is represented by a symtab_t*
 *
 * Entries in the symbol table are symtab_entry_t*
 *
 * Every entry can have a non-null type_information field type_t*
 * 
 * A type_t* represents a full C++ type in a hierarchical way.
 *   -> pointer
 *   -> array
 *   -> function
 *   -> type (direct type including builtin's, class, enums, typedef)
 */

enum cxx_symbol_kind
{
	SK_UNDEFINED = 0,
	SK_CLASS,
	SK_ENUM,
	SK_ENUMERATOR, // The elements an enum is made of
	SK_FUNCTION,
	SK_LABEL,
	SK_NAMESPACE,
	SK_VARIABLE,
	SK_TYPEDEF,
	// Lots of stuff related to the C++ "template madness"
	SK_TEMPLATE_CLASS,
	SK_TEMPLATE_FUNCTION,
	SK_TEMPLATE_PARAMETER
};

typedef enum {
	CV_NONE = 0,
	CV_CONST = BITMAP(1),
	CV_VOLATILE = BITMAP(2),
	CV_RESTRICT = BITMAP(3)
} cv_qualifier_t;

typedef struct {
	int num_exception_types;
	struct type_tag** exception_type_seq;
} exception_spec_t;

// For type_t
enum type_kind
{
	TK_DIRECT,
	TK_POINTER,
	TK_REFERENCE,
	TK_POINTER_TO_MEMBER,
	TK_ARRAY,
	TK_FUNCTION
};

// For simple_type_t
typedef enum builtin_type_tag
{
	BT_UNKNOWN = 0,
	BT_INT,
	BT_BOOL,
	BT_FLOAT,
	BT_DOUBLE,
	BT_CHAR,
	BT_WCHAR,
	BT_VOID,
} builtin_type_t;

typedef enum simple_type_kind_tag
{
	STK_BUILTIN_TYPE,
	STK_CLASS,
	STK_ENUM,
	STK_TYPEDEF,
	STK_USER_DEFINED,
	// Templates stuff
	STK_TEMPLATE_CLASS,
	STK_TYPE_TEMPLATE_PARAMETER
} simple_type_kind_t;

struct symtab_entry_tag;

typedef struct {
	char* name;
	AST value;
} enumeration_item_t;

typedef struct enum_information_tag {
	int num_enumeration;
	struct symtab_entry_tag** enumeration_list;
} enum_info_t;

enum template_parameter_kind
{
	TPK_UNKNOWN = 0,
	TPK_NONTYPE,
	TPK_TYPE,
	TPK_TEMPLATE
};

typedef struct template_parameter {
	enum template_parameter_kind kind;

	struct type_tag* type_info;

	AST default_argument;
} template_parameter_t;

typedef enum access_specifier_t
{
	AS_UNKNOWN = 0,
	AS_PUBLIC,
	AS_PRIVATE,
	AS_PROTECTED
} access_specifier_t;

struct simple_type_tag;

typedef struct {
	// Access specifier
	access_specifier_t access_spec;
	// Member name, this will come from the declarator 
	char* name;
	// Type of the member
	struct type_tag* type_info;
} member_item_t;

enum class_kind_t {
	CK_STRUCT,
	CK_CLASS,
	CK_UNION
};

typedef struct class_information_tag {
	enum class_kind_t class_kind;
	int num_members;
	member_item_t** member_list;

	int is_template;

	// Special functions
	struct symtab_entry_tag* destructor;
	struct symtab_entry_tag** conversion_function_list;
	struct symtab_entry_tag** operator_function_list;
} class_info_t;

// Direct type (including classes and enums)
typedef struct simple_type_tag {
	simple_type_kind_t kind;
	builtin_type_t builtin_type;
	char is_long; // This can be 0, 1 or 2
	char is_short;
	char is_unsigned;
	char is_signed;

	// Previously declared type. should be completely "cv-unqualified"
	//
	// If this is a STK_TYPE_TEMPLATE_PARAMETER this will be NULL since there
	// is no "real type" backing this
	struct symtab_entry_tag* user_defined_type;

	// For typedefs
	struct type_tag* aliased_type;

	// For enums
	enum_info_t* enum_info;
	
	// For classes
	class_info_t* class_info;

	// For template classes

	cv_qualifier_t cv_qualifier;
} simple_type_t;

// Function information
typedef struct function_tag
{
	struct type_tag* return_type;
	int num_parameters;
	struct type_tag** parameter_list;
	cv_qualifier_t cv_qualifier;
	exception_spec_t* exception_spec;

	AST function_body;

	int is_template;

	int is_inline;
	int is_virtual;
	int is_pure; // is_pure implies is_virtual
	int is_static; // local linkage
	int is_explicit;
} function_info_t;

// Pointers, references and pointers to members
typedef struct pointer_tag
{
	cv_qualifier_t cv_qualifier;
	struct type_tag* pointee;

	struct symtab_entry_tag* pointee_class;
} pointer_info_t;

// Array information
typedef struct array_tag
{
	AST array_expr;
	struct type_tag* element_type;
} array_info_t;

// This structure is able to hold type information for a given symbol
// note it being decoupled from its declarator 
typedef struct type_tag
{
	// Kind of the type
	enum type_kind kind;

	// Pointer
	pointer_info_t* pointer;

	// Array
	array_info_t* array;

	// Function
	function_info_t* function;

	// "Simple" type
	simple_type_t* type;
} type_t;

struct symtab_tag;

// This is an entry in the symbol table
typedef struct symtab_entry_tag
{
	enum cxx_symbol_kind kind;
	char* symbol_name;

	// This allows us to enforce the one-definition-rule within a translation unit
	int defined;

	// Scope of this symbol when declared
	struct symtab_tag* scope;

	// For everything related to a type
	type_t* type_information;

	// Related scope. For scopes defined within this symbol
	// e.g. namespaces, classes, etc
	struct symtab_tag* inner_scope;

	// Initializations of several kind are saved here
	//   - initialization of const objects
	//   - enumerator values
	AST expression_value;

	// For template parameters
	int num_template_parameters;
	template_parameter_t** template_parameter_info;

	// Linkage
	char* linkage_spec;
} symtab_entry_t;

// This is what the symbol table returns
typedef struct symtab_entry_list
{
	// The current entry
	symtab_entry_t* entry;
	
	// Next entry under this name (NULL if last)
	struct symtab_entry_list* next;
} symtab_entry_list_t;

// This is the symbol table
typedef struct symtab_tag
{
	// Hash of symtab_entry_list
	Hash* hash;

	// Can be null 
	struct symtab_tag* parent;
} symtab_t;

#undef BITMAP

// Functions to handle symbol table
symtab_t* new_symtab();
symtab_t* enter_scope(symtab_t* parent);
symtab_entry_t* new_symbol(symtab_t* st, char* name);
symtab_entry_list_t* query_in_current_scope(symtab_t* st, char* name);
symtab_entry_list_t* query_in_current_and_upper_scope(symtab_t* st, char* name);
symtab_entry_list_t* create_list_from_entry(symtab_entry_t* entry);

// Higher level functions when dealing with the symtab
symtab_entry_t* filter_simple_type_specifier(symtab_entry_list_t* entry_list);

// Everything built by an id_expression can be queried with this function
symtab_entry_list_t* query_id_expression(symtab_t* st, AST id_expr);

// Nested names
symtab_entry_list_t* query_nested_name_spec(symtab_t* st, symtab_t** result_lookup_scope, AST global_op, AST nested_name);

#endif // CXX_SYMTAB_H
