/* functions to encode inheritance for object classes	    */
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
 * FACILITY:
 *	RULEWORKS run time system and compiler
 *
 * ABSTRACT:
 *	This module provides functions for creation and manipulation of
 *	the class encoding structures used by the compiler to generate
 *	fast subtype tests for use in generated code and for use by PPWM.
 *
 *	The same routines are used at both compile time and run time.
 *
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
**	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *
 *	6-Mar-1991	DEc	Initial version
 *
 *	14-Mar-1991	DEC	Added functions to allow access to the
 *					encoding information which needs to be
 *					saved in the .USE file and in the generated
 *					code, and then recreated at run-time.
 *
 *	15-May-1991	DEC	Fixed bugs around RTS initialization of $ROOT
 *					encoding, zeroing mask words beyond those
 *					set with any 1's, and a problem with encodings
 *					that spanned multiple words.
 *
 *	 1-Dec-1992	DEC	Hid declaration block structure from
 *			        class code
 *
 *	 6-May-1994	DEC	Fix code in #ifdef DEBUG
 *
 *	18-May-1994	DEC	Change "1 << x" to "1l << x" where long needed.
 *
 *	01-Dec-1999	CPQ	Release with GPL
 */

#include <common.h>
#include <decl.h>		/* Declarations of these functions */
#include <hash.h>		/* Needed by decl_c_p.h */
#include <decl_c_p.h> 		/* For Class and Decl_Block */
#include <mol.h>
#include <limits.h>		/* For ULONG_MAX */
#include <ios.h>

struct class_encoding {
  long pattern_words;
  long mask_bit_count;
  unsigned long *mask;
  unsigned long *pattern;
};

/*
 * Private function forward declarations
 */

static void 
determine_max_siblings (long level,  Class cl_ptr);

static void
generate_inh_patterns (long level, Class cl_ptr, long sib_num, long inh_bits);

static Class_Encoding
copy_a_class_encoding (Class_Encoding ce);

static Class_Encoding
make_empty_class_encoding (long inh_code_size);

static void
append_to_class_encoding (Class_Encoding c_enc,
	long sib_num, long inh_bits, long local_bits);

#ifdef DEBUG
static void
print_a_class_encoding (Decl_Block block, Class_Encoding c_enc);
#endif

static unsigned long
make_simple_mask (long bit_count);

static void 
enlarge_sib_count_array (Decl_Block block);

static long
ilog2 (unsigned long value);

static void 
set_root_inh_encoding (Class root);


Boolean
rul__decl_is_subclass_of (Class instance_class_desc, Class parent_class_desc)
    /*
    **  Return TRUE iff the instance class is a descendent of
    **  the specified parent class.
    */
{
    unsigned long  *ch_pattern;	/* child */
    unsigned long  *pa_pattern;	/* parent */
    unsigned long  *pa_mask;
    Decl_Block instance_block = instance_class_desc->block;
    Decl_Block parent_block = parent_class_desc->block;
    long patt_siz = rul__decl_get_block_code_size(parent_block);
    long i;

    assert (instance_class_desc != NULL && parent_class_desc != NULL);
    assert ((instance_block != NULL) && (parent_block != NULL));

    /* both classes must be declared in the same declaration block */
    if (instance_block != parent_block)
	return FALSE;			

    if (instance_class_desc == parent_class_desc) {
	return TRUE;		/* a class is always a subclass of itself */
    }
    else {

#ifdef DEBUG
	{
	    static Class last_parent = NULL;
	    char buffer[RUL_C_MAX_SYMBOL_SIZE];

	    if (parent_class_desc != last_parent) {
		last_parent = parent_class_desc;
		rul__mol_use_printform (parent_class_desc->name,
					buffer, RUL_C_MAX_SYMBOL_SIZE);
		rul__ios_printf ( RUL__C_STD_ERR, "\n  Encoding for class, '%s'", buffer);
		print_a_class_encoding (parent_block,
					parent_class_desc->encoding);
		rul__ios_printf ( RUL__C_STD_ERR, "\n");
	    }
	}
#endif

	ch_pattern = instance_class_desc->encoding->pattern;
	pa_pattern = parent_class_desc->encoding->pattern;
	pa_mask = parent_class_desc->encoding->mask;

	for (i=0; i<patt_siz; i++) {
	    if (pa_pattern[i] != (ch_pattern[i] & pa_mask[i])) {
	 	return FALSE;	/* doesn't match */
	    }
	}
	return TRUE;		/* did match */
    }
}


	/*
	**	The following static variables have
	**	file scope and are used only by the
	**	inheritance encoding functions.
	*/

