/* cmp_sem_gen_vars.c - RULEWORKS RHS Auxiliary Semantic Checking */
/****************************************************************************
**                           
RuleWorks - Rules based application development tool.

Copyright (C) 1999  Compaq Computer Corporation

This program is free software; you can redistribute it and/or modify it 
under the terms of the GNU General Public License as published by the 
Free Software Foundation; either version 2 of the License, or any later 
version. 

This program is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranty of 
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General 
Public License for more details. 

You should have received a copy of the GNU General Public License along 
with this program; if not, write to the Free Software Foundation, 
Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Email: info@ruleworks.co.uk
****************************************************************************/

/*
 *  FACILITY:
 *	RULEWORKS compiler
 *
 *  ABSTRACT:
 *	RHS Semantic checking routines.  Includes semantic
 *	checks for variables and intermediate cached values
 *
 *  MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 *  MODIFICATION HISTORY:
 *
 *	15-Jan-1992	DEC	Initial version.
 *	16-Feb-1998	DEC	class type changed to rclass
 *	01-Dec-1999	CPQ	Releasew ith GPL
 */


#include <common.h>
#include <cmp_comm.h>
#include <ast.h>
#include <sem.h>
#include <val.h>
#include <lvar.h>
#include <gen.h>
#include <emit.h>
#include <msg.h>
#include <cmp_msg.h>
#include <mol.h>
#include <dyar.h>

/* Static variables that identify the amount of storage
 * needed for external routine calls on the current RHS */

#define NUMBER_OF_EXT_TYPES (EXT_TYPE_LAST - EXT_TYPE_FIRST + 1)
#define INDEX_EXT_TYPE_ARRAY(array,index) array[index - EXT_TYPE_FIRST]
#define SCALAR_ARRAY 2
#define VAR_C_INIT_FROM_LHS (void *) -1

/* number of implicit temporary variables indexed by type used per action */
static	long	rhs_space [NUMBER_OF_EXT_TYPES][SCALAR_ARRAY];
/* number of implicit temporary variables indexed by type used per RHS */
static	long	max_rhs_space [NUMBER_OF_EXT_TYPES][SCALAR_ARRAY];

/* RHS variable code generation data structure */

typedef struct rhs_var *Rhs_Var;
struct rhs_var {
  Mol_Symbol	name;		/* name of RHS variable used in this rule */
  long		index;		/* where RHS var's value is stored in array */
  Class		rclass;		/* class of RHS variable if bound to WMO id */
  Decl_Domain	domain;		/* domain of RHS variable - "type" of value */
  Decl_Shape	shape;		/* shape of RHS variable (mol/atom/compound)*/
  Value		initializer;	/* value used to initialize the variable */
  Value		binding_site;	/* function value used to gen var bind code */
  Rhs_Var	next;		/* pointer to next RHS var used in this rule*/
};  

/* Static variables that store the how to evaluate and quantify
 * RHS variables used on the current RHS. */

static  Boolean         use_entry_vars = FALSE;
static	long		number_rhs_vars = 0;
static  Rhs_Var		first_rhs_var = NULL;
static	Rhs_Var		last_rhs_var = NULL;
static	long		number_entry_rhs_vars = 0;
static  Rhs_Var		first_entry_rhs_var = NULL;
static	Rhs_Var		last_entry_rhs_var = NULL;
static  Dynamic_Array   action_tmp_vars = NULL;

/* some variables in the generated code are needed to convert values between
   their internal and external representations (to call external routines) */

typedef struct ext_temp_var *Ext_Temp_Var;

struct ext_temp_var {
  Ext_Type	type;	/* external type to convert to/from */
  Ext_Mech	mech;	/* how is the external value passed in/out (val/ref)*/
  long		index;	/* used to identify the generated var of this type */
  Cardinality	a_len;	/* is this variable used to store an array of values*/
  Ext_Temp_Var	next;
};

static Ext_Temp_Var first_rhs_ext_temp_var [NUMBER_OF_EXT_TYPES][SCALAR_ARRAY];

/* Static variables that identify the kind and amount of storage
 * needed for external routines calls on the LHS */

static	long	       lhs_space [NUMBER_OF_EXT_TYPES][SCALAR_ARRAY];
static	Ext_Temp_Var   first_lhs_ext_temp_var [NUMBER_OF_EXT_TYPES][SCALAR_ARRAY];

