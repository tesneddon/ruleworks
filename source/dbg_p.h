/*
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

*/

/* structure for maintaining lists of constructs */
typedef struct _rb_cons {
  struct _rb_cons *rb_next;
  long             rb_count;
  Molecule        *rb_names;
  Construct_Type  *rb_types;
} Rb_Cons;

#define PAT_SCALAR 0
#define PAT_DISJ   1
#define PAT_CONJ   2

#define PAT_CE_POS 0
#define PAT_CE_NEG 1

typedef enum {
   DBG__E_PL_MOL = 0,
   DBG__E_PL_RUL,
   DBG__E_PL_PAT,
   DBG__E_PL_INT,
   DBG__E_PL_FLT,
   DBG__E_PL_ASZ
} Param_Type;

typedef struct _pl {
   struct _pl  *next;
   Param_Type   type;
   void        *data;
} *Param_List;


typedef struct _pt {
  struct _pt	*next;		/* next value test		*/
  long 		 ttype;		/* 0 scalar, 1 disj, 2 conj	*/
  long 	         pred;		/* 0 for eq else token value	*/
  union {
    Molecule     value;		/* if scalar, conj, disj        */
    struct _pt  *pt;		/* if conj of disj              */
  } u;
} Pat_Tests;

typedef struct _pa {
  struct _pa    *next;		/* pointer to next apv		*/
  Molecule       attr;  	/* the attr atom      		*/
  long           idx;   	/* 0 or compound attr index 	*/
  Pat_Tests    	*pt;		/* test	           		*/
} Pat_Attrs;

typedef struct _pc {
  struct _pc    *next;
  Class          cls;
  long		 ctype;		/* 0 = positive, 1 = negated	*/
  Pat_Attrs     *pa;
} Pat_Classes;

typedef struct {
  long		 ptype;		/* 0 scalar, 1 disj, 2 = conj	*/
  Pat_Classes   *pc;
} Pat_Pattern;

struct rulename {
  Mol_Symbol     rb_name;
  Mol_Symbol     rule_name;
};

typedef struct rulename *RuleName;

/* debugger routines */

void        rul__dbg_trace (long arg_count, ...);

void        rul__debug (void);

Entry_Data  rul__dbg_get_ebdata (void);

void        rul__dbg_print (Object wmo);/* equivalent to
				   rul__wm_print_readform.  Note that it
				   must be a function vs. a macro
				   so it can be called from an iterator */
void        rul__dbg_pp_print (Object wmo);

void        rul__dbg_remove_class_wmes (Class cls);

void        rul__dbg_show_ebreaks (void);
void        rul__dbg_ebreak (Boolean on, Param_List pl);
Boolean     rul__dbg_match_ebreak (Mol_Symbol ebname);

void        rul__dbg_show_rbreaks (void);
void        rul__dbg_rbreak (Boolean on, Param_List pl);
Boolean     rul__dbg_match_rbreak (Conflict_Set_Entry cse);/* check */

void	    rul__dbg_show_wbreaks (void);
void	    rul__dbg_wbreak (Boolean on, Param_List pl);
Boolean     rul__dbg_match_wbreak (Object wmo);

void        rul__dbg_add_data_to_list (Param_List *pl,
				       Param_Type type, void *data);
Param_List  rul__dbg_in_mol_in_list (Molecule mol, Param_List pl);
void	    rul__dbg_free_param_list (Param_List pl);
void	    rul__dbg_free_pattern (Pat_Pattern **pat);
void	    rul__dbg_set_pp_pat (Pat_Pattern *pat);
void        rul__dbg_print_class_list (void);
Boolean     rul__dbg_is_visible_class (Mol_Symbol rclass);
void        rul__dbg_show_amb_class (Mol_Symbol rclass);

void        rul__dbg_set_prompt (Molecule prompt);
void        rul__dbg_set_exit (long exit_value);
void        rul__dbg_flag_changes (void);
void        rul__dbg_print_cs (void);
void        rul__dbg_show_matches (Param_List pl);
void        rul__dbg_show_next (void);
void        rul__dbg_set_run (long run_count);
void	    rul__dbg_set_return (Molecule ret_val);

Boolean     rul__dbg_is_dup_rule (Mol_Symbol);/* checks for dups */
void        rul__dbg_show_amb_rule (Mol_Symbol);
Boolean     rul__dbg_is_valid_db (Mol_Symbol);/* checks for decl blocks */
Boolean     rul__dbg_is_valid_rb (long *, Mol_Symbol);/* checks for rb */
Boolean     rul__dbg_is_valid_rule (long *, Mol_Symbol);/* checks for rule */
void        rul__dbg_set_catcher (Molecule after_count, Molecule catcher);

void        rul__dbg_addstate (Mol_Symbol filename);
void        rul__dbg_restorestate (Mol_Symbol filename);
void        rul__dbg_savestate (Mol_Symbol filename);

void        rul__dbg_set_group (void);
Mol_Symbol  rul__dbg_get_group (Conflict_Set_Entry);
char       *rul__dbg_toupper (char *);	/* uppercase routine */
void        rul__dbg_enable_control_y (void);
void        rul__dbg_enable_control_c (void);
void        rul__dbg_disable_control_c (void);

#ifdef __VMS
extern Boolean    rul__smg_init (void);
extern char      *rul__smg_input (char *buf, unsigned nbytes,
			          char *prompt, Boolean cmd_continue);
#endif /* __VMS */