#define MIN_SIB_ARRAY_LEN 100 /*initial value for block->sib_array_len */

static	long	rul__SL_bits_per_long;
		/*  The total number of bits in a longword  */


void
rul__decl_encode_inheritance (Mol_Symbol decl_block_name)
	/*
	**	For each class in the class hierarchy, determine
	**	the appropriate pattern and mask for the use in
	**	the class/subclass tests.
	*/
{
  Class root;
  long i, this_lev;
  Decl_Block block;
  static Mol_Symbol	name_of_root_class = NULL; /* Static to init once */
  

  block = rul__decl_get_block(decl_block_name);
  
  /* Initialize name_of_root_class first time through. */
  if (name_of_root_class == NULL)
    name_of_root_class = rul__mol_symbol_root();
  
  root = rul__decl_get_class (block, name_of_root_class);
  
  if (!rul__decl_get_block_code_size(block)) {
    
    rul__decl_set_block_level_count(block,1);
    
    rul__decl_set_block_sib_a_len(block,MIN_SIB_ARRAY_LEN);
    rul__decl_set_block_sca(
	block,
	(long *) rul__mem_calloc (rul__decl_get_block_sib_a_len(block),
				  sizeof(long)));

    /*   $ROOT, the root class  */
    rul__decl_set_block_sca_elt(block, 0, 1);
    
    
    /*  Step 1:  find the maximum number of siblings at each level  */
    
    determine_max_siblings (1, root);
    
    
    /*  Step 2:  determine the bits per level needed, and the total  */
    
    rul__decl_set_block_total_bits(block, 0);
    rul__decl_set_block_bpl(
	block,
	(long *) rul__mem_calloc (rul__decl_get_block_level_count(block),
				  sizeof(long)));
    
    for (i=0; i<rul__decl_get_block_level_count(block); i++) {
      this_lev = ilog2 (1 + rul__decl_get_block_sca_elt(block,i));
      rul__decl_set_block_bpl_elt(block, i, this_lev);
      rul__decl_set_block_total_bits (
	block,
        rul__decl_get_block_total_bits(block) + this_lev);
    }
    rul__SL_bits_per_long = ilog2 (ULONG_MAX);
    rul__decl_set_block_code_size(
	block,
	1 + (rul__decl_get_block_total_bits(block) / rul__SL_bits_per_long));
    
#ifdef DEBUG
    rul__ios_printf ( RUL__C_STD_ERR, "\n  Number of class levels   = %ld",
		      rul__decl_get_block_level_count(block));
    for (i=0; i<rul__decl_get_block_level_count(block); i++) {
      rul__ios_printf ( RUL__C_STD_ERR, "\n    level %ld:  ", i);
      rul__ios_printf ( RUL__C_STD_ERR, "  max sibs = %ld,",
			rul__decl_get_block_sca_elt(block,i));
      rul__ios_printf ( RUL__C_STD_ERR, "  bits = %ld",
			rul__decl_get_block_bpl_elt(block,i));
    }
    rul__ios_printf ( RUL__C_STD_ERR, "\n  Number of bits required  = %ld\n", 
	    rul__decl_get_block_total_bits(block));
#endif
    
    /*  Step 3:  Assign indexes, patterns, and masks
     *
     *	The pattern of all 0's is reserved for old-style wme's
     *	The pattern of 1 followed by all zeros will be allocated
     *	by the following recursive routine to the class $ROOT.
     */
    
    generate_inh_patterns (0, root, 1, 0);
  }
}