/* this routine will answer if a explicitly named RHS variable is registered*/
static Boolean rhs_variable_registered (Mol_Symbol name);

static void add_rhs_tmp_var (Ext_Type type, Cardinality a_len,
			     Ext_Mech mech, long index);
static void gen_decr_for_rhs_tmp_mol (Value tmp_mol_var);
static void sem_clear_rhs_used_lhs_vars ();


/* these are structure deallocation routines */

static void free_rhs_var_list (Rhs_Var var)
{
  if (var == NULL)
    return;
  else {
    free_rhs_var_list (var->next);
    if (var->name != NULL)
      rul__mol_decr_uses (var->name);
    rul__mem_free (var);
  }
}

static void free_ext_temp_var_list (Ext_Temp_Var var)
{
  if (var == NULL)
    return;
  else {
    free_ext_temp_var_list (var->next);
    rul__mem_free (var);
  }
}

/* these routines track LHS, RHS and temp. ext. vars needed in the gend code */

/* Note that this just initializes the temporary external variables */
/* (it doesn't initialize the list of explicit variables named on the RHS). */
/* This was done so that it could be called while processing multiple ON-* */
/* clauses which share variable bindings with the entry block and each other.*/

void rul__sem_init_temp_rhs_vars (void)
{
  register Ext_Type type;
  register long     i;

  /* temporary external type variables */
  for (i = EXT__C_NOT_ARRAY; i < SCALAR_ARRAY; i++) {
    for (type = EXT_TYPE_FIRST; type <= EXT_TYPE_LAST; type++) {

      /* zero the running variable counters of each type */
      INDEX_EXT_TYPE_ARRAY (rhs_space, type)[i] = 0;   	 /* per RHS action */
      INDEX_EXT_TYPE_ARRAY (max_rhs_space, type)[i] = 0; /* cumulative RHS*/

      /* clear the list of variables for each type */
      free_ext_temp_var_list (INDEX_EXT_TYPE_ARRAY (first_rhs_ext_temp_var,
						    type)[i]);
      INDEX_EXT_TYPE_ARRAY(first_rhs_ext_temp_var, type)[i] = NULL;
    }
  }

  /* the counter variable num_temp_rhs_vars is now represented as
   * INDEX_EXT_TYPE_ARRAY (rhs_space, ext_type_atom)[i]
   * and its initialization is included in the loop above
   */
}

static void clear_rhs_vars (void)
{
  /* named variables (visible in source code) */
  if (use_entry_vars) {
    free_rhs_var_list (first_entry_rhs_var);
    first_entry_rhs_var = NULL;
    last_entry_rhs_var = NULL;
    number_entry_rhs_vars = 0;
  }
  else {
    free_rhs_var_list (first_rhs_var);
    first_rhs_var = NULL;
    last_rhs_var = NULL;
    number_rhs_vars = 0;
  }
  if (action_tmp_vars)
    rul__dyar_set_array_empty (action_tmp_vars);
  else
    action_tmp_vars = rul__dyar_create_array (10);
}


/* This routine initializes all the data structures used for
 * RHS code generation.  It is called before at the start of
 * semantic checking for a rule RHS.
 */
void rul__sem_initialize_rhs_vars (void)
{
  rul__sem_init_temp_rhs_vars ();
  clear_rhs_vars ();
  sem_clear_rhs_used_lhs_vars ();
}

/* This routine sets a boolean for the usage of ENTRY block vars
 * versus the usage of normal rhs vars
 */
void rul__sem_use_entry_vars (Boolean use)
{
  use_entry_vars = use;
}



/* This routine resets temp assignment variables to allow
 * the amount of temporary value storage to be the maximum
 * needed by a single RHS action (rather than the total 
 * needed by all the RHS actions).  It is called at the end 
 * of semantic checking for Each RHS action in a rule RHS.
 */
void rul__sem_rhs_action_end_vars (void)
{
  register Ext_Type type;
  register long     i;

  /*
   * track the most number of temp ext vars that we'll need for any action
   */

  for (i = EXT__C_NOT_ARRAY; i < SCALAR_ARRAY; i++) {
    for (type = EXT_TYPE_FIRST; type <= EXT_TYPE_LAST; type++) {
      INDEX_EXT_TYPE_ARRAY (max_rhs_space,type)[i] =
	MAX (INDEX_EXT_TYPE_ARRAY (max_rhs_space, type)[i],
	     INDEX_EXT_TYPE_ARRAY (rhs_space, type)[i]);
      /* keep track of each actions count of tmp_mol's for decr */
      if (type == ext_type_atom && i == EXT__C_NOT_ARRAY)
	rul__dyar_append (action_tmp_vars,
			  (void *) INDEX_EXT_TYPE_ARRAY(rhs_space, type)[i]);
      if (type != ext_type_for_each)
	INDEX_EXT_TYPE_ARRAY(rhs_space, type)[i] = 0;
    }
  }
}

