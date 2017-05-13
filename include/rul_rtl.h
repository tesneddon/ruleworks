
#ifndef RUL_RTL
#define RUL_RTL

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#ifdef OPS_COMPATIBILITY_DEFS
/* some defines for ops compatibility...*/

#define OPS_C_MAX_SYMBOL_SIZE		RUL_C_MAX_SYMBOL_SIZE
#define OPS_C_INVALID_LENGTH		RUL_C_INVALID_LENGTH
#define OPS_C_INVALID_ATOM		RUL_C_INVALID_ATOM
#define OPS_C_RESET_WM_ATOM		RUL_C_RESET_WM_ATOM
#define ops_atom_is_fatom		rul_atom_is_fatom
#define ops_atom_is_iatom		rul_atom_is_iatom
#define ops_atom_is_symbol		rul_atom_is_symbol
#define ops_atom_is_instance_id		rul_atom_is_instance_id
#define ops_atom_is_compound		rul_atom_is_compound
#define ops_atom_is_oatom		rul_atom_is_oatom
#define ops_fatom_to_float		rul_fatom_to_float
#define ops_fatom_to_double		rul_fatom_to_double
#define ops_float_to_fatom		rul_float_to_fatom
#define ops_double_to_fatom		rul_double_to_fatom
#define ops_gensym			rul_gensym
#define ops_gensymp			rul_gensymp
#define ops_genint			rul_genint
#define ops_iatom_to_integer		rul_iatom_to_integer
#define ops_integer_to_iatom		rul_integer_to_iatom
#define ops_oatom_to_ptr 		rul_oatom_to_ptr
#define ops_ptr_to_oatom		rul_ptr_to_oatom
#define ops_string_to_symbol		rul_ptr_to_symbol
#define ops_symbol_to_string		rul_symbol_to_string
#define ops_string_to_atom		rul_string_to_atom
#define ops_atom_to_string		rul_atom_to_string
#define ops_atom_to_string_length	rul_atom_to_string_length
#define ops_string_to_compound		rul_string_to_compound
#define ops_attr_is_compound		rul_attr_is_compound
#define ops_is_attribute		rul_is_attribute
#define ops_is_class			rul_is_class
#define ops_is_subclass			rul_is_subclass
#define ops_is_instance			rul_is_instance
#define ops_get_attr_atom		rul_get_attr_atom
#define ops_get_class_string		rul_get_class_string
#define ops_get_class_string_length	rul_get_class_string_length
#define ops_get_comp_attr_length	rul_get_comp_attr_length
#define ops_get_comp_attr_string	rul_get_comp_attr_string
#define ops_get_comp_attr_string_len	rul_get_comp_attr_string_len
#define ops_get_comp_elem_atom		rul_get_comp_elem_atom
#define ops_get_instance		rul_get_instance
#define ops_get_instance_length		rul_get_instance_length
#define ops_get_next_instance		rul_get_next_instance
#define ops_set_attr_atom		rul_set_attr_atom
#define ops_set_attr_float		rul_set_attr_float
#define ops_set_attr_double		rul_set_attr_double
#define ops_set_attr_integer		rul_set_attr_integer
#define ops_set_attr_string		rul_set_attr_string
#define ops_set_comp_attr_string	rul_set_comp_attr_string
#define ops_set_comp_elem_atom		rul_set_comp_elem_atom
#define ops_set_comp_elem_float		rul_set_comp_elem_float
#define ops_set_comp_elem_double	rul_set_comp_elem_double
#define ops_set_comp_elem_integer	rul_set_comp_elem_integer
#define ops_set_comp_elem_opaque	rul_set_comp_elem_opaque
#define ops_set_comp_elem_string	rul_set_comp_elem_string
#define ops_copy_instance		rul_copy_instance
#define ops_make_instance		rul_make_instance
#define ops_modify_instance_start	rul_modify_instance_start
#define ops_modify_instance_end		rul_modify_instance_end
#define ops_remove_instance		rul_remove_instance
#define ops_specialize_instance		rul_specialize_instance
#define ops_clear			rul_clear
#define ops_completion			rul_completion
#define ops_debug			rul_debug
#define ops_get_firing_rule		rul_get_firing_rule
#define ops_initialize			rul_initialize
#define ops_run				rul_run
#define ops_startup			rul_startup
#define ops_start_id_translation	rul_start_id_translation
#define ops_end_id_translation		rul_end_id_translation