/*****************************
**                          **
**  DETERMINE_MAX_SIBLINGS  **
**                          **
*****************************/

static void
determine_max_siblings (
	long level,
	Class class_ptr )
{
  Decl_Block block = class_ptr->block;

	/*
	**	This function walks the class tree to determine
	**	the maximum number of siblings at each level in the
	**	class hierarchy.
	*/
	if (level >= rul__decl_get_block_sib_a_len(block))
	  enlarge_sib_count_array (block);

	rul__decl_set_block_sca_elt(
		block, level,
		MAX ( rul__decl_get_block_sca_elt(block,level),
		      class_ptr->number_of_children ));

	if (class_ptr->child != NULL) {
		/*  First traverse down to children  */
		rul__decl_set_block_level_count(
			block,
			MAX (rul__decl_get_block_level_count(block),
			     level + 1));
		determine_max_siblings (level + 1, class_ptr->child);
	}
	if (class_ptr->sibling != NULL) {
		/*  Then traverse over to siblings  */
		determine_max_siblings (level, class_ptr->sibling);
	}
}



/******************************
**                           **
**  ENLARGE_SIB_COUNT_ARRAY  **
**                           **
******************************/

static void
enlarge_sib_count_array (Decl_Block block)
{
	long	i;
	long	new_len;

	new_len = rul__decl_get_block_sib_a_len(block) * 2;

	rul__decl_set_block_sca(
		block,
		rul__mem_realloc(rul__decl_get_block_sca(block), new_len));

	for (i=rul__decl_get_block_sib_a_len(block); i<new_len; i++) {
		rul__decl_set_block_sca_elt(block,i, 0);
	}
	rul__decl_set_block_sib_a_len(block, new_len);
}



/************
**         **
**  ILOG2  **
**         **
************/

static long
ilog2 (	unsigned long value )
{
	/*	Compute the number of bits required to
	**	uniquely represent VALUE different objects.
	*/
	long shift_count = 0;

	if (value == 0) return (0);
	if (value == 1) return (1);
	value -= 1;

	while (value > 0) {
		shift_count ++;
		value = value >> 1;
	}
	return (shift_count);
}



/****************************
**                         **
**  GENERATE_INH_PATTERNS  **
**                         **
****************************/

static void
generate_inh_patterns (
	long level,
	Class class_ptr,
	long sib_num,
	long inh_bits )
{
	Decl_Block block = class_ptr->block;

	/*  Start with the parent class's encoding  */
	if (class_ptr->parent  &&  class_ptr->parent->encoding) {
		class_ptr->encoding =
			copy_a_class_encoding (class_ptr->parent->encoding);
	}
	else {
		class_ptr->encoding =
		  make_empty_class_encoding (
			rul__decl_get_block_code_size(block));
	}

	/*  Then add the local extensions to the encoding  */

	append_to_class_encoding (class_ptr->encoding, sib_num, inh_bits,
				  rul__decl_get_block_bpl_elt(block, level));

#ifdef DEBUG
	rul__ios_printf ( RUL__C_STD_ERR, "\n  Encoding for class, '%s'",
		rul__mol_get_printform (class_ptr->name));
	rul__ios_printf ( RUL__C_STD_ERR, "\n      level =%2d,", level);
	rul__ios_printf ( RUL__C_STD_ERR, "  sib_num =%2d,", sib_num);
	rul__ios_printf ( RUL__C_STD_ERR, "  inh_bits =%2d,", inh_bits);
	rul__ios_printf ( RUL__C_STD_ERR, "  loc_bits =%2d,",
			  rul__decl_get_block_bpl_elt(block, level));
	print_a_class_encoding (block, class_ptr->encoding);
#endif

	/*  Then proceed recursively to other classes in the hierarchy  */

	if (class_ptr->child != NULL) {
		/*  First traverse down to children  */
		generate_inh_patterns (
			level + 1,  class_ptr->child, 1,
			inh_bits + rul__decl_get_block_bpl_elt(block, level));
	}
	if (class_ptr->sibling != NULL) {
		/*  Then traverse over to siblings  */
		generate_inh_patterns ( level, class_ptr->sibling,
					sib_num + 1, inh_bits);
	}
}