void rul__sem_initialize_lhs_vars (void)
{
  register Ext_Type type;
  register long     i;

  for (i = EXT__C_NOT_ARRAY; i < SCALAR_ARRAY; i++) {

    /* temporary external type variables */
    for (type = EXT_TYPE_FIRST; type <= EXT_TYPE_LAST; type++) {
      INDEX_EXT_TYPE_ARRAY (lhs_space, type)[i] = 0;
      free_ext_temp_var_list (INDEX_EXT_TYPE_ARRAY (first_lhs_ext_temp_var,
						    type)[i]);
      INDEX_EXT_TYPE_ARRAY (first_lhs_ext_temp_var, type)[i] = NULL;
    }

    /* temporary molecules (must be reference count decremented after use) */
    /* the counter variable num_temp_lhs_vars is now represented as
     * INDEX_EXT_TYPE_ARRAY (lhs_space, ext_type_atom)[i]
     * and its initialization is included in the loop above
     */
  }
}


/* RHS VARIABLE SEMANTIC CHECKING & PROCESSING */

static Rhs_Var find_rhs_var (Mol_Symbol name)
{
  Rhs_Var var;

  if (use_entry_vars)
    var = first_entry_rhs_var;
  else
    var = first_rhs_var;

  for (; var != NULL; var = var->next)
    if (var->name == name)
      return var;

  return NULL;
}

Boolean rul__sem_is_var_registered (Mol_Symbol var_name)
{
  /*  
   **  This routine will return TRUE if an explicitly 
   **  named RHS variable of the specified name is registered
   */
  return (rhs_variable_registered (var_name));
}

static Boolean rhs_variable_registered (Mol_Symbol var_name)
{
  return (find_rhs_var(var_name) != NULL);
}

static Decl_Shape get_current_rhs_var_shape (Mol_Symbol name)
{
  Rhs_Var var;
  
  var = find_rhs_var(name);
  assert (var != NULL);

  return var->shape;
}

static Decl_Domain get_current_rhs_var_domain (Mol_Symbol name)
{
  Rhs_Var var;

  var = find_rhs_var(name);
  assert (var != NULL);

  return var->domain;
}

static Class get_current_rhs_var_class (Mol_Symbol name)
{
  Rhs_Var var;

  var = find_rhs_var (name);
  assert (var != NULL);

  return var->rclass;
}


long rul__sem_number_of_rhs_vars (void)
{
  if (use_entry_vars)
    return number_entry_rhs_vars;
  else
    return number_rhs_vars;
}

Boolean rul__sem_rhs_var_is_bound (Mol_Symbol name)
{
  Rhs_Var var;

  var = find_rhs_var (name);

  assert (var != NULL /* Unregistered RHS Variable */ );

  if (var->binding_site != NULL  ||  var->initializer != NULL)
    return TRUE;
  return FALSE;
}

Value rul__sem_curr_rhs_var_binding (Mol_Symbol name)
{
  Rhs_Var var;

  var = find_rhs_var(name);

  assert (var != NULL);

  return var->binding_site;
}


/* the following routines create and modify structures to track RHS variables*/

void rul__sem_reset_var_binding_site (Mol_Symbol name, Value var_value)
{
  Rhs_Var var;

  var = find_rhs_var (name);

  assert (var != NULL /* Unregistered RHS Variable */ );

  var->binding_site = var_value;
  var->domain = rul__val_get_domain (var_value);
  var->shape  = rul__val_get_shape  (var_value);
  var->rclass  = rul__val_get_class  (var_value);
}


void rul__sem_set_var_initializer (Mol_Symbol name, Value var_value)
{
  Rhs_Var var;

  var = find_rhs_var (name);

  assert (var != NULL /* Unregistered RHS Variable */ );
  assert (var->initializer == NULL  ||  
	  var->initializer == VAR_C_INIT_FROM_LHS);

  var->initializer = var_value;

  var->domain = rul__val_get_domain (var_value);
  var->shape  = rul__val_get_shape  (var_value);
  var->rclass  = rul__val_get_class  (var_value);
}