#define ops_atom                        rul_atom
#define ops_boolean                     rul_boolean

#endif

/* real defines */

#define RUL_C_MAX_SYMBOL_SIZE	256	/* Max characters in symbolic atom */
#define RUL_C_INVALID_LENGTH	-1
#define RUL_C_INVALID_ATOM	(rul_atom)NULL
#define RUL_C_RESET_WM_ATOM	(rul_atom)NULL

typedef void *rul_atom;
typedef long rul_boolean;


#if (defined(__VMS) || defined(VAXC)) && !defined(USE_LOWERCASE_BINDINGS)

/*
 * The following #defines allow programs compiled without /NAME=AS_IS to
 * link with the C bindings by replacing symbols of the form "rul_*" with
 * "RUL_*_C".  The "RUL_*_C" routines are identical to the "rul_*"
 * routines, except for their names.
 */
#define rul_atom_is_fatom		RUL_ATOM_IS_FATOM_C
#define rul_atom_is_iatom		RUL_ATOM_IS_IATOM_C
#define rul_atom_is_symbol		RUL_ATOM_IS_SYMBOL_C
#define rul_atom_is_instance_id		RUL_ATOM_IS_INSTANCE_ID_C
#define rul_atom_is_compound		RUL_ATOM_IS_COMPOUND_C
#define rul_atom_is_oatom		RUL_ATOM_IS_OATOM_C
#define rul_fatom_to_float		RUL_FATOM_TO_FLOAT_C
#define rul_fatom_to_double		RUL_FATOM_TO_DOUBLE_C
#define rul_float_to_fatom		RUL_FLOAT_TO_FATOM_C
#define rul_double_to_fatom		RUL_DOUBLE_TO_FATOM_C
#define rul_gensym			RUL_GENSYM_C
#define rul_gensymp			RUL_GENSYMP_C
#define rul_genint			RUL_GENINT_C
#define rul_iatom_to_integer		RUL_IATOM_TO_INTEGER_C
#define rul_integer_to_iatom		RUL_INTEGER_TO_IATOM_C
#define rul_oatom_to_ptr		RUL_OATOM_TO_PTR_C
#define rul_ptr_to_oatom		RUL_PTR_TO_OATOM_C
#define rul_string_to_symbol		RUL_STRING_TO_SYMBOL_C
#define rul_symbol_to_string		RUL_SYMBOL_TO_STRING_C
#define rul_string_to_atom		RUL_STRING_TO_ATOM_C
#define rul_atom_to_string		RUL_ATOM_TO_STRING_C
#define rul_atom_to_string_length	RUL_ATOM_TO_STRING_LENGTH_C
#define rul_string_to_compound		RUL_STRING_TO_COMPOUND_C
#define rul_attr_is_compound		RUL_ATTR_IS_COMPOUND_C
#define rul_is_attribute		RUL_IS_ATTRIBUTE_C
#define rul_is_class			RUL_IS_CLASS_C
#define rul_is_subclass			RUL_IS_SUBCLASS_C
#define rul_is_instance			RUL_IS_INSTANCE_C
#define rul_get_attr_atom		RUL_GET_ATTR_ATOM_C
#define rul_get_class_string		RUL_GET_CLASS_STRING_C
#define rul_get_class_string_length	RUL_GET_CLASS_STRING_LENGTH_C
#define rul_get_comp_attr_length	RUL_GET_COMP_ATTR_LENGTH_C
#define rul_get_comp_attr_string	RUL_GET_COMP_ATTR_STRING_C
#define rul_get_comp_attr_string_len	RUL_GET_COMP_ATTR_STRING_LEN_C
#define rul_get_comp_elem_atom		RUL_GET_COMP_ELEM_ATOM_C
#define rul_get_instance		RUL_GET_INSTANCE_C
#define rul_get_instance_length		RUL_GET_INSTANCE_LENGTH_C
#define rul_get_next_instance		RUL_GET_NEXT_INSTANCE_C
#define rul_set_attr_atom		RUL_SET_ATTR_ATOM_C
#define rul_set_attr_float		RUL_SET_ATTR_FLOAT_C
#define rul_set_attr_double		RUL_SET_ATTR_DOUBLE_C
#define rul_set_attr_integer		RUL_SET_ATTR_INTEGER_C
#define rul_set_attr_string		RUL_SET_ATTR_STRING_C
#define rul_set_comp_attr_string	RUL_SET_COMP_ATTR_STRING_C
#define rul_set_comp_elem_atom		RUL_SET_COMP_ELEM_ATOM_C
#define rul_set_comp_elem_float		RUL_SET_COMP_ELEM_FLOAT_C
#define rul_set_comp_elem_double	RUL_SET_COMP_ELEM_DOUBLE_C
#define rul_set_comp_elem_integer	RUL_SET_COMP_ELEM_INTEGER_C
#define rul_set_comp_elem_opaque	RUL_SET_COMP_ELEM_OPAQUE_C
#define rul_set_comp_elem_string	RUL_SET_COMP_ELEM_STRING_C
#define rul_copy_instance		RUL_COPY_INSTANCE_C
#define rul_make_instance		RUL_MAKE_INSTANCE_C
#define rul_modify_instance_start	RUL_MODIFY_INSTANCE_START_C
#define rul_modify_instance_end		RUL_MODIFY_INSTANCE_END_C
#define rul_remove_instance		RUL_REMOVE_INSTANCE_C
#define rul_specialize_instance		RUL_SPECIALIZE_INSTANCE_C
#define rul_clear			RUL_CLEAR_C
#define rul_completion			RUL_COMPLETION_C
#define rul_debug			RUL_DEBUG_C
#define rul_get_firing_rule		RUL_GET_FIRING_RULE_C
#define rul_initialize			RUL_INITIALIZE_C
#define rul_run				RUL_RUN_C
#define rul_startup			RUL_STARTUP_C
#define rul_start_id_translation	RUL_START_ID_TRANSLATION_C
#define rul_end_id_translation		RUL_END_ID_TRANSLATION_C