/********************************
**                             **
**  MAKE_EMPTY_CLASS_ENCODING  **
**                             **
********************************/

static Class_Encoding
make_empty_class_encoding (long code_size)
{
	Class_Encoding	c_enc;
	long	size = 	sizeof (struct class_encoding) +
			(sizeof(long) * 2 * code_size);

	c_enc = (Class_Encoding) rul__mem_calloc (1, size);

	c_enc->mask    = (unsigned long *) (c_enc + 1);
			/* Jump over class_encoding struct */
	c_enc->pattern = c_enc->mask + code_size;
				/* Jump over mask array */
	c_enc->pattern_words = code_size;
	c_enc->mask_bit_count = 0;

	return (c_enc);
}



/****************************
**                         **
**  COPY_A_CLASS_ENCODING  **
**                         **
****************************/

static Class_Encoding
copy_a_class_encoding (Class_Encoding old_c_enc)
{
	Class_Encoding	c_enc=
	  make_empty_class_encoding (old_c_enc->pattern_words);
	long  i;

	for (i=0; i < old_c_enc->pattern_words; i++) {
		c_enc->mask[i]     = old_c_enc->mask[i];
		c_enc->pattern[i]  = old_c_enc->pattern[i];
	}
	c_enc->mask_bit_count = old_c_enc->mask_bit_count;

	return (c_enc);
}




/*******************************
**                            **
**  APPEND_TO_CLASS_ENCODING  **
**                            **
*******************************/

static void
append_to_class_encoding (
	Class_Encoding	c_enc,  /*  copy of parent's encoding structure  */
	long  sib_num,		/*  bit pattern to be appended  */
	long  inh_bits,		/*  bit count from the parent's code  */
	long  local_bits )	/*  bit count for the code to be appended  */
{
	long  index, second, shift, piece_1_bits;
	unsigned long  local_pat;
	unsigned long  local_mask;

	index  = inh_bits / rul__SL_bits_per_long;
	second = (inh_bits + local_bits) / rul__SL_bits_per_long;

	if (index == second) {
		shift = (rul__SL_bits_per_long * (1 + index))
			- inh_bits - local_bits;
		/*
		**  The local pattern is simply the SIB_NUM,
		**  so all that needs to be done is to shift
		**  it into the proper position, and the bitwise-OR
		**  it with the parent's pattern.
		*/
		local_pat = sib_num << shift;
		c_enc->pattern[index] = c_enc->pattern[index] | local_pat;

		/*
		**  The local mask must first be created, 
		**  shifted into the proper position, and then
		**  bitwise-OR'd with the parent's mask.
		*/
		local_mask = make_simple_mask (local_bits);
		local_mask = local_mask << shift;
		c_enc->mask[index] = c_enc->mask[index] | local_mask;

	}
	else {
		/*
		**  When the mask crosses a longword boundary, then split
		**  it into the 2 pieces, doing the higher order first.
		*/
		shift = (inh_bits + local_bits) % rul__SL_bits_per_long;
		piece_1_bits = local_bits - shift;

		local_pat = sib_num >> shift;
		c_enc->pattern[index] = c_enc->pattern[index] | local_pat;

		c_enc->mask[index] = make_simple_mask (rul__SL_bits_per_long);

		/*
		**  Then do the low order piece
		*/
		shift = rul__SL_bits_per_long - local_bits + piece_1_bits;

		local_pat = sib_num << shift;
		c_enc->pattern[second] = local_pat;

		local_mask = make_simple_mask (local_bits - piece_1_bits);
		local_mask = local_mask << shift;
		c_enc->mask[second] = local_mask;
	}
	c_enc->mask_bit_count += local_bits;
}




