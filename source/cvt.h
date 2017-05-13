/****************************************************************************
**                                                                         **
**                               C V T . H                                 **
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
 *	RULEWORKS Run Time System
 *
 * ABSTRACT:
 *	All External to Tin conversion subsystem routines.
 *
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *
 *	11-Mar-1993	DEC	Initial version
 *	28-Oct-1993	DEC	additions
 *	01-Dec-1999	CPQ	Release with GPL
 */


#define CVT_STRTRUNC \
  "STRTRUNC W Length of output buffer too small, atom %s string truncated"

/* INCLUDE-IN-GEND.H  *********************************************** */
long		 rul__cvt_s_in_s_lo (Mol_Int_Atom tin_atom);
Mol_Int_Atom	 rul__cvt_s_lo_s_in (long ext_obj);
short		 rul__cvt_s_in_s_sh (Mol_Int_Atom tin_atom);
Mol_Int_Atom	 rul__cvt_s_sh_s_in (short ext_obj);
char		 rul__cvt_s_in_s_by (Mol_Int_Atom tin_atom);
Mol_Int_Atom	 rul__cvt_s_by_s_in (char ext_obj);
unsigned long	 rul__cvt_s_in_s_ul (Mol_Int_Atom tin_atom);
Mol_Int_Atom	 rul__cvt_s_ul_s_in (unsigned long ext_obj);
unsigned short	 rul__cvt_s_in_s_us (Mol_Int_Atom tin_atom);
Mol_Int_Atom	 rul__cvt_s_us_s_in (unsigned short ext_obj);
unsigned char	 rul__cvt_s_in_s_ub (Mol_Int_Atom tin_atom);
Mol_Int_Atom	 rul__cvt_s_ub_s_in (unsigned char ext_obj);
float		 rul__cvt_s_in_s_sf (Mol_Int_Atom tin_atom);
Mol_Int_Atom	 rul__cvt_s_sf_s_in (float ext_obj);
double		 rul__cvt_s_in_s_df (Mol_Int_Atom tin_atom);
Mol_Int_Atom	 rul__cvt_s_df_s_in (double ext_obj);


long		 rul__cvt_s_db_s_lo (Mol_Dbl_Atom tin_atom);
Mol_Dbl_Atom	 rul__cvt_s_lo_s_db (long ext_obj);
short		 rul__cvt_s_db_s_sh (Mol_Dbl_Atom tin_atom);
Mol_Dbl_Atom	 rul__cvt_s_sh_s_db (short ext_obj);
char		 rul__cvt_s_db_s_by (Mol_Dbl_Atom tin_atom);
Mol_Dbl_Atom	 rul__cvt_s_by_s_db (char ext_obj);
unsigned long	 rul__cvt_s_db_s_ul (Mol_Dbl_Atom tin_atom);
Mol_Dbl_Atom	 rul__cvt_s_ul_s_db (unsigned long ext_obj);
unsigned short	 rul__cvt_s_db_s_us (Mol_Dbl_Atom tin_atom);
Mol_Dbl_Atom	 rul__cvt_s_us_s_db (unsigned short ext_obj);
unsigned char	 rul__cvt_s_db_s_ub (Mol_Dbl_Atom tin_atom);
Mol_Dbl_Atom	 rul__cvt_s_ub_s_db (unsigned char ext_obj);
float		 rul__cvt_s_db_s_sf (Mol_Dbl_Atom tin_atom);
Mol_Dbl_Atom	 rul__cvt_s_sf_s_db (float ext_obj);
double		 rul__cvt_s_db_s_df (Mol_Dbl_Atom tin_atom);
Mol_Dbl_Atom	 rul__cvt_s_df_s_db (double ext_obj);


void	        *rul__cvt_s_op_s_vd (Mol_Opaque tin_atom);
Mol_Opaque	 rul__cvt_s_vd_s_op (void *ext_obj );


char	        *rul__cvt_s_at_s_az (Mol_Atom tin_atom, char *optr);
Mol_Symbol	 rul__cvt_s_az_s_sy (char *ext_pointer);
Mol_Atom	 rul__cvt_s_az_s_at (char *ext_pointer);
String	        *rul__cvt_s_at_s_ad (Mol_Atom tin_atom, String *optr);
Mol_Symbol	 rul__cvt_s_ad_s_sy (String *descr);
Mol_Atom	 rul__cvt_s_ad_s_at (String *descr);