#endif


/*
 *	ATOM TABLE ROUTINES
 */

        /*  Atom Predicate Routines --	*/

rul_boolean  rul_atom_is_fatom       (rul_atom atom_value);
rul_boolean  rul_atom_is_iatom       (rul_atom atom_value);
rul_boolean  rul_atom_is_symbol      (rul_atom atom_value);
rul_boolean  rul_atom_is_compound    (rul_atom atom_value);
rul_boolean  rul_atom_is_oatom       (rul_atom atom_value);
rul_boolean  rul_atom_is_instance_id (rul_atom atom_value);

        /*  Atom Conversion Routines --	 */

float    rul_fatom_to_float          (rul_atom atom_value);
double   rul_fatom_to_double         (rul_atom atom_value);
rul_atom rul_float_to_fatom          (float float_value);
rul_atom rul_double_to_fatom         (double float_value);
rul_atom rul_gensym		     (void);
rul_atom rul_gensymp		     (char *char_string);
rul_atom rul_genint		     (void);
long	 rul_iatom_to_integer	     (rul_atom atom_value);
rul_atom rul_integer_to_iatom	     (long long_value);
void *	 rul_oatom_to_ptr	     (rul_atom atom_value);
rul_atom rul_ptr_to_oatom	     (void *ptr_value);
rul_atom rul_string_to_symbol	     (char *char_string);
long 	 rul_symbol_to_string	     (char *char_buffer,
				      long buffer_size,
				      rul_atom atom_value);
rul_atom rul_string_to_atom	     (char *char_string);
rul_atom rul_string_to_compound	     (char *char_string);
long	 rul_atom_to_string	     (char *char_buffer,
				      long buffer_size,
				      rul_atom atom_value);