static Value make_rhs_variable_val (Mol_Symbol name)
{
  Value var_value;
  Rhs_Var var;

  var = find_rhs_var (name);

  assert (var != NULL);

  var_value = (Value) rul__val_create_rhs_variable (name);

  rul__val_set_rhs_var_index (var_value, var->index);

  rul__val_set_shape_and_domain (var_value, var->shape, var->domain);
  rul__val_set_class (var_value, var->rclass);

  return var_value;
}


static void add_rhs_variable_entry (Mol_Symbol name)
{
  Rhs_Var	var;

  var = RUL_MEM_ALLOCATE (struct rhs_var, 1);

  /* set the variable's data */
  var->name = name;
  rul__mol_incr_uses (name);

  if (use_entry_vars)
    var->index = number_entry_rhs_vars++;
  else
    var->index = number_rhs_vars++;
  var->domain = dom_any;
  var->shape = shape_molecule;
  var->rclass = NULL;
  var->initializer = NULL;
  var->binding_site = NULL;
  var->next = NULL;

  /* add this variable to the RHS variable list */
  if (use_entry_vars) {

    if (first_entry_rhs_var == NULL)
      first_entry_rhs_var = var;
    else
      last_entry_rhs_var->next = var;

    last_entry_rhs_var = var;
  }

  else {
    if (first_rhs_var == NULL)
      first_rhs_var = var;
    else
      last_rhs_var->next = var;
 
    last_rhs_var = var;
  }
}


/* The following routine semantically checks RHS variables and records
 * data to allow code generation of RHS variable initialization, assignment,
 * and value access.
 * It returns NULL if the variable is semantically unusable; otherwise,
 * it returns a rhs variable value with the variable name,
 * current shape and current domain.
 */
Value rul__sem_check_rhs_var (Ast_Node ast, Boolean accessing_value)
{
  long	lhs_bind_cnt;
  Value	val;
  Mol_Symbol  name = (Molecule) rul__ast_get_value(ast);

  if (accessing_value) {
    /* Variable used requires previous binding 
     * Check for LHS binding */
    lhs_bind_cnt = rul__lvar_get_binding_count (name);

    if (lhs_bind_cnt < 0) {
      /* Variable was LHS bound in a negated context
       * But a bound value is required here.
       * Check to see if it was defined as an
       * RHS only variable */
      if (rhs_variable_registered (name))
	return make_rhs_variable_val (name);

      else {
	rul__msg_cmp_print_w_atoms (CMP_VARNEGLHS, ast, 1, name);
	return NULL;
      }
    }

    else if (lhs_bind_cnt == 0) {
      /* Variable is used on RHS only and requires a 
       * RHS binding prior to this point.
       * Check for previous RHS variable registration */
      if (rhs_variable_registered (name) &&
	  rul__sem_rhs_var_is_bound (name)) {
	return make_rhs_variable_val (name);
      }		
      else {
	/* Unbound variable used as a value on RHS */
	rul__msg_cmp_print_w_atoms (CMP_VARNOTBOUND, ast, 1, name);
	return NULL;
      }
    }

    else {
      /* Variable was bound on the LHS */
      if (!rhs_variable_registered (name)) {
	rul__sem_tag_rhs_use_of_lhs_var (name);
      }
      return make_rhs_variable_val (name);
    }
  }
  else
    /* the variable will be assigned at this time */
    if (!rhs_variable_registered (name))
      add_rhs_variable_entry (name);
  return make_rhs_variable_val (name);
}


Value rul__sem_check_lhs_var (Ast_Node ast, long ce_index, 
			      Decl_Domain domain, Decl_Shape shape,
			      char *usage)
{
  Molecule var_name;
  
  /* Bogus usage of this LHS variable funtion */
  assert (ce_index != SEM__C_UNDEFINED_CE_INDEX);

  var_name = (Molecule) rul__ast_get_value (ast);

  if (!rul__lvar_variable_is_visible (var_name, ce_index)) {
    rul__msg_cmp_print_w_atoms (CMP_VARNOTBOUND, ast, 1, var_name);
    return NULL;
  }
  else if (!rul__sem_check_value_type (ast, domain, shape,
				       rul__lvar_get_domain (var_name),
				       rul__lvar_get_shape (var_name),
				       usage)) {
    return NULL;
  }
  else 
    return (rul__val_create_lhs_variable (var_name, 
					  rul__lvar_get_id_number (var_name,
								   ce_index)));
}


/* TEMPORARY VARIABLE PROCESSING */

static Ext_Temp_Var make_ext_temp_var (Ext_Type type, Cardinality a_len,
				       Ext_Mech mech, long index)
{
  register Ext_Temp_Var var;

  var = RUL_MEM_ALLOCATE (struct ext_temp_var, 1);
  var->type = type;
  var->mech = mech;
  var->index = index;
  var->a_len = a_len;
  var->next = NULL;

  return var;
}

/* The following routine returns the next index for temporary 
 * external variable for the specified type.
 * It is assume that registration occurs at assigment time,
 * value access will use the assignment. 
 */
long rul__sem_register_ext_temp (Ext_Type type, Cardinality a_len,
				 Ext_Mech mech, Boolean on_lhs)
{
  register long index;
  Ext_Temp_Var  var;
  long          i = 0;

  assert ((type >= EXT_TYPE_FIRST) && (type <= EXT_TYPE_LAST));

  if (a_len != EXT__C_NOT_ARRAY)
    i = 1;
   
  if (on_lhs)
    index = INDEX_EXT_TYPE_ARRAY (lhs_space, type)[i]++;

  else { /* on rhs */
    index = INDEX_EXT_TYPE_ARRAY (rhs_space, type)[i]++;
    add_rhs_tmp_var (type, a_len, mech, index);
  }
    
  return index;
}


/* The following routine returns the next index for temporary 
 * molecular (need to have reference count decremented at end
 * of RHS) variable.  It is assume that registration occurs
 * at assigment time, value access will use the assignment. 
 */
long rul__sem_register_temp_var (Boolean on_lhs)
{
  return rul__sem_register_ext_temp (ext_type_atom, EXT__C_NOT_ARRAY,
				     ext_mech_value, on_lhs);
}

Value rul__sem_create_temp_var (Boolean on_lhs)
{
  return (rul__val_create_unnamed_mol_var(rul__sem_register_temp_var(on_lhs)));
}

void add_ext_temp_var (Ext_Temp_Var *var_list, Ext_Type type,
		       Cardinality a_len, Ext_Mech mech, long index)
{
  register Ext_Temp_Var var;
    
  if (*var_list == NULL) {
    *var_list = make_ext_temp_var (type, a_len, mech, index);
    return;
  }
  else
    var = *var_list;

  while ((var->index != index) && (var->next != NULL))
    var = var->next;

  if (var->index == index)
      return;
  else
    var->next = make_ext_temp_var (type, a_len, mech, index);
}

void rul__sem_add_lhs_tmp_var (Ext_Type type, Cardinality a_len,
			       Ext_Mech mech, long index)
{
  long i = 0;

  if (a_len != EXT__C_NOT_ARRAY)
    i = 1;

  add_ext_temp_var(&(INDEX_EXT_TYPE_ARRAY (first_lhs_ext_temp_var, type)[i]),
		   type, a_len, mech, index);
}

static void add_rhs_tmp_var (Ext_Type type, Cardinality a_len,
			     Ext_Mech mech, long index)
{
  long i = 0;

  if (a_len != EXT__C_NOT_ARRAY)
    i = 1;

  add_ext_temp_var(&(INDEX_EXT_TYPE_ARRAY (first_rhs_ext_temp_var, type)[i]),
		   type, a_len, mech, index);
}



/* Helper function for return values. */

Value rul__sem_return_value (Value return_to_val, Value func_val,
			     Decl_Shape shape, Decl_Domain domain,
			     Boolean on_lhs, Class class_id)
	
{
  Value ret_val, asn_val, incr_val;

  if (return_to_val == NULL)
    ret_val = rul__sem_create_temp_var (on_lhs);
  else 
    ret_val = return_to_val;

  rul__val_set_shape_and_domain (ret_val, shape, domain);
  rul__val_set_shape_and_domain (func_val, shape, domain);
  if (domain == dom_instance_id) {
    rul__val_set_class (ret_val, class_id);
    rul__val_set_class (func_val, class_id);
  }

  asn_val = rul__val_create_assignment (ret_val, func_val);

  if (return_to_val != NULL  &&  !rul__val_is_rhs_variable (return_to_val)) {
#ifndef NDEBUG
    incr_val = rul__val_create_function_str ("rul__mol_incr_uses", 1);
#else
    incr_val = rul__val_create_function_str ("MIU", 1);
#endif
    rul__val_set_nth_subvalue (incr_val, 1, rul__val_copy_value(ret_val));
    rul__val_set_next_value (asn_val, incr_val);
  }

  return asn_val;
}


/* ------------------- start of code generation routines ------------------- */

static Boolean gen_stack_ext_temp_vars (Ext_Temp_Var var, Ext_Type type,
					Boolean need_comment)
{
  Boolean first_done = FALSE;
  Ext_Mech mech;
  Cardinality a_len;
  int i;

  if (var != NULL) {
    if (need_comment)
      rul__emit_comment ("Temporary External Variables");
    
    rul__emit_stack_tmp_start (type);

    for (; var != NULL; var = var->next) {
      mech = var->mech;
      a_len = var->a_len;
      first_done = rul__emit_stack_tmp (type, a_len, mech,
					var->index, first_done);
    }

    rul__emit_stack_tmp_end ();
    return FALSE;
  }
  else
    return need_comment;
}

void rul__gen_rhs_tmp_decl (void)
{
  Boolean           need_comment = TRUE;
  register Ext_Type type;
  long              i;

  for (i = EXT__C_NOT_ARRAY; i < SCALAR_ARRAY; i++) {
    for (type = EXT_TYPE_FIRST; type <= EXT_TYPE_LAST; type++)
      need_comment = gen_stack_ext_temp_vars (
		      INDEX_EXT_TYPE_ARRAY (first_rhs_ext_temp_var, type)[i],
					      type, need_comment);
  }
  if (!need_comment)
    rul__emit_blank_line ();
}
void rul__gen_lhs_tmp_decl (void)
{
  Boolean  need_comment = TRUE;
  Ext_Type type;
  long     i;

  for (i = EXT__C_NOT_ARRAY; i < SCALAR_ARRAY; i++) {
    for (type = EXT_TYPE_FIRST; type <= EXT_TYPE_LAST; type++)
      need_comment = gen_stack_ext_temp_vars (
		      INDEX_EXT_TYPE_ARRAY (first_lhs_ext_temp_var, type)[i],
					      type, need_comment);
  }
  if (!need_comment)
    rul__emit_blank_line ();
}


void rul__gen_rhs_tmp_mol_inits (void)
{
  Ext_Temp_Var var;
  Ext_Temp_Var used_rhs_molecules =
          INDEX_EXT_TYPE_ARRAY (first_rhs_ext_temp_var,
				ext_type_atom)[EXT__C_NOT_ARRAY];

  /* !!! does not init arrays or array pointers !!! */

  if (used_rhs_molecules != NULL) {
    rul__emit_comment ("Initialize Unnamed RHS Temp Mol Variables");
    var = used_rhs_molecules;
    while (var != NULL) {
      rul__emit_stack_mol_init (var->index);
      var = var->next;
    }
    rul__emit_blank_line ();
  }
}

void rul__gen_lhs_tmp_mol_inits (void)
{
  Ext_Temp_Var var;
  Ext_Temp_Var used_lhs_molecules =
          INDEX_EXT_TYPE_ARRAY (first_lhs_ext_temp_var,
				ext_type_atom)[EXT__C_NOT_ARRAY];

  /* !!! does not init arrays or array pointers !!! */

  if (used_lhs_molecules != NULL) {
    rul__emit_comment ("Initialize Temp lhs Molecule Variables");
    var = used_lhs_molecules;
    while (var != NULL) {
      rul__emit_stack_mol_init (var->index);
      var = var->next;
    }
  }
}


void rul__gen_rhs_var_decl (void)
{
  long     num_vars;

  if (use_entry_vars)
    num_vars = number_entry_rhs_vars;
  else
    num_vars = number_rhs_vars;

  if (num_vars > 0) {
    rul__emit_comment ("Named RHS Variables");
    /* allocate stack space for RHS variables */
    rul__emit_rhs_var_decl (num_vars);
    /* emit code to initialize each variable */
    rul__emit_blank_line ();
  }
}




void rul__gen_rhs_var_inits (void)
{
  Rhs_Var  var;
  Value	   init_val = NULL, rhs_var, asn_val, func1_val, func2_val;
  char	   buffer[RUL_C_MAX_SYMBOL_SIZE+20];
  long     num_vars;

  if (use_entry_vars) {
    num_vars = 	number_entry_rhs_vars;
    var = 	first_entry_rhs_var;
  } else {
    num_vars = 	number_rhs_vars;
    var = 	first_rhs_var;
  }

  if (num_vars > 0) {
    for (; var != NULL; var = var->next) {

      sprintf (buffer, "Initialization of %s", 
	       rul__mol_get_printform (var->name));
      rul__emit_comment (buffer);
      rhs_var = rul__val_create_rhs_variable (var->name);
      rul__val_set_rhs_var_index (rhs_var, var->index);

      if (var->initializer == NULL) {
	init_val = rul__val_create_assignment (rhs_var,
			       rul__val_create_null (CMP__E_INT_MOLECULE));
      }

      else {
	asn_val = rul__val_create_assignment (rhs_var,  var->initializer);
#ifndef NDEBUG
	init_val = rul__val_create_function_str ("rul__mol_incr_uses", 1);
#else
	init_val = rul__val_create_function_str ("MIU", 1);
#endif
	rul__val_set_nth_subvalue (init_val, 1, asn_val);
      }

      rul__emit_value (init_val);
      rul__val_for_each_unnamed_mol (init_val,
				     gen_decr_for_rhs_tmp_mol);
      rul__val_free_value (init_val);
    }
    rul__emit_blank_line ();
  }
}


static void gen_decr_for_rhs_tmp_mol (Value tmp_mol_var)
{
	Value func_val;

	assert (rul__val_is_mol_variable (tmp_mol_var));
#ifndef NDEBUG
	func_val = rul__val_create_function_str ("rul__mol_decr_uses", 1);
#else
	func_val = rul__val_create_function_str ("MDU", 1);
#endif
	rul__val_set_nth_subvalue (func_val, 1, 
				   rul__val_copy_value (tmp_mol_var));
	rul__emit_value (func_val);
	rul__val_free_value (func_val);
}



void rul__gen_decr_rhs_vars (void)
  /*
  **  This function is used at the end of a right-hand-side function
  **  (or an on-exit function) to generate the decrements for all the
  **  named right-hand-side variables, and then clear the rhs_var data
  **  structures in the compiler.
  */
{
  rul__gen_decr_rhs_vars_simple ();  
  clear_rhs_vars ();
}


void rul__gen_decr_rhs_vars_simple (void)
  /*
  **  This function is usually called from rul__gen_decr_rhs_vars, but
  **  it is called directly when generating code for a return action 
  **  within a non-top-level action sequence.  It generates the 
  **  decr_uses for all the named rhs_vars before doing the actual C return.
  */
{
  Rhs_Var var;	/* used to index through the variable list */
  Value   func_val;	/* used for the function call to rul__mol_decr_uses */
  Value   rhs_var;	/* used as a parameter to the function call above   */
  long    num_vars;

  if (use_entry_vars)
    num_vars = number_entry_rhs_vars;
  else
    num_vars = number_rhs_vars;

  if (num_vars > 0) {
    rul__emit_comment (
       "Decrement reference count on Molecules in Named RHS Variables");

    if (use_entry_vars)
      var = first_entry_rhs_var;
    else
      var = first_rhs_var;

    for (; var != NULL; var = var->next) {
#ifndef NDEBUG
      func_val = rul__val_create_function_str ("rul__mol_decr_uses", 1);
#else
      func_val = rul__val_create_function_str ("MDU", 1);
#endif
      rhs_var = rul__val_create_rhs_variable (var->name);
      rul__val_set_rhs_var_index (rhs_var, var->index);
      rul__val_set_nth_subvalue (func_val, 1, rhs_var);
      rul__emit_value (func_val);
      rul__val_free_value (func_val);
    }
    rul__emit_blank_line ();
  }
}


void rul__gen_decr_rhs_tmps_count (long j)
{
  long         i;
  Value        func_val;

  for (i = 0; i < j; i++) {

#ifndef NDEBUG
    func_val = rul__val_create_function_str ("rul__mol_decr_uses", 1);
#else
    func_val = rul__val_create_function_str ("MDU", 1);
#endif
    rul__val_set_nth_subvalue (func_val, 1,
			       rul__val_create_unnamed_mol_var (i));
    rul__emit_value (func_val);
    rul__val_free_value (func_val);
  }
}