long		 rul__cvt_s_at_s_lo (Mol_Atom tin_atom);
short		 rul__cvt_s_at_s_sh (Mol_Atom tin_atom);
char		 rul__cvt_s_at_s_by (Mol_Atom tin_atom);
unsigned long	 rul__cvt_s_at_s_ul (Mol_Atom tin_atom);
unsigned short	 rul__cvt_s_at_s_us (Mol_Atom tin_atom);
unsigned char	 rul__cvt_s_at_s_ub (Mol_Atom tin_atom);
float		 rul__cvt_s_at_s_sf (Mol_Atom tin_atom);
double		 rul__cvt_s_at_s_df (Mol_Atom tin_atom);
void	        *rul__cvt_s_at_s_vd (Mol_Atom tin_atom);


Molecule	 rul__cvt_s_at_s_ta (Molecule tin_atom);
Molecule         rul__cvt_s_ta_s_at (Molecule tin_atom);

Mol_Atom         rul__cvt_s_ta_s_na (Mol_Atom tin_atom);
Mol_Atom         rul__cvt_s_ta_s_in (Mol_Atom tin_atom);
Mol_Atom         rul__cvt_s_ta_s_db (Mol_Atom tin_atom);
Mol_Atom         rul__cvt_s_ta_s_sy (Mol_Atom tin_atom);
Mol_Atom         rul__cvt_s_at_s_sy (Mol_Atom tin_atom);
Mol_Atom         rul__cvt_s_ta_s_op (Mol_Atom tin_atom);
Mol_Atom         rul__cvt_s_ta_s_id (Mol_Atom tin_atom);
    

Mol_Compound	 rul__cvt_a_lo_c_in (long *array, long len);
Mol_Compound	 rul__cvt_a_sh_c_in (short *array, long len);
Mol_Compound	 rul__cvt_a_by_c_in (char *array, long len);
Mol_Compound	 rul__cvt_a_ul_c_in (unsigned long *array, long len);
Mol_Compound	 rul__cvt_a_us_c_in (unsigned short *array, long len);
Mol_Compound	 rul__cvt_a_ub_c_in (unsigned char *array, long len);
Mol_Compound	 rul__cvt_a_sf_c_db (float *array, long len);
Mol_Compound	 rul__cvt_a_df_c_db (double *array, long len);
Mol_Compound	 rul__cvt_a_az_c_sy (char **array, long len);
Mol_Compound	 rul__cvt_a_az_c_at (char **array, long len);
Mol_Compound	 rul__cvt_a_ad_c_sy (String **array, long len);
Mol_Compound	 rul__cvt_a_ad_c_at (String **array, long len);
Mol_Compound	 rul__cvt_a_ta_c_at (Mol_Atom *array, long len);
Mol_Compound	 rul__cvt_a_vd_c_op (void **array, long len);


long	        *rul__cvt_c_at_a_lo (Mol_Compound tptr, long len, long *optr);
short	        *rul__cvt_c_at_a_sh (Mol_Compound tptr, long len, short *optr);
char	        *rul__cvt_c_at_a_by (Mol_Compound tptr, long len, char *optr);
unsigned long   *rul__cvt_c_at_a_ul (Mol_Compound tptr,
				     long len, unsigned long *optr);
unsigned short  *rul__cvt_c_at_a_us (Mol_Compound tptr,
				     long len, unsigned short *optr);
unsigned char   *rul__cvt_c_at_a_ub (Mol_Compound tptr,
				     long len, unsigned char *optr);
float	     *rul__cvt_c_at_a_sf (Mol_Compound tptr, long len, float *optr);
double	     *rul__cvt_c_at_a_df (Mol_Compound tptr, long len, double *optr);
char	    **rul__cvt_c_at_a_az (Mol_Compound tptr, long len, char **optr);
String	    **rul__cvt_c_at_a_ad (Mol_Compound tptr, long len, String **optr);
Mol_Atom     *rul__cvt_c_at_a_ta (Mol_Compound tptr, long len, Mol_Atom *optr);
void	    **rul__cvt_c_at_a_vd (Mol_Compound tptr, long len, void **optr);


void		 rul__cvt_free (void *ext_pointer);

/* END-INCLUDE-IN-GEND.H  *********************************************** */