long	 rul_atom_to_string_length   (rul_atom atom_value);



/*
 *	WM QUERY ROUTINES
 */

        /*  WME Predicate Routines --	*/

rul_boolean  rul_attr_is_compound (char *class_name, 
				   char *attr_name,
				   char *extra_name);

rul_boolean  rul_is_attribute     (char *class_name, 
				   char *attr_name,
				   char *extra_name);

rul_boolean  rul_is_class         (char *class_name, 
				   char *extra_name);

rul_boolean  rul_is_subclass      (char *child_name,
				   char *parent_name,
				   char *extra_name);

rul_boolean  rul_is_instance	  (rul_atom instance_id);


	/* WME Retrival Routines --  */


rul_atom rul_get_attr_atom        (rul_atom instance_id,
				   char *attr_name);

long  rul_get_class_string     (char *char_string, 
				long	string_size,
				rul_atom instance_id);

long  rul_get_class_string_length (rul_atom instance_id);

long  rul_get_comp_attr_length (rul_atom instance_id,
				char *attr_name);

long  rul_get_comp_attr_string (char *values_string,
				long	string_size,
				rul_atom instance_id,
				char *attr_name);

long  rul_get_comp_attr_string_len (rul_atom instance_id,
				    char *attr_name);

rul_atom rul_get_comp_elem_atom   (rul_atom instance_id,
				   char *attr_name,
				   long element_index);

long rul_get_instance		  (char *print_string,
			           long string_size,
			           rul_atom instance_id);

long rul_get_instance_length	  (rul_atom instance_id);

rul_atom rul_get_next_instance	  (rul_atom instance_id);


	/* WME MODIFICATION ROUTINES -- */

rul_boolean  rul_set_attr_atom     (rul_atom instance_id,
			            char *attr_name,
				    rul_atom value);

rul_boolean  rul_set_attr_float    (rul_atom instance_id,
			            char *attr_name,
				    float value);

rul_boolean  rul_set_attr_double    (rul_atom instance_id,
				     char *attr_name,
				     double value);

rul_boolean  rul_set_attr_integer  (rul_atom instance_id,
			            char *attr_name,
				    long value);

rul_boolean  rul_set_attr_string   (rul_atom instance_id,
			            char *attr_name,
				    char *value);

rul_boolean  rul_set_comp_attr_string (rul_atom instance_id,
				       char *attr_name,
				       char *values);

rul_boolean  rul_set_comp_elem_atom (rul_atom instance_id,
				     char *attr_name,
				     long element_index,
				     rul_atom value);

rul_boolean  rul_set_comp_elem_float (rul_atom instance_id,
				      char *attr_name,
				      long element_index,
				      float value);

rul_boolean  rul_set_comp_elem_double (rul_atom instance_id,
				       char *attr_name,
				       long element_index,
				       double value);

rul_boolean  rul_set_comp_elem_integer (rul_atom instance_id,
					char *attr_name,
					long element_index,
					long value);

rul_boolean  rul_set_comp_elem_opaque (rul_atom instance_id,
					char *attr_name,
					long element_index,
					void *value);

rul_boolean  rul_set_comp_elem_string (rul_atom instance_id,
				       char *attr_name,
				       long element_index,
				       char *value);

rul_atom rul_copy_instance         (rul_atom old_instance_id);

rul_atom rul_make_instance         (char *char_string,
				    char *extra_name);

rul_boolean  rul_modify_instance_start (rul_atom instance_id);

rul_boolean  rul_modify_instance_end (rul_atom instance_id);

rul_boolean  rul_remove_instance   (rul_atom instance_id);

rul_atom  rul_specialize_instance  (rul_atom instance_id,
				    char *subclass);


	/* MISCELLANEOUS ROUTINES */

void rul_clear  (void);

void rul_completion (void address());

void rul_debug (void);

rul_atom rul_get_firing_rule (void);

void rul_initialize (void);

void rul_run (void);

void rul_startup (void);

void rul_start_id_translation (void);

void rul_end_id_translation (void);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

