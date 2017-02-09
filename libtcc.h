#ifndef LIBTCC_H
#define LIBTCC_H

#ifndef LIBTCCAPI
# define LIBTCCAPI
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct TCCState;

typedef struct TCCState TCCState;

/* create a new TCC compilation context */
LIBTCCAPI TCCState *tcc_new(void);

/* free a TCC compilation context */
LIBTCCAPI void tcc_delete(TCCState *s);

/* set CONFIG_TCCDIR at runtime */
LIBTCCAPI void tcc_set_lib_path(TCCState *s, const char *path);

/* set error/warning display callback */
LIBTCCAPI void tcc_set_error_func(TCCState *s, void *error_opaque,
    void (*error_func)(void *opaque, const char *msg));

/* set options as from command line (multiple supported) */
LIBTCCAPI int tcc_set_options(TCCState *s, const char *str);

/*****************************/
/* preprocessor */

/* add include path */
LIBTCCAPI int tcc_add_include_path(TCCState *s, const char *pathname);

/* add in system include path */
LIBTCCAPI int tcc_add_sysinclude_path(TCCState *s, const char *pathname);

/* define preprocessor symbol 'sym'. Can put optional value */
LIBTCCAPI void tcc_define_symbol(TCCState *s, const char *sym, const char *value);

/* undefine preprocess symbol 'sym' */
LIBTCCAPI void tcc_undefine_symbol(TCCState *s, const char *sym);

/*****************************/
/* compiling */

/* add a file (C file, dll, object, library, ld script). Return -1 if error. */
LIBTCCAPI int tcc_add_file(TCCState *s, const char *filename);

/* compile a string containing a C source. Return -1 if error. */
LIBTCCAPI int tcc_compile_string(TCCState *s, const char *buf);

#ifdef CONFIG_TCC_EXSYMTAB
/* like tcc_compile_string, but also lets you specify the length of the
 * string to compile, the filename, and the line number from whence this
 * code came. */
LIBTCCAPI int tcc_compile_string_ex(TCCState *s, const char *str, int len, const char * filename, int line_num);
#endif

/*****************************/
/* linking commands */

/* set output type. MUST BE CALLED before any compilation */
LIBTCCAPI int tcc_set_output_type(TCCState *s, int output_type);
#define TCC_OUTPUT_MEMORY   1 /* output will be run in memory (default) */
#define TCC_OUTPUT_EXE      2 /* executable file */
#define TCC_OUTPUT_DLL      3 /* dynamic library */
#define TCC_OUTPUT_OBJ      4 /* object file */
#define TCC_OUTPUT_PREPROCESS 5 /* only preprocess (used internally) */

/* equivalent to -Lpath option */
LIBTCCAPI int tcc_add_library_path(TCCState *s, const char *pathname);

/* the library name is the same as the argument of the '-l' option */
LIBTCCAPI int tcc_add_library(TCCState *s, const char *libraryname);

/* add a symbol to the compiled program */
LIBTCCAPI int tcc_add_symbol(TCCState *s, const char *name, const void *val);

/* output an executable, library or object file. DO NOT call
   tcc_relocate() before. */
LIBTCCAPI int tcc_output_file(TCCState *s, const char *filename);

/* link and run main() function and return its value. DO NOT call
   tcc_relocate() before. */
LIBTCCAPI int tcc_run(TCCState *s, int argc, char **argv);

/* do all relocations (needed before using tcc_get_symbol()) */
LIBTCCAPI int tcc_relocate(TCCState *s1, void *ptr);
/* possible values for 'ptr':
   - TCC_RELOCATE_AUTO : Allocate and manage memory internally
   - NULL              : return required memory size for the step below
   - memory address    : copy code to memory passed by the caller
   returns -1 if error. */
#define TCC_RELOCATE_AUTO (void*)1

/* return symbol value or NULL if not found */
LIBTCCAPI void *tcc_get_symbol(TCCState *s, const char *name);

#ifdef CONFIG_TCC_EXSYMTAB
/**********************************************/
/* ---- Extended symbol table management ---- */
/**********************************************/

typedef struct TokenSym* TokenSym_p;
typedef struct extended_symtab* extended_symtab_p;

/*** For building a symbol table ***/
/* Set flag in compiler state to save an extended symbol table */
LIBTCCAPI void tcc_save_extended_symtab(TCCState * s);
/* Retrieve an extended symbol table after compilation or relocation */
LIBTCCAPI extended_symtab_p tcc_get_extended_symbol_table(TCCState * s);
/* Free an extended symbol table */
LIBTCCAPI void tcc_delete_extended_symbol_table (extended_symtab_p symtab);
/* Get a TokenSym from an extended symbol table */
LIBTCCAPI TokenSym_p tcc_get_extended_tokensym(extended_symtab_p symtab, const char * name);
/* Get a Symbol (i.e. function pointer) from an extended symbol table */
LIBTCCAPI void * tcc_get_extended_symbol(extended_symtab_p symtab, const char * name);
/* Set a Symbol pointer, i.e. handle linking by hand without needing a TCCState */
LIBTCCAPI int tcc_set_extended_symbol(extended_symtab_p symtab, const char * name, const void * pointer);

/*** For using a symbol table in a dependent compilation unit ***/
/* Adds details for TokenSyms whose token ids are added to *all* compilation
 * contexts, tokens like "push" and "str". NOTE: This function acts on
 * the currently compiling table of identifiers by modifying table_ident
 * directly. Only use this after you have initialized a compiler state!!!! */
LIBTCCAPI void tcc_prep_tokensym_list(extended_symtab_p symtab);
/* Callback function signature for lookup-by-name:
 * TokenSym_p my_callback(char * name, int len, void * data, extended_symtab_p* containing_symtab) */
typedef TokenSym_p (*extended_symtab_lookup_by_name_callback)(char * name,
       int len, void * data, extended_symtab_p* containing_symtab);
/* Callback function signature for sym-is-used:
 * void my_callback(char * name, int len, void * data) */
typedef void (*extended_symtab_sym_used_callback)(char * sym_name, int len, void * data);
/* Callback function signature for compilation unit preparation:
 * void my_callback(TokenSym_p* local_ts_list, void * data) */
typedef void (*extended_symtab_prep_callback)(void * data);
/* Set the lookup/sym-used callback functions */
LIBTCCAPI void tcc_set_extended_symtab_callbacks (
       TCCState * compiler_state,
       extended_symtab_lookup_by_name_callback new_name_callback,
       extended_symtab_sym_used_callback new_sym_used_callback,
       extended_symtab_prep_callback new_prep_callback,
       void * data
);

/*** For symbol table caching ***/
LIBTCCAPI extended_symtab_p tcc_deserialize_extended_symtab(const char * input_filename);
LIBTCCAPI int tcc_serialize_extended_symtab(extended_symtab_p symtab, const char * output_filename);
LIBTCCAPI void tcc_dump_identifier_names(extended_symtab_p symtab, char * outfile);

/* Testing functions, not really for general use */
LIBTCCAPI int tcc_extended_symtab_test(extended_symtab_p, int to_test, const char * name);
LIBTCCAPI char* tcc_get_next_extended_symbol_name(extended_symtab_p symtab, int * poffset);

#ifndef SYM_EXTENDED
       #define SYM_EXTENDED   0x40000000 /* extended symbol space */
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif
