/****************************************************************************
**                                                                         **
**                C M P _ S E M _ C O N S T R U C T . C                    **
**                                                                         **
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
**                                                                         **
****************************************************************************/


/*
 * FACILITY:
 *	RULEWORKS compiler
 *
 * ABSTRACT:
 *	This module provides the top-level semantic check routines.
 *
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *
 *	16-Dec-1992	DEC	Initial version
 *
 *	 3-Jun-1994	DEC	Add message for missing RETURN
 *
 *	30-Aug-1994	DEC	Allow AFTERs to reference CATCHers not yet
 *					defined
 *
 *	01-Dec-1999	CPQ	Release with GPL
 */

#include <common.h>
#include <cmp_comm.h>
#include <ast.h>
#include <conrg.h>
#include <decl.h>
#include <gen.h>
#include <ios.h>
#include <mol.h>
#include <msg.h>
#include <cmp_msg.h>
#include <sem.h>
#include <dyar.h>

/* Forward declarations */

static Boolean	check_block_nesting (Ast_Node ast);

static Boolean	verify_decls_allowed (Ast_Node ast);

static void	done_with_decls (void);

static void	sem_finish_current_block (Ast_Node ast);


static Construct_Type	SE_block_type = RUL__C_NOISE; 
				/*  Type of block we're in  */
static Mol_Symbol	SA_construct_name = NULL;
				/*  Name of current construct  */
static Boolean		SB_declarations_done = FALSE;
				/* No declarations allowed after
				   the first executable construct */
static Boolean 		SB_entry_block_code_gen_pending = FALSE;
static Boolean 		SB_rule_block_code_gen_pending = FALSE;

static Boolean		SB_rule_seen = FALSE;
static Boolean		SB_return_seen = FALSE;	/* Seen RETURN in this EB? */

static Dynamic_Array	SA_afters_referenced = NULL;
				/* names (mols) of CATCHers used in AFTERs */




/**************************
**                       **
**  RUL__SEM_RULES_SEEN  **
**                       **
**************************/
Boolean
rul__sem_rules_seen (void)
{
  return SB_rule_seen;
}


/**********************************
**				 **
**  RUL__SEM_EB_RETURN_WAS_SEEN  **
**				 **
**********************************/
/*
 * State that an entry-block RETURN action has been seen in this entry block.
 */
void
rul__sem_eb_return_was_seen (void)
{
    SB_return_seen = TRUE;
}
    

/*******************************
**                            **
**  RUL__SEM_CHECK_CONSTRUCT  **
**                            **
*******************************/