/***********************
**                    **
**  MAKE_SIMPLE_MASK  **
**                    **
***********************/

static unsigned long
make_simple_mask (long bit_count)
{
	/*
	**  Make a simple bit mask composed of BIT_COUNT 1's
	*/
	unsigned long  local_mask;
	long  i;

	local_mask = 0;
	for (i=0; i<bit_count; i++) {
		local_mask = (local_mask << 1) + 1;
	}
	return (local_mask);
}


/*****************************
**                          **
**  PRINT_A_CLASS_ENCODING  **
**                          **
*****************************/

#ifdef DEBUG
static void
print_a_class_encoding ( Decl_Block block, Class_Encoding c_enc )
{
	long  i, j, tmp;
	long  bit_count;

	rul__ios_printf ( RUL__C_STD_ERR, "\n      pattern_words = %ld,",
			  c_enc->pattern_words);
	rul__ios_printf ( RUL__C_STD_ERR, "  mask_bit_count = %ld",
			  c_enc->mask_bit_count);
	rul__ios_printf ( RUL__C_STD_ERR, "\n    mask:     ");
	bit_count = 0;
	for (i=0; i < c_enc->pattern_words; i++) {
		j = 0;
		while ( j < rul__SL_bits_per_long   &&
			bit_count < rul__decl_get_block_total_bits(block) )
		{
			tmp = c_enc->mask[i] >> 
				(rul__SL_bits_per_long - j - 1);
			tmp &= 1;
			rul__ios_printf ( RUL__C_STD_ERR, "%ld", tmp);
			bit_count++;
			j++;
		}
		rul__ios_printf ( RUL__C_STD_ERR, " ");
				/* include a space between longwords */
	}
	bit_count = 0;
	rul__ios_printf ( RUL__C_STD_ERR, "\n    pattern:  ");
	for (i=0; i < c_enc->pattern_words; i++) {
		j = 0;
		while ( j < rul__SL_bits_per_long   &&
			bit_count < rul__decl_get_block_total_bits(block) )
		{
			tmp = c_enc->pattern[i] >> 
				(rul__SL_bits_per_long - j - 1);
			tmp &= 1;
			rul__ios_printf ( RUL__C_STD_ERR, "%ld", tmp);
			bit_count++;
			j++;
		}
		rul__ios_printf ( RUL__C_STD_ERR, " ");
				/* include a space between longwords */
	}
	rul__ios_printf ( RUL__C_STD_ERR, "\n");
}
#endif


long
rul__decl_get_inh_pattern_size (Mol_Symbol decl_block_name)
{
	return rul__decl_get_block_code_size(
			rul__decl_get_block(decl_block_name));
}




void
rul__decl_set_inh_pattern_size (Mol_Symbol decl_block_name, long size)
				/*  Size in longwords  */
{
	Decl_Block block = rul__decl_get_block(decl_block_name);
	static Mol_Symbol name_of_root_class;

	if (name_of_root_class == NULL)
	   name_of_root_class = rul__mol_symbol_root();

	/*
	**  The pattern size is the one parameter which
	**  must be known at run-time as well as at
	**  compile time.
	**
	**  Thus this function is called from generated code.
	*/
	rul__decl_set_block_code_size(block, size);

	/*
	**  While we are here, do some other run-time initialization.
	*/
	rul__SL_bits_per_long = ilog2 (ULONG_MAX);

	/*
	**  The initialization includes initializing the special
	**  $ROOT class
	*/
	set_root_inh_encoding (
		rul__decl_get_class (block, name_of_root_class));
}


unsigned long
rul__decl_get_inh_pattern_part (Class class_desc, long index)
{
	/*
	**  Return a piece of the inheritance encoding
	**  pattern for a given class.
	*/
	Class_Encoding c_enc;
	Decl_Block block = class_desc->block;

	if (index >= rul__decl_get_block_code_size(block)  ||  index < 0)
	    return (0);

	c_enc = class_desc->encoding;
	if (c_enc == NULL)  return (0);

	return (c_enc->pattern[index]);
}


