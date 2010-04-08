#ifndef fvm_h
#define fvm_h

#include <stdio.h>
#include <stdlib.h>

#include <gc.h>
#include "vendor/khash.h"

#define FDEBUG 1

//just for opcode
typedef short int Op;

// tags for each object, which indicated types
typedef enum {
    T_SYM=0,
    T_NIL,
    T_NUM,
    T_STR,
    T_FUNC
} Tag;

#define FVM_TAG_NAMES \
    "t_sym", \
    "t_nil", \
    "t_num", \
    "t_str", \
    "t_func" \

typedef unsigned long Addr;
typedef unsigned long Obj;

#define Vnil    0
#define Vtrue   2
#define Vfalse  4
#define Vundef  6

#define Vfunc(o) ((Func *)(o)
#define Vstr(o)  (((FStr *)o)->cstr)
#define Vnum(o)  ((int)(o>>1))

//Obj about
//Obj Header, every obj has got this
typedef struct OBasic {
    unsigned long       type;
} OBasic;

#define OFLAG_NUM 1

typedef struct FStr {
    OBasic              obasic;
    char                *cstr;  
    size_t              len;
} FStr;

// all obj stores in the heap
// so easier to implement the closure
// 
// var
typedef struct {
    char        *name;
    Obj         *ref;
} Var;

// hash data type initd
KHASH_MAP_INIT_STR(str, Var*);

// TODO: 
//  when a new Proto is created, 
//  lexer should find all names inside it.
//  parameters are the top local_names
typedef struct Proto {
    Op                  *opcodes;
    Obj                 consts[255];
    size_t              c_consts;
    size_t              c_params;
    // for closure
    char                *lnames[255];
    char                *onames[255];
    size_t              c_lvars;
    size_t              c_ovars;
} Proto;

// only one stack inside a VM
// root is just the global
typedef struct VM {
    Obj                 *stack;
    Obj                 *sp;
    Proto               **protos;
    size_t              c_protos;
    struct Env          *root;
} VM;

// TODO:
//  each Var have got a name, and stored in lvars[]
//  when a Func inited, each var->obj inited with Fnil (memset?) as default 
//  if a var is accessed, it will seek obj* in h_locals, and cache it
//
//  when a Func dead, it will tranverse all its children
//  if posibble, pass all the values of outer_names to children
typedef struct Env {
    VM                  *vm;
    struct Env          *from;
    khash_t(str)        *h_locals;
    Var                 *lvars;
    Var                 *ovars;
    size_t              c_lvars;
    size_t              c_ovars;
} Env;

// TODO:
typedef struct Func {
    OBasic              obasic;
    Proto               *proto;
    Var                 *ovars;
} Func;



// macros on gc
#define fvm_malloc   GC_MALLOC
#define fvm_alloc(N) (N*)GC_MALLOC(sizeof(N))
#define fvm_realloc  GC_REALLOC
#define fvm_free  GC_FREE

// print a message & die; in fprintf style
#define fvm_panic(fmt, ...) do{ fprintf(stderr, fmt, ##__VA_ARGS__); exit(1); }while(0);

VM*         fvm_init     ();
VM*         fvm_current  ();

// Func is with Env ALWAYS
Env*        fnew_env     ();
Obj         fget_local   ();
Obj*        fset_local   ();
Obj         fget_outer   ();
Obj         fset_outer   ();
Var*        freg_binding ();
Var*        fget_binding ();
Obj         fget_name    ();
Obj         fset_name    ();

Proto*      fnew_proto   ();
int         freg_const   ();
Obj         fget_const   ();
int         freg_lname   ();
int         freg_oname   ();

int         freg_proto   ();

Func*       fnew_func    ();

// values
int     ftype_of        (Obj);
Obj     fnil            ();
Obj     fnum            (int);
Obj     fstr            (char*);
Obj     ffunc           (Func*);
int     fis_nil         (Obj);
int     fnot_nil        (Obj);
int     feq             (Obj,Obj);
int     fgt             (Obj,Obj);
int     flt             (Obj,Obj);
int     fis_str         (Obj);
int     fis_func        (Obj);
int     fis_num         (Obj);
Obj     f2_str          (Obj);
char*   f2_cstr         (Obj);
int     f2_cint         (Obj);

Obj     fio_puts        (Obj);

/**  main stack **/
Obj  fpush       (Obj);
Obj  fpop        ();

/**  execution and calling **/
// call a function; function is an Obj
Obj fcall           ();
Obj fvm_run         ();

// for debug
//void __print_stack  (Obj *stack, Obj *sp);

char*   _dbg_tag_names(int);
char*   _dbg_obj(Obj);
int     _dbg_show_proto(Proto*);
int     _dbg_show_env(Env*);


#endif