Boolean
rul__sem_check_construct (Ast_Node ast)
{
	Ast_Node_Type	ast_typ = rul__ast_get_type(ast);
	Boolean		status = TRUE; /* Assume success */
	Ast_Node	ast_child; /* Child of passed ast */

	/* Get this construct's name. */
	ast_child = rul__ast_get_child(ast);

	if (SA_construct_name != NULL)
	    {
	    /* if there was an old construct name, reduce refs to it first */
	    rul__mol_decr_uses (SA_construct_name);
	    }

	if (ast_child != NULL)
	    {
	    SA_construct_name = rul__ast_get_value(ast_child);
	    if (SA_construct_name == NULL)
		SA_construct_name = rul__mol_make_symbol("");
	    else
		rul__mol_incr_uses (SA_construct_name);
	    }
	else
	    SA_construct_name = rul__mol_make_symbol("");

	/* If not starting a new block, ensure we're already in a block. */
	if ((ast_typ != AST__E_ENTRY_BLOCK) &&
	    (ast_typ != AST__E_DECL_BLOCK) &&
	    (ast_typ != AST__E_RULE_BLOCK) &&
	    (! check_block_nesting(ast))) /* Complain if not in block */
	/* If we're not in a block, don't try checking the construct. */
	  return FALSE;	/* Failure */
	
	switch (ast_typ) {

	  case AST__E_ENTRY_BLOCK:
	    rul__sem_prev_block_closed (ast);
	    SE_block_type = RUL__C_ENTRY_BLOCK;
	    SB_declarations_done = FALSE; /* Decls are allowed now */
	    SB_rule_seen = FALSE; /*no rules have been seen in this block yet*/
	    SB_return_seen = FALSE;
	    status = rul__sem_entry_block(ast);
	    if (! status) 
		{
		SE_block_type = RUL__C_NOISE;
		SB_declarations_done = TRUE;
		}
	    else
	      SB_entry_block_code_gen_pending = TRUE;
	    break;

	  case AST__E_DECL_BLOCK:
	    rul__sem_prev_block_closed (ast);
	    SE_block_type = RUL__C_DECL_BLOCK;
	    SB_declarations_done = FALSE; /* Decls are allowed now */
	    status = rul__sem_decl_block(ast);
	    if (! status) 
		{
		SE_block_type = RUL__C_NOISE;
		SB_declarations_done = TRUE;
		}
	    break;

	  case AST__E_RULE_BLOCK:
	    rul__sem_prev_block_closed (ast);
	    SE_block_type = RUL__C_RULE_BLOCK;
	    SB_declarations_done = FALSE; /* Decls are allowed now */
	    SB_rule_seen = FALSE; /*no rules have been seen in this block yet*/
	    status = rul__sem_rule_block(ast);
	    if (! status) 
		{
		SE_block_type = RUL__C_NOISE;
		SB_declarations_done = TRUE;
		}
	    else
	      SB_rule_block_code_gen_pending = TRUE;
	    break;


	  case AST__E_END_BLOCK:
	    sem_finish_current_block (ast);
	    SE_block_type = RUL__C_NOISE; /* No longer in any block */
	    break;

	  case AST__E_OBJ_CLASS:
	    status = (verify_decls_allowed(ast) &&
		      rul__sem_object_class(ast));
	    break;

	  case AST__E_METHOD:
	    status = (verify_decls_allowed(ast) &&
		      rul__sem_check_method(ast));
	    break;

	  case AST__E_GENERIC_METHOD:
	    status = (verify_decls_allowed(ast) &&
		      rul__sem_check_generic_method(ast));
	    break;

	  case AST__E_RULE:
	    done_with_decls();
	    SB_rule_seen = TRUE;
	    if (SB_entry_block_code_gen_pending) {
	      rul__gen_entry_block ();
	      SB_entry_block_code_gen_pending = FALSE;
	    }
	    if (SB_rule_block_code_gen_pending) {
	      rul__gen_rule_block ();
	      SB_rule_block_code_gen_pending = FALSE;
	    }
	    status = rul__sem_check_rule (ast);
	    break;

	  case AST__E_EXT_RT_DECL:
	    status = (verify_decls_allowed(ast) &&
		      rul__sem_check_ext_rt_decl (ast));
	    break;

	  case AST__E_CATCH:
	    done_with_decls();
	    status = rul__sem_check_catch (ast);
	    break;

	  case AST__E_ON_ENTRY:
	  case AST__E_ON_EVERY:
	  case AST__E_ON_EMPTY:
	  case AST__E_ON_EXIT:
	    done_with_decls();
	    if (SB_rule_seen) {
	      rul__msg_cmp_print (CMP_ONAFTRUL, ast,
				  rul___ast_type_to_string(ast_typ));
	      status = FALSE;
	    }
	    else
	      status = rul__sem_check_on_clause (ast);
	    break;

	  case AST__E_RULE_GROUP:
	    done_with_decls();
	    status = rul__sem_rule_group (ast);
	    break;

	  case AST__E_END_GROUP:
	    done_with_decls();
	    status = rul__sem_end_group (ast);
	    break;

	  default:
	    /*?*/ fprintf (stderr,
			   "\n  Un-checkable construct type:   %s\n",
			   rul___ast_type_to_string (ast_typ));
	    status = FALSE;   /* Failure */
	}
	return status;
}

/*****************************************************************************/
static Boolean
check_block_nesting(Ast_Node ast)
{
  Mol_Symbol type_sym;

  if (SE_block_type == RUL__C_NOISE) { /* Not in any block */

    type_sym =
      rul__mol_make_symbol(rul___ast_type_to_string(rul__ast_get_type(ast)));

    rul__msg_cmp_print_w_atoms(CMP_NOTINBLK, ast, 2, type_sym, SA_construct_name);
    rul__mol_decr_uses (type_sym);
    return FALSE;		/* Unsuccessful */
    }
  else
    return TRUE;
}

/*****************************************************************************/
static Boolean
verify_decls_allowed(Ast_Node ast)
{
/*
 * All declaration constructs must precede all executable constructs within
 * a block.  If they don't, we complain and ignore the declaration.
 */
    if (SB_declarations_done) {	/* If declarations not allowed here */
	rul__msg_cmp_print_w_atoms(CMP_DECLIGNORED, ast,
				   1, rul__ast_get_value(
					  rul__ast_get_child(ast)));
	return FALSE;		/* Error */
    }
    else
	return TRUE;		/* Verified correct */
}

/*****************************************************************************/
static void
done_with_decls(void)
{
    if (!SB_declarations_done) {
	SB_declarations_done = TRUE; /* Don't allow any more declarations */
	rul__decl_finish_decl_block (); /* Notify decl subsystem that the
					   declaring block is finished */

	rul__gen_declare_ext_rtns ();	/* declares all external routines */

	/* declare the static dblock varables before the on clauses */
	if (SE_block_type == RUL__C_ENTRY_BLOCK)
	  rul__gen_declare_dblock_vars ();
    }
}