void
rul__decl_set_inh_pattern_part (Class class_desc, long index,
				unsigned long pattern_part)
{
	Decl_Block block = class_desc->block;

	/*
	**  At initialization time, allows the generated code to
	**  set a piece of the pattern for a given class to what
	**  it was at compile-time.
	*/
	if (index >= rul__decl_get_block_code_size(block)  ||  index < 0)
	    return;

	if (class_desc->encoding == NULL)
	  class_desc->encoding =
	      make_empty_class_encoding(rul__decl_get_block_code_size(block));

	class_desc->encoding->pattern[index] = pattern_part;
}


long
rul__decl_get_inh_mask_length (Class class_desc)
{
	/*  Return the inheritance mask size for a given class.  */
	Class_Encoding c_enc;

	c_enc = class_desc->encoding;
	if (c_enc == NULL)  return (1);

	return (c_enc->mask_bit_count);
}


unsigned long
rul__decl_get_inh_mask_part (Class class_desc, long index)
{
	/*
	**  Return a piece of the inheritance encoding
	**  mask for a given class.
	*/
	Decl_Block block = class_desc->block;
	Class_Encoding c_enc;

	if (index >= rul__decl_get_block_code_size(block)  ||  index < 0)
	    return (0);

	c_enc = class_desc->encoding;
	if (c_enc == NULL)  return (0);

	return (c_enc->mask[index]);
}



void
rul__decl_set_inh_mask_length (Class class_desc, long mask_length)
{
	/*
	**  At initialization time, allows the generated code to
	**  set the inheritance mask length for a given class to
	**  what it was at compile-time.
	*/
  	Decl_Block block = class_desc->block;
	Class_Encoding c_enc;
	long  i, bits_left, temp, top_bit, bits;

	if (mask_length > (rul__decl_get_block_code_size(block) *
				rul__SL_bits_per_long))
	    return;

	c_enc = class_desc->encoding;
	if (c_enc == NULL) {
	  c_enc = make_empty_class_encoding(
			rul__decl_get_block_code_size(block));
	  class_desc->encoding = c_enc;
	}

	rul__decl_set_block_total_bits(
		block,
		MAX (rul__decl_get_block_total_bits(block), mask_length));
	c_enc->mask_bit_count = mask_length;

	bits_left = mask_length;
	for (i=0; i < c_enc->pattern_words; i++) {
	    if (bits_left >=  rul__SL_bits_per_long) {

		/*  Full long word of mask  */

                c_enc->mask[i] = make_simple_mask (rul__SL_bits_per_long);

	    }
	    else if (bits_left <= 0) {

		/*  Full longword of zeros  */

		c_enc->mask[i] = 0;

	    }
	    else {

		/*  Partial long word of mask  */

		bits = bits_left;
		top_bit = 1l << (rul__SL_bits_per_long - 1);
		temp = 0;

		while (bits > 0) {
			temp = temp >> 1;
			temp |= top_bit;
			bits--;
		}
                c_enc->mask[i] = temp;
            }

	    bits_left -= rul__SL_bits_per_long;
	}
}



/****************************
**                         **
**  SET_ROOT_INH_ENCODING  **
**                         **
*****************************/

static void 
set_root_inh_encoding (Class root)
{
	Decl_Block block = root->block;
	long i;

	/*
	**  At initialization time, set the known pattern
	**  and mask for the special $ROOT class.
	*/
	if (root->encoding == NULL) {
	    root->encoding = make_empty_class_encoding(
				rul__decl_get_block_code_size(block));
	}

	rul__decl_set_inh_mask_length (root, 1);

	for (i=0; i < root->encoding->pattern_words; i++) {
		root->encoding->pattern[i] = 0;
	}
	if (root->encoding->pattern_words > 0)
	    root->encoding->pattern[0] = 1l << (rul__SL_bits_per_long - 1);
}