long rul__gen_decr_rhs_tmps (void)
{
  long         i, j;
  Value        func_val;

  j = (long) rul__dyar_pop_first (action_tmp_vars);

  for (i = 0; i < j; i++) {

#ifndef NDEBUG
    func_val = rul__val_create_function_str ("rul__mol_decr_uses", 1);
#else
    func_val = rul__val_create_function_str ("MDU", 1);
#endif
    rul__val_set_nth_subvalue (func_val, 1,
			       rul__val_create_unnamed_mol_var (i));
    rul__emit_value (func_val);
    rul__val_free_value (func_val);
  }
  return j;
}

void rul__gen_decr_lhs_tmps (void)
{
  Ext_Temp_Var var;
  Value        func_val;
  Ext_Type     type;
  Ext_Temp_Var used_lhs_molecules =
    INDEX_EXT_TYPE_ARRAY (first_lhs_ext_temp_var,
			  ext_type_atom)[EXT__C_NOT_ARRAY];

  if (used_lhs_molecules != NULL) {
    rul__emit_comment ("Decrement reference count on Temporary Molecules");

    for (var = used_lhs_molecules; var != NULL; var = var->next) {
#ifndef NDEBUG
      func_val = rul__val_create_function_str ("rul__mol_decr_uses", 1);
#else
      func_val = rul__val_create_function_str ("MDU", 1);
#endif
      rul__val_set_nth_subvalue (func_val, 1,
				 rul__val_create_unnamed_mol_var (var->index));
      rul__emit_value (func_val);
      rul__val_free_value (func_val);
    }
    rul__emit_blank_line ();
  }

  /* clear registered lhs tmp vars */
  rul__sem_initialize_lhs_vars ();
}



static  Dynamic_Array	SA_rhs_used_lhs_vars = NULL;


void rul__sem_tag_rhs_use_of_lhs_var (Molecule var_name)
	/*
	**  Add the specified LHS variable to the set of
	**  LHS variables that are used within the RHS.
	*/
{
	long i;

	if (SA_rhs_used_lhs_vars == NULL) {
	    SA_rhs_used_lhs_vars = rul__dyar_create_array (10);
	}

	if (!rhs_variable_registered (var_name)) {
	    /*
	    **  We need to recursively check for LHS variables that are
	    **  needed to initialize this LHS variable being used on the
	    **  right-hand-side, and we need to do so BEFORE we add this
	    **  LHS variable to the init-list and to RHS variable table.
	    */
	    rul__lvar_find_nested_lhs_vars (var_name);

	    /*  Now add the entry for this variable  */
	    rul__dyar_append (SA_rhs_used_lhs_vars, (void *) var_name);
	    add_rhs_variable_entry (var_name);
	    rul__sem_set_var_type_from_lhs (var_name);
	}
}


void rul__sem_set_inits_of_lhs_vars (void)
{
	long i;
	Molecule var_name;
	Value var_init_val;

	if (SA_rhs_used_lhs_vars != NULL) {

	    for (i=0; i<rul__dyar_get_length(SA_rhs_used_lhs_vars); i++) {
		/*
		**  For each LHS variable being used on the RHS, 
		**  build the initializer, and attach it to the RHS variable
		*/
		var_name = (Molecule) 
			rul__dyar_get_nth (SA_rhs_used_lhs_vars, i);

		var_init_val = rul__lvar_build_rhs_value (var_name);
		rul__sem_set_var_initializer (var_name, var_init_val);
	    }
	}
}


static void sem_clear_rhs_used_lhs_vars ()
	/*
	**  Toss the old array contents, so we're ready for the next rule.
	*/
{
	if (SA_rhs_used_lhs_vars != NULL) {
	    rul__dyar_set_array_empty (SA_rhs_used_lhs_vars);
	}
}


void rul__sem_set_var_type_from_lhs (Mol_Symbol name)
{
	Rhs_Var var;

	var = find_rhs_var (name);
	assert (var != NULL /* Unregistered RHS Variable */ );
	assert (var->initializer == NULL);

	var->initializer = VAR_C_INIT_FROM_LHS;
	var->domain = rul__lvar_get_rhs_domain (name);
	var->shape  = rul__lvar_get_rhs_shape  (name);
	var->rclass  = rul__lvar_get_rhs_class  (name);
}