Boolean
rul__sem_check_catch (Ast_Node ast)
{
  Boolean	status = TRUE;
  Ast_Node	node;
  Mol_Symbol    catch_name;

  /* Get this construct's name. */
  node = rul__ast_get_child(ast);
  assert (rul__ast_get_type (node) == AST__E_CONSTANT);
  assert (rul__ast_get_value_type (node) == AST__E_TYPE_MOL);
  catch_name = (Molecule) rul__ast_get_value (node);

  if (rul__conrg_get_construct_type (catch_name) != RUL__C_NOISE) {
    status = FALSE;
    rul__msg_cmp_print_w_atoms (CMP_DUPRULENAME, ast, 1, catch_name);
    return status;
  }
  
  status = rul__sem_check_rhs(ast);

  /*  If there were no serious semantic errors  */
  if (status == TRUE)
    rul__conrg_register_construct (catch_name, RUL__C_CATCH,
				   rul__ast_nearest_line_number (ast));
  return status;
}

void
rul__sem_clear_construct_name (void)
{
  if (SA_construct_name != NULL) {
    rul__mol_decr_uses (SA_construct_name);
    SA_construct_name = NULL;
  }
}


/*****************************************************************************/
void
rul__sem_catcher_referenced(Molecule mol_name)
{
/*
 * The given molecule has been used in an AFTER.  It must be defined as a
 * CATCHer before the end of the block.  We keep track of all CATCHers
 * referenced in AFTERs and check the list at the end of the block.
 */
    if (SA_afters_referenced == NULL)
	SA_afters_referenced = rul__dyar_create_array(10);

    rul__dyar_append_if_unique(SA_afters_referenced, mol_name);
}


/*****************************************************************************/
static void
sem_ensure_catchers_defined(void)
{
/*
 * The name of each CATCHer which has been referenced (in and AFTER) is stored
 * in the dynamic array SA_afters_referenced.  We ensure that each name in that
 * array has been defined as a CATCHer.  If not, we complain.
 */
    Molecule mol;
    if (SA_afters_referenced != NULL) {
	while (mol = (Molecule) rul__dyar_pop_first(SA_afters_referenced)) {
	    if (rul__conrg_get_construct_type(mol) != RUL__C_CATCH)
		rul__msg_print_w_atoms(CMP_INVCATNAM, 1, mol);
	}
	rul__dyar_set_array_empty(SA_afters_referenced);
    }
}	    


/*********************************
**                              **
**  RUL__SEM_PREV_BLOCK_CLOSED  **
**                              **
*********************************/

void
rul__sem_prev_block_closed (Ast_Node ast)
{
	char *name;
	Ast_Node tmp_ast;

	if (SE_block_type != RUL__C_NOISE) {
	    /*  The previous block was never closed!  First, complain.  */

	    if (ast == NULL) {
		name = rul__mol_get_readform (
			rul__conrg_get_cur_block_name());
		rul__msg_cmp_print_w_line (
			CMP_MISENDBLK,
			rul__ios_curr_line_num (RUL__C_SOURCE),
			name);
		rul__mem_free (name);

	    } else {

		rul__msg_cmp_print_w_atoms(
			CMP_MISENDBLK,
			ast, 1,
			rul__conrg_get_cur_block_name());
	    }

	    /*  Then do the block close ourselves  */
	    tmp_ast = rul__ast_node (AST__E_END_BLOCK);
	    sem_finish_current_block (tmp_ast);
	    rul__gen_end_block ();
	    rul__ast_free (tmp_ast);
	}
}



/*******************************
**                            **
**  SEM_FINISH_CURRENT_BLOCK  **
**                            **
*******************************/

static void
sem_finish_current_block (Ast_Node ast)
{
	done_with_decls();
	if (SB_entry_block_code_gen_pending) {
	    rul__gen_entry_block ();
	    SB_entry_block_code_gen_pending = FALSE;
	}
	if (SB_rule_block_code_gen_pending) {
	    rul__gen_rule_block ();
	    SB_rule_block_code_gen_pending = FALSE;
	}

	/*
	 * Make sure a CATCHer has been defined for each CATCHer referenced
	 * in an AFTER action.
	 */
	sem_ensure_catchers_defined();

	/*
	 * If the entry block was declared to return a value, and there
	 * was no RETURN, we have a problem.  main() implicitly returns
	 * a value, but we don't insist on a RETURN action for it.
	 */
	if (SE_block_type == RUL__C_ENTRY_BLOCK &&
	    !rul__conrg_is_main_block() && /* Don't complain for main() */
	    !SB_return_seen &&
	    rul__decl_ext_rt_ret_val ( /* TRUE if eb returns value  */
		rul__decl_get_visible_ext_rt (
		    rul__conrg_get_cur_block_name()))) {
	    rul__msg_cmp_print_w_atoms(CMP_MISRETURN,
				       ast, 1,
				       rul__conrg_get_cur_block_name());
	}	    

	(void) rul__sem_end_block (ast);

	SE_block_type = RUL__C_NOISE; /* No longer in any block */
}
