/* GCC MELT GENERATED FILE warmelt-hooks+02.c - DO NOT EDIT */
/* secondary MELT generated C file of rank #2 */
#include "melt-run.h"


/* used hash from melt-run.h when compiling this file: */
MELT_EXTERN const char meltrun_used_md5_melt_f2[] =
  MELT_RUN_HASHMD5 /* from melt-run.h */ ;


/**** warmelt-hooks+02.c declarations ****/
/**** MELT GENERATED DECLARATIONS for warmelt-hooks ** DO NOT EDIT ; see gcc-melt.org ****/

/****++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
***
    Copyright 2012 - 2013 Free Software Foundation, Inc.
    Contributed by Basile Starynkevitch <basile@starynkevitch.net>

    This file is part of GCC.

    GCC is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3, or (at your option)
    any later version.

    GCC is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with GCC; see the file COPYING3.  If not see
    <http://www.gnu.org/licenses/>.
***
----------------------------------------------------------------****/


/** ordinary MELT module meltbuild-sources/warmelt-hooks**/
#define MELT_HAS_INITIAL_ENVIRONMENT 1	/*usualmodule */


/**** no MELT module variables ****/

/*** 3 MELT called hook declarations ***/

/*declare MELT called hook #0 HOOK_FRESH_ENVIRONMENT_REFERENCE_MAKER **/
MELT_EXTERN melt_ptr_t
melthook_HOOK_FRESH_ENVIRONMENT_REFERENCE_MAKER (melt_ptr_t melthookdata,
						 melt_ptr_t meltinp0_PREVENV,
						 const char
						 *meltinp1_MODULNAME);

/*declare MELT called hook #1 HOOK_SYMBOL_IMPORTER **/
MELT_EXTERN melt_ptr_t melthook_HOOK_SYMBOL_IMPORTER (melt_ptr_t melthookdata,
						      const char
						      *meltinp0_SYMNAMESTR,
						      const char
						      *meltinp1_MODULENAMESTR,
						      melt_ptr_t
						      meltinp2_PARENV);

/*declare MELT called hook #2 HOOK_VALUE_EXPORTER **/
MELT_EXTERN void melthook_HOOK_VALUE_EXPORTER (melt_ptr_t melthookdata,
					       melt_ptr_t meltinp0_SYM,
					       melt_ptr_t meltinp1_VAL,
					       melt_ptr_t meltinp2_CONTENV);

/*** end of 3 MELT called hook declarations ***/

/*** 26 extra MELT c-headers ***/


/** MELT extra c-header 1 : **/


static void
melt_override_gate_callback (void *gccdata, void *userdata ATTRIBUTE_UNUSED)
{
  bool *pgatestatus = (bool *) gccdata;
  bool gatsta = false;
  long oldgatstalng = 0, newgatstalng = 0;
  gcc_assert (pgatestatus != NULL);
  gatsta = *pgatestatus;
  oldgatstalng = newgatstalng = (long) gatsta;
  melthookproc_HOOK_OVERRIDE_GATE (oldgatstalng, &newgatstalng);
  gatsta = (newgatstalng != 0L);
  *pgatestatus = gatsta;
}				/* end of  melt_override_gate_callback */

/*** end of 26 extra MELT c-headers ***/


/** MELT extra c-header 2 : **/

/* flag for PLUGIN_OVERRIDE_GATE */
MELT_EXTERN bool MELT_MODULE_VISIBILITY melthk_override_gate_registered_flag;

/*** end of 26 extra MELT c-headers ***/


/** MELT extra c-header 3 : **/

/* flag for PLUGIN_START_UNIT */
MELT_EXTERN bool MELT_MODULE_VISIBILITY melthk_start_unit_registered_flag;

/*** end of 26 extra MELT c-headers ***/


/** MELT extra c-header 4 : **/


void MELT_MODULE_VISIBILITY
melt_startunithook_callback (void *gcc_data ATTRIBUTE_UNUSED,
			     void *user_data ATTRIBUTE_UNUSED);

/*** end of 26 extra MELT c-headers ***/


/** MELT extra c-header 5 : **/

/* flag for PLUGIN_FINISH_UNIT */
MELT_EXTERN bool MELT_MODULE_VISIBILITY melthk_finish_unit_registered_flag;

/*** end of 26 extra MELT c-headers ***/


/** MELT extra c-header 6 : **/


void MELT_MODULE_VISIBILITY
melt_finishunithook_callback (void *gcc_data ATTRIBUTE_UNUSED,
			      void *user_data ATTRIBUTE_UNUSED);

/*** end of 26 extra MELT c-headers ***/


/** MELT extra c-header 7 : **/

/* flag for PLUGIN_ALL_PASSES_START declare */
MELT_EXTERN bool MELT_MODULE_VISIBILITY
  melthk_all_passes_start_registered_flag;

/*** end of 26 extra MELT c-headers ***/


/** MELT extra c-header 8 : **/


void MELT_MODULE_VISIBILITY
melt_all_passes_start_hook_callback (void *gcc_data ATTRIBUTE_UNUSED,
				     void *user_data ATTRIBUTE_UNUSED);

/*** end of 26 extra MELT c-headers ***/


/** MELT extra c-header 9 : **/

/* flag for PLUGIN_ALL_PASSES_END declare */
MELT_EXTERN bool MELT_MODULE_VISIBILITY melthk_all_passes_end_registered_flag;

/*** end of 26 extra MELT c-headers ***/


/** MELT extra c-header 10 : **/


void MELT_MODULE_VISIBILITY
melt_all_passes_end_hook_callback (void *gcc_data ATTRIBUTE_UNUSED,
				   void *user_data ATTRIBUTE_UNUSED);

/*** end of 26 extra MELT c-headers ***/


/** MELT extra c-header 11 : **/

/* flag for PLUGIN_ALL_IPA_PASSES_START declare */
MELT_EXTERN bool MELT_MODULE_VISIBILITY
  melthk_all_ipa_passes_start_registered_flag;

/*** end of 26 extra MELT c-headers ***/


/** MELT extra c-header 12 : **/


void MELT_MODULE_VISIBILITY
melt_all_ipa_passes_start_hook_callback (void *gcc_data ATTRIBUTE_UNUSED,
					 void *user_data ATTRIBUTE_UNUSED);

/*** end of 26 extra MELT c-headers ***/


/** MELT extra c-header 13 : **/

/* flag for PLUGIN_ALL_IPA_PASSES_END declare */
MELT_EXTERN bool MELT_MODULE_VISIBILITY
  melthk_all_ipa_passes_end_registered_flag;

/*** end of 26 extra MELT c-headers ***/


/** MELT extra c-header 14 : **/


void MELT_MODULE_VISIBILITY
melt_all_ipa_passes_end_hook_callback (void *gcc_data ATTRIBUTE_UNUSED,
				       void *user_data ATTRIBUTE_UNUSED);

/*** end of 26 extra MELT c-headers ***/


/** MELT extra c-header 15 : **/

/* flag for PLUGIN_EARLY_GIMPLE_PASSES_START declare */
MELT_EXTERN bool MELT_MODULE_VISIBILITY
  melthk_early_gimple_passes_start_registered_flag;

/*** end of 26 extra MELT c-headers ***/


/** MELT extra c-header 16 : **/


void MELT_MODULE_VISIBILITY
melt_early_gimple_passes_start_hook_callback (void *gcc_data ATTRIBUTE_UNUSED,
					      void *user_data
					      ATTRIBUTE_UNUSED);

/*** end of 26 extra MELT c-headers ***/


/** MELT extra c-header 17 : **/

/* flag for PLUGIN_EARLY_GIMPLE_PASSES_END declare */
MELT_EXTERN bool MELT_MODULE_VISIBILITY
  melthk_early_gimple_passes_end_registered_flag;

/*** end of 26 extra MELT c-headers ***/


/** MELT extra c-header 18 : **/


void MELT_MODULE_VISIBILITY
melt_early_gimple_passes_end_hook_callback (void *gcc_data ATTRIBUTE_UNUSED,
					    void *user_data ATTRIBUTE_UNUSED);

/*** end of 26 extra MELT c-headers ***/


/** MELT extra c-header 19 : **/

/* flag for PLUGIN_PRE_GENERICIZE declare */
MELT_EXTERN bool MELT_MODULE_VISIBILITY melthk_pre_genericize_registered_flag;

/*** end of 26 extra MELT c-headers ***/


/** MELT extra c-header 20 : **/


void MELT_MODULE_VISIBILITY
melt_pre_genericize_hook_callback (void *gcc_data ATTRIBUTE_UNUSED,
				   void *user_data ATTRIBUTE_UNUSED);

/*** end of 26 extra MELT c-headers ***/


/** MELT extra c-header 21 : **/

/* flag for PLUGIN_FINISH_TYPE declare */
MELT_EXTERN bool MELT_MODULE_VISIBILITY melthk_finish_type_registered_flag;

/*** end of 26 extra MELT c-headers ***/


/** MELT extra c-header 22 : **/


void MELT_MODULE_VISIBILITY
melt_finish_type_hook_callback (void *gcc_data ATTRIBUTE_UNUSED,
				void *user_data ATTRIBUTE_UNUSED);

/*** end of 26 extra MELT c-headers ***/


/** MELT extra c-header 23 : **/

/* flag for PLUGIN_FINISH_DECL declare */
MELT_EXTERN bool MELT_MODULE_VISIBILITY melthk_finish_decl_registered_flag;

/*** end of 26 extra MELT c-headers ***/


/** MELT extra c-header 24 : **/


void MELT_MODULE_VISIBILITY
melt_finish_decl_hook_callback (void *gcc_data ATTRIBUTE_UNUSED,
				void *user_data ATTRIBUTE_UNUSED);

/*** end of 26 extra MELT c-headers ***/


/** MELT extra c-header 25 : **/

/* flag for PLUGIN_PASS_EXECUTION declare */
MELT_EXTERN bool MELT_MODULE_VISIBILITY melthk_pass_execution_registered_flag;

/*** end of 26 extra MELT c-headers ***/


/** MELT extra c-header 26 : **/


void MELT_MODULE_VISIBILITY
melt_pass_execution_hook_callback (void *gcc_data ATTRIBUTE_UNUSED,
				   void *user_data ATTRIBUTE_UNUSED);

/*** end of 26 extra MELT c-headers ***/

/** declaration of hook melthook_HOOK_LOW_DEBUG_VALUE_AT */
MELT_EXTERN
  void melthook_HOOK_LOW_DEBUG_VALUE_AT (melt_ptr_t melthookdata,
					 melt_ptr_t meltinp0_VAL,
					 const char *meltinp1_FILENAME,
					 long meltinp2_LINENO,
					 const char *meltinp3_MSG,
					 long meltinp4_COUNT);


/* declare hook frame marking routine for melthook_HOOK_LOW_DEBUG_VALUE_AT hook */
void MELT_MODULE_VISIBILITY melthookmark_HOOK_LOW_DEBUG_VALUE_AT_rout (struct
								       melt_callframe_st
								       *,
								       int);




/** declaration of hook melthook_HOOK_OVERRIDE_GATE */
MELT_EXTERN
  void melthook_HOOK_OVERRIDE_GATE (melt_ptr_t melthookdata,
				    long meltinp0_BEFOREGATE,
				    long *meltoutp0_AFTERGATE);


/* declare hook frame marking routine for melthook_HOOK_OVERRIDE_GATE hook */
void MELT_MODULE_VISIBILITY melthookmark_HOOK_OVERRIDE_GATE_rout (struct
								  melt_callframe_st
								  *, int);







melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_3_WARMELTmiHOOKS_ENABLE_OVERRIDE_GATE (meltclosure_ptr_t meltclosp_,
						melt_ptr_t meltfirstargp_,
						const melt_argdescr_cell_t
						meltxargdescr_[],
						union meltparam_un
						*meltxargtab_,
						const melt_argdescr_cell_t
						meltxresdescr_[],
						union meltparam_un
						*meltxrestab_);




melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_4_WARMELTmiHOOKS_MAYBE_DISABLE_OVERRIDE_GATE (meltclosure_ptr_t
						       meltclosp_,
						       melt_ptr_t
						       meltfirstargp_,
						       const
						       melt_argdescr_cell_t
						       meltxargdescr_[],
						       union meltparam_un
						       *meltxargtab_,
						       const
						       melt_argdescr_cell_t
						       meltxresdescr_[],
						       union meltparam_un
						       *meltxrestab_);




melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_5_WARMELTmiHOOKS_REGISTER_OVERRIDE_GATE_FIRST (meltclosure_ptr_t
							meltclosp_,
							melt_ptr_t
							meltfirstargp_,
							const
							melt_argdescr_cell_t
							meltxargdescr_[],
							union meltparam_un
							*meltxargtab_,
							const
							melt_argdescr_cell_t
							meltxresdescr_[],
							union meltparam_un
							*meltxrestab_);




melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_6_WARMELTmiHOOKS_REGISTER_OVERRIDE_GATE_LAST (meltclosure_ptr_t
						       meltclosp_,
						       melt_ptr_t
						       meltfirstargp_,
						       const
						       melt_argdescr_cell_t
						       meltxargdescr_[],
						       union meltparam_un
						       *meltxargtab_,
						       const
						       melt_argdescr_cell_t
						       meltxresdescr_[],
						       union meltparam_un
						       *meltxrestab_);




melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_7_WARMELTmiHOOKS_UNREGISTER_OVERRIDE_GATE_FIRST (meltclosure_ptr_t
							  meltclosp_,
							  melt_ptr_t
							  meltfirstargp_,
							  const
							  melt_argdescr_cell_t
							  meltxargdescr_[],
							  union meltparam_un
							  *meltxargtab_,
							  const
							  melt_argdescr_cell_t
							  meltxresdescr_[],
							  union meltparam_un
							  *meltxrestab_);




melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_8_WARMELTmiHOOKS_UNREGISTER_OVERRIDE_GATE_LAST (meltclosure_ptr_t
							 meltclosp_,
							 melt_ptr_t
							 meltfirstargp_,
							 const
							 melt_argdescr_cell_t
							 meltxargdescr_[],
							 union meltparam_un
							 *meltxargtab_,
							 const
							 melt_argdescr_cell_t
							 meltxresdescr_[],
							 union meltparam_un
							 *meltxrestab_);




melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_9_WARMELTmiHOOKS_LAMBDA_cl1 (meltclosure_ptr_t meltclosp_,
				      melt_ptr_t meltfirstargp_,
				      const melt_argdescr_cell_t
				      meltxargdescr_[],
				      union meltparam_un *meltxargtab_,
				      const melt_argdescr_cell_t
				      meltxresdescr_[],
				      union meltparam_un *meltxrestab_);




melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_10_WARMELTmiHOOKS_LAMBDA_cl2 (meltclosure_ptr_t meltclosp_,
				       melt_ptr_t meltfirstargp_,
				       const melt_argdescr_cell_t
				       meltxargdescr_[],
				       union meltparam_un *meltxargtab_,
				       const melt_argdescr_cell_t
				       meltxresdescr_[],
				       union meltparam_un *meltxrestab_);




melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_11_WARMELTmiHOOKS_LAMBDA_cl3 (meltclosure_ptr_t meltclosp_,
				       melt_ptr_t meltfirstargp_,
				       const melt_argdescr_cell_t
				       meltxargdescr_[],
				       union meltparam_un *meltxargtab_,
				       const melt_argdescr_cell_t
				       meltxresdescr_[],
				       union meltparam_un *meltxrestab_);

/** declaration of hook melthook_HOOK_START_UNIT */
MELT_EXTERN void melthook_HOOK_START_UNIT (melt_ptr_t melthookdata);


/* declare hook frame marking routine for melthook_HOOK_START_UNIT hook */
void MELT_MODULE_VISIBILITY melthookmark_HOOK_START_UNIT_rout (struct
							       melt_callframe_st
							       *, int);







melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_13_WARMELTmiHOOKS_ENABLE_START_UNIT (meltclosure_ptr_t meltclosp_,
					      melt_ptr_t meltfirstargp_,
					      const melt_argdescr_cell_t
					      meltxargdescr_[],
					      union meltparam_un
					      *meltxargtab_,
					      const melt_argdescr_cell_t
					      meltxresdescr_[],
					      union meltparam_un
					      *meltxrestab_);




melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_14_WARMELTmiHOOKS_AT_START_UNIT_FIRST (meltclosure_ptr_t meltclosp_,
						melt_ptr_t meltfirstargp_,
						const melt_argdescr_cell_t
						meltxargdescr_[],
						union meltparam_un
						*meltxargtab_,
						const melt_argdescr_cell_t
						meltxresdescr_[],
						union meltparam_un
						*meltxrestab_);




melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_15_WARMELTmiHOOKS_AT_START_UNIT_LAST (meltclosure_ptr_t meltclosp_,
					       melt_ptr_t meltfirstargp_,
					       const melt_argdescr_cell_t
					       meltxargdescr_[],
					       union meltparam_un
					       *meltxargtab_,
					       const melt_argdescr_cell_t
					       meltxresdescr_[],
					       union meltparam_un
					       *meltxrestab_);




melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_16_WARMELTmiHOOKS_LAMBDA_cl4 (meltclosure_ptr_t meltclosp_,
				       melt_ptr_t meltfirstargp_,
				       const melt_argdescr_cell_t
				       meltxargdescr_[],
				       union meltparam_un *meltxargtab_,
				       const melt_argdescr_cell_t
				       meltxresdescr_[],
				       union meltparam_un *meltxrestab_);




melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_17_WARMELTmiHOOKS_LAMBDA_cl5 (meltclosure_ptr_t meltclosp_,
				       melt_ptr_t meltfirstargp_,
				       const melt_argdescr_cell_t
				       meltxargdescr_[],
				       union meltparam_un *meltxargtab_,
				       const melt_argdescr_cell_t
				       meltxresdescr_[],
				       union meltparam_un *meltxrestab_);




melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_18_WARMELTmiHOOKS_LAMBDA_cl6 (meltclosure_ptr_t meltclosp_,
				       melt_ptr_t meltfirstargp_,
				       const melt_argdescr_cell_t
				       meltxargdescr_[],
				       union meltparam_un *meltxargtab_,
				       const melt_argdescr_cell_t
				       meltxresdescr_[],
				       union meltparam_un *meltxrestab_);

/** declaration of hook melthook_HOOK_FINISH_UNIT */
MELT_EXTERN void melthook_HOOK_FINISH_UNIT (melt_ptr_t melthookdata);


/* declare hook frame marking routine for melthook_HOOK_FINISH_UNIT hook */
void MELT_MODULE_VISIBILITY melthookmark_HOOK_FINISH_UNIT_rout (struct
								melt_callframe_st
								*, int);







melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_20_WARMELTmiHOOKS_ENABLE_FINISH_UNIT (meltclosure_ptr_t meltclosp_,
					       melt_ptr_t meltfirstargp_,
					       const melt_argdescr_cell_t
					       meltxargdescr_[],
					       union meltparam_un
					       *meltxargtab_,
					       const melt_argdescr_cell_t
					       meltxresdescr_[],
					       union meltparam_un
					       *meltxrestab_);




melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_21_WARMELTmiHOOKS_AT_FINISH_UNIT_FIRST (meltclosure_ptr_t meltclosp_,
						 melt_ptr_t meltfirstargp_,
						 const melt_argdescr_cell_t
						 meltxargdescr_[],
						 union meltparam_un
						 *meltxargtab_,
						 const melt_argdescr_cell_t
						 meltxresdescr_[],
						 union meltparam_un
						 *meltxrestab_);




melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_22_WARMELTmiHOOKS_AT_FINISH_UNIT_LAST (meltclosure_ptr_t meltclosp_,
						melt_ptr_t meltfirstargp_,
						const melt_argdescr_cell_t
						meltxargdescr_[],
						union meltparam_un
						*meltxargtab_,
						const melt_argdescr_cell_t
						meltxresdescr_[],
						union meltparam_un
						*meltxrestab_);

/** declaration of hook melthook_HOOK_ALL_PASSES_START */
MELT_EXTERN void melthook_HOOK_ALL_PASSES_START (melt_ptr_t melthookdata);


/* declare hook frame marking routine for melthook_HOOK_ALL_PASSES_START hook */
void MELT_MODULE_VISIBILITY melthookmark_HOOK_ALL_PASSES_START_rout (struct
								     melt_callframe_st
								     *, int);







melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_24_WARMELTmiHOOKS_ENABLE_ALL_PASSES_START (meltclosure_ptr_t
						    meltclosp_,
						    melt_ptr_t meltfirstargp_,
						    const melt_argdescr_cell_t
						    meltxargdescr_[],
						    union meltparam_un
						    *meltxargtab_,
						    const melt_argdescr_cell_t
						    meltxresdescr_[],
						    union meltparam_un
						    *meltxrestab_);




melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_25_WARMELTmiHOOKS_REGISTER_ALL_PASSES_START_FIRST (meltclosure_ptr_t
							    meltclosp_,
							    melt_ptr_t
							    meltfirstargp_,
							    const
							    melt_argdescr_cell_t
							    meltxargdescr_[],
							    union meltparam_un
							    *meltxargtab_,
							    const
							    melt_argdescr_cell_t
							    meltxresdescr_[],
							    union meltparam_un
							    *meltxrestab_);




melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_26_WARMELTmiHOOKS_REGISTER_ALL_PASSES_START_LAST (meltclosure_ptr_t
							   meltclosp_,
							   melt_ptr_t
							   meltfirstargp_,
							   const
							   melt_argdescr_cell_t
							   meltxargdescr_[],
							   union meltparam_un
							   *meltxargtab_,
							   const
							   melt_argdescr_cell_t
							   meltxresdescr_[],
							   union meltparam_un
							   *meltxrestab_);

/** declaration of hook melthook_HOOK_ALL_PASSES_END */
MELT_EXTERN void melthook_HOOK_ALL_PASSES_END (melt_ptr_t melthookdata);


/* declare hook frame marking routine for melthook_HOOK_ALL_PASSES_END hook */
void MELT_MODULE_VISIBILITY melthookmark_HOOK_ALL_PASSES_END_rout (struct
								   melt_callframe_st
								   *, int);







melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_28_WARMELTmiHOOKS_ENABLE_ALL_PASSES_END (meltclosure_ptr_t
						  meltclosp_,
						  melt_ptr_t meltfirstargp_,
						  const melt_argdescr_cell_t
						  meltxargdescr_[],
						  union meltparam_un
						  *meltxargtab_,
						  const melt_argdescr_cell_t
						  meltxresdescr_[],
						  union meltparam_un
						  *meltxrestab_);




melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_29_WARMELTmiHOOKS_REGISTER_ALL_PASSES_END_FIRST (meltclosure_ptr_t
							  meltclosp_,
							  melt_ptr_t
							  meltfirstargp_,
							  const
							  melt_argdescr_cell_t
							  meltxargdescr_[],
							  union meltparam_un
							  *meltxargtab_,
							  const
							  melt_argdescr_cell_t
							  meltxresdescr_[],
							  union meltparam_un
							  *meltxrestab_);




melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_30_WARMELTmiHOOKS_REGISTER_ALL_PASSES_END_LAST (meltclosure_ptr_t
							 meltclosp_,
							 melt_ptr_t
							 meltfirstargp_,
							 const
							 melt_argdescr_cell_t
							 meltxargdescr_[],
							 union meltparam_un
							 *meltxargtab_,
							 const
							 melt_argdescr_cell_t
							 meltxresdescr_[],
							 union meltparam_un
							 *meltxrestab_);

/** declaration of hook melthook_HOOK_ALL_IPA_PASSES_START */
MELT_EXTERN void melthook_HOOK_ALL_IPA_PASSES_START (melt_ptr_t melthookdata);


/* declare hook frame marking routine for melthook_HOOK_ALL_IPA_PASSES_START hook */
void MELT_MODULE_VISIBILITY
melthookmark_HOOK_ALL_IPA_PASSES_START_rout (struct melt_callframe_st *, int);







melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_32_WARMELTmiHOOKS_ENABLE_ALL_IPA_PASSES_START (meltclosure_ptr_t
							meltclosp_,
							melt_ptr_t
							meltfirstargp_,
							const
							melt_argdescr_cell_t
							meltxargdescr_[],
							union meltparam_un
							*meltxargtab_,
							const
							melt_argdescr_cell_t
							meltxresdescr_[],
							union meltparam_un
							*meltxrestab_);




melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_33_WARMELTmiHOOKS_REGISTER_ALL_IPA_PASSES_START_FIRST
(meltclosure_ptr_t meltclosp_, melt_ptr_t meltfirstargp_, const melt_argdescr_cell_t meltxargdescr_[],
union meltparam_un *meltxargtab_, const melt_argdescr_cell_t meltxresdescr_[],
union meltparam_un *meltxrestab_);




melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_34_WARMELTmiHOOKS_REGISTER_ALL_IPA_PASSES_START_LAST
(meltclosure_ptr_t meltclosp_, melt_ptr_t meltfirstargp_, const melt_argdescr_cell_t meltxargdescr_[],
union meltparam_un *meltxargtab_, const melt_argdescr_cell_t meltxresdescr_[],
union meltparam_un *meltxrestab_);

/** declaration of hook melthook_HOOK_ALL_IPA_PASSES_END */
MELT_EXTERN void melthook_HOOK_ALL_IPA_PASSES_END (melt_ptr_t melthookdata);


/* declare hook frame marking routine for melthook_HOOK_ALL_IPA_PASSES_END hook */
void MELT_MODULE_VISIBILITY melthookmark_HOOK_ALL_IPA_PASSES_END_rout (struct
								       melt_callframe_st
								       *,
								       int);







melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_36_WARMELTmiHOOKS_ENABLE_ALL_IPA_PASSES_END (meltclosure_ptr_t
						      meltclosp_,
						      melt_ptr_t
						      meltfirstargp_,
						      const
						      melt_argdescr_cell_t
						      meltxargdescr_[],
						      union meltparam_un
						      *meltxargtab_,
						      const
						      melt_argdescr_cell_t
						      meltxresdescr_[],
						      union meltparam_un
						      *meltxrestab_);




melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_37_WARMELTmiHOOKS_REGISTER_ALL_IPA_PASSES_END_FIRST
(meltclosure_ptr_t meltclosp_, melt_ptr_t meltfirstargp_, const melt_argdescr_cell_t meltxargdescr_[],
union meltparam_un *meltxargtab_, const melt_argdescr_cell_t meltxresdescr_[],
union meltparam_un *meltxrestab_);




melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_38_WARMELTmiHOOKS_REGISTER_ALL_IPA_PASSES_END_LAST (meltclosure_ptr_t
							     meltclosp_,
							     melt_ptr_t
							     meltfirstargp_,
							     const
							     melt_argdescr_cell_t
							     meltxargdescr_[],
							     union
							     meltparam_un
							     *meltxargtab_,
							     const
							     melt_argdescr_cell_t
							     meltxresdescr_[],
							     union
							     meltparam_un
							     *meltxrestab_);

/** declaration of hook melthook_HOOK_EARLY_GIMPLE_PASSES_START */
MELT_EXTERN
  void melthook_HOOK_EARLY_GIMPLE_PASSES_START (melt_ptr_t melthookdata);


/* declare hook frame marking routine for melthook_HOOK_EARLY_GIMPLE_PASSES_START hook */
void MELT_MODULE_VISIBILITY
melthookmark_HOOK_EARLY_GIMPLE_PASSES_START_rout (struct melt_callframe_st *,
						  int);







melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_40_WARMELTmiHOOKS_ENABLE_EARLY_GIMPLE_PASSES_START (meltclosure_ptr_t
							     meltclosp_,
							     melt_ptr_t
							     meltfirstargp_,
							     const
							     melt_argdescr_cell_t
							     meltxargdescr_[],
							     union
							     meltparam_un
							     *meltxargtab_,
							     const
							     melt_argdescr_cell_t
							     meltxresdescr_[],
							     union
							     meltparam_un
							     *meltxrestab_);




melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_41_WARMELTmiHOOKS_REGISTER_EARLY_GIMPLE_PASSES_START_FIRST
(meltclosure_ptr_t meltclosp_, melt_ptr_t meltfirstargp_, const melt_argdescr_cell_t meltxargdescr_[],
union meltparam_un *meltxargtab_, const melt_argdescr_cell_t meltxresdescr_[],
union meltparam_un *meltxrestab_);




melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_42_WARMELTmiHOOKS_REGISTER_EARLY_GIMPLE_PASSES_START_LAST
(meltclosure_ptr_t meltclosp_, melt_ptr_t meltfirstargp_, const melt_argdescr_cell_t meltxargdescr_[],
union meltparam_un *meltxargtab_, const melt_argdescr_cell_t meltxresdescr_[],
union meltparam_un *meltxrestab_);

/** declaration of hook melthook_HOOK_EARLY_GIMPLE_PASSES_END */
MELT_EXTERN
  void melthook_HOOK_EARLY_GIMPLE_PASSES_END (melt_ptr_t melthookdata);


/* declare hook frame marking routine for melthook_HOOK_EARLY_GIMPLE_PASSES_END hook */
void MELT_MODULE_VISIBILITY
melthookmark_HOOK_EARLY_GIMPLE_PASSES_END_rout (struct melt_callframe_st *,
						int);







melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_44_WARMELTmiHOOKS_ENABLE_EARLY_GIMPLE_PASSES_END (meltclosure_ptr_t
							   meltclosp_,
							   melt_ptr_t
							   meltfirstargp_,
							   const
							   melt_argdescr_cell_t
							   meltxargdescr_[],
							   union meltparam_un
							   *meltxargtab_,
							   const
							   melt_argdescr_cell_t
							   meltxresdescr_[],
							   union meltparam_un
							   *meltxrestab_);




melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_45_WARMELTmiHOOKS_REGISTER_EARLY_GIMPLE_PASSES_END_FIRST
(meltclosure_ptr_t meltclosp_, melt_ptr_t meltfirstargp_, const melt_argdescr_cell_t meltxargdescr_[],
union meltparam_un *meltxargtab_, const melt_argdescr_cell_t meltxresdescr_[],
union meltparam_un *meltxrestab_);




melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_46_WARMELTmiHOOKS_REGISTER_EARLY_GIMPLE_PASSES_END_LAST
(meltclosure_ptr_t meltclosp_, melt_ptr_t meltfirstargp_, const melt_argdescr_cell_t meltxargdescr_[],
union meltparam_un *meltxargtab_, const melt_argdescr_cell_t meltxresdescr_[],
union meltparam_un *meltxrestab_);




melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_47_WARMELTmiHOOKS_ENABLE_PRE_GENERICIZE (meltclosure_ptr_t
						  meltclosp_,
						  melt_ptr_t meltfirstargp_,
						  const melt_argdescr_cell_t
						  meltxargdescr_[],
						  union meltparam_un
						  *meltxargtab_,
						  const melt_argdescr_cell_t
						  meltxresdescr_[],
						  union meltparam_un
						  *meltxrestab_);

/** declaration of hook melthook_HOOK_PRE_GENERICIZE */
MELT_EXTERN
  void melthook_HOOK_PRE_GENERICIZE (melt_ptr_t melthookdata,
				     tree meltinp0_TFNDECL);


/* declare hook frame marking routine for melthook_HOOK_PRE_GENERICIZE hook */
void MELT_MODULE_VISIBILITY melthookmark_HOOK_PRE_GENERICIZE_rout (struct
								   melt_callframe_st
								   *, int);







melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_49_WARMELTmiHOOKS_REGISTER_PRE_GENERICIZE_FIRST (meltclosure_ptr_t
							  meltclosp_,
							  melt_ptr_t
							  meltfirstargp_,
							  const
							  melt_argdescr_cell_t
							  meltxargdescr_[],
							  union meltparam_un
							  *meltxargtab_,
							  const
							  melt_argdescr_cell_t
							  meltxresdescr_[],
							  union meltparam_un
							  *meltxrestab_);




melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_50_WARMELTmiHOOKS_REGISTER_PRE_GENERICIZE_LAST (meltclosure_ptr_t
							 meltclosp_,
							 melt_ptr_t
							 meltfirstargp_,
							 const
							 melt_argdescr_cell_t
							 meltxargdescr_[],
							 union meltparam_un
							 *meltxargtab_,
							 const
							 melt_argdescr_cell_t
							 meltxresdescr_[],
							 union meltparam_un
							 *meltxrestab_);




melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_51_WARMELTmiHOOKS_ENABLE_FINISH_TYPE (meltclosure_ptr_t meltclosp_,
					       melt_ptr_t meltfirstargp_,
					       const melt_argdescr_cell_t
					       meltxargdescr_[],
					       union meltparam_un
					       *meltxargtab_,
					       const melt_argdescr_cell_t
					       meltxresdescr_[],
					       union meltparam_un
					       *meltxrestab_);

/** declaration of hook melthook_HOOK_FINISH_TYPE */
MELT_EXTERN
  void melthook_HOOK_FINISH_TYPE (melt_ptr_t melthookdata,
				  tree meltinp0_TFNDECL);


/* declare hook frame marking routine for melthook_HOOK_FINISH_TYPE hook */
void MELT_MODULE_VISIBILITY melthookmark_HOOK_FINISH_TYPE_rout (struct
								melt_callframe_st
								*, int);







melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_53_WARMELTmiHOOKS_REGISTER_FINISH_TYPE_FIRST (meltclosure_ptr_t
						       meltclosp_,
						       melt_ptr_t
						       meltfirstargp_,
						       const
						       melt_argdescr_cell_t
						       meltxargdescr_[],
						       union meltparam_un
						       *meltxargtab_,
						       const
						       melt_argdescr_cell_t
						       meltxresdescr_[],
						       union meltparam_un
						       *meltxrestab_);




melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_54_WARMELTmiHOOKS_REGISTER_FINISH_TYPE_LAST (meltclosure_ptr_t
						      meltclosp_,
						      melt_ptr_t
						      meltfirstargp_,
						      const
						      melt_argdescr_cell_t
						      meltxargdescr_[],
						      union meltparam_un
						      *meltxargtab_,
						      const
						      melt_argdescr_cell_t
						      meltxresdescr_[],
						      union meltparam_un
						      *meltxrestab_);




melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_55_WARMELTmiHOOKS_ENABLE_FINISH_DECL (meltclosure_ptr_t meltclosp_,
					       melt_ptr_t meltfirstargp_,
					       const melt_argdescr_cell_t
					       meltxargdescr_[],
					       union meltparam_un
					       *meltxargtab_,
					       const melt_argdescr_cell_t
					       meltxresdescr_[],
					       union meltparam_un
					       *meltxrestab_);

/** declaration of hook melthook_HOOK_FINISH_DECL */
MELT_EXTERN
  void melthook_HOOK_FINISH_DECL (melt_ptr_t melthookdata,
				  tree meltinp0_TFNDECL);


/* declare hook frame marking routine for melthook_HOOK_FINISH_DECL hook */
void MELT_MODULE_VISIBILITY melthookmark_HOOK_FINISH_DECL_rout (struct
								melt_callframe_st
								*, int);







melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_57_WARMELTmiHOOKS_REGISTER_FINISH_DECL_FIRST (meltclosure_ptr_t
						       meltclosp_,
						       melt_ptr_t
						       meltfirstargp_,
						       const
						       melt_argdescr_cell_t
						       meltxargdescr_[],
						       union meltparam_un
						       *meltxargtab_,
						       const
						       melt_argdescr_cell_t
						       meltxresdescr_[],
						       union meltparam_un
						       *meltxrestab_);




melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_58_WARMELTmiHOOKS_REGISTER_FINISH_DECL_LAST (meltclosure_ptr_t
						      meltclosp_,
						      melt_ptr_t
						      meltfirstargp_,
						      const
						      melt_argdescr_cell_t
						      meltxargdescr_[],
						      union meltparam_un
						      *meltxargtab_,
						      const
						      melt_argdescr_cell_t
						      meltxresdescr_[],
						      union meltparam_un
						      *meltxrestab_);

/** declaration of hook melthook_HOOK_PASS_EXECUTION */
MELT_EXTERN
  void melthook_HOOK_PASS_EXECUTION (melt_ptr_t melthookdata,
				     const char *meltinp0_PASSNAME,
				     long meltinp1_PASSNUM);


/* declare hook frame marking routine for melthook_HOOK_PASS_EXECUTION hook */
void MELT_MODULE_VISIBILITY melthookmark_HOOK_PASS_EXECUTION_rout (struct
								   melt_callframe_st
								   *, int);







melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_60_WARMELTmiHOOKS_REGISTER_PASS_EXECUTION_FIRST (meltclosure_ptr_t
							  meltclosp_,
							  melt_ptr_t
							  meltfirstargp_,
							  const
							  melt_argdescr_cell_t
							  meltxargdescr_[],
							  union meltparam_un
							  *meltxargtab_,
							  const
							  melt_argdescr_cell_t
							  meltxresdescr_[],
							  union meltparam_un
							  *meltxrestab_);




melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_61_WARMELTmiHOOKS_REGISTER_PASS_EXECUTION_LAST (meltclosure_ptr_t
							 meltclosp_,
							 melt_ptr_t
							 meltfirstargp_,
							 const
							 melt_argdescr_cell_t
							 meltxargdescr_[],
							 union meltparam_un
							 *meltxargtab_,
							 const
							 melt_argdescr_cell_t
							 meltxresdescr_[],
							 union meltparam_un
							 *meltxrestab_);

/** declaration of hook melthook_HOOK_GIMPLE_GATE */
MELT_EXTERN long melthook_HOOK_GIMPLE_GATE (melt_ptr_t melthookdata);


/* declare hook frame marking routine for melthook_HOOK_GIMPLE_GATE hook */
void MELT_MODULE_VISIBILITY melthookmark_HOOK_GIMPLE_GATE_rout (struct
								melt_callframe_st
								*, int);




/** declaration of hook melthook_HOOK_GIMPLE_EXECUTE */
MELT_EXTERN long melthook_HOOK_GIMPLE_EXECUTE (melt_ptr_t melthookdata);


/* declare hook frame marking routine for melthook_HOOK_GIMPLE_EXECUTE hook */
void MELT_MODULE_VISIBILITY melthookmark_HOOK_GIMPLE_EXECUTE_rout (struct
								   melt_callframe_st
								   *, int);




/** declaration of hook melthook_HOOK_RTL_GATE */
MELT_EXTERN long melthook_HOOK_RTL_GATE (melt_ptr_t melthookdata);


/* declare hook frame marking routine for melthook_HOOK_RTL_GATE hook */
void MELT_MODULE_VISIBILITY melthookmark_HOOK_RTL_GATE_rout (struct
							     melt_callframe_st
							     *, int);




/** declaration of hook melthook_HOOK_RTL_EXECUTE */
MELT_EXTERN long melthook_HOOK_RTL_EXECUTE (melt_ptr_t melthookdata);


/* declare hook frame marking routine for melthook_HOOK_RTL_EXECUTE hook */
void MELT_MODULE_VISIBILITY melthookmark_HOOK_RTL_EXECUTE_rout (struct
								melt_callframe_st
								*, int);




/** declaration of hook melthook_HOOK_SIMPLE_IPA_GATE */
MELT_EXTERN long melthook_HOOK_SIMPLE_IPA_GATE (melt_ptr_t melthookdata);


/* declare hook frame marking routine for melthook_HOOK_SIMPLE_IPA_GATE hook */
void MELT_MODULE_VISIBILITY melthookmark_HOOK_SIMPLE_IPA_GATE_rout (struct
								    melt_callframe_st
								    *, int);




/** declaration of hook melthook_HOOK_SIMPLE_IPA_EXECUTE */
MELT_EXTERN long melthook_HOOK_SIMPLE_IPA_EXECUTE (melt_ptr_t melthookdata);


/* declare hook frame marking routine for melthook_HOOK_SIMPLE_IPA_EXECUTE hook */
void MELT_MODULE_VISIBILITY melthookmark_HOOK_SIMPLE_IPA_EXECUTE_rout (struct
								       melt_callframe_st
								       *,
								       int);




/** declaration of hook melthook_HOOK_EXIT_FINALIZER */
MELT_EXTERN void melthook_HOOK_EXIT_FINALIZER (melt_ptr_t melthookdata);


/* declare hook frame marking routine for melthook_HOOK_EXIT_FINALIZER hook */
void MELT_MODULE_VISIBILITY melthookmark_HOOK_EXIT_FINALIZER_rout (struct
								   melt_callframe_st
								   *, int);







melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_69_WARMELTmiHOOKS_AT_EXIT_FIRST (meltclosure_ptr_t meltclosp_,
					  melt_ptr_t meltfirstargp_,
					  const melt_argdescr_cell_t
					  meltxargdescr_[],
					  union meltparam_un *meltxargtab_,
					  const melt_argdescr_cell_t
					  meltxresdescr_[],
					  union meltparam_un *meltxrestab_);




melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_70_WARMELTmiHOOKS_AT_EXIT_LAST (meltclosure_ptr_t meltclosp_,
					 melt_ptr_t meltfirstargp_,
					 const melt_argdescr_cell_t
					 meltxargdescr_[],
					 union meltparam_un *meltxargtab_,
					 const melt_argdescr_cell_t
					 meltxresdescr_[],
					 union meltparam_un *meltxrestab_);



MELT_EXTERN void *melt_start_this_module (void *);
struct melt_initial_frame_st;

/*declare opaque initial frame: */
typedef struct melt_initial_frame_st meltinitial_frame_t;


/* define different names when debugging or not */
#if MELT_HAVE_DEBUG
MELT_EXTERN const char meltmodule_WARMELTmiHOOKS__melt_have_debug_enabled[];
#define melt_have_debug_string meltmodule_WARMELTmiHOOKS__melt_have_debug_enabled
#else /*!MELT_HAVE_DEBUG */
MELT_EXTERN const char meltmodule_WARMELTmiHOOKS__melt_have_debug_disabled[];
#define melt_have_debug_string meltmodule_WARMELTmiHOOKS__melt_have_debug_disabled
#endif /*!MELT_HAVE_DEBUG */



void MELT_MODULE_VISIBILITY
meltmod__WARMELTmiHOOKS__initialmeltchunk_0 (meltinitial_frame_t *, char *);
void MELT_MODULE_VISIBILITY
meltmod__WARMELTmiHOOKS__initialmeltchunk_1 (meltinitial_frame_t *, char *);
void MELT_MODULE_VISIBILITY
meltmod__WARMELTmiHOOKS__initialmeltchunk_2 (meltinitial_frame_t *, char *);
void MELT_MODULE_VISIBILITY
meltmod__WARMELTmiHOOKS__initialmeltchunk_3 (meltinitial_frame_t *, char *);
void MELT_MODULE_VISIBILITY
meltmod__WARMELTmiHOOKS__initialmeltchunk_4 (meltinitial_frame_t *, char *);
void MELT_MODULE_VISIBILITY
meltmod__WARMELTmiHOOKS__initialmeltchunk_5 (meltinitial_frame_t *, char *);
void MELT_MODULE_VISIBILITY
meltmod__WARMELTmiHOOKS__initialmeltchunk_6 (meltinitial_frame_t *, char *);
void MELT_MODULE_VISIBILITY
meltmod__WARMELTmiHOOKS__initialmeltchunk_7 (meltinitial_frame_t *, char *);
void MELT_MODULE_VISIBILITY
meltmod__WARMELTmiHOOKS__forward_or_mark_module_start_frame (struct
							     melt_callframe_st
							     *fp,
							     int marking);



/**** warmelt-hooks+02.c implementations ****/




melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_44_WARMELTmiHOOKS_ENABLE_EARLY_GIMPLE_PASSES_END (meltclosure_ptr_t
							   meltclosp_,
							   melt_ptr_t
							   meltfirstargp_,
							   const
							   melt_argdescr_cell_t
							   meltxargdescr_[],
							   union meltparam_un
							   *meltxargtab_,
							   const
							   melt_argdescr_cell_t
							   meltxresdescr_[],
							   union meltparam_un
							   *meltxrestab_)
{
  long
    current_blocklevel_signals_meltrout_44_WARMELTmiHOOKS_ENABLE_EARLY_GIMPLE_PASSES_END_melt
    = melt_blocklevel_signals;

#if MELT_HAVE_DEBUG
  static long melt_call_counter__;
  long melt_thiscallcounter__ ATTRIBUTE_UNUSED = ++melt_call_counter__;
#undef meltcallcount
#define meltcallcount melt_thiscallcounter__
#else
#undef meltcallcount
#define meltcallcount 0L
#endif

  struct meltframe_meltrout_44_WARMELTmiHOOKS_ENABLE_EARLY_GIMPLE_PASSES_END_st
  {
    int mcfr_nbvar;
#if MELT_HAVE_DEBUG
    const char *mcfr_flocs;
#else				/*!MELT_HAVE_DEBUG */
    const char *mcfr_unusedflocs;
#endif				/*MELT_HAVE_DEBUG */
    struct meltclosure_st *mcfr_clos;
    struct excepth_melt_st *mcfr_exh;
    struct melt_callframe_st *mcfr_prev;
#undef MELTFRAM_NBVARPTR
#undef MELTFRAM_NBVARNUM
#define MELTFRAM_NBVARPTR 1
    melt_ptr_t mcfr_varptr[1];
/*no varnum*/
#define MELTFRAM_NBVARNUM /*none*/0
/*others*/
    long _spare_;
  }
   *meltframptr_ = 0, meltfram__;	/*declfrastruct */
#define meltframe meltfram__
  /*meltrout_44_WARMELTmiHOOKS_ENABLE_EARLY_GIMPLE_PASSES_END is not initial declstructinit */
  if (MELT_UNLIKELY (meltxargdescr_ == MELTPAR_MARKGGC))
    {				/*mark for ggc */
      int meltix;
      meltframptr_ =
	(struct
	 meltframe_meltrout_44_WARMELTmiHOOKS_ENABLE_EARLY_GIMPLE_PASSES_END_st
	 *) meltfirstargp_;
      /* use arguments meltrout_44_WARMELTmiHOOKS_ENABLE_EARLY_GIMPLE_PASSES_END output_curframe_declstruct_init */
      (void) meltclosp_;
      (void) meltfirstargp_;
      (void) meltxargdescr_;
      (void) meltxargtab_;
      (void) meltxresdescr_;
      (void) meltxrestab_;
      /* marking closure */
      gt_ggc_mx_melt_un (meltframptr_->mcfr_clos);
      for (meltix = 0; meltix < 1; meltix++)
	if (meltframptr_->mcfr_varptr[meltix])
	  gt_ggc_mx_melt_un (meltframptr_->mcfr_varptr[meltix]);
      return NULL;
    } /*end markggc */ ;
  memset (&meltfram__, 0, sizeof (meltfram__));
/* declstructinit plain routine meltrout_44_WARMELTmiHOOKS_ENABLE_EARLY_GIMPLE_PASSES_END nbval 1*/
  meltfram__.mcfr_nbvar = 1 /*nbval */ ;
  meltfram__.mcfr_clos = meltclosp_;
  meltfram__.mcfr_prev = (struct melt_callframe_st *) melt_topframe;
  melt_topframe = (struct melt_callframe_st *) &meltfram__;
  melt_trace_start ("ENABLE_EARLY_GIMPLE_PASSES_END", meltcallcount);
/*getargs*/
  ;
  goto meltlab_endgetargs;
meltlab_endgetargs:;
/*body*/
  MELT_LOCATION ("warmelt-hooks.melt:1133:/ block");
/*anyblock*/
  {


    {
      MELT_LOCATION ("warmelt-hooks.melt:1134:/ locexp");
      /* enable_early_gimple_passes_end ENALLPASSESEND_CHK__3 */
      if (!melthk_early_gimple_passes_end_registered_flag)
	{
	  melthk_early_gimple_passes_end_registered_flag = true;
	  register_callback (melt_plugin_name, PLUGIN_EARLY_GIMPLE_PASSES_END,
			     melt_early_gimple_passes_end_hook_callback,
			     NULL);
	}
      ;
    }
    ;
    /*epilog */
  }

  ;
  goto meltlabend_rout;
meltlabend_rout:
  melt_trace_end ("ENABLE_EARLY_GIMPLE_PASSES_END", meltcallcount);
  MELT_TRACE_EXIT_LOCATION ();
  melt_blocklevel_signals =
    current_blocklevel_signals_meltrout_44_WARMELTmiHOOKS_ENABLE_EARLY_GIMPLE_PASSES_END_melt;
  melt_topframe = (struct melt_callframe_st *) meltfram__.mcfr_prev;
  return (melt_ptr_t) ( /*noretval */ NULL);
#undef meltcallcount
#undef meltfram__
#undef MELTFRAM_NBVARNUM
#undef MELTFRAM_NBVARPTR
}				/*end meltrout_44_WARMELTmiHOOKS_ENABLE_EARLY_GIMPLE_PASSES_END */





melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_45_WARMELTmiHOOKS_REGISTER_EARLY_GIMPLE_PASSES_END_FIRST
  (meltclosure_ptr_t meltclosp_, melt_ptr_t meltfirstargp_,
   const melt_argdescr_cell_t meltxargdescr_[],
   union meltparam_un * meltxargtab_,
   const melt_argdescr_cell_t meltxresdescr_[],
   union meltparam_un * meltxrestab_)
{
  long
    current_blocklevel_signals_meltrout_45_WARMELTmiHOOKS_REGISTER_EARLY_GIMPLE_PASSES_END_FIRST_melt
    = melt_blocklevel_signals;

#if MELT_HAVE_DEBUG
  static long melt_call_counter__;
  long melt_thiscallcounter__ ATTRIBUTE_UNUSED = ++melt_call_counter__;
#undef meltcallcount
#define meltcallcount melt_thiscallcounter__
#else
#undef meltcallcount
#define meltcallcount 0L
#endif

  struct meltframe_meltrout_45_WARMELTmiHOOKS_REGISTER_EARLY_GIMPLE_PASSES_END_FIRST_st
  {
    int mcfr_nbvar;
#if MELT_HAVE_DEBUG
    const char *mcfr_flocs;
#else				/*!MELT_HAVE_DEBUG */
    const char *mcfr_unusedflocs;
#endif				/*MELT_HAVE_DEBUG */
    struct meltclosure_st *mcfr_clos;
    struct excepth_melt_st *mcfr_exh;
    struct melt_callframe_st *mcfr_prev;
#undef MELTFRAM_NBVARPTR
#undef MELTFRAM_NBVARNUM
#define MELTFRAM_NBVARPTR 7
    melt_ptr_t mcfr_varptr[7];
#define MELTFRAM_NBVARNUM 2
    long mcfr_varnum[2];
/*others*/
    long _spare_;
  }
   *meltframptr_ = 0, meltfram__;	/*declfrastruct */
#define meltframe meltfram__
  /*meltrout_45_WARMELTmiHOOKS_REGISTER_EARLY_GIMPLE_PASSES_END_FIRST is not initial declstructinit */
  if (MELT_UNLIKELY (meltxargdescr_ == MELTPAR_MARKGGC))
    {				/*mark for ggc */
      int meltix;
      meltframptr_ =
	(struct
	 meltframe_meltrout_45_WARMELTmiHOOKS_REGISTER_EARLY_GIMPLE_PASSES_END_FIRST_st
	 *) meltfirstargp_;
      /* use arguments meltrout_45_WARMELTmiHOOKS_REGISTER_EARLY_GIMPLE_PASSES_END_FIRST output_curframe_declstruct_init */
      (void) meltclosp_;
      (void) meltfirstargp_;
      (void) meltxargdescr_;
      (void) meltxargtab_;
      (void) meltxresdescr_;
      (void) meltxrestab_;
      /* marking closure */
      gt_ggc_mx_melt_un (meltframptr_->mcfr_clos);
      for (meltix = 0; meltix < 7; meltix++)
	if (meltframptr_->mcfr_varptr[meltix])
	  gt_ggc_mx_melt_un (meltframptr_->mcfr_varptr[meltix]);
      return NULL;
    } /*end markggc */ ;
  memset (&meltfram__, 0, sizeof (meltfram__));
/* declstructinit plain routine meltrout_45_WARMELTmiHOOKS_REGISTER_EARLY_GIMPLE_PASSES_END_FIRST nbval 7*/
  meltfram__.mcfr_nbvar = 7 /*nbval */ ;
  meltfram__.mcfr_clos = meltclosp_;
  meltfram__.mcfr_prev = (struct melt_callframe_st *) melt_topframe;
  melt_topframe = (struct melt_callframe_st *) &meltfram__;
  melt_trace_start ("REGISTER_EARLY_GIMPLE_PASSES_END_FIRST", meltcallcount);
/*getargs*/

  /*getarg#0 */
  MELT_LOCATION ("warmelt-hooks.melt:1146:/ getarg");
 /*_.FUN__V2*/ meltfptr[1] = (melt_ptr_t) meltfirstargp_;
  ;
  goto meltlab_endgetargs;
meltlab_endgetargs:;
/*body*/
/*^block*/
/*anyblock*/
  {


    MELT_CHECK_SIGNAL ();
    ;
 /*_#IS_CLOSURE__L1*/ meltfnum[0] =
      (melt_magic_discr ((melt_ptr_t) ( /*_.FUN__V2*/ meltfptr[1])) ==
       MELTOBMAG_CLOSURE);;
    MELT_LOCATION ("warmelt-hooks.melt:1150:/ cond");
    /*cond */ if ( /*_#IS_CLOSURE__L1*/ meltfnum[0])	/*then */
      {
	/*^cond.then */
	/*^block */
	/*anyblock */
	{

	  MELT_LOCATION ("warmelt-hooks.melt:1151:/ quasiblock");


	  /*^cond */
	  /*cond */ if (
			 /*ifisa */
			 melt_is_instance_of ((melt_ptr_t)
					      (( /*!EARLY_GIMPLE_PASSES_END_DELAYED_QUEUE */ meltfrout->tabval[0])),
					      (melt_ptr_t) (( /*!CLASS_DELAYED_QUEUE */ meltfrout->tabval[1])))
	    )			/*then */
	    {
	      /*^cond.then */
	      /*^getslot */
	      {
		melt_ptr_t slot = NULL, obj = NULL;
		obj =
		  (melt_ptr_t) (( /*!EARLY_GIMPLE_PASSES_END_DELAYED_QUEUE */
				 meltfrout->tabval[0])) /*=obj*/ ;
		melt_object_get_field (slot, obj, 2, "DELQU_FIRST");
     /*_.FIRSTLIST__V3*/ meltfptr[2] = slot;
	      };
	      ;
	    }
	  else
	    {			/*^cond.else */

    /*_.FIRSTLIST__V3*/ meltfptr[2] = NULL;;
	    }
	  ;

	  MELT_CHECK_SIGNAL ();
	  ;
	  MELT_LOCATION ("warmelt-hooks.melt:1153:/ apply");
	  /*apply */
	  {
	    /*_.ENABLE_EARLY_GIMPLE_PASSES_END__V4*/ meltfptr[3] =
	      melt_apply ((meltclosure_ptr_t)
			  (( /*!ENABLE_EARLY_GIMPLE_PASSES_END */ meltfrout->
			    tabval[2])), (melt_ptr_t) (NULL), (""),
			  (union meltparam_un *) 0, "",
			  (union meltparam_un *) 0);
	  }
	  ;

	  MELT_CHECK_SIGNAL ();
	  ;
   /*_#NULL__L2*/ meltfnum[1] =
	    (( /*_.FIRSTLIST__V3*/ meltfptr[2]) == NULL);;
	  MELT_LOCATION ("warmelt-hooks.melt:1154:/ cond");
	  /*cond */ if ( /*_#NULL__L2*/ meltfnum[1])	/*then */
	    {
	      /*^cond.then */
	      /*^block */
	      /*anyblock */
	      {

     /*_.MAKE_LIST__V5*/ meltfptr[4] =
		  (meltgc_new_list
		   ((meltobject_ptr_t)
		    (( /*!DISCR_LIST */ meltfrout->tabval[3]))));;
		MELT_LOCATION ("warmelt-hooks.melt:1156:/ compute");
		/*_.FIRSTLIST__V3*/ meltfptr[2] =
		  /*_.SETQ___V6*/ meltfptr[5] =
		  /*_.MAKE_LIST__V5*/ meltfptr[4];;

		MELT_CHECK_SIGNAL ();
		;
     /*_.MAKE_LIST__V7*/ meltfptr[6] =
		  (meltgc_new_list
		   ((meltobject_ptr_t)
		    (( /*!DISCR_LIST */ meltfrout->tabval[3]))));;
		MELT_LOCATION ("warmelt-hooks.melt:1157:/ cond");
		/*cond */ if (
			       /*ifisa */
			       melt_is_instance_of ((melt_ptr_t)
						    (( /*!EARLY_GIMPLE_PASSES_END_DELAYED_QUEUE */ meltfrout->tabval[0])),
						    (melt_ptr_t) (( /*!CLASS_DELAYED_QUEUE */ meltfrout->tabval[1])))
		  )		/*then */
		  {
		    /*^cond.then */
		    /*^block */
		    /*anyblock */
		    {

		      /*^putslot */
		      /*putslot */
		      melt_assertmsg ("putslot checkobj @DELQU_FIRST",
				      melt_magic_discr ((melt_ptr_t)
							(( /*!EARLY_GIMPLE_PASSES_END_DELAYED_QUEUE */ meltfrout->tabval[0]))) == MELTOBMAG_OBJECT);
		      melt_putfield_object ((( /*!EARLY_GIMPLE_PASSES_END_DELAYED_QUEUE */ meltfrout->tabval[0])), (2), ( /*_.FIRSTLIST__V3*/ meltfptr[2]), "DELQU_FIRST");
		      ;
		      /*^putslot */
		      /*putslot */
		      melt_assertmsg ("putslot checkobj @DELQU_LAST",
				      melt_magic_discr ((melt_ptr_t)
							(( /*!EARLY_GIMPLE_PASSES_END_DELAYED_QUEUE */ meltfrout->tabval[0]))) == MELTOBMAG_OBJECT);
		      melt_putfield_object ((( /*!EARLY_GIMPLE_PASSES_END_DELAYED_QUEUE */ meltfrout->tabval[0])), (3), ( /*_.MAKE_LIST__V7*/ meltfptr[6]), "DELQU_LAST");
		      ;
		      /*^touch */
		      meltgc_touch (( /*!EARLY_GIMPLE_PASSES_END_DELAYED_QUEUE */ meltfrout->tabval[0]));
		      ;
		      /*^touchobj */

		      melt_dbgtrace_written_object (( /*!EARLY_GIMPLE_PASSES_END_DELAYED_QUEUE */ meltfrout->tabval[0]), "put-fields");
		      ;
		      /*epilog */
		    }
		    ;
		  }		/*noelse */
		;
		MELT_LOCATION ("warmelt-hooks.melt:1155:/ quasiblock");


		/*epilog */

		MELT_LOCATION ("warmelt-hooks.melt:1154:/ clear");
	       /*clear *//*_.MAKE_LIST__V5*/ meltfptr[4] = 0;
		/*^clear */
	       /*clear *//*_.SETQ___V6*/ meltfptr[5] = 0;
		/*^clear */
	       /*clear *//*_.MAKE_LIST__V7*/ meltfptr[6] = 0;
	      }
	      ;
	    }			/*noelse */
	  ;

	  {
	    MELT_LOCATION ("warmelt-hooks.melt:1162:/ locexp");
	    meltgc_append_list ((melt_ptr_t)
				( /*_.FIRSTLIST__V3*/ meltfptr[2]),
				(melt_ptr_t) ( /*_.FUN__V2*/ meltfptr[1]));
	  }
	  ;

	  MELT_LOCATION ("warmelt-hooks.melt:1151:/ clear");
	     /*clear *//*_.FIRSTLIST__V3*/ meltfptr[2] = 0;
	  /*^clear */
	     /*clear *//*_.ENABLE_EARLY_GIMPLE_PASSES_END__V4*/ meltfptr[3] = 0;
	  /*^clear */
	     /*clear *//*_#NULL__L2*/ meltfnum[1] = 0;
	  /*epilog */
	}
	;
      }
    else
      {
	MELT_LOCATION ("warmelt-hooks.melt:1150:/ cond.else");

	/*^block */
	/*anyblock */
	{


	  {
	    MELT_LOCATION ("warmelt-hooks.melt:1165:/ locexp");
	    error ("MELT ERROR MSG [#%ld]::: %s", melt_dbgcounter,
		   ("Bad function (non closure) passed to register_early_gimple_passes_end_first"));
	  }
	  ;
	  /*epilog */
	}
	;
      }
    ;
    /*epilog */

    MELT_LOCATION ("warmelt-hooks.melt:1146:/ clear");
	   /*clear *//*_#IS_CLOSURE__L1*/ meltfnum[0] = 0;
  }

  ;
  goto meltlabend_rout;
meltlabend_rout:
  melt_trace_end ("REGISTER_EARLY_GIMPLE_PASSES_END_FIRST", meltcallcount);
  MELT_TRACE_EXIT_LOCATION ();
  melt_blocklevel_signals =
    current_blocklevel_signals_meltrout_45_WARMELTmiHOOKS_REGISTER_EARLY_GIMPLE_PASSES_END_FIRST_melt;
  melt_topframe = (struct melt_callframe_st *) meltfram__.mcfr_prev;
  return (melt_ptr_t) ( /*noretval */ NULL);
#undef meltcallcount
#undef meltfram__
#undef MELTFRAM_NBVARNUM
#undef MELTFRAM_NBVARPTR
}				/*end meltrout_45_WARMELTmiHOOKS_REGISTER_EARLY_GIMPLE_PASSES_END_FIRST */





melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_46_WARMELTmiHOOKS_REGISTER_EARLY_GIMPLE_PASSES_END_LAST
  (meltclosure_ptr_t meltclosp_, melt_ptr_t meltfirstargp_,
   const melt_argdescr_cell_t meltxargdescr_[],
   union meltparam_un * meltxargtab_,
   const melt_argdescr_cell_t meltxresdescr_[],
   union meltparam_un * meltxrestab_)
{
  long
    current_blocklevel_signals_meltrout_46_WARMELTmiHOOKS_REGISTER_EARLY_GIMPLE_PASSES_END_LAST_melt
    = melt_blocklevel_signals;

#if MELT_HAVE_DEBUG
  static long melt_call_counter__;
  long melt_thiscallcounter__ ATTRIBUTE_UNUSED = ++melt_call_counter__;
#undef meltcallcount
#define meltcallcount melt_thiscallcounter__
#else
#undef meltcallcount
#define meltcallcount 0L
#endif

  struct meltframe_meltrout_46_WARMELTmiHOOKS_REGISTER_EARLY_GIMPLE_PASSES_END_LAST_st
  {
    int mcfr_nbvar;
#if MELT_HAVE_DEBUG
    const char *mcfr_flocs;
#else				/*!MELT_HAVE_DEBUG */
    const char *mcfr_unusedflocs;
#endif				/*MELT_HAVE_DEBUG */
    struct meltclosure_st *mcfr_clos;
    struct excepth_melt_st *mcfr_exh;
    struct melt_callframe_st *mcfr_prev;
#undef MELTFRAM_NBVARPTR
#undef MELTFRAM_NBVARNUM
#define MELTFRAM_NBVARPTR 7
    melt_ptr_t mcfr_varptr[7];
#define MELTFRAM_NBVARNUM 2
    long mcfr_varnum[2];
/*others*/
    long _spare_;
  }
   *meltframptr_ = 0, meltfram__;	/*declfrastruct */
#define meltframe meltfram__
  /*meltrout_46_WARMELTmiHOOKS_REGISTER_EARLY_GIMPLE_PASSES_END_LAST is not initial declstructinit */
  if (MELT_UNLIKELY (meltxargdescr_ == MELTPAR_MARKGGC))
    {				/*mark for ggc */
      int meltix;
      meltframptr_ =
	(struct
	 meltframe_meltrout_46_WARMELTmiHOOKS_REGISTER_EARLY_GIMPLE_PASSES_END_LAST_st
	 *) meltfirstargp_;
      /* use arguments meltrout_46_WARMELTmiHOOKS_REGISTER_EARLY_GIMPLE_PASSES_END_LAST output_curframe_declstruct_init */
      (void) meltclosp_;
      (void) meltfirstargp_;
      (void) meltxargdescr_;
      (void) meltxargtab_;
      (void) meltxresdescr_;
      (void) meltxrestab_;
      /* marking closure */
      gt_ggc_mx_melt_un (meltframptr_->mcfr_clos);
      for (meltix = 0; meltix < 7; meltix++)
	if (meltframptr_->mcfr_varptr[meltix])
	  gt_ggc_mx_melt_un (meltframptr_->mcfr_varptr[meltix]);
      return NULL;
    } /*end markggc */ ;
  memset (&meltfram__, 0, sizeof (meltfram__));
/* declstructinit plain routine meltrout_46_WARMELTmiHOOKS_REGISTER_EARLY_GIMPLE_PASSES_END_LAST nbval 7*/
  meltfram__.mcfr_nbvar = 7 /*nbval */ ;
  meltfram__.mcfr_clos = meltclosp_;
  meltfram__.mcfr_prev = (struct melt_callframe_st *) melt_topframe;
  melt_topframe = (struct melt_callframe_st *) &meltfram__;
  melt_trace_start ("REGISTER_EARLY_GIMPLE_PASSES_END_LAST", meltcallcount);
/*getargs*/

  /*getarg#0 */
  MELT_LOCATION ("warmelt-hooks.melt:1169:/ getarg");
 /*_.FUN__V2*/ meltfptr[1] = (melt_ptr_t) meltfirstargp_;
  ;
  goto meltlab_endgetargs;
meltlab_endgetargs:;
/*body*/
/*^block*/
/*anyblock*/
  {


    MELT_CHECK_SIGNAL ();
    ;
 /*_#IS_CLOSURE__L1*/ meltfnum[0] =
      (melt_magic_discr ((melt_ptr_t) ( /*_.FUN__V2*/ meltfptr[1])) ==
       MELTOBMAG_CLOSURE);;
    MELT_LOCATION ("warmelt-hooks.melt:1173:/ cond");
    /*cond */ if ( /*_#IS_CLOSURE__L1*/ meltfnum[0])	/*then */
      {
	/*^cond.then */
	/*^block */
	/*anyblock */
	{

	  MELT_LOCATION ("warmelt-hooks.melt:1174:/ quasiblock");


	  /*^cond */
	  /*cond */ if (
			 /*ifisa */
			 melt_is_instance_of ((melt_ptr_t)
					      (( /*!EARLY_GIMPLE_PASSES_END_DELAYED_QUEUE */ meltfrout->tabval[0])),
					      (melt_ptr_t) (( /*!CLASS_DELAYED_QUEUE */ meltfrout->tabval[1])))
	    )			/*then */
	    {
	      /*^cond.then */
	      /*^getslot */
	      {
		melt_ptr_t slot = NULL, obj = NULL;
		obj =
		  (melt_ptr_t) (( /*!EARLY_GIMPLE_PASSES_END_DELAYED_QUEUE */
				 meltfrout->tabval[0])) /*=obj*/ ;
		melt_object_get_field (slot, obj, 3, "DELQU_LAST");
     /*_.LASTLIST__V3*/ meltfptr[2] = slot;
	      };
	      ;
	    }
	  else
	    {			/*^cond.else */

    /*_.LASTLIST__V3*/ meltfptr[2] = NULL;;
	    }
	  ;

	  MELT_CHECK_SIGNAL ();
	  ;
	  MELT_LOCATION ("warmelt-hooks.melt:1176:/ apply");
	  /*apply */
	  {
	    /*_.ENABLE_EARLY_GIMPLE_PASSES_END__V4*/ meltfptr[3] =
	      melt_apply ((meltclosure_ptr_t)
			  (( /*!ENABLE_EARLY_GIMPLE_PASSES_END */ meltfrout->
			    tabval[2])), (melt_ptr_t) (NULL), (""),
			  (union meltparam_un *) 0, "",
			  (union meltparam_un *) 0);
	  }
	  ;

	  MELT_CHECK_SIGNAL ();
	  ;
   /*_#NULL__L2*/ meltfnum[1] =
	    (( /*_.LASTLIST__V3*/ meltfptr[2]) == NULL);;
	  MELT_LOCATION ("warmelt-hooks.melt:1177:/ cond");
	  /*cond */ if ( /*_#NULL__L2*/ meltfnum[1])	/*then */
	    {
	      /*^cond.then */
	      /*^block */
	      /*anyblock */
	      {

     /*_.MAKE_LIST__V5*/ meltfptr[4] =
		  (meltgc_new_list
		   ((meltobject_ptr_t)
		    (( /*!DISCR_LIST */ meltfrout->tabval[3]))));;
		MELT_LOCATION ("warmelt-hooks.melt:1179:/ compute");
		/*_.LASTLIST__V3*/ meltfptr[2] = /*_.SETQ___V6*/ meltfptr[5] =
		  /*_.MAKE_LIST__V5*/ meltfptr[4];;

		MELT_CHECK_SIGNAL ();
		;
     /*_.MAKE_LIST__V7*/ meltfptr[6] =
		  (meltgc_new_list
		   ((meltobject_ptr_t)
		    (( /*!DISCR_LIST */ meltfrout->tabval[3]))));;
		MELT_LOCATION ("warmelt-hooks.melt:1180:/ cond");
		/*cond */ if (
			       /*ifisa */
			       melt_is_instance_of ((melt_ptr_t)
						    (( /*!EARLY_GIMPLE_PASSES_END_DELAYED_QUEUE */ meltfrout->tabval[0])),
						    (melt_ptr_t) (( /*!CLASS_DELAYED_QUEUE */ meltfrout->tabval[1])))
		  )		/*then */
		  {
		    /*^cond.then */
		    /*^block */
		    /*anyblock */
		    {

		      /*^putslot */
		      /*putslot */
		      melt_assertmsg ("putslot checkobj @DELQU_FIRST",
				      melt_magic_discr ((melt_ptr_t)
							(( /*!EARLY_GIMPLE_PASSES_END_DELAYED_QUEUE */ meltfrout->tabval[0]))) == MELTOBMAG_OBJECT);
		      melt_putfield_object ((( /*!EARLY_GIMPLE_PASSES_END_DELAYED_QUEUE */ meltfrout->tabval[0])), (2), ( /*_.MAKE_LIST__V7*/ meltfptr[6]), "DELQU_FIRST");
		      ;
		      /*^putslot */
		      /*putslot */
		      melt_assertmsg ("putslot checkobj @DELQU_LAST",
				      melt_magic_discr ((melt_ptr_t)
							(( /*!EARLY_GIMPLE_PASSES_END_DELAYED_QUEUE */ meltfrout->tabval[0]))) == MELTOBMAG_OBJECT);
		      melt_putfield_object ((( /*!EARLY_GIMPLE_PASSES_END_DELAYED_QUEUE */ meltfrout->tabval[0])), (3), ( /*_.LASTLIST__V3*/ meltfptr[2]), "DELQU_LAST");
		      ;
		      /*^touch */
		      meltgc_touch (( /*!EARLY_GIMPLE_PASSES_END_DELAYED_QUEUE */ meltfrout->tabval[0]));
		      ;
		      /*^touchobj */

		      melt_dbgtrace_written_object (( /*!EARLY_GIMPLE_PASSES_END_DELAYED_QUEUE */ meltfrout->tabval[0]), "put-fields");
		      ;
		      /*epilog */
		    }
		    ;
		  }		/*noelse */
		;
		MELT_LOCATION ("warmelt-hooks.melt:1178:/ quasiblock");


		/*epilog */

		MELT_LOCATION ("warmelt-hooks.melt:1177:/ clear");
	       /*clear *//*_.MAKE_LIST__V5*/ meltfptr[4] = 0;
		/*^clear */
	       /*clear *//*_.SETQ___V6*/ meltfptr[5] = 0;
		/*^clear */
	       /*clear *//*_.MAKE_LIST__V7*/ meltfptr[6] = 0;
	      }
	      ;
	    }			/*noelse */
	  ;

	  {
	    MELT_LOCATION ("warmelt-hooks.melt:1185:/ locexp");
	    meltgc_append_list ((melt_ptr_t)
				( /*_.LASTLIST__V3*/ meltfptr[2]),
				(melt_ptr_t) ( /*_.FUN__V2*/ meltfptr[1]));
	  }
	  ;

	  MELT_LOCATION ("warmelt-hooks.melt:1174:/ clear");
	     /*clear *//*_.LASTLIST__V3*/ meltfptr[2] = 0;
	  /*^clear */
	     /*clear *//*_.ENABLE_EARLY_GIMPLE_PASSES_END__V4*/ meltfptr[3] = 0;
	  /*^clear */
	     /*clear *//*_#NULL__L2*/ meltfnum[1] = 0;
	  /*epilog */
	}
	;
      }
    else
      {
	MELT_LOCATION ("warmelt-hooks.melt:1173:/ cond.else");

	/*^block */
	/*anyblock */
	{


	  {
	    MELT_LOCATION ("warmelt-hooks.melt:1188:/ locexp");
	    error ("MELT ERROR MSG [#%ld]::: %s", melt_dbgcounter,
		   ("Bad function (non closure) passed to register_early_gimple_passes_end_last"));
	  }
	  ;
	  /*epilog */
	}
	;
      }
    ;
    /*epilog */

    MELT_LOCATION ("warmelt-hooks.melt:1169:/ clear");
	   /*clear *//*_#IS_CLOSURE__L1*/ meltfnum[0] = 0;
  }

  ;
  goto meltlabend_rout;
meltlabend_rout:
  melt_trace_end ("REGISTER_EARLY_GIMPLE_PASSES_END_LAST", meltcallcount);
  MELT_TRACE_EXIT_LOCATION ();
  melt_blocklevel_signals =
    current_blocklevel_signals_meltrout_46_WARMELTmiHOOKS_REGISTER_EARLY_GIMPLE_PASSES_END_LAST_melt;
  melt_topframe = (struct melt_callframe_st *) meltfram__.mcfr_prev;
  return (melt_ptr_t) ( /*noretval */ NULL);
#undef meltcallcount
#undef meltfram__
#undef MELTFRAM_NBVARNUM
#undef MELTFRAM_NBVARPTR
}				/*end meltrout_46_WARMELTmiHOOKS_REGISTER_EARLY_GIMPLE_PASSES_END_LAST */





melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_47_WARMELTmiHOOKS_ENABLE_PRE_GENERICIZE (meltclosure_ptr_t
						  meltclosp_,
						  melt_ptr_t meltfirstargp_,
						  const melt_argdescr_cell_t
						  meltxargdescr_[],
						  union meltparam_un *
						  meltxargtab_,
						  const melt_argdescr_cell_t
						  meltxresdescr_[],
						  union meltparam_un *
						  meltxrestab_)
{
  long
    current_blocklevel_signals_meltrout_47_WARMELTmiHOOKS_ENABLE_PRE_GENERICIZE_melt
    = melt_blocklevel_signals;

#if MELT_HAVE_DEBUG
  static long melt_call_counter__;
  long melt_thiscallcounter__ ATTRIBUTE_UNUSED = ++melt_call_counter__;
#undef meltcallcount
#define meltcallcount melt_thiscallcounter__
#else
#undef meltcallcount
#define meltcallcount 0L
#endif

  struct meltframe_meltrout_47_WARMELTmiHOOKS_ENABLE_PRE_GENERICIZE_st
  {
    int mcfr_nbvar;
#if MELT_HAVE_DEBUG
    const char *mcfr_flocs;
#else				/*!MELT_HAVE_DEBUG */
    const char *mcfr_unusedflocs;
#endif				/*MELT_HAVE_DEBUG */
    struct meltclosure_st *mcfr_clos;
    struct excepth_melt_st *mcfr_exh;
    struct melt_callframe_st *mcfr_prev;
#undef MELTFRAM_NBVARPTR
#undef MELTFRAM_NBVARNUM
#define MELTFRAM_NBVARPTR 1
    melt_ptr_t mcfr_varptr[1];
/*no varnum*/
#define MELTFRAM_NBVARNUM /*none*/0
/*others*/
    long _spare_;
  }
   *meltframptr_ = 0, meltfram__;	/*declfrastruct */
#define meltframe meltfram__
  /*meltrout_47_WARMELTmiHOOKS_ENABLE_PRE_GENERICIZE is not initial declstructinit */
  if (MELT_UNLIKELY (meltxargdescr_ == MELTPAR_MARKGGC))
    {				/*mark for ggc */
      int meltix;
      meltframptr_ =
	(struct meltframe_meltrout_47_WARMELTmiHOOKS_ENABLE_PRE_GENERICIZE_st
	 *) meltfirstargp_;
      /* use arguments meltrout_47_WARMELTmiHOOKS_ENABLE_PRE_GENERICIZE output_curframe_declstruct_init */
      (void) meltclosp_;
      (void) meltfirstargp_;
      (void) meltxargdescr_;
      (void) meltxargtab_;
      (void) meltxresdescr_;
      (void) meltxrestab_;
      /* marking closure */
      gt_ggc_mx_melt_un (meltframptr_->mcfr_clos);
      for (meltix = 0; meltix < 1; meltix++)
	if (meltframptr_->mcfr_varptr[meltix])
	  gt_ggc_mx_melt_un (meltframptr_->mcfr_varptr[meltix]);
      return NULL;
    } /*end markggc */ ;
  memset (&meltfram__, 0, sizeof (meltfram__));
/* declstructinit plain routine meltrout_47_WARMELTmiHOOKS_ENABLE_PRE_GENERICIZE nbval 1*/
  meltfram__.mcfr_nbvar = 1 /*nbval */ ;
  meltfram__.mcfr_clos = meltclosp_;
  meltfram__.mcfr_prev = (struct melt_callframe_st *) melt_topframe;
  melt_topframe = (struct melt_callframe_st *) &meltfram__;
  melt_trace_start ("ENABLE_PRE_GENERICIZE", meltcallcount);
/*getargs*/
  ;
  goto meltlab_endgetargs;
meltlab_endgetargs:;
/*body*/
  MELT_LOCATION ("warmelt-hooks.melt:1229:/ block");
/*anyblock*/
  {


    {
      MELT_LOCATION ("warmelt-hooks.melt:1230:/ locexp");
      /* enable_pre_genericize ENABLEPREGEN_CHK__1 */
      if (!melthk_pre_genericize_registered_flag)
	{
	  melthk_pre_genericize_registered_flag = true;
/*
  register_callback (melt_plugin_name, PLUGIN_PRE_GENERICIZE,
                     melt_pre_genericize_hook_callback, NULL);
*/
	}
      ;
    }
    ;
    /*epilog */
  }

  ;
  goto meltlabend_rout;
meltlabend_rout:
  melt_trace_end ("ENABLE_PRE_GENERICIZE", meltcallcount);
  MELT_TRACE_EXIT_LOCATION ();
  melt_blocklevel_signals =
    current_blocklevel_signals_meltrout_47_WARMELTmiHOOKS_ENABLE_PRE_GENERICIZE_melt;
  melt_topframe = (struct melt_callframe_st *) meltfram__.mcfr_prev;
  return (melt_ptr_t) ( /*noretval */ NULL);
#undef meltcallcount
#undef meltfram__
#undef MELTFRAM_NBVARNUM
#undef MELTFRAM_NBVARPTR
}				/*end meltrout_47_WARMELTmiHOOKS_ENABLE_PRE_GENERICIZE */





melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_49_WARMELTmiHOOKS_REGISTER_PRE_GENERICIZE_FIRST (meltclosure_ptr_t
							  meltclosp_,
							  melt_ptr_t
							  meltfirstargp_,
							  const
							  melt_argdescr_cell_t
							  meltxargdescr_[],
							  union meltparam_un *
							  meltxargtab_,
							  const
							  melt_argdescr_cell_t
							  meltxresdescr_[],
							  union meltparam_un *
							  meltxrestab_)
{
  long
    current_blocklevel_signals_meltrout_49_WARMELTmiHOOKS_REGISTER_PRE_GENERICIZE_FIRST_melt
    = melt_blocklevel_signals;

#if MELT_HAVE_DEBUG
  static long melt_call_counter__;
  long melt_thiscallcounter__ ATTRIBUTE_UNUSED = ++melt_call_counter__;
#undef meltcallcount
#define meltcallcount melt_thiscallcounter__
#else
#undef meltcallcount
#define meltcallcount 0L
#endif

  struct meltframe_meltrout_49_WARMELTmiHOOKS_REGISTER_PRE_GENERICIZE_FIRST_st
  {
    int mcfr_nbvar;
#if MELT_HAVE_DEBUG
    const char *mcfr_flocs;
#else				/*!MELT_HAVE_DEBUG */
    const char *mcfr_unusedflocs;
#endif				/*MELT_HAVE_DEBUG */
    struct meltclosure_st *mcfr_clos;
    struct excepth_melt_st *mcfr_exh;
    struct melt_callframe_st *mcfr_prev;
#undef MELTFRAM_NBVARPTR
#undef MELTFRAM_NBVARNUM
#define MELTFRAM_NBVARPTR 10
    melt_ptr_t mcfr_varptr[10];
#define MELTFRAM_NBVARNUM 2
    long mcfr_varnum[2];
/*others*/
    long _spare_;
  }
   *meltframptr_ = 0, meltfram__;	/*declfrastruct */
#define meltframe meltfram__
  /*meltrout_49_WARMELTmiHOOKS_REGISTER_PRE_GENERICIZE_FIRST is not initial declstructinit */
  if (MELT_UNLIKELY (meltxargdescr_ == MELTPAR_MARKGGC))
    {				/*mark for ggc */
      int meltix;
      meltframptr_ =
	(struct
	 meltframe_meltrout_49_WARMELTmiHOOKS_REGISTER_PRE_GENERICIZE_FIRST_st
	 *) meltfirstargp_;
      /* use arguments meltrout_49_WARMELTmiHOOKS_REGISTER_PRE_GENERICIZE_FIRST output_curframe_declstruct_init */
      (void) meltclosp_;
      (void) meltfirstargp_;
      (void) meltxargdescr_;
      (void) meltxargtab_;
      (void) meltxresdescr_;
      (void) meltxrestab_;
      /* marking closure */
      gt_ggc_mx_melt_un (meltframptr_->mcfr_clos);
      for (meltix = 0; meltix < 10; meltix++)
	if (meltframptr_->mcfr_varptr[meltix])
	  gt_ggc_mx_melt_un (meltframptr_->mcfr_varptr[meltix]);
      return NULL;
    } /*end markggc */ ;
  memset (&meltfram__, 0, sizeof (meltfram__));
/* declstructinit plain routine meltrout_49_WARMELTmiHOOKS_REGISTER_PRE_GENERICIZE_FIRST nbval 10*/
  meltfram__.mcfr_nbvar = 10 /*nbval */ ;
  meltfram__.mcfr_clos = meltclosp_;
  meltfram__.mcfr_prev = (struct melt_callframe_st *) melt_topframe;
  melt_topframe = (struct melt_callframe_st *) &meltfram__;
  melt_trace_start ("REGISTER_PRE_GENERICIZE_FIRST", meltcallcount);
/*getargs*/

  /*getarg#0 */
  MELT_LOCATION ("warmelt-hooks.melt:1274:/ getarg");
 /*_.FUN__V2*/ meltfptr[1] = (melt_ptr_t) meltfirstargp_;
  ;
  goto meltlab_endgetargs;
meltlab_endgetargs:;
/*body*/
/*^block*/
/*anyblock*/
  {

    MELT_LOCATION ("warmelt-hooks.melt:1278:/ quasiblock");


    /*^cond */
    /*cond */ if (
		   /*ifisa */
		   melt_is_instance_of ((melt_ptr_t)
					(( /*!PREGENERICIZE_DELAYED_QUEUE */
					  meltfrout->tabval[0])),
					(melt_ptr_t) (( /*!CLASS_DELAYED_QUEUE */ meltfrout->tabval[1])))
      )				/*then */
      {
	/*^cond.then */
	/*^getslot */
	{
	  melt_ptr_t slot = NULL, obj = NULL;
	  obj =
	    (melt_ptr_t) (( /*!PREGENERICIZE_DELAYED_QUEUE */ meltfrout->
			   tabval[0])) /*=obj*/ ;
	  melt_object_get_field (slot, obj, 2, "DELQU_FIRST");
   /*_.FIRSTLIST__V4*/ meltfptr[3] = slot;
	};
	;
      }
    else
      {				/*^cond.else */

  /*_.FIRSTLIST__V4*/ meltfptr[3] = NULL;;
      }
    ;

    MELT_CHECK_SIGNAL ();
    ;
    /*^compute */
 /*_#NULL__L1*/ meltfnum[0] =
      (( /*_.FIRSTLIST__V4*/ meltfptr[3]) == NULL);;
    MELT_LOCATION ("warmelt-hooks.melt:1280:/ cond");
    /*cond */ if ( /*_#NULL__L1*/ meltfnum[0])	/*then */
      {
	/*^cond.then */
	/*^block */
	/*anyblock */
	{

   /*_.MAKE_LIST__V5*/ meltfptr[4] =
	    (meltgc_new_list
	     ((meltobject_ptr_t)
	      (( /*!DISCR_LIST */ meltfrout->tabval[2]))));;
	  MELT_LOCATION ("warmelt-hooks.melt:1282:/ compute");
	  /*_.FIRSTLIST__V4*/ meltfptr[3] = /*_.SETQ___V6*/ meltfptr[5] =
	    /*_.MAKE_LIST__V5*/ meltfptr[4];;

	  MELT_CHECK_SIGNAL ();
	  ;
   /*_.MAKE_LIST__V7*/ meltfptr[6] =
	    (meltgc_new_list
	     ((meltobject_ptr_t)
	      (( /*!DISCR_LIST */ meltfrout->tabval[2]))));;
	  MELT_LOCATION ("warmelt-hooks.melt:1283:/ cond");
	  /*cond */ if (
			 /*ifisa */
			 melt_is_instance_of ((melt_ptr_t)
					      (( /*!PREGENERICIZE_DELAYED_QUEUE */ meltfrout->tabval[0])),
					      (melt_ptr_t) (( /*!CLASS_DELAYED_QUEUE */ meltfrout->tabval[1])))
	    )			/*then */
	    {
	      /*^cond.then */
	      /*^block */
	      /*anyblock */
	      {

		/*^putslot */
		/*putslot */
		melt_assertmsg ("putslot checkobj @DELQU_FIRST",
				melt_magic_discr ((melt_ptr_t)
						  (( /*!PREGENERICIZE_DELAYED_QUEUE */ meltfrout->tabval[0]))) == MELTOBMAG_OBJECT);
		melt_putfield_object ((( /*!PREGENERICIZE_DELAYED_QUEUE */
					meltfrout->tabval[0])), (2),
				      ( /*_.FIRSTLIST__V4*/ meltfptr[3]),
				      "DELQU_FIRST");
		;
		/*^putslot */
		/*putslot */
		melt_assertmsg ("putslot checkobj @DELQU_LAST",
				melt_magic_discr ((melt_ptr_t)
						  (( /*!PREGENERICIZE_DELAYED_QUEUE */ meltfrout->tabval[0]))) == MELTOBMAG_OBJECT);
		melt_putfield_object ((( /*!PREGENERICIZE_DELAYED_QUEUE */
					meltfrout->tabval[0])), (3),
				      ( /*_.MAKE_LIST__V7*/ meltfptr[6]),
				      "DELQU_LAST");
		;
		/*^touch */
		meltgc_touch (( /*!PREGENERICIZE_DELAYED_QUEUE */ meltfrout->
			       tabval[0]));
		;
		/*^touchobj */

		melt_dbgtrace_written_object (( /*!PREGENERICIZE_DELAYED_QUEUE */ meltfrout->tabval[0]), "put-fields");
		;
		/*epilog */
	      }
	      ;
	    }			/*noelse */
	  ;
	  MELT_LOCATION ("warmelt-hooks.melt:1281:/ quasiblock");


	  /*epilog */

	  MELT_LOCATION ("warmelt-hooks.melt:1280:/ clear");
	     /*clear *//*_.MAKE_LIST__V5*/ meltfptr[4] = 0;
	  /*^clear */
	     /*clear *//*_.SETQ___V6*/ meltfptr[5] = 0;
	  /*^clear */
	     /*clear *//*_.MAKE_LIST__V7*/ meltfptr[6] = 0;
	}
	;
      }				/*noelse */
    ;

    MELT_CHECK_SIGNAL ();
    ;
 /*_#IS_CLOSURE__L2*/ meltfnum[1] =
      (melt_magic_discr ((melt_ptr_t) ( /*_.FUN__V2*/ meltfptr[1])) ==
       MELTOBMAG_CLOSURE);;
    MELT_LOCATION ("warmelt-hooks.melt:1288:/ cond");
    /*cond */ if ( /*_#IS_CLOSURE__L2*/ meltfnum[1])	/*then */
      {
	/*^cond.then */
	/*^block */
	/*anyblock */
	{


	  {
	    MELT_LOCATION ("warmelt-hooks.melt:1290:/ locexp");
	    meltgc_append_list ((melt_ptr_t)
				( /*_.FIRSTLIST__V4*/ meltfptr[3]),
				(melt_ptr_t) ( /*_.FUN__V2*/ meltfptr[1]));
	  }
	  ;

	  MELT_CHECK_SIGNAL ();
	  ;
	  MELT_LOCATION ("warmelt-hooks.melt:1291:/ apply");
	  /*apply */
	  {
	    /*_.ENABLE_PRE_GENERICIZE__V9*/ meltfptr[5] =
	      melt_apply ((meltclosure_ptr_t)
			  (( /*!ENABLE_PRE_GENERICIZE */ meltfrout->
			    tabval[3])), (melt_ptr_t) (NULL), (""),
			  (union meltparam_un *) 0, "",
			  (union meltparam_un *) 0);
	  }
	  ;
	  MELT_LOCATION ("warmelt-hooks.melt:1289:/ quasiblock");


	  /*_.PROGN___V10*/ meltfptr[6] =
	    /*_.ENABLE_PRE_GENERICIZE__V9*/ meltfptr[5];;
	  /*^compute */
	  /*_._IFELSE___V8*/ meltfptr[4] = /*_.PROGN___V10*/ meltfptr[6];;
	  /*epilog */

	  MELT_LOCATION ("warmelt-hooks.melt:1288:/ clear");
	     /*clear *//*_.ENABLE_PRE_GENERICIZE__V9*/ meltfptr[5] = 0;
	  /*^clear */
	     /*clear *//*_.PROGN___V10*/ meltfptr[6] = 0;
	}
	;
      }
    else
      {				/*^cond.else */

	/*^block */
	/*anyblock */
	{




	  {
	    MELT_LOCATION ("warmelt-hooks.melt:1292:/ locexp");
	    error ("MELT ERROR MSG [#%ld]::: %s", melt_dbgcounter,
		   ("Bad hook passed to register_pre_genericize_hook"));
	  }
	  ;
	     /*clear *//*_._IFELSE___V8*/ meltfptr[4] = 0;
	  /*epilog */
	}
	;
      }
    ;
    /*^compute */
    /*_.LET___V3*/ meltfptr[2] = /*_._IFELSE___V8*/ meltfptr[4];;

    MELT_LOCATION ("warmelt-hooks.melt:1278:/ clear");
	   /*clear *//*_.FIRSTLIST__V4*/ meltfptr[3] = 0;
    /*^clear */
	   /*clear *//*_#NULL__L1*/ meltfnum[0] = 0;
    /*^clear */
	   /*clear *//*_#IS_CLOSURE__L2*/ meltfnum[1] = 0;
    /*^clear */
	   /*clear *//*_._IFELSE___V8*/ meltfptr[4] = 0;
    MELT_LOCATION ("warmelt-hooks.melt:1274:/ quasiblock");


    /*_._RETVAL___V1*/ meltfptr[0] = /*_.LET___V3*/ meltfptr[2];;

    {
      MELT_LOCATION ("warmelt-hooks.melt:1274:/ locexp");
      /*ochecknores compilobj_nrep_return */
#if MELT_HAVE_DEBUG
      if (meltxresdescr_ && meltxresdescr_[0] && meltxrestab_)
	melt_warn_for_no_expected_secondary_results ();
/* we warned when secondary results are expected but not returned. */
#endif /*MELT_HAVE_DEBUG */
      ;
    }
    ;
    /*^finalreturn */
    ;
    /*finalret */ goto meltlabend_rout;
    /*epilog */

    /*^clear */
	   /*clear *//*_.LET___V3*/ meltfptr[2] = 0;
  }

  ;
  goto meltlabend_rout;
meltlabend_rout:
  melt_trace_end ("REGISTER_PRE_GENERICIZE_FIRST", meltcallcount);
  MELT_TRACE_EXIT_LOCATION ();
  melt_blocklevel_signals =
    current_blocklevel_signals_meltrout_49_WARMELTmiHOOKS_REGISTER_PRE_GENERICIZE_FIRST_melt;
  melt_topframe = (struct melt_callframe_st *) meltfram__.mcfr_prev;
  return (melt_ptr_t) ( /*_._RETVAL___V1*/ meltfptr[0]);
#undef meltcallcount
#undef meltfram__
#undef MELTFRAM_NBVARNUM
#undef MELTFRAM_NBVARPTR
}				/*end meltrout_49_WARMELTmiHOOKS_REGISTER_PRE_GENERICIZE_FIRST */





melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_50_WARMELTmiHOOKS_REGISTER_PRE_GENERICIZE_LAST (meltclosure_ptr_t
							 meltclosp_,
							 melt_ptr_t
							 meltfirstargp_,
							 const
							 melt_argdescr_cell_t
							 meltxargdescr_[],
							 union meltparam_un *
							 meltxargtab_,
							 const
							 melt_argdescr_cell_t
							 meltxresdescr_[],
							 union meltparam_un *
							 meltxrestab_)
{
  long
    current_blocklevel_signals_meltrout_50_WARMELTmiHOOKS_REGISTER_PRE_GENERICIZE_LAST_melt
    = melt_blocklevel_signals;

#if MELT_HAVE_DEBUG
  static long melt_call_counter__;
  long melt_thiscallcounter__ ATTRIBUTE_UNUSED = ++melt_call_counter__;
#undef meltcallcount
#define meltcallcount melt_thiscallcounter__
#else
#undef meltcallcount
#define meltcallcount 0L
#endif

  struct meltframe_meltrout_50_WARMELTmiHOOKS_REGISTER_PRE_GENERICIZE_LAST_st
  {
    int mcfr_nbvar;
#if MELT_HAVE_DEBUG
    const char *mcfr_flocs;
#else				/*!MELT_HAVE_DEBUG */
    const char *mcfr_unusedflocs;
#endif				/*MELT_HAVE_DEBUG */
    struct meltclosure_st *mcfr_clos;
    struct excepth_melt_st *mcfr_exh;
    struct melt_callframe_st *mcfr_prev;
#undef MELTFRAM_NBVARPTR
#undef MELTFRAM_NBVARNUM
#define MELTFRAM_NBVARPTR 10
    melt_ptr_t mcfr_varptr[10];
#define MELTFRAM_NBVARNUM 2
    long mcfr_varnum[2];
/*others*/
    long _spare_;
  }
   *meltframptr_ = 0, meltfram__;	/*declfrastruct */
#define meltframe meltfram__
  /*meltrout_50_WARMELTmiHOOKS_REGISTER_PRE_GENERICIZE_LAST is not initial declstructinit */
  if (MELT_UNLIKELY (meltxargdescr_ == MELTPAR_MARKGGC))
    {				/*mark for ggc */
      int meltix;
      meltframptr_ =
	(struct
	 meltframe_meltrout_50_WARMELTmiHOOKS_REGISTER_PRE_GENERICIZE_LAST_st
	 *) meltfirstargp_;
      /* use arguments meltrout_50_WARMELTmiHOOKS_REGISTER_PRE_GENERICIZE_LAST output_curframe_declstruct_init */
      (void) meltclosp_;
      (void) meltfirstargp_;
      (void) meltxargdescr_;
      (void) meltxargtab_;
      (void) meltxresdescr_;
      (void) meltxrestab_;
      /* marking closure */
      gt_ggc_mx_melt_un (meltframptr_->mcfr_clos);
      for (meltix = 0; meltix < 10; meltix++)
	if (meltframptr_->mcfr_varptr[meltix])
	  gt_ggc_mx_melt_un (meltframptr_->mcfr_varptr[meltix]);
      return NULL;
    } /*end markggc */ ;
  memset (&meltfram__, 0, sizeof (meltfram__));
/* declstructinit plain routine meltrout_50_WARMELTmiHOOKS_REGISTER_PRE_GENERICIZE_LAST nbval 10*/
  meltfram__.mcfr_nbvar = 10 /*nbval */ ;
  meltfram__.mcfr_clos = meltclosp_;
  meltfram__.mcfr_prev = (struct melt_callframe_st *) melt_topframe;
  melt_topframe = (struct melt_callframe_st *) &meltfram__;
  melt_trace_start ("REGISTER_PRE_GENERICIZE_LAST", meltcallcount);
/*getargs*/

  /*getarg#0 */
  MELT_LOCATION ("warmelt-hooks.melt:1297:/ getarg");
 /*_.FUN__V2*/ meltfptr[1] = (melt_ptr_t) meltfirstargp_;
  ;
  goto meltlab_endgetargs;
meltlab_endgetargs:;
/*body*/
/*^block*/
/*anyblock*/
  {

    MELT_LOCATION ("warmelt-hooks.melt:1301:/ quasiblock");


    /*^cond */
    /*cond */ if (
		   /*ifisa */
		   melt_is_instance_of ((melt_ptr_t)
					(( /*!PREGENERICIZE_DELAYED_QUEUE */
					  meltfrout->tabval[0])),
					(melt_ptr_t) (( /*!CLASS_DELAYED_QUEUE */ meltfrout->tabval[1])))
      )				/*then */
      {
	/*^cond.then */
	/*^getslot */
	{
	  melt_ptr_t slot = NULL, obj = NULL;
	  obj =
	    (melt_ptr_t) (( /*!PREGENERICIZE_DELAYED_QUEUE */ meltfrout->
			   tabval[0])) /*=obj*/ ;
	  melt_object_get_field (slot, obj, 2, "DELQU_FIRST");
   /*_.LASTLIST__V4*/ meltfptr[3] = slot;
	};
	;
      }
    else
      {				/*^cond.else */

  /*_.LASTLIST__V4*/ meltfptr[3] = NULL;;
      }
    ;

    MELT_CHECK_SIGNAL ();
    ;
    /*^compute */
 /*_#NULL__L1*/ meltfnum[0] =
      (( /*_.LASTLIST__V4*/ meltfptr[3]) == NULL);;
    MELT_LOCATION ("warmelt-hooks.melt:1303:/ cond");
    /*cond */ if ( /*_#NULL__L1*/ meltfnum[0])	/*then */
      {
	/*^cond.then */
	/*^block */
	/*anyblock */
	{

   /*_.MAKE_LIST__V5*/ meltfptr[4] =
	    (meltgc_new_list
	     ((meltobject_ptr_t)
	      (( /*!DISCR_LIST */ meltfrout->tabval[2]))));;
	  MELT_LOCATION ("warmelt-hooks.melt:1305:/ compute");
	  /*_.LASTLIST__V4*/ meltfptr[3] = /*_.SETQ___V6*/ meltfptr[5] =
	    /*_.MAKE_LIST__V5*/ meltfptr[4];;

	  MELT_CHECK_SIGNAL ();
	  ;
   /*_.MAKE_LIST__V7*/ meltfptr[6] =
	    (meltgc_new_list
	     ((meltobject_ptr_t)
	      (( /*!DISCR_LIST */ meltfrout->tabval[2]))));;
	  MELT_LOCATION ("warmelt-hooks.melt:1306:/ cond");
	  /*cond */ if (
			 /*ifisa */
			 melt_is_instance_of ((melt_ptr_t)
					      (( /*!PREGENERICIZE_DELAYED_QUEUE */ meltfrout->tabval[0])),
					      (melt_ptr_t) (( /*!CLASS_DELAYED_QUEUE */ meltfrout->tabval[1])))
	    )			/*then */
	    {
	      /*^cond.then */
	      /*^block */
	      /*anyblock */
	      {

		/*^putslot */
		/*putslot */
		melt_assertmsg ("putslot checkobj @DELQU_FIRST",
				melt_magic_discr ((melt_ptr_t)
						  (( /*!PREGENERICIZE_DELAYED_QUEUE */ meltfrout->tabval[0]))) == MELTOBMAG_OBJECT);
		melt_putfield_object ((( /*!PREGENERICIZE_DELAYED_QUEUE */
					meltfrout->tabval[0])), (2),
				      ( /*_.LASTLIST__V4*/ meltfptr[3]),
				      "DELQU_FIRST");
		;
		/*^putslot */
		/*putslot */
		melt_assertmsg ("putslot checkobj @DELQU_LAST",
				melt_magic_discr ((melt_ptr_t)
						  (( /*!PREGENERICIZE_DELAYED_QUEUE */ meltfrout->tabval[0]))) == MELTOBMAG_OBJECT);
		melt_putfield_object ((( /*!PREGENERICIZE_DELAYED_QUEUE */
					meltfrout->tabval[0])), (3),
				      ( /*_.MAKE_LIST__V7*/ meltfptr[6]),
				      "DELQU_LAST");
		;
		/*^touch */
		meltgc_touch (( /*!PREGENERICIZE_DELAYED_QUEUE */ meltfrout->
			       tabval[0]));
		;
		/*^touchobj */

		melt_dbgtrace_written_object (( /*!PREGENERICIZE_DELAYED_QUEUE */ meltfrout->tabval[0]), "put-fields");
		;
		/*epilog */
	      }
	      ;
	    }			/*noelse */
	  ;
	  MELT_LOCATION ("warmelt-hooks.melt:1304:/ quasiblock");


	  /*epilog */

	  MELT_LOCATION ("warmelt-hooks.melt:1303:/ clear");
	     /*clear *//*_.MAKE_LIST__V5*/ meltfptr[4] = 0;
	  /*^clear */
	     /*clear *//*_.SETQ___V6*/ meltfptr[5] = 0;
	  /*^clear */
	     /*clear *//*_.MAKE_LIST__V7*/ meltfptr[6] = 0;
	}
	;
      }				/*noelse */
    ;

    MELT_CHECK_SIGNAL ();
    ;
 /*_#IS_CLOSURE__L2*/ meltfnum[1] =
      (melt_magic_discr ((melt_ptr_t) ( /*_.FUN__V2*/ meltfptr[1])) ==
       MELTOBMAG_CLOSURE);;
    MELT_LOCATION ("warmelt-hooks.melt:1311:/ cond");
    /*cond */ if ( /*_#IS_CLOSURE__L2*/ meltfnum[1])	/*then */
      {
	/*^cond.then */
	/*^block */
	/*anyblock */
	{


	  {
	    MELT_LOCATION ("warmelt-hooks.melt:1313:/ locexp");
	    meltgc_append_list ((melt_ptr_t)
				( /*_.LASTLIST__V4*/ meltfptr[3]),
				(melt_ptr_t) ( /*_.FUN__V2*/ meltfptr[1]));
	  }
	  ;

	  MELT_CHECK_SIGNAL ();
	  ;
	  MELT_LOCATION ("warmelt-hooks.melt:1314:/ apply");
	  /*apply */
	  {
	    /*_.ENABLE_PRE_GENERICIZE__V9*/ meltfptr[5] =
	      melt_apply ((meltclosure_ptr_t)
			  (( /*!ENABLE_PRE_GENERICIZE */ meltfrout->
			    tabval[3])), (melt_ptr_t) (NULL), (""),
			  (union meltparam_un *) 0, "",
			  (union meltparam_un *) 0);
	  }
	  ;
	  MELT_LOCATION ("warmelt-hooks.melt:1312:/ quasiblock");


	  /*_.PROGN___V10*/ meltfptr[6] =
	    /*_.ENABLE_PRE_GENERICIZE__V9*/ meltfptr[5];;
	  /*^compute */
	  /*_._IFELSE___V8*/ meltfptr[4] = /*_.PROGN___V10*/ meltfptr[6];;
	  /*epilog */

	  MELT_LOCATION ("warmelt-hooks.melt:1311:/ clear");
	     /*clear *//*_.ENABLE_PRE_GENERICIZE__V9*/ meltfptr[5] = 0;
	  /*^clear */
	     /*clear *//*_.PROGN___V10*/ meltfptr[6] = 0;
	}
	;
      }
    else
      {				/*^cond.else */

	/*^block */
	/*anyblock */
	{




	  {
	    MELT_LOCATION ("warmelt-hooks.melt:1315:/ locexp");
	    error ("MELT ERROR MSG [#%ld]::: %s", melt_dbgcounter,
		   ("Bad hook passed to register_pre_genericize_hook"));
	  }
	  ;
	     /*clear *//*_._IFELSE___V8*/ meltfptr[4] = 0;
	  /*epilog */
	}
	;
      }
    ;
    /*^compute */
    /*_.LET___V3*/ meltfptr[2] = /*_._IFELSE___V8*/ meltfptr[4];;

    MELT_LOCATION ("warmelt-hooks.melt:1301:/ clear");
	   /*clear *//*_.LASTLIST__V4*/ meltfptr[3] = 0;
    /*^clear */
	   /*clear *//*_#NULL__L1*/ meltfnum[0] = 0;
    /*^clear */
	   /*clear *//*_#IS_CLOSURE__L2*/ meltfnum[1] = 0;
    /*^clear */
	   /*clear *//*_._IFELSE___V8*/ meltfptr[4] = 0;
    MELT_LOCATION ("warmelt-hooks.melt:1297:/ quasiblock");


    /*_._RETVAL___V1*/ meltfptr[0] = /*_.LET___V3*/ meltfptr[2];;

    {
      MELT_LOCATION ("warmelt-hooks.melt:1297:/ locexp");
      /*ochecknores compilobj_nrep_return */
#if MELT_HAVE_DEBUG
      if (meltxresdescr_ && meltxresdescr_[0] && meltxrestab_)
	melt_warn_for_no_expected_secondary_results ();
/* we warned when secondary results are expected but not returned. */
#endif /*MELT_HAVE_DEBUG */
      ;
    }
    ;
    /*^finalreturn */
    ;
    /*finalret */ goto meltlabend_rout;
    /*epilog */

    /*^clear */
	   /*clear *//*_.LET___V3*/ meltfptr[2] = 0;
  }

  ;
  goto meltlabend_rout;
meltlabend_rout:
  melt_trace_end ("REGISTER_PRE_GENERICIZE_LAST", meltcallcount);
  MELT_TRACE_EXIT_LOCATION ();
  melt_blocklevel_signals =
    current_blocklevel_signals_meltrout_50_WARMELTmiHOOKS_REGISTER_PRE_GENERICIZE_LAST_melt;
  melt_topframe = (struct melt_callframe_st *) meltfram__.mcfr_prev;
  return (melt_ptr_t) ( /*_._RETVAL___V1*/ meltfptr[0]);
#undef meltcallcount
#undef meltfram__
#undef MELTFRAM_NBVARNUM
#undef MELTFRAM_NBVARPTR
}				/*end meltrout_50_WARMELTmiHOOKS_REGISTER_PRE_GENERICIZE_LAST */





melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_51_WARMELTmiHOOKS_ENABLE_FINISH_TYPE (meltclosure_ptr_t meltclosp_,
					       melt_ptr_t meltfirstargp_,
					       const melt_argdescr_cell_t
					       meltxargdescr_[],
					       union meltparam_un *
					       meltxargtab_,
					       const melt_argdescr_cell_t
					       meltxresdescr_[],
					       union meltparam_un *
					       meltxrestab_)
{
  long
    current_blocklevel_signals_meltrout_51_WARMELTmiHOOKS_ENABLE_FINISH_TYPE_melt
    = melt_blocklevel_signals;

#if MELT_HAVE_DEBUG
  static long melt_call_counter__;
  long melt_thiscallcounter__ ATTRIBUTE_UNUSED = ++melt_call_counter__;
#undef meltcallcount
#define meltcallcount melt_thiscallcounter__
#else
#undef meltcallcount
#define meltcallcount 0L
#endif

  struct meltframe_meltrout_51_WARMELTmiHOOKS_ENABLE_FINISH_TYPE_st
  {
    int mcfr_nbvar;
#if MELT_HAVE_DEBUG
    const char *mcfr_flocs;
#else				/*!MELT_HAVE_DEBUG */
    const char *mcfr_unusedflocs;
#endif				/*MELT_HAVE_DEBUG */
    struct meltclosure_st *mcfr_clos;
    struct excepth_melt_st *mcfr_exh;
    struct melt_callframe_st *mcfr_prev;
#undef MELTFRAM_NBVARPTR
#undef MELTFRAM_NBVARNUM
#define MELTFRAM_NBVARPTR 1
    melt_ptr_t mcfr_varptr[1];
/*no varnum*/
#define MELTFRAM_NBVARNUM /*none*/0
/*others*/
    long _spare_;
  }
   *meltframptr_ = 0, meltfram__;	/*declfrastruct */
#define meltframe meltfram__
  /*meltrout_51_WARMELTmiHOOKS_ENABLE_FINISH_TYPE is not initial declstructinit */
  if (MELT_UNLIKELY (meltxargdescr_ == MELTPAR_MARKGGC))
    {				/*mark for ggc */
      int meltix;
      meltframptr_ =
	(struct meltframe_meltrout_51_WARMELTmiHOOKS_ENABLE_FINISH_TYPE_st *)
	meltfirstargp_;
      /* use arguments meltrout_51_WARMELTmiHOOKS_ENABLE_FINISH_TYPE output_curframe_declstruct_init */
      (void) meltclosp_;
      (void) meltfirstargp_;
      (void) meltxargdescr_;
      (void) meltxargtab_;
      (void) meltxresdescr_;
      (void) meltxrestab_;
      /* marking closure */
      gt_ggc_mx_melt_un (meltframptr_->mcfr_clos);
      for (meltix = 0; meltix < 1; meltix++)
	if (meltframptr_->mcfr_varptr[meltix])
	  gt_ggc_mx_melt_un (meltframptr_->mcfr_varptr[meltix]);
      return NULL;
    } /*end markggc */ ;
  memset (&meltfram__, 0, sizeof (meltfram__));
/* declstructinit plain routine meltrout_51_WARMELTmiHOOKS_ENABLE_FINISH_TYPE nbval 1*/
  meltfram__.mcfr_nbvar = 1 /*nbval */ ;
  meltfram__.mcfr_clos = meltclosp_;
  meltfram__.mcfr_prev = (struct melt_callframe_st *) melt_topframe;
  melt_topframe = (struct melt_callframe_st *) &meltfram__;
  melt_trace_start ("ENABLE_FINISH_TYPE", meltcallcount);
/*getargs*/
  ;
  goto meltlab_endgetargs;
meltlab_endgetargs:;
/*body*/
  MELT_LOCATION ("warmelt-hooks.melt:1360:/ block");
/*anyblock*/
  {


    {
      MELT_LOCATION ("warmelt-hooks.melt:1361:/ locexp");
      /* enable_finish_type ENABLEPREGEN_CHK__2 */
      if (!melthk_finish_type_registered_flag)
	{
	  melthk_finish_type_registered_flag = true;
	  register_callback (melt_plugin_name, PLUGIN_FINISH_TYPE,
			     melt_finish_type_hook_callback, NULL);
	}
      ;
    }
    ;
    /*epilog */
  }

  ;
  goto meltlabend_rout;
meltlabend_rout:
  melt_trace_end ("ENABLE_FINISH_TYPE", meltcallcount);
  MELT_TRACE_EXIT_LOCATION ();
  melt_blocklevel_signals =
    current_blocklevel_signals_meltrout_51_WARMELTmiHOOKS_ENABLE_FINISH_TYPE_melt;
  melt_topframe = (struct melt_callframe_st *) meltfram__.mcfr_prev;
  return (melt_ptr_t) ( /*noretval */ NULL);
#undef meltcallcount
#undef meltfram__
#undef MELTFRAM_NBVARNUM
#undef MELTFRAM_NBVARPTR
}				/*end meltrout_51_WARMELTmiHOOKS_ENABLE_FINISH_TYPE */





melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_53_WARMELTmiHOOKS_REGISTER_FINISH_TYPE_FIRST (meltclosure_ptr_t
						       meltclosp_,
						       melt_ptr_t
						       meltfirstargp_,
						       const
						       melt_argdescr_cell_t
						       meltxargdescr_[],
						       union meltparam_un *
						       meltxargtab_,
						       const
						       melt_argdescr_cell_t
						       meltxresdescr_[],
						       union meltparam_un *
						       meltxrestab_)
{
  long
    current_blocklevel_signals_meltrout_53_WARMELTmiHOOKS_REGISTER_FINISH_TYPE_FIRST_melt
    = melt_blocklevel_signals;

#if MELT_HAVE_DEBUG
  static long melt_call_counter__;
  long melt_thiscallcounter__ ATTRIBUTE_UNUSED = ++melt_call_counter__;
#undef meltcallcount
#define meltcallcount melt_thiscallcounter__
#else
#undef meltcallcount
#define meltcallcount 0L
#endif

  struct meltframe_meltrout_53_WARMELTmiHOOKS_REGISTER_FINISH_TYPE_FIRST_st
  {
    int mcfr_nbvar;
#if MELT_HAVE_DEBUG
    const char *mcfr_flocs;
#else				/*!MELT_HAVE_DEBUG */
    const char *mcfr_unusedflocs;
#endif				/*MELT_HAVE_DEBUG */
    struct meltclosure_st *mcfr_clos;
    struct excepth_melt_st *mcfr_exh;
    struct melt_callframe_st *mcfr_prev;
#undef MELTFRAM_NBVARPTR
#undef MELTFRAM_NBVARNUM
#define MELTFRAM_NBVARPTR 10
    melt_ptr_t mcfr_varptr[10];
#define MELTFRAM_NBVARNUM 2
    long mcfr_varnum[2];
/*others*/
    long _spare_;
  }
   *meltframptr_ = 0, meltfram__;	/*declfrastruct */
#define meltframe meltfram__
  /*meltrout_53_WARMELTmiHOOKS_REGISTER_FINISH_TYPE_FIRST is not initial declstructinit */
  if (MELT_UNLIKELY (meltxargdescr_ == MELTPAR_MARKGGC))
    {				/*mark for ggc */
      int meltix;
      meltframptr_ =
	(struct
	 meltframe_meltrout_53_WARMELTmiHOOKS_REGISTER_FINISH_TYPE_FIRST_st *)
	meltfirstargp_;
      /* use arguments meltrout_53_WARMELTmiHOOKS_REGISTER_FINISH_TYPE_FIRST output_curframe_declstruct_init */
      (void) meltclosp_;
      (void) meltfirstargp_;
      (void) meltxargdescr_;
      (void) meltxargtab_;
      (void) meltxresdescr_;
      (void) meltxrestab_;
      /* marking closure */
      gt_ggc_mx_melt_un (meltframptr_->mcfr_clos);
      for (meltix = 0; meltix < 10; meltix++)
	if (meltframptr_->mcfr_varptr[meltix])
	  gt_ggc_mx_melt_un (meltframptr_->mcfr_varptr[meltix]);
      return NULL;
    } /*end markggc */ ;
  memset (&meltfram__, 0, sizeof (meltfram__));
/* declstructinit plain routine meltrout_53_WARMELTmiHOOKS_REGISTER_FINISH_TYPE_FIRST nbval 10*/
  meltfram__.mcfr_nbvar = 10 /*nbval */ ;
  meltfram__.mcfr_clos = meltclosp_;
  meltfram__.mcfr_prev = (struct melt_callframe_st *) melt_topframe;
  melt_topframe = (struct melt_callframe_st *) &meltfram__;
  melt_trace_start ("REGISTER_FINISH_TYPE_FIRST", meltcallcount);
/*getargs*/

  /*getarg#0 */
  MELT_LOCATION ("warmelt-hooks.melt:1403:/ getarg");
 /*_.FUN__V2*/ meltfptr[1] = (melt_ptr_t) meltfirstargp_;
  ;
  goto meltlab_endgetargs;
meltlab_endgetargs:;
/*body*/
/*^block*/
/*anyblock*/
  {

    MELT_LOCATION ("warmelt-hooks.melt:1407:/ quasiblock");


    /*^cond */
    /*cond */ if (
		   /*ifisa */
		   melt_is_instance_of ((melt_ptr_t)
					(( /*!FINISHTYPE_DELAYED_QUEUE */
					  meltfrout->tabval[0])),
					(melt_ptr_t) (( /*!CLASS_DELAYED_QUEUE */ meltfrout->tabval[1])))
      )				/*then */
      {
	/*^cond.then */
	/*^getslot */
	{
	  melt_ptr_t slot = NULL, obj = NULL;
	  obj =
	    (melt_ptr_t) (( /*!FINISHTYPE_DELAYED_QUEUE */ meltfrout->
			   tabval[0])) /*=obj*/ ;
	  melt_object_get_field (slot, obj, 2, "DELQU_FIRST");
   /*_.FIRSTLIST__V4*/ meltfptr[3] = slot;
	};
	;
      }
    else
      {				/*^cond.else */

  /*_.FIRSTLIST__V4*/ meltfptr[3] = NULL;;
      }
    ;

    MELT_CHECK_SIGNAL ();
    ;
    /*^compute */
 /*_#NULL__L1*/ meltfnum[0] =
      (( /*_.FIRSTLIST__V4*/ meltfptr[3]) == NULL);;
    MELT_LOCATION ("warmelt-hooks.melt:1409:/ cond");
    /*cond */ if ( /*_#NULL__L1*/ meltfnum[0])	/*then */
      {
	/*^cond.then */
	/*^block */
	/*anyblock */
	{

   /*_.MAKE_LIST__V5*/ meltfptr[4] =
	    (meltgc_new_list
	     ((meltobject_ptr_t)
	      (( /*!DISCR_LIST */ meltfrout->tabval[2]))));;
	  MELT_LOCATION ("warmelt-hooks.melt:1411:/ compute");
	  /*_.FIRSTLIST__V4*/ meltfptr[3] = /*_.SETQ___V6*/ meltfptr[5] =
	    /*_.MAKE_LIST__V5*/ meltfptr[4];;

	  MELT_CHECK_SIGNAL ();
	  ;
   /*_.MAKE_LIST__V7*/ meltfptr[6] =
	    (meltgc_new_list
	     ((meltobject_ptr_t)
	      (( /*!DISCR_LIST */ meltfrout->tabval[2]))));;
	  MELT_LOCATION ("warmelt-hooks.melt:1412:/ cond");
	  /*cond */ if (
			 /*ifisa */
			 melt_is_instance_of ((melt_ptr_t)
					      (( /*!FINISHTYPE_DELAYED_QUEUE */ meltfrout->tabval[0])),
					      (melt_ptr_t) (( /*!CLASS_DELAYED_QUEUE */ meltfrout->tabval[1])))
	    )			/*then */
	    {
	      /*^cond.then */
	      /*^block */
	      /*anyblock */
	      {

		/*^putslot */
		/*putslot */
		melt_assertmsg ("putslot checkobj @DELQU_FIRST",
				melt_magic_discr ((melt_ptr_t)
						  (( /*!FINISHTYPE_DELAYED_QUEUE */ meltfrout->tabval[0]))) == MELTOBMAG_OBJECT);
		melt_putfield_object ((( /*!FINISHTYPE_DELAYED_QUEUE */
					meltfrout->tabval[0])), (2),
				      ( /*_.FIRSTLIST__V4*/ meltfptr[3]),
				      "DELQU_FIRST");
		;
		/*^putslot */
		/*putslot */
		melt_assertmsg ("putslot checkobj @DELQU_LAST",
				melt_magic_discr ((melt_ptr_t)
						  (( /*!FINISHTYPE_DELAYED_QUEUE */ meltfrout->tabval[0]))) == MELTOBMAG_OBJECT);
		melt_putfield_object ((( /*!FINISHTYPE_DELAYED_QUEUE */
					meltfrout->tabval[0])), (3),
				      ( /*_.MAKE_LIST__V7*/ meltfptr[6]),
				      "DELQU_LAST");
		;
		/*^touch */
		meltgc_touch (( /*!FINISHTYPE_DELAYED_QUEUE */ meltfrout->
			       tabval[0]));
		;
		/*^touchobj */

		melt_dbgtrace_written_object (( /*!FINISHTYPE_DELAYED_QUEUE */
					       meltfrout->tabval[0]),
					      "put-fields");
		;
		/*epilog */
	      }
	      ;
	    }			/*noelse */
	  ;
	  MELT_LOCATION ("warmelt-hooks.melt:1410:/ quasiblock");


	  /*epilog */

	  MELT_LOCATION ("warmelt-hooks.melt:1409:/ clear");
	     /*clear *//*_.MAKE_LIST__V5*/ meltfptr[4] = 0;
	  /*^clear */
	     /*clear *//*_.SETQ___V6*/ meltfptr[5] = 0;
	  /*^clear */
	     /*clear *//*_.MAKE_LIST__V7*/ meltfptr[6] = 0;
	}
	;
      }				/*noelse */
    ;

    MELT_CHECK_SIGNAL ();
    ;
 /*_#IS_CLOSURE__L2*/ meltfnum[1] =
      (melt_magic_discr ((melt_ptr_t) ( /*_.FUN__V2*/ meltfptr[1])) ==
       MELTOBMAG_CLOSURE);;
    MELT_LOCATION ("warmelt-hooks.melt:1417:/ cond");
    /*cond */ if ( /*_#IS_CLOSURE__L2*/ meltfnum[1])	/*then */
      {
	/*^cond.then */
	/*^block */
	/*anyblock */
	{


	  {
	    MELT_LOCATION ("warmelt-hooks.melt:1419:/ locexp");
	    meltgc_append_list ((melt_ptr_t)
				( /*_.FIRSTLIST__V4*/ meltfptr[3]),
				(melt_ptr_t) ( /*_.FUN__V2*/ meltfptr[1]));
	  }
	  ;

	  MELT_CHECK_SIGNAL ();
	  ;
	  MELT_LOCATION ("warmelt-hooks.melt:1420:/ apply");
	  /*apply */
	  {
	    /*_.ENABLE_FINISH_TYPE__V9*/ meltfptr[5] =
	      melt_apply ((meltclosure_ptr_t)
			  (( /*!ENABLE_FINISH_TYPE */ meltfrout->tabval[3])),
			  (melt_ptr_t) (NULL), (""), (union meltparam_un *) 0,
			  "", (union meltparam_un *) 0);
	  }
	  ;
	  MELT_LOCATION ("warmelt-hooks.melt:1418:/ quasiblock");


	  /*_.PROGN___V10*/ meltfptr[6] =
	    /*_.ENABLE_FINISH_TYPE__V9*/ meltfptr[5];;
	  /*^compute */
	  /*_._IFELSE___V8*/ meltfptr[4] = /*_.PROGN___V10*/ meltfptr[6];;
	  /*epilog */

	  MELT_LOCATION ("warmelt-hooks.melt:1417:/ clear");
	     /*clear *//*_.ENABLE_FINISH_TYPE__V9*/ meltfptr[5] = 0;
	  /*^clear */
	     /*clear *//*_.PROGN___V10*/ meltfptr[6] = 0;
	}
	;
      }
    else
      {				/*^cond.else */

	/*^block */
	/*anyblock */
	{




	  {
	    MELT_LOCATION ("warmelt-hooks.melt:1421:/ locexp");
	    error ("MELT ERROR MSG [#%ld]::: %s", melt_dbgcounter,
		   ("Bad hook passed to register_finish_type_first"));
	  }
	  ;
	     /*clear *//*_._IFELSE___V8*/ meltfptr[4] = 0;
	  /*epilog */
	}
	;
      }
    ;
    /*^compute */
    /*_.LET___V3*/ meltfptr[2] = /*_._IFELSE___V8*/ meltfptr[4];;

    MELT_LOCATION ("warmelt-hooks.melt:1407:/ clear");
	   /*clear *//*_.FIRSTLIST__V4*/ meltfptr[3] = 0;
    /*^clear */
	   /*clear *//*_#NULL__L1*/ meltfnum[0] = 0;
    /*^clear */
	   /*clear *//*_#IS_CLOSURE__L2*/ meltfnum[1] = 0;
    /*^clear */
	   /*clear *//*_._IFELSE___V8*/ meltfptr[4] = 0;
    MELT_LOCATION ("warmelt-hooks.melt:1403:/ quasiblock");


    /*_._RETVAL___V1*/ meltfptr[0] = /*_.LET___V3*/ meltfptr[2];;

    {
      MELT_LOCATION ("warmelt-hooks.melt:1403:/ locexp");
      /*ochecknores compilobj_nrep_return */
#if MELT_HAVE_DEBUG
      if (meltxresdescr_ && meltxresdescr_[0] && meltxrestab_)
	melt_warn_for_no_expected_secondary_results ();
/* we warned when secondary results are expected but not returned. */
#endif /*MELT_HAVE_DEBUG */
      ;
    }
    ;
    /*^finalreturn */
    ;
    /*finalret */ goto meltlabend_rout;
    /*epilog */

    /*^clear */
	   /*clear *//*_.LET___V3*/ meltfptr[2] = 0;
  }

  ;
  goto meltlabend_rout;
meltlabend_rout:
  melt_trace_end ("REGISTER_FINISH_TYPE_FIRST", meltcallcount);
  MELT_TRACE_EXIT_LOCATION ();
  melt_blocklevel_signals =
    current_blocklevel_signals_meltrout_53_WARMELTmiHOOKS_REGISTER_FINISH_TYPE_FIRST_melt;
  melt_topframe = (struct melt_callframe_st *) meltfram__.mcfr_prev;
  return (melt_ptr_t) ( /*_._RETVAL___V1*/ meltfptr[0]);
#undef meltcallcount
#undef meltfram__
#undef MELTFRAM_NBVARNUM
#undef MELTFRAM_NBVARPTR
}				/*end meltrout_53_WARMELTmiHOOKS_REGISTER_FINISH_TYPE_FIRST */





melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_54_WARMELTmiHOOKS_REGISTER_FINISH_TYPE_LAST (meltclosure_ptr_t
						      meltclosp_,
						      melt_ptr_t
						      meltfirstargp_,
						      const
						      melt_argdescr_cell_t
						      meltxargdescr_[],
						      union meltparam_un *
						      meltxargtab_,
						      const
						      melt_argdescr_cell_t
						      meltxresdescr_[],
						      union meltparam_un *
						      meltxrestab_)
{
  long
    current_blocklevel_signals_meltrout_54_WARMELTmiHOOKS_REGISTER_FINISH_TYPE_LAST_melt
    = melt_blocklevel_signals;

#if MELT_HAVE_DEBUG
  static long melt_call_counter__;
  long melt_thiscallcounter__ ATTRIBUTE_UNUSED = ++melt_call_counter__;
#undef meltcallcount
#define meltcallcount melt_thiscallcounter__
#else
#undef meltcallcount
#define meltcallcount 0L
#endif

  struct meltframe_meltrout_54_WARMELTmiHOOKS_REGISTER_FINISH_TYPE_LAST_st
  {
    int mcfr_nbvar;
#if MELT_HAVE_DEBUG
    const char *mcfr_flocs;
#else				/*!MELT_HAVE_DEBUG */
    const char *mcfr_unusedflocs;
#endif				/*MELT_HAVE_DEBUG */
    struct meltclosure_st *mcfr_clos;
    struct excepth_melt_st *mcfr_exh;
    struct melt_callframe_st *mcfr_prev;
#undef MELTFRAM_NBVARPTR
#undef MELTFRAM_NBVARNUM
#define MELTFRAM_NBVARPTR 10
    melt_ptr_t mcfr_varptr[10];
#define MELTFRAM_NBVARNUM 2
    long mcfr_varnum[2];
/*others*/
    long _spare_;
  }
   *meltframptr_ = 0, meltfram__;	/*declfrastruct */
#define meltframe meltfram__
  /*meltrout_54_WARMELTmiHOOKS_REGISTER_FINISH_TYPE_LAST is not initial declstructinit */
  if (MELT_UNLIKELY (meltxargdescr_ == MELTPAR_MARKGGC))
    {				/*mark for ggc */
      int meltix;
      meltframptr_ =
	(struct
	 meltframe_meltrout_54_WARMELTmiHOOKS_REGISTER_FINISH_TYPE_LAST_st *)
	meltfirstargp_;
      /* use arguments meltrout_54_WARMELTmiHOOKS_REGISTER_FINISH_TYPE_LAST output_curframe_declstruct_init */
      (void) meltclosp_;
      (void) meltfirstargp_;
      (void) meltxargdescr_;
      (void) meltxargtab_;
      (void) meltxresdescr_;
      (void) meltxrestab_;
      /* marking closure */
      gt_ggc_mx_melt_un (meltframptr_->mcfr_clos);
      for (meltix = 0; meltix < 10; meltix++)
	if (meltframptr_->mcfr_varptr[meltix])
	  gt_ggc_mx_melt_un (meltframptr_->mcfr_varptr[meltix]);
      return NULL;
    } /*end markggc */ ;
  memset (&meltfram__, 0, sizeof (meltfram__));
/* declstructinit plain routine meltrout_54_WARMELTmiHOOKS_REGISTER_FINISH_TYPE_LAST nbval 10*/
  meltfram__.mcfr_nbvar = 10 /*nbval */ ;
  meltfram__.mcfr_clos = meltclosp_;
  meltfram__.mcfr_prev = (struct melt_callframe_st *) melt_topframe;
  melt_topframe = (struct melt_callframe_st *) &meltfram__;
  melt_trace_start ("REGISTER_FINISH_TYPE_LAST", meltcallcount);
/*getargs*/

  /*getarg#0 */
  MELT_LOCATION ("warmelt-hooks.melt:1426:/ getarg");
 /*_.FUN__V2*/ meltfptr[1] = (melt_ptr_t) meltfirstargp_;
  ;
  goto meltlab_endgetargs;
meltlab_endgetargs:;
/*body*/
/*^block*/
/*anyblock*/
  {

    MELT_LOCATION ("warmelt-hooks.melt:1430:/ quasiblock");


    /*^cond */
    /*cond */ if (
		   /*ifisa */
		   melt_is_instance_of ((melt_ptr_t)
					(( /*!FINISHTYPE_DELAYED_QUEUE */
					  meltfrout->tabval[0])),
					(melt_ptr_t) (( /*!CLASS_DELAYED_QUEUE */ meltfrout->tabval[1])))
      )				/*then */
      {
	/*^cond.then */
	/*^getslot */
	{
	  melt_ptr_t slot = NULL, obj = NULL;
	  obj =
	    (melt_ptr_t) (( /*!FINISHTYPE_DELAYED_QUEUE */ meltfrout->
			   tabval[0])) /*=obj*/ ;
	  melt_object_get_field (slot, obj, 2, "DELQU_FIRST");
   /*_.LASTLIST__V4*/ meltfptr[3] = slot;
	};
	;
      }
    else
      {				/*^cond.else */

  /*_.LASTLIST__V4*/ meltfptr[3] = NULL;;
      }
    ;

    MELT_CHECK_SIGNAL ();
    ;
    /*^compute */
 /*_#NULL__L1*/ meltfnum[0] =
      (( /*_.LASTLIST__V4*/ meltfptr[3]) == NULL);;
    MELT_LOCATION ("warmelt-hooks.melt:1432:/ cond");
    /*cond */ if ( /*_#NULL__L1*/ meltfnum[0])	/*then */
      {
	/*^cond.then */
	/*^block */
	/*anyblock */
	{

   /*_.MAKE_LIST__V5*/ meltfptr[4] =
	    (meltgc_new_list
	     ((meltobject_ptr_t)
	      (( /*!DISCR_LIST */ meltfrout->tabval[2]))));;
	  MELT_LOCATION ("warmelt-hooks.melt:1434:/ compute");
	  /*_.LASTLIST__V4*/ meltfptr[3] = /*_.SETQ___V6*/ meltfptr[5] =
	    /*_.MAKE_LIST__V5*/ meltfptr[4];;

	  MELT_CHECK_SIGNAL ();
	  ;
   /*_.MAKE_LIST__V7*/ meltfptr[6] =
	    (meltgc_new_list
	     ((meltobject_ptr_t)
	      (( /*!DISCR_LIST */ meltfrout->tabval[2]))));;
	  MELT_LOCATION ("warmelt-hooks.melt:1435:/ cond");
	  /*cond */ if (
			 /*ifisa */
			 melt_is_instance_of ((melt_ptr_t)
					      (( /*!FINISHTYPE_DELAYED_QUEUE */ meltfrout->tabval[0])),
					      (melt_ptr_t) (( /*!CLASS_DELAYED_QUEUE */ meltfrout->tabval[1])))
	    )			/*then */
	    {
	      /*^cond.then */
	      /*^block */
	      /*anyblock */
	      {

		/*^putslot */
		/*putslot */
		melt_assertmsg ("putslot checkobj @DELQU_FIRST",
				melt_magic_discr ((melt_ptr_t)
						  (( /*!FINISHTYPE_DELAYED_QUEUE */ meltfrout->tabval[0]))) == MELTOBMAG_OBJECT);
		melt_putfield_object ((( /*!FINISHTYPE_DELAYED_QUEUE */
					meltfrout->tabval[0])), (2),
				      ( /*_.LASTLIST__V4*/ meltfptr[3]),
				      "DELQU_FIRST");
		;
		/*^putslot */
		/*putslot */
		melt_assertmsg ("putslot checkobj @DELQU_LAST",
				melt_magic_discr ((melt_ptr_t)
						  (( /*!FINISHTYPE_DELAYED_QUEUE */ meltfrout->tabval[0]))) == MELTOBMAG_OBJECT);
		melt_putfield_object ((( /*!FINISHTYPE_DELAYED_QUEUE */
					meltfrout->tabval[0])), (3),
				      ( /*_.MAKE_LIST__V7*/ meltfptr[6]),
				      "DELQU_LAST");
		;
		/*^touch */
		meltgc_touch (( /*!FINISHTYPE_DELAYED_QUEUE */ meltfrout->
			       tabval[0]));
		;
		/*^touchobj */

		melt_dbgtrace_written_object (( /*!FINISHTYPE_DELAYED_QUEUE */
					       meltfrout->tabval[0]),
					      "put-fields");
		;
		/*epilog */
	      }
	      ;
	    }			/*noelse */
	  ;
	  MELT_LOCATION ("warmelt-hooks.melt:1433:/ quasiblock");


	  /*epilog */

	  MELT_LOCATION ("warmelt-hooks.melt:1432:/ clear");
	     /*clear *//*_.MAKE_LIST__V5*/ meltfptr[4] = 0;
	  /*^clear */
	     /*clear *//*_.SETQ___V6*/ meltfptr[5] = 0;
	  /*^clear */
	     /*clear *//*_.MAKE_LIST__V7*/ meltfptr[6] = 0;
	}
	;
      }				/*noelse */
    ;

    MELT_CHECK_SIGNAL ();
    ;
 /*_#IS_CLOSURE__L2*/ meltfnum[1] =
      (melt_magic_discr ((melt_ptr_t) ( /*_.FUN__V2*/ meltfptr[1])) ==
       MELTOBMAG_CLOSURE);;
    MELT_LOCATION ("warmelt-hooks.melt:1440:/ cond");
    /*cond */ if ( /*_#IS_CLOSURE__L2*/ meltfnum[1])	/*then */
      {
	/*^cond.then */
	/*^block */
	/*anyblock */
	{


	  {
	    MELT_LOCATION ("warmelt-hooks.melt:1442:/ locexp");
	    meltgc_append_list ((melt_ptr_t)
				( /*_.LASTLIST__V4*/ meltfptr[3]),
				(melt_ptr_t) ( /*_.FUN__V2*/ meltfptr[1]));
	  }
	  ;

	  MELT_CHECK_SIGNAL ();
	  ;
	  MELT_LOCATION ("warmelt-hooks.melt:1443:/ apply");
	  /*apply */
	  {
	    /*_.ENABLE_FINISH_TYPE__V9*/ meltfptr[5] =
	      melt_apply ((meltclosure_ptr_t)
			  (( /*!ENABLE_FINISH_TYPE */ meltfrout->tabval[3])),
			  (melt_ptr_t) (NULL), (""), (union meltparam_un *) 0,
			  "", (union meltparam_un *) 0);
	  }
	  ;
	  MELT_LOCATION ("warmelt-hooks.melt:1441:/ quasiblock");


	  /*_.PROGN___V10*/ meltfptr[6] =
	    /*_.ENABLE_FINISH_TYPE__V9*/ meltfptr[5];;
	  /*^compute */
	  /*_._IFELSE___V8*/ meltfptr[4] = /*_.PROGN___V10*/ meltfptr[6];;
	  /*epilog */

	  MELT_LOCATION ("warmelt-hooks.melt:1440:/ clear");
	     /*clear *//*_.ENABLE_FINISH_TYPE__V9*/ meltfptr[5] = 0;
	  /*^clear */
	     /*clear *//*_.PROGN___V10*/ meltfptr[6] = 0;
	}
	;
      }
    else
      {				/*^cond.else */

	/*^block */
	/*anyblock */
	{




	  {
	    MELT_LOCATION ("warmelt-hooks.melt:1444:/ locexp");
	    error ("MELT ERROR MSG [#%ld]::: %s", melt_dbgcounter,
		   ("Bad hook passed to register_finish_type_last"));
	  }
	  ;
	     /*clear *//*_._IFELSE___V8*/ meltfptr[4] = 0;
	  /*epilog */
	}
	;
      }
    ;
    /*^compute */
    /*_.LET___V3*/ meltfptr[2] = /*_._IFELSE___V8*/ meltfptr[4];;

    MELT_LOCATION ("warmelt-hooks.melt:1430:/ clear");
	   /*clear *//*_.LASTLIST__V4*/ meltfptr[3] = 0;
    /*^clear */
	   /*clear *//*_#NULL__L1*/ meltfnum[0] = 0;
    /*^clear */
	   /*clear *//*_#IS_CLOSURE__L2*/ meltfnum[1] = 0;
    /*^clear */
	   /*clear *//*_._IFELSE___V8*/ meltfptr[4] = 0;
    MELT_LOCATION ("warmelt-hooks.melt:1426:/ quasiblock");


    /*_._RETVAL___V1*/ meltfptr[0] = /*_.LET___V3*/ meltfptr[2];;

    {
      MELT_LOCATION ("warmelt-hooks.melt:1426:/ locexp");
      /*ochecknores compilobj_nrep_return */
#if MELT_HAVE_DEBUG
      if (meltxresdescr_ && meltxresdescr_[0] && meltxrestab_)
	melt_warn_for_no_expected_secondary_results ();
/* we warned when secondary results are expected but not returned. */
#endif /*MELT_HAVE_DEBUG */
      ;
    }
    ;
    /*^finalreturn */
    ;
    /*finalret */ goto meltlabend_rout;
    /*epilog */

    /*^clear */
	   /*clear *//*_.LET___V3*/ meltfptr[2] = 0;
  }

  ;
  goto meltlabend_rout;
meltlabend_rout:
  melt_trace_end ("REGISTER_FINISH_TYPE_LAST", meltcallcount);
  MELT_TRACE_EXIT_LOCATION ();
  melt_blocklevel_signals =
    current_blocklevel_signals_meltrout_54_WARMELTmiHOOKS_REGISTER_FINISH_TYPE_LAST_melt;
  melt_topframe = (struct melt_callframe_st *) meltfram__.mcfr_prev;
  return (melt_ptr_t) ( /*_._RETVAL___V1*/ meltfptr[0]);
#undef meltcallcount
#undef meltfram__
#undef MELTFRAM_NBVARNUM
#undef MELTFRAM_NBVARPTR
}				/*end meltrout_54_WARMELTmiHOOKS_REGISTER_FINISH_TYPE_LAST */





melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_55_WARMELTmiHOOKS_ENABLE_FINISH_DECL (meltclosure_ptr_t meltclosp_,
					       melt_ptr_t meltfirstargp_,
					       const melt_argdescr_cell_t
					       meltxargdescr_[],
					       union meltparam_un *
					       meltxargtab_,
					       const melt_argdescr_cell_t
					       meltxresdescr_[],
					       union meltparam_un *
					       meltxrestab_)
{
  long
    current_blocklevel_signals_meltrout_55_WARMELTmiHOOKS_ENABLE_FINISH_DECL_melt
    = melt_blocklevel_signals;

#if MELT_HAVE_DEBUG
  static long melt_call_counter__;
  long melt_thiscallcounter__ ATTRIBUTE_UNUSED = ++melt_call_counter__;
#undef meltcallcount
#define meltcallcount melt_thiscallcounter__
#else
#undef meltcallcount
#define meltcallcount 0L
#endif

  struct meltframe_meltrout_55_WARMELTmiHOOKS_ENABLE_FINISH_DECL_st
  {
    int mcfr_nbvar;
#if MELT_HAVE_DEBUG
    const char *mcfr_flocs;
#else				/*!MELT_HAVE_DEBUG */
    const char *mcfr_unusedflocs;
#endif				/*MELT_HAVE_DEBUG */
    struct meltclosure_st *mcfr_clos;
    struct excepth_melt_st *mcfr_exh;
    struct melt_callframe_st *mcfr_prev;
#undef MELTFRAM_NBVARPTR
#undef MELTFRAM_NBVARNUM
#define MELTFRAM_NBVARPTR 1
    melt_ptr_t mcfr_varptr[1];
/*no varnum*/
#define MELTFRAM_NBVARNUM /*none*/0
/*others*/
    long _spare_;
  }
   *meltframptr_ = 0, meltfram__;	/*declfrastruct */
#define meltframe meltfram__
  /*meltrout_55_WARMELTmiHOOKS_ENABLE_FINISH_DECL is not initial declstructinit */
  if (MELT_UNLIKELY (meltxargdescr_ == MELTPAR_MARKGGC))
    {				/*mark for ggc */
      int meltix;
      meltframptr_ =
	(struct meltframe_meltrout_55_WARMELTmiHOOKS_ENABLE_FINISH_DECL_st *)
	meltfirstargp_;
      /* use arguments meltrout_55_WARMELTmiHOOKS_ENABLE_FINISH_DECL output_curframe_declstruct_init */
      (void) meltclosp_;
      (void) meltfirstargp_;
      (void) meltxargdescr_;
      (void) meltxargtab_;
      (void) meltxresdescr_;
      (void) meltxrestab_;
      /* marking closure */
      gt_ggc_mx_melt_un (meltframptr_->mcfr_clos);
      for (meltix = 0; meltix < 1; meltix++)
	if (meltframptr_->mcfr_varptr[meltix])
	  gt_ggc_mx_melt_un (meltframptr_->mcfr_varptr[meltix]);
      return NULL;
    } /*end markggc */ ;
  memset (&meltfram__, 0, sizeof (meltfram__));
/* declstructinit plain routine meltrout_55_WARMELTmiHOOKS_ENABLE_FINISH_DECL nbval 1*/
  meltfram__.mcfr_nbvar = 1 /*nbval */ ;
  meltfram__.mcfr_clos = meltclosp_;
  meltfram__.mcfr_prev = (struct melt_callframe_st *) melt_topframe;
  melt_topframe = (struct melt_callframe_st *) &meltfram__;
  melt_trace_start ("ENABLE_FINISH_DECL", meltcallcount);
/*getargs*/
  ;
  goto meltlab_endgetargs;
meltlab_endgetargs:;
/*body*/
  MELT_LOCATION ("warmelt-hooks.melt:1488:/ block");
/*anyblock*/
  {


    {
      MELT_LOCATION ("warmelt-hooks.melt:1489:/ locexp");
      /* enable_finish_decl ENABLEPREGEN_CHK__3 */
      if (!melthk_finish_decl_registered_flag)
	{
	  melthk_finish_decl_registered_flag = true;
#if MELT_GCC_VERSION < 4007
	  melt_fatal_error ("PLUGIN_FINISH_DECL does not exist in GCC 4.6");
#else
	  register_callback (melt_plugin_name, PLUGIN_FINISH_DECL,
			     melt_finish_decl_hook_callback, NULL);
#endif
	}
      ;
    }
    ;
    /*epilog */
  }

  ;
  goto meltlabend_rout;
meltlabend_rout:
  melt_trace_end ("ENABLE_FINISH_DECL", meltcallcount);
  MELT_TRACE_EXIT_LOCATION ();
  melt_blocklevel_signals =
    current_blocklevel_signals_meltrout_55_WARMELTmiHOOKS_ENABLE_FINISH_DECL_melt;
  melt_topframe = (struct melt_callframe_st *) meltfram__.mcfr_prev;
  return (melt_ptr_t) ( /*noretval */ NULL);
#undef meltcallcount
#undef meltfram__
#undef MELTFRAM_NBVARNUM
#undef MELTFRAM_NBVARPTR
}				/*end meltrout_55_WARMELTmiHOOKS_ENABLE_FINISH_DECL */





melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_57_WARMELTmiHOOKS_REGISTER_FINISH_DECL_FIRST (meltclosure_ptr_t
						       meltclosp_,
						       melt_ptr_t
						       meltfirstargp_,
						       const
						       melt_argdescr_cell_t
						       meltxargdescr_[],
						       union meltparam_un *
						       meltxargtab_,
						       const
						       melt_argdescr_cell_t
						       meltxresdescr_[],
						       union meltparam_un *
						       meltxrestab_)
{
  long
    current_blocklevel_signals_meltrout_57_WARMELTmiHOOKS_REGISTER_FINISH_DECL_FIRST_melt
    = melt_blocklevel_signals;

#if MELT_HAVE_DEBUG
  static long melt_call_counter__;
  long melt_thiscallcounter__ ATTRIBUTE_UNUSED = ++melt_call_counter__;
#undef meltcallcount
#define meltcallcount melt_thiscallcounter__
#else
#undef meltcallcount
#define meltcallcount 0L
#endif

  struct meltframe_meltrout_57_WARMELTmiHOOKS_REGISTER_FINISH_DECL_FIRST_st
  {
    int mcfr_nbvar;
#if MELT_HAVE_DEBUG
    const char *mcfr_flocs;
#else				/*!MELT_HAVE_DEBUG */
    const char *mcfr_unusedflocs;
#endif				/*MELT_HAVE_DEBUG */
    struct meltclosure_st *mcfr_clos;
    struct excepth_melt_st *mcfr_exh;
    struct melt_callframe_st *mcfr_prev;
#undef MELTFRAM_NBVARPTR
#undef MELTFRAM_NBVARNUM
#define MELTFRAM_NBVARPTR 10
    melt_ptr_t mcfr_varptr[10];
#define MELTFRAM_NBVARNUM 2
    long mcfr_varnum[2];
/*others*/
    long _spare_;
  }
   *meltframptr_ = 0, meltfram__;	/*declfrastruct */
#define meltframe meltfram__
  /*meltrout_57_WARMELTmiHOOKS_REGISTER_FINISH_DECL_FIRST is not initial declstructinit */
  if (MELT_UNLIKELY (meltxargdescr_ == MELTPAR_MARKGGC))
    {				/*mark for ggc */
      int meltix;
      meltframptr_ =
	(struct
	 meltframe_meltrout_57_WARMELTmiHOOKS_REGISTER_FINISH_DECL_FIRST_st *)
	meltfirstargp_;
      /* use arguments meltrout_57_WARMELTmiHOOKS_REGISTER_FINISH_DECL_FIRST output_curframe_declstruct_init */
      (void) meltclosp_;
      (void) meltfirstargp_;
      (void) meltxargdescr_;
      (void) meltxargtab_;
      (void) meltxresdescr_;
      (void) meltxrestab_;
      /* marking closure */
      gt_ggc_mx_melt_un (meltframptr_->mcfr_clos);
      for (meltix = 0; meltix < 10; meltix++)
	if (meltframptr_->mcfr_varptr[meltix])
	  gt_ggc_mx_melt_un (meltframptr_->mcfr_varptr[meltix]);
      return NULL;
    } /*end markggc */ ;
  memset (&meltfram__, 0, sizeof (meltfram__));
/* declstructinit plain routine meltrout_57_WARMELTmiHOOKS_REGISTER_FINISH_DECL_FIRST nbval 10*/
  meltfram__.mcfr_nbvar = 10 /*nbval */ ;
  meltfram__.mcfr_clos = meltclosp_;
  meltfram__.mcfr_prev = (struct melt_callframe_st *) melt_topframe;
  melt_topframe = (struct melt_callframe_st *) &meltfram__;
  melt_trace_start ("REGISTER_FINISH_DECL_FIRST", meltcallcount);
/*getargs*/

  /*getarg#0 */
  MELT_LOCATION ("warmelt-hooks.melt:1535:/ getarg");
 /*_.FUN__V2*/ meltfptr[1] = (melt_ptr_t) meltfirstargp_;
  ;
  goto meltlab_endgetargs;
meltlab_endgetargs:;
/*body*/
/*^block*/
/*anyblock*/
  {

    MELT_LOCATION ("warmelt-hooks.melt:1539:/ quasiblock");


    /*^cond */
    /*cond */ if (
		   /*ifisa */
		   melt_is_instance_of ((melt_ptr_t)
					(( /*!FINISHDECL_DELAYED_QUEUE */
					  meltfrout->tabval[0])),
					(melt_ptr_t) (( /*!CLASS_DELAYED_QUEUE */ meltfrout->tabval[1])))
      )				/*then */
      {
	/*^cond.then */
	/*^getslot */
	{
	  melt_ptr_t slot = NULL, obj = NULL;
	  obj =
	    (melt_ptr_t) (( /*!FINISHDECL_DELAYED_QUEUE */ meltfrout->
			   tabval[0])) /*=obj*/ ;
	  melt_object_get_field (slot, obj, 2, "DELQU_FIRST");
   /*_.FIRSTLIST__V4*/ meltfptr[3] = slot;
	};
	;
      }
    else
      {				/*^cond.else */

  /*_.FIRSTLIST__V4*/ meltfptr[3] = NULL;;
      }
    ;

    MELT_CHECK_SIGNAL ();
    ;
    /*^compute */
 /*_#NULL__L1*/ meltfnum[0] =
      (( /*_.FIRSTLIST__V4*/ meltfptr[3]) == NULL);;
    MELT_LOCATION ("warmelt-hooks.melt:1541:/ cond");
    /*cond */ if ( /*_#NULL__L1*/ meltfnum[0])	/*then */
      {
	/*^cond.then */
	/*^block */
	/*anyblock */
	{

   /*_.MAKE_LIST__V5*/ meltfptr[4] =
	    (meltgc_new_list
	     ((meltobject_ptr_t)
	      (( /*!DISCR_LIST */ meltfrout->tabval[2]))));;
	  MELT_LOCATION ("warmelt-hooks.melt:1543:/ compute");
	  /*_.FIRSTLIST__V4*/ meltfptr[3] = /*_.SETQ___V6*/ meltfptr[5] =
	    /*_.MAKE_LIST__V5*/ meltfptr[4];;

	  MELT_CHECK_SIGNAL ();
	  ;
   /*_.MAKE_LIST__V7*/ meltfptr[6] =
	    (meltgc_new_list
	     ((meltobject_ptr_t)
	      (( /*!DISCR_LIST */ meltfrout->tabval[2]))));;
	  MELT_LOCATION ("warmelt-hooks.melt:1544:/ cond");
	  /*cond */ if (
			 /*ifisa */
			 melt_is_instance_of ((melt_ptr_t)
					      (( /*!FINISHDECL_DELAYED_QUEUE */ meltfrout->tabval[0])),
					      (melt_ptr_t) (( /*!CLASS_DELAYED_QUEUE */ meltfrout->tabval[1])))
	    )			/*then */
	    {
	      /*^cond.then */
	      /*^block */
	      /*anyblock */
	      {

		/*^putslot */
		/*putslot */
		melt_assertmsg ("putslot checkobj @DELQU_FIRST",
				melt_magic_discr ((melt_ptr_t)
						  (( /*!FINISHDECL_DELAYED_QUEUE */ meltfrout->tabval[0]))) == MELTOBMAG_OBJECT);
		melt_putfield_object ((( /*!FINISHDECL_DELAYED_QUEUE */
					meltfrout->tabval[0])), (2),
				      ( /*_.FIRSTLIST__V4*/ meltfptr[3]),
				      "DELQU_FIRST");
		;
		/*^putslot */
		/*putslot */
		melt_assertmsg ("putslot checkobj @DELQU_LAST",
				melt_magic_discr ((melt_ptr_t)
						  (( /*!FINISHDECL_DELAYED_QUEUE */ meltfrout->tabval[0]))) == MELTOBMAG_OBJECT);
		melt_putfield_object ((( /*!FINISHDECL_DELAYED_QUEUE */
					meltfrout->tabval[0])), (3),
				      ( /*_.MAKE_LIST__V7*/ meltfptr[6]),
				      "DELQU_LAST");
		;
		/*^touch */
		meltgc_touch (( /*!FINISHDECL_DELAYED_QUEUE */ meltfrout->
			       tabval[0]));
		;
		/*^touchobj */

		melt_dbgtrace_written_object (( /*!FINISHDECL_DELAYED_QUEUE */
					       meltfrout->tabval[0]),
					      "put-fields");
		;
		/*epilog */
	      }
	      ;
	    }			/*noelse */
	  ;
	  MELT_LOCATION ("warmelt-hooks.melt:1542:/ quasiblock");


	  /*epilog */

	  MELT_LOCATION ("warmelt-hooks.melt:1541:/ clear");
	     /*clear *//*_.MAKE_LIST__V5*/ meltfptr[4] = 0;
	  /*^clear */
	     /*clear *//*_.SETQ___V6*/ meltfptr[5] = 0;
	  /*^clear */
	     /*clear *//*_.MAKE_LIST__V7*/ meltfptr[6] = 0;
	}
	;
      }				/*noelse */
    ;

    MELT_CHECK_SIGNAL ();
    ;
 /*_#IS_CLOSURE__L2*/ meltfnum[1] =
      (melt_magic_discr ((melt_ptr_t) ( /*_.FUN__V2*/ meltfptr[1])) ==
       MELTOBMAG_CLOSURE);;
    MELT_LOCATION ("warmelt-hooks.melt:1549:/ cond");
    /*cond */ if ( /*_#IS_CLOSURE__L2*/ meltfnum[1])	/*then */
      {
	/*^cond.then */
	/*^block */
	/*anyblock */
	{


	  {
	    MELT_LOCATION ("warmelt-hooks.melt:1551:/ locexp");
	    meltgc_append_list ((melt_ptr_t)
				( /*_.FIRSTLIST__V4*/ meltfptr[3]),
				(melt_ptr_t) ( /*_.FUN__V2*/ meltfptr[1]));
	  }
	  ;

	  MELT_CHECK_SIGNAL ();
	  ;
	  MELT_LOCATION ("warmelt-hooks.melt:1552:/ apply");
	  /*apply */
	  {
	    /*_.ENABLE_FINISH_DECL__V9*/ meltfptr[5] =
	      melt_apply ((meltclosure_ptr_t)
			  (( /*!ENABLE_FINISH_DECL */ meltfrout->tabval[3])),
			  (melt_ptr_t) (NULL), (""), (union meltparam_un *) 0,
			  "", (union meltparam_un *) 0);
	  }
	  ;
	  MELT_LOCATION ("warmelt-hooks.melt:1550:/ quasiblock");


	  /*_.PROGN___V10*/ meltfptr[6] =
	    /*_.ENABLE_FINISH_DECL__V9*/ meltfptr[5];;
	  /*^compute */
	  /*_._IFELSE___V8*/ meltfptr[4] = /*_.PROGN___V10*/ meltfptr[6];;
	  /*epilog */

	  MELT_LOCATION ("warmelt-hooks.melt:1549:/ clear");
	     /*clear *//*_.ENABLE_FINISH_DECL__V9*/ meltfptr[5] = 0;
	  /*^clear */
	     /*clear *//*_.PROGN___V10*/ meltfptr[6] = 0;
	}
	;
      }
    else
      {				/*^cond.else */

	/*^block */
	/*anyblock */
	{




	  {
	    MELT_LOCATION ("warmelt-hooks.melt:1553:/ locexp");
	    error ("MELT ERROR MSG [#%ld]::: %s", melt_dbgcounter,
		   ("Bad hook passed to register_finish_decl_first"));
	  }
	  ;
	     /*clear *//*_._IFELSE___V8*/ meltfptr[4] = 0;
	  /*epilog */
	}
	;
      }
    ;
    /*^compute */
    /*_.LET___V3*/ meltfptr[2] = /*_._IFELSE___V8*/ meltfptr[4];;

    MELT_LOCATION ("warmelt-hooks.melt:1539:/ clear");
	   /*clear *//*_.FIRSTLIST__V4*/ meltfptr[3] = 0;
    /*^clear */
	   /*clear *//*_#NULL__L1*/ meltfnum[0] = 0;
    /*^clear */
	   /*clear *//*_#IS_CLOSURE__L2*/ meltfnum[1] = 0;
    /*^clear */
	   /*clear *//*_._IFELSE___V8*/ meltfptr[4] = 0;
    MELT_LOCATION ("warmelt-hooks.melt:1535:/ quasiblock");


    /*_._RETVAL___V1*/ meltfptr[0] = /*_.LET___V3*/ meltfptr[2];;

    {
      MELT_LOCATION ("warmelt-hooks.melt:1535:/ locexp");
      /*ochecknores compilobj_nrep_return */
#if MELT_HAVE_DEBUG
      if (meltxresdescr_ && meltxresdescr_[0] && meltxrestab_)
	melt_warn_for_no_expected_secondary_results ();
/* we warned when secondary results are expected but not returned. */
#endif /*MELT_HAVE_DEBUG */
      ;
    }
    ;
    /*^finalreturn */
    ;
    /*finalret */ goto meltlabend_rout;
    /*epilog */

    /*^clear */
	   /*clear *//*_.LET___V3*/ meltfptr[2] = 0;
  }

  ;
  goto meltlabend_rout;
meltlabend_rout:
  melt_trace_end ("REGISTER_FINISH_DECL_FIRST", meltcallcount);
  MELT_TRACE_EXIT_LOCATION ();
  melt_blocklevel_signals =
    current_blocklevel_signals_meltrout_57_WARMELTmiHOOKS_REGISTER_FINISH_DECL_FIRST_melt;
  melt_topframe = (struct melt_callframe_st *) meltfram__.mcfr_prev;
  return (melt_ptr_t) ( /*_._RETVAL___V1*/ meltfptr[0]);
#undef meltcallcount
#undef meltfram__
#undef MELTFRAM_NBVARNUM
#undef MELTFRAM_NBVARPTR
}				/*end meltrout_57_WARMELTmiHOOKS_REGISTER_FINISH_DECL_FIRST */





melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_58_WARMELTmiHOOKS_REGISTER_FINISH_DECL_LAST (meltclosure_ptr_t
						      meltclosp_,
						      melt_ptr_t
						      meltfirstargp_,
						      const
						      melt_argdescr_cell_t
						      meltxargdescr_[],
						      union meltparam_un *
						      meltxargtab_,
						      const
						      melt_argdescr_cell_t
						      meltxresdescr_[],
						      union meltparam_un *
						      meltxrestab_)
{
  long
    current_blocklevel_signals_meltrout_58_WARMELTmiHOOKS_REGISTER_FINISH_DECL_LAST_melt
    = melt_blocklevel_signals;

#if MELT_HAVE_DEBUG
  static long melt_call_counter__;
  long melt_thiscallcounter__ ATTRIBUTE_UNUSED = ++melt_call_counter__;
#undef meltcallcount
#define meltcallcount melt_thiscallcounter__
#else
#undef meltcallcount
#define meltcallcount 0L
#endif

  struct meltframe_meltrout_58_WARMELTmiHOOKS_REGISTER_FINISH_DECL_LAST_st
  {
    int mcfr_nbvar;
#if MELT_HAVE_DEBUG
    const char *mcfr_flocs;
#else				/*!MELT_HAVE_DEBUG */
    const char *mcfr_unusedflocs;
#endif				/*MELT_HAVE_DEBUG */
    struct meltclosure_st *mcfr_clos;
    struct excepth_melt_st *mcfr_exh;
    struct melt_callframe_st *mcfr_prev;
#undef MELTFRAM_NBVARPTR
#undef MELTFRAM_NBVARNUM
#define MELTFRAM_NBVARPTR 10
    melt_ptr_t mcfr_varptr[10];
#define MELTFRAM_NBVARNUM 2
    long mcfr_varnum[2];
/*others*/
    long _spare_;
  }
   *meltframptr_ = 0, meltfram__;	/*declfrastruct */
#define meltframe meltfram__
  /*meltrout_58_WARMELTmiHOOKS_REGISTER_FINISH_DECL_LAST is not initial declstructinit */
  if (MELT_UNLIKELY (meltxargdescr_ == MELTPAR_MARKGGC))
    {				/*mark for ggc */
      int meltix;
      meltframptr_ =
	(struct
	 meltframe_meltrout_58_WARMELTmiHOOKS_REGISTER_FINISH_DECL_LAST_st *)
	meltfirstargp_;
      /* use arguments meltrout_58_WARMELTmiHOOKS_REGISTER_FINISH_DECL_LAST output_curframe_declstruct_init */
      (void) meltclosp_;
      (void) meltfirstargp_;
      (void) meltxargdescr_;
      (void) meltxargtab_;
      (void) meltxresdescr_;
      (void) meltxrestab_;
      /* marking closure */
      gt_ggc_mx_melt_un (meltframptr_->mcfr_clos);
      for (meltix = 0; meltix < 10; meltix++)
	if (meltframptr_->mcfr_varptr[meltix])
	  gt_ggc_mx_melt_un (meltframptr_->mcfr_varptr[meltix]);
      return NULL;
    } /*end markggc */ ;
  memset (&meltfram__, 0, sizeof (meltfram__));
/* declstructinit plain routine meltrout_58_WARMELTmiHOOKS_REGISTER_FINISH_DECL_LAST nbval 10*/
  meltfram__.mcfr_nbvar = 10 /*nbval */ ;
  meltfram__.mcfr_clos = meltclosp_;
  meltfram__.mcfr_prev = (struct melt_callframe_st *) melt_topframe;
  melt_topframe = (struct melt_callframe_st *) &meltfram__;
  melt_trace_start ("REGISTER_FINISH_DECL_LAST", meltcallcount);
/*getargs*/

  /*getarg#0 */
  MELT_LOCATION ("warmelt-hooks.melt:1558:/ getarg");
 /*_.FUN__V2*/ meltfptr[1] = (melt_ptr_t) meltfirstargp_;
  ;
  goto meltlab_endgetargs;
meltlab_endgetargs:;
/*body*/
/*^block*/
/*anyblock*/
  {

    MELT_LOCATION ("warmelt-hooks.melt:1562:/ quasiblock");


    /*^cond */
    /*cond */ if (
		   /*ifisa */
		   melt_is_instance_of ((melt_ptr_t)
					(( /*!FINISHDECL_DELAYED_QUEUE */
					  meltfrout->tabval[0])),
					(melt_ptr_t) (( /*!CLASS_DELAYED_QUEUE */ meltfrout->tabval[1])))
      )				/*then */
      {
	/*^cond.then */
	/*^getslot */
	{
	  melt_ptr_t slot = NULL, obj = NULL;
	  obj =
	    (melt_ptr_t) (( /*!FINISHDECL_DELAYED_QUEUE */ meltfrout->
			   tabval[0])) /*=obj*/ ;
	  melt_object_get_field (slot, obj, 2, "DELQU_FIRST");
   /*_.LASTLIST__V4*/ meltfptr[3] = slot;
	};
	;
      }
    else
      {				/*^cond.else */

  /*_.LASTLIST__V4*/ meltfptr[3] = NULL;;
      }
    ;

    MELT_CHECK_SIGNAL ();
    ;
    /*^compute */
 /*_#NULL__L1*/ meltfnum[0] =
      (( /*_.LASTLIST__V4*/ meltfptr[3]) == NULL);;
    MELT_LOCATION ("warmelt-hooks.melt:1564:/ cond");
    /*cond */ if ( /*_#NULL__L1*/ meltfnum[0])	/*then */
      {
	/*^cond.then */
	/*^block */
	/*anyblock */
	{

   /*_.MAKE_LIST__V5*/ meltfptr[4] =
	    (meltgc_new_list
	     ((meltobject_ptr_t)
	      (( /*!DISCR_LIST */ meltfrout->tabval[2]))));;
	  MELT_LOCATION ("warmelt-hooks.melt:1566:/ compute");
	  /*_.LASTLIST__V4*/ meltfptr[3] = /*_.SETQ___V6*/ meltfptr[5] =
	    /*_.MAKE_LIST__V5*/ meltfptr[4];;

	  MELT_CHECK_SIGNAL ();
	  ;
   /*_.MAKE_LIST__V7*/ meltfptr[6] =
	    (meltgc_new_list
	     ((meltobject_ptr_t)
	      (( /*!DISCR_LIST */ meltfrout->tabval[2]))));;
	  MELT_LOCATION ("warmelt-hooks.melt:1567:/ cond");
	  /*cond */ if (
			 /*ifisa */
			 melt_is_instance_of ((melt_ptr_t)
					      (( /*!FINISHDECL_DELAYED_QUEUE */ meltfrout->tabval[0])),
					      (melt_ptr_t) (( /*!CLASS_DELAYED_QUEUE */ meltfrout->tabval[1])))
	    )			/*then */
	    {
	      /*^cond.then */
	      /*^block */
	      /*anyblock */
	      {

		/*^putslot */
		/*putslot */
		melt_assertmsg ("putslot checkobj @DELQU_FIRST",
				melt_magic_discr ((melt_ptr_t)
						  (( /*!FINISHDECL_DELAYED_QUEUE */ meltfrout->tabval[0]))) == MELTOBMAG_OBJECT);
		melt_putfield_object ((( /*!FINISHDECL_DELAYED_QUEUE */
					meltfrout->tabval[0])), (2),
				      ( /*_.LASTLIST__V4*/ meltfptr[3]),
				      "DELQU_FIRST");
		;
		/*^putslot */
		/*putslot */
		melt_assertmsg ("putslot checkobj @DELQU_LAST",
				melt_magic_discr ((melt_ptr_t)
						  (( /*!FINISHDECL_DELAYED_QUEUE */ meltfrout->tabval[0]))) == MELTOBMAG_OBJECT);
		melt_putfield_object ((( /*!FINISHDECL_DELAYED_QUEUE */
					meltfrout->tabval[0])), (3),
				      ( /*_.MAKE_LIST__V7*/ meltfptr[6]),
				      "DELQU_LAST");
		;
		/*^touch */
		meltgc_touch (( /*!FINISHDECL_DELAYED_QUEUE */ meltfrout->
			       tabval[0]));
		;
		/*^touchobj */

		melt_dbgtrace_written_object (( /*!FINISHDECL_DELAYED_QUEUE */
					       meltfrout->tabval[0]),
					      "put-fields");
		;
		/*epilog */
	      }
	      ;
	    }			/*noelse */
	  ;
	  MELT_LOCATION ("warmelt-hooks.melt:1565:/ quasiblock");


	  /*epilog */

	  MELT_LOCATION ("warmelt-hooks.melt:1564:/ clear");
	     /*clear *//*_.MAKE_LIST__V5*/ meltfptr[4] = 0;
	  /*^clear */
	     /*clear *//*_.SETQ___V6*/ meltfptr[5] = 0;
	  /*^clear */
	     /*clear *//*_.MAKE_LIST__V7*/ meltfptr[6] = 0;
	}
	;
      }				/*noelse */
    ;

    MELT_CHECK_SIGNAL ();
    ;
 /*_#IS_CLOSURE__L2*/ meltfnum[1] =
      (melt_magic_discr ((melt_ptr_t) ( /*_.FUN__V2*/ meltfptr[1])) ==
       MELTOBMAG_CLOSURE);;
    MELT_LOCATION ("warmelt-hooks.melt:1572:/ cond");
    /*cond */ if ( /*_#IS_CLOSURE__L2*/ meltfnum[1])	/*then */
      {
	/*^cond.then */
	/*^block */
	/*anyblock */
	{


	  {
	    MELT_LOCATION ("warmelt-hooks.melt:1574:/ locexp");
	    meltgc_append_list ((melt_ptr_t)
				( /*_.LASTLIST__V4*/ meltfptr[3]),
				(melt_ptr_t) ( /*_.FUN__V2*/ meltfptr[1]));
	  }
	  ;

	  MELT_CHECK_SIGNAL ();
	  ;
	  MELT_LOCATION ("warmelt-hooks.melt:1575:/ apply");
	  /*apply */
	  {
	    /*_.ENABLE_FINISH_DECL__V9*/ meltfptr[5] =
	      melt_apply ((meltclosure_ptr_t)
			  (( /*!ENABLE_FINISH_DECL */ meltfrout->tabval[3])),
			  (melt_ptr_t) (NULL), (""), (union meltparam_un *) 0,
			  "", (union meltparam_un *) 0);
	  }
	  ;
	  MELT_LOCATION ("warmelt-hooks.melt:1573:/ quasiblock");


	  /*_.PROGN___V10*/ meltfptr[6] =
	    /*_.ENABLE_FINISH_DECL__V9*/ meltfptr[5];;
	  /*^compute */
	  /*_._IFELSE___V8*/ meltfptr[4] = /*_.PROGN___V10*/ meltfptr[6];;
	  /*epilog */

	  MELT_LOCATION ("warmelt-hooks.melt:1572:/ clear");
	     /*clear *//*_.ENABLE_FINISH_DECL__V9*/ meltfptr[5] = 0;
	  /*^clear */
	     /*clear *//*_.PROGN___V10*/ meltfptr[6] = 0;
	}
	;
      }
    else
      {				/*^cond.else */

	/*^block */
	/*anyblock */
	{




	  {
	    MELT_LOCATION ("warmelt-hooks.melt:1576:/ locexp");
	    error ("MELT ERROR MSG [#%ld]::: %s", melt_dbgcounter,
		   ("Bad hook passed to register_finish_decl_last"));
	  }
	  ;
	     /*clear *//*_._IFELSE___V8*/ meltfptr[4] = 0;
	  /*epilog */
	}
	;
      }
    ;
    /*^compute */
    /*_.LET___V3*/ meltfptr[2] = /*_._IFELSE___V8*/ meltfptr[4];;

    MELT_LOCATION ("warmelt-hooks.melt:1562:/ clear");
	   /*clear *//*_.LASTLIST__V4*/ meltfptr[3] = 0;
    /*^clear */
	   /*clear *//*_#NULL__L1*/ meltfnum[0] = 0;
    /*^clear */
	   /*clear *//*_#IS_CLOSURE__L2*/ meltfnum[1] = 0;
    /*^clear */
	   /*clear *//*_._IFELSE___V8*/ meltfptr[4] = 0;
    MELT_LOCATION ("warmelt-hooks.melt:1558:/ quasiblock");


    /*_._RETVAL___V1*/ meltfptr[0] = /*_.LET___V3*/ meltfptr[2];;

    {
      MELT_LOCATION ("warmelt-hooks.melt:1558:/ locexp");
      /*ochecknores compilobj_nrep_return */
#if MELT_HAVE_DEBUG
      if (meltxresdescr_ && meltxresdescr_[0] && meltxrestab_)
	melt_warn_for_no_expected_secondary_results ();
/* we warned when secondary results are expected but not returned. */
#endif /*MELT_HAVE_DEBUG */
      ;
    }
    ;
    /*^finalreturn */
    ;
    /*finalret */ goto meltlabend_rout;
    /*epilog */

    /*^clear */
	   /*clear *//*_.LET___V3*/ meltfptr[2] = 0;
  }

  ;
  goto meltlabend_rout;
meltlabend_rout:
  melt_trace_end ("REGISTER_FINISH_DECL_LAST", meltcallcount);
  MELT_TRACE_EXIT_LOCATION ();
  melt_blocklevel_signals =
    current_blocklevel_signals_meltrout_58_WARMELTmiHOOKS_REGISTER_FINISH_DECL_LAST_melt;
  melt_topframe = (struct melt_callframe_st *) meltfram__.mcfr_prev;
  return (melt_ptr_t) ( /*_._RETVAL___V1*/ meltfptr[0]);
#undef meltcallcount
#undef meltfram__
#undef MELTFRAM_NBVARNUM
#undef MELTFRAM_NBVARPTR
}				/*end meltrout_58_WARMELTmiHOOKS_REGISTER_FINISH_DECL_LAST */





melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_60_WARMELTmiHOOKS_REGISTER_PASS_EXECUTION_FIRST (meltclosure_ptr_t
							  meltclosp_,
							  melt_ptr_t
							  meltfirstargp_,
							  const
							  melt_argdescr_cell_t
							  meltxargdescr_[],
							  union meltparam_un *
							  meltxargtab_,
							  const
							  melt_argdescr_cell_t
							  meltxresdescr_[],
							  union meltparam_un *
							  meltxrestab_)
{
  long
    current_blocklevel_signals_meltrout_60_WARMELTmiHOOKS_REGISTER_PASS_EXECUTION_FIRST_melt
    = melt_blocklevel_signals;

#if MELT_HAVE_DEBUG
  static long melt_call_counter__;
  long melt_thiscallcounter__ ATTRIBUTE_UNUSED = ++melt_call_counter__;
#undef meltcallcount
#define meltcallcount melt_thiscallcounter__
#else
#undef meltcallcount
#define meltcallcount 0L
#endif

  struct meltframe_meltrout_60_WARMELTmiHOOKS_REGISTER_PASS_EXECUTION_FIRST_st
  {
    int mcfr_nbvar;
#if MELT_HAVE_DEBUG
    const char *mcfr_flocs;
#else				/*!MELT_HAVE_DEBUG */
    const char *mcfr_unusedflocs;
#endif				/*MELT_HAVE_DEBUG */
    struct meltclosure_st *mcfr_clos;
    struct excepth_melt_st *mcfr_exh;
    struct melt_callframe_st *mcfr_prev;
#undef MELTFRAM_NBVARPTR
#undef MELTFRAM_NBVARNUM
#define MELTFRAM_NBVARPTR 6
    melt_ptr_t mcfr_varptr[6];
#define MELTFRAM_NBVARNUM 2
    long mcfr_varnum[2];
/*others*/
    long _spare_;
  }
   *meltframptr_ = 0, meltfram__;	/*declfrastruct */
#define meltframe meltfram__
  /*meltrout_60_WARMELTmiHOOKS_REGISTER_PASS_EXECUTION_FIRST is not initial declstructinit */
  if (MELT_UNLIKELY (meltxargdescr_ == MELTPAR_MARKGGC))
    {				/*mark for ggc */
      int meltix;
      meltframptr_ =
	(struct
	 meltframe_meltrout_60_WARMELTmiHOOKS_REGISTER_PASS_EXECUTION_FIRST_st
	 *) meltfirstargp_;
      /* use arguments meltrout_60_WARMELTmiHOOKS_REGISTER_PASS_EXECUTION_FIRST output_curframe_declstruct_init */
      (void) meltclosp_;
      (void) meltfirstargp_;
      (void) meltxargdescr_;
      (void) meltxargtab_;
      (void) meltxresdescr_;
      (void) meltxrestab_;
      /* marking closure */
      gt_ggc_mx_melt_un (meltframptr_->mcfr_clos);
      for (meltix = 0; meltix < 6; meltix++)
	if (meltframptr_->mcfr_varptr[meltix])
	  gt_ggc_mx_melt_un (meltframptr_->mcfr_varptr[meltix]);
      return NULL;
    } /*end markggc */ ;
  memset (&meltfram__, 0, sizeof (meltfram__));
/* declstructinit plain routine meltrout_60_WARMELTmiHOOKS_REGISTER_PASS_EXECUTION_FIRST nbval 6*/
  meltfram__.mcfr_nbvar = 6 /*nbval */ ;
  meltfram__.mcfr_clos = meltclosp_;
  meltfram__.mcfr_prev = (struct melt_callframe_st *) melt_topframe;
  melt_topframe = (struct melt_callframe_st *) &meltfram__;
  melt_trace_start ("REGISTER_PASS_EXECUTION_FIRST", meltcallcount);
/*getargs*/

  /*getarg#0 */
  MELT_LOCATION ("warmelt-hooks.melt:1670:/ getarg");
 /*_.FUN__V2*/ meltfptr[1] = (melt_ptr_t) meltfirstargp_;
  ;
  goto meltlab_endgetargs;
meltlab_endgetargs:;
/*body*/
/*^block*/
/*anyblock*/
  {

    MELT_LOCATION ("warmelt-hooks.melt:1674:/ quasiblock");


    /*^cond */
    /*cond */ if (
		   /*ifisa */
		   melt_is_instance_of ((melt_ptr_t)
					(( /*!PASS_EXECUTION_DELAYED_QUEUE */
					  meltfrout->tabval[0])),
					(melt_ptr_t) (( /*!CLASS_DELAYED_QUEUE */ meltfrout->tabval[1])))
      )				/*then */
      {
	/*^cond.then */
	/*^getslot */
	{
	  melt_ptr_t slot = NULL, obj = NULL;
	  obj =
	    (melt_ptr_t) (( /*!PASS_EXECUTION_DELAYED_QUEUE */ meltfrout->
			   tabval[0])) /*=obj*/ ;
	  melt_object_get_field (slot, obj, 2, "DELQU_FIRST");
   /*_.FIRSTLIST__V3*/ meltfptr[2] = slot;
	};
	;
      }
    else
      {				/*^cond.else */

  /*_.FIRSTLIST__V3*/ meltfptr[2] = NULL;;
      }
    ;

    MELT_CHECK_SIGNAL ();
    ;
    /*^compute */
 /*_#NULL__L1*/ meltfnum[0] =
      (( /*_.FIRSTLIST__V3*/ meltfptr[2]) == NULL);;
    MELT_LOCATION ("warmelt-hooks.melt:1676:/ cond");
    /*cond */ if ( /*_#NULL__L1*/ meltfnum[0])	/*then */
      {
	/*^cond.then */
	/*^block */
	/*anyblock */
	{

   /*_.MAKE_LIST__V4*/ meltfptr[3] =
	    (meltgc_new_list
	     ((meltobject_ptr_t)
	      (( /*!DISCR_LIST */ meltfrout->tabval[2]))));;
	  MELT_LOCATION ("warmelt-hooks.melt:1678:/ compute");
	  /*_.FIRSTLIST__V3*/ meltfptr[2] = /*_.SETQ___V5*/ meltfptr[4] =
	    /*_.MAKE_LIST__V4*/ meltfptr[3];;

	  MELT_CHECK_SIGNAL ();
	  ;
   /*_.MAKE_LIST__V6*/ meltfptr[5] =
	    (meltgc_new_list
	     ((meltobject_ptr_t)
	      (( /*!DISCR_LIST */ meltfrout->tabval[2]))));;
	  MELT_LOCATION ("warmelt-hooks.melt:1679:/ cond");
	  /*cond */ if (
			 /*ifisa */
			 melt_is_instance_of ((melt_ptr_t)
					      (( /*!PASS_EXECUTION_DELAYED_QUEUE */ meltfrout->tabval[0])),
					      (melt_ptr_t) (( /*!CLASS_DELAYED_QUEUE */ meltfrout->tabval[1])))
	    )			/*then */
	    {
	      /*^cond.then */
	      /*^block */
	      /*anyblock */
	      {

		/*^putslot */
		/*putslot */
		melt_assertmsg ("putslot checkobj @DELQU_FIRST",
				melt_magic_discr ((melt_ptr_t)
						  (( /*!PASS_EXECUTION_DELAYED_QUEUE */ meltfrout->tabval[0]))) == MELTOBMAG_OBJECT);
		melt_putfield_object ((( /*!PASS_EXECUTION_DELAYED_QUEUE */
					meltfrout->tabval[0])), (2),
				      ( /*_.FIRSTLIST__V3*/ meltfptr[2]),
				      "DELQU_FIRST");
		;
		/*^putslot */
		/*putslot */
		melt_assertmsg ("putslot checkobj @DELQU_LAST",
				melt_magic_discr ((melt_ptr_t)
						  (( /*!PASS_EXECUTION_DELAYED_QUEUE */ meltfrout->tabval[0]))) == MELTOBMAG_OBJECT);
		melt_putfield_object ((( /*!PASS_EXECUTION_DELAYED_QUEUE */
					meltfrout->tabval[0])), (3),
				      ( /*_.MAKE_LIST__V6*/ meltfptr[5]),
				      "DELQU_LAST");
		;
		/*^touch */
		meltgc_touch (( /*!PASS_EXECUTION_DELAYED_QUEUE */ meltfrout->
			       tabval[0]));
		;
		/*^touchobj */

		melt_dbgtrace_written_object (( /*!PASS_EXECUTION_DELAYED_QUEUE */ meltfrout->tabval[0]), "put-fields");
		;
		/*epilog */
	      }
	      ;
	    }			/*noelse */
	  ;
	  MELT_LOCATION ("warmelt-hooks.melt:1677:/ quasiblock");


	  /*epilog */

	  MELT_LOCATION ("warmelt-hooks.melt:1676:/ clear");
	     /*clear *//*_.MAKE_LIST__V4*/ meltfptr[3] = 0;
	  /*^clear */
	     /*clear *//*_.SETQ___V5*/ meltfptr[4] = 0;
	  /*^clear */
	     /*clear *//*_.MAKE_LIST__V6*/ meltfptr[5] = 0;
	}
	;
      }				/*noelse */
    ;

    MELT_CHECK_SIGNAL ();
    ;
 /*_#IS_CLOSURE__L2*/ meltfnum[1] =
      (melt_magic_discr ((melt_ptr_t) ( /*_.FUN__V2*/ meltfptr[1])) ==
       MELTOBMAG_CLOSURE);;
    MELT_LOCATION ("warmelt-hooks.melt:1684:/ cond");
    /*cond */ if ( /*_#IS_CLOSURE__L2*/ meltfnum[1])	/*then */
      {
	/*^cond.then */
	/*^block */
	/*anyblock */
	{


	  {
	    MELT_LOCATION ("warmelt-hooks.melt:1685:/ locexp");
	    meltgc_append_list ((melt_ptr_t)
				( /*_.FIRSTLIST__V3*/ meltfptr[2]),
				(melt_ptr_t) ( /*_.FUN__V2*/ meltfptr[1]));
	  }
	  ;
	  /*epilog */
	}
	;
      }
    else
      {
	MELT_LOCATION ("warmelt-hooks.melt:1684:/ cond.else");

	/*^block */
	/*anyblock */
	{


	  {
	    MELT_LOCATION ("warmelt-hooks.melt:1686:/ locexp");
	    error ("MELT ERROR MSG [#%ld]::: %s", melt_dbgcounter,
		   ("Bad hook passed to register_pass_execution_hook"));
	  }
	  ;
	  /*epilog */
	}
	;
      }
    ;

    MELT_LOCATION ("warmelt-hooks.melt:1674:/ clear");
	   /*clear *//*_.FIRSTLIST__V3*/ meltfptr[2] = 0;
    /*^clear */
	   /*clear *//*_#NULL__L1*/ meltfnum[0] = 0;
    /*^clear */
	   /*clear *//*_#IS_CLOSURE__L2*/ meltfnum[1] = 0;
    /*epilog */
  }

  ;
  goto meltlabend_rout;
meltlabend_rout:
  melt_trace_end ("REGISTER_PASS_EXECUTION_FIRST", meltcallcount);
  MELT_TRACE_EXIT_LOCATION ();
  melt_blocklevel_signals =
    current_blocklevel_signals_meltrout_60_WARMELTmiHOOKS_REGISTER_PASS_EXECUTION_FIRST_melt;
  melt_topframe = (struct melt_callframe_st *) meltfram__.mcfr_prev;
  return (melt_ptr_t) ( /*noretval */ NULL);
#undef meltcallcount
#undef meltfram__
#undef MELTFRAM_NBVARNUM
#undef MELTFRAM_NBVARPTR
}				/*end meltrout_60_WARMELTmiHOOKS_REGISTER_PASS_EXECUTION_FIRST */





melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_61_WARMELTmiHOOKS_REGISTER_PASS_EXECUTION_LAST (meltclosure_ptr_t
							 meltclosp_,
							 melt_ptr_t
							 meltfirstargp_,
							 const
							 melt_argdescr_cell_t
							 meltxargdescr_[],
							 union meltparam_un *
							 meltxargtab_,
							 const
							 melt_argdescr_cell_t
							 meltxresdescr_[],
							 union meltparam_un *
							 meltxrestab_)
{
  long
    current_blocklevel_signals_meltrout_61_WARMELTmiHOOKS_REGISTER_PASS_EXECUTION_LAST_melt
    = melt_blocklevel_signals;

#if MELT_HAVE_DEBUG
  static long melt_call_counter__;
  long melt_thiscallcounter__ ATTRIBUTE_UNUSED = ++melt_call_counter__;
#undef meltcallcount
#define meltcallcount melt_thiscallcounter__
#else
#undef meltcallcount
#define meltcallcount 0L
#endif

  struct meltframe_meltrout_61_WARMELTmiHOOKS_REGISTER_PASS_EXECUTION_LAST_st
  {
    int mcfr_nbvar;
#if MELT_HAVE_DEBUG
    const char *mcfr_flocs;
#else				/*!MELT_HAVE_DEBUG */
    const char *mcfr_unusedflocs;
#endif				/*MELT_HAVE_DEBUG */
    struct meltclosure_st *mcfr_clos;
    struct excepth_melt_st *mcfr_exh;
    struct melt_callframe_st *mcfr_prev;
#undef MELTFRAM_NBVARPTR
#undef MELTFRAM_NBVARNUM
#define MELTFRAM_NBVARPTR 6
    melt_ptr_t mcfr_varptr[6];
#define MELTFRAM_NBVARNUM 2
    long mcfr_varnum[2];
/*others*/
    long _spare_;
  }
   *meltframptr_ = 0, meltfram__;	/*declfrastruct */
#define meltframe meltfram__
  /*meltrout_61_WARMELTmiHOOKS_REGISTER_PASS_EXECUTION_LAST is not initial declstructinit */
  if (MELT_UNLIKELY (meltxargdescr_ == MELTPAR_MARKGGC))
    {				/*mark for ggc */
      int meltix;
      meltframptr_ =
	(struct
	 meltframe_meltrout_61_WARMELTmiHOOKS_REGISTER_PASS_EXECUTION_LAST_st
	 *) meltfirstargp_;
      /* use arguments meltrout_61_WARMELTmiHOOKS_REGISTER_PASS_EXECUTION_LAST output_curframe_declstruct_init */
      (void) meltclosp_;
      (void) meltfirstargp_;
      (void) meltxargdescr_;
      (void) meltxargtab_;
      (void) meltxresdescr_;
      (void) meltxrestab_;
      /* marking closure */
      gt_ggc_mx_melt_un (meltframptr_->mcfr_clos);
      for (meltix = 0; meltix < 6; meltix++)
	if (meltframptr_->mcfr_varptr[meltix])
	  gt_ggc_mx_melt_un (meltframptr_->mcfr_varptr[meltix]);
      return NULL;
    } /*end markggc */ ;
  memset (&meltfram__, 0, sizeof (meltfram__));
/* declstructinit plain routine meltrout_61_WARMELTmiHOOKS_REGISTER_PASS_EXECUTION_LAST nbval 6*/
  meltfram__.mcfr_nbvar = 6 /*nbval */ ;
  meltfram__.mcfr_clos = meltclosp_;
  meltfram__.mcfr_prev = (struct melt_callframe_st *) melt_topframe;
  melt_topframe = (struct melt_callframe_st *) &meltfram__;
  melt_trace_start ("REGISTER_PASS_EXECUTION_LAST", meltcallcount);
/*getargs*/

  /*getarg#0 */
  MELT_LOCATION ("warmelt-hooks.melt:1691:/ getarg");
 /*_.FUN__V2*/ meltfptr[1] = (melt_ptr_t) meltfirstargp_;
  ;
  goto meltlab_endgetargs;
meltlab_endgetargs:;
/*body*/
/*^block*/
/*anyblock*/
  {

    MELT_LOCATION ("warmelt-hooks.melt:1695:/ quasiblock");


    /*^cond */
    /*cond */ if (
		   /*ifisa */
		   melt_is_instance_of ((melt_ptr_t)
					(( /*!PASS_EXECUTION_DELAYED_QUEUE */
					  meltfrout->tabval[0])),
					(melt_ptr_t) (( /*!CLASS_DELAYED_QUEUE */ meltfrout->tabval[1])))
      )				/*then */
      {
	/*^cond.then */
	/*^getslot */
	{
	  melt_ptr_t slot = NULL, obj = NULL;
	  obj =
	    (melt_ptr_t) (( /*!PASS_EXECUTION_DELAYED_QUEUE */ meltfrout->
			   tabval[0])) /*=obj*/ ;
	  melt_object_get_field (slot, obj, 2, "DELQU_FIRST");
   /*_.LASTLIST__V3*/ meltfptr[2] = slot;
	};
	;
      }
    else
      {				/*^cond.else */

  /*_.LASTLIST__V3*/ meltfptr[2] = NULL;;
      }
    ;

    MELT_CHECK_SIGNAL ();
    ;
    /*^compute */
 /*_#NULL__L1*/ meltfnum[0] =
      (( /*_.LASTLIST__V3*/ meltfptr[2]) == NULL);;
    MELT_LOCATION ("warmelt-hooks.melt:1697:/ cond");
    /*cond */ if ( /*_#NULL__L1*/ meltfnum[0])	/*then */
      {
	/*^cond.then */
	/*^block */
	/*anyblock */
	{

   /*_.MAKE_LIST__V4*/ meltfptr[3] =
	    (meltgc_new_list
	     ((meltobject_ptr_t)
	      (( /*!DISCR_LIST */ meltfrout->tabval[2]))));;
	  MELT_LOCATION ("warmelt-hooks.melt:1699:/ compute");
	  /*_.LASTLIST__V3*/ meltfptr[2] = /*_.SETQ___V5*/ meltfptr[4] =
	    /*_.MAKE_LIST__V4*/ meltfptr[3];;

	  MELT_CHECK_SIGNAL ();
	  ;
   /*_.MAKE_LIST__V6*/ meltfptr[5] =
	    (meltgc_new_list
	     ((meltobject_ptr_t)
	      (( /*!DISCR_LIST */ meltfrout->tabval[2]))));;
	  MELT_LOCATION ("warmelt-hooks.melt:1700:/ cond");
	  /*cond */ if (
			 /*ifisa */
			 melt_is_instance_of ((melt_ptr_t)
					      (( /*!PASS_EXECUTION_DELAYED_QUEUE */ meltfrout->tabval[0])),
					      (melt_ptr_t) (( /*!CLASS_DELAYED_QUEUE */ meltfrout->tabval[1])))
	    )			/*then */
	    {
	      /*^cond.then */
	      /*^block */
	      /*anyblock */
	      {

		/*^putslot */
		/*putslot */
		melt_assertmsg ("putslot checkobj @DELQU_FIRST",
				melt_magic_discr ((melt_ptr_t)
						  (( /*!PASS_EXECUTION_DELAYED_QUEUE */ meltfrout->tabval[0]))) == MELTOBMAG_OBJECT);
		melt_putfield_object ((( /*!PASS_EXECUTION_DELAYED_QUEUE */
					meltfrout->tabval[0])), (2),
				      ( /*_.LASTLIST__V3*/ meltfptr[2]),
				      "DELQU_FIRST");
		;
		/*^putslot */
		/*putslot */
		melt_assertmsg ("putslot checkobj @DELQU_LAST",
				melt_magic_discr ((melt_ptr_t)
						  (( /*!PASS_EXECUTION_DELAYED_QUEUE */ meltfrout->tabval[0]))) == MELTOBMAG_OBJECT);
		melt_putfield_object ((( /*!PASS_EXECUTION_DELAYED_QUEUE */
					meltfrout->tabval[0])), (3),
				      ( /*_.MAKE_LIST__V6*/ meltfptr[5]),
				      "DELQU_LAST");
		;
		/*^touch */
		meltgc_touch (( /*!PASS_EXECUTION_DELAYED_QUEUE */ meltfrout->
			       tabval[0]));
		;
		/*^touchobj */

		melt_dbgtrace_written_object (( /*!PASS_EXECUTION_DELAYED_QUEUE */ meltfrout->tabval[0]), "put-fields");
		;
		/*epilog */
	      }
	      ;
	    }			/*noelse */
	  ;
	  MELT_LOCATION ("warmelt-hooks.melt:1698:/ quasiblock");


	  /*epilog */

	  MELT_LOCATION ("warmelt-hooks.melt:1697:/ clear");
	     /*clear *//*_.MAKE_LIST__V4*/ meltfptr[3] = 0;
	  /*^clear */
	     /*clear *//*_.SETQ___V5*/ meltfptr[4] = 0;
	  /*^clear */
	     /*clear *//*_.MAKE_LIST__V6*/ meltfptr[5] = 0;
	}
	;
      }				/*noelse */
    ;

    MELT_CHECK_SIGNAL ();
    ;
 /*_#IS_CLOSURE__L2*/ meltfnum[1] =
      (melt_magic_discr ((melt_ptr_t) ( /*_.FUN__V2*/ meltfptr[1])) ==
       MELTOBMAG_CLOSURE);;
    MELT_LOCATION ("warmelt-hooks.melt:1705:/ cond");
    /*cond */ if ( /*_#IS_CLOSURE__L2*/ meltfnum[1])	/*then */
      {
	/*^cond.then */
	/*^block */
	/*anyblock */
	{


	  {
	    MELT_LOCATION ("warmelt-hooks.melt:1706:/ locexp");
	    meltgc_append_list ((melt_ptr_t)
				( /*_.LASTLIST__V3*/ meltfptr[2]),
				(melt_ptr_t) ( /*_.FUN__V2*/ meltfptr[1]));
	  }
	  ;
	  /*epilog */
	}
	;
      }
    else
      {
	MELT_LOCATION ("warmelt-hooks.melt:1705:/ cond.else");

	/*^block */
	/*anyblock */
	{


	  {
	    MELT_LOCATION ("warmelt-hooks.melt:1707:/ locexp");
	    error ("MELT ERROR MSG [#%ld]::: %s", melt_dbgcounter,
		   ("Bad hook passed to register_pass_execution_last_hook"));
	  }
	  ;
	  /*epilog */
	}
	;
      }
    ;

    MELT_LOCATION ("warmelt-hooks.melt:1695:/ clear");
	   /*clear *//*_.LASTLIST__V3*/ meltfptr[2] = 0;
    /*^clear */
	   /*clear *//*_#NULL__L1*/ meltfnum[0] = 0;
    /*^clear */
	   /*clear *//*_#IS_CLOSURE__L2*/ meltfnum[1] = 0;
    /*epilog */
  }

  ;
  goto meltlabend_rout;
meltlabend_rout:
  melt_trace_end ("REGISTER_PASS_EXECUTION_LAST", meltcallcount);
  MELT_TRACE_EXIT_LOCATION ();
  melt_blocklevel_signals =
    current_blocklevel_signals_meltrout_61_WARMELTmiHOOKS_REGISTER_PASS_EXECUTION_LAST_melt;
  melt_topframe = (struct melt_callframe_st *) meltfram__.mcfr_prev;
  return (melt_ptr_t) ( /*noretval */ NULL);
#undef meltcallcount
#undef meltfram__
#undef MELTFRAM_NBVARNUM
#undef MELTFRAM_NBVARPTR
}				/*end meltrout_61_WARMELTmiHOOKS_REGISTER_PASS_EXECUTION_LAST */





melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_69_WARMELTmiHOOKS_AT_EXIT_FIRST (meltclosure_ptr_t meltclosp_,
					  melt_ptr_t meltfirstargp_,
					  const melt_argdescr_cell_t
					  meltxargdescr_[],
					  union meltparam_un * meltxargtab_,
					  const melt_argdescr_cell_t
					  meltxresdescr_[],
					  union meltparam_un * meltxrestab_)
{
  long
    current_blocklevel_signals_meltrout_69_WARMELTmiHOOKS_AT_EXIT_FIRST_melt =
    melt_blocklevel_signals;

#if MELT_HAVE_DEBUG
  static long melt_call_counter__;
  long melt_thiscallcounter__ ATTRIBUTE_UNUSED = ++melt_call_counter__;
#undef meltcallcount
#define meltcallcount melt_thiscallcounter__
#else
#undef meltcallcount
#define meltcallcount 0L
#endif

  struct meltframe_meltrout_69_WARMELTmiHOOKS_AT_EXIT_FIRST_st
  {
    int mcfr_nbvar;
#if MELT_HAVE_DEBUG
    const char *mcfr_flocs;
#else				/*!MELT_HAVE_DEBUG */
    const char *mcfr_unusedflocs;
#endif				/*MELT_HAVE_DEBUG */
    struct meltclosure_st *mcfr_clos;
    struct excepth_melt_st *mcfr_exh;
    struct melt_callframe_st *mcfr_prev;
#undef MELTFRAM_NBVARPTR
#undef MELTFRAM_NBVARNUM
#define MELTFRAM_NBVARPTR 3
    melt_ptr_t mcfr_varptr[3];
#define MELTFRAM_NBVARNUM 1
    long mcfr_varnum[1];
/*others*/
    long _spare_;
  }
   *meltframptr_ = 0, meltfram__;	/*declfrastruct */
#define meltframe meltfram__
  /*meltrout_69_WARMELTmiHOOKS_AT_EXIT_FIRST is not initial declstructinit */
  if (MELT_UNLIKELY (meltxargdescr_ == MELTPAR_MARKGGC))
    {				/*mark for ggc */
      int meltix;
      meltframptr_ =
	(struct meltframe_meltrout_69_WARMELTmiHOOKS_AT_EXIT_FIRST_st *)
	meltfirstargp_;
      /* use arguments meltrout_69_WARMELTmiHOOKS_AT_EXIT_FIRST output_curframe_declstruct_init */
      (void) meltclosp_;
      (void) meltfirstargp_;
      (void) meltxargdescr_;
      (void) meltxargtab_;
      (void) meltxresdescr_;
      (void) meltxrestab_;
      /* marking closure */
      gt_ggc_mx_melt_un (meltframptr_->mcfr_clos);
      for (meltix = 0; meltix < 3; meltix++)
	if (meltframptr_->mcfr_varptr[meltix])
	  gt_ggc_mx_melt_un (meltframptr_->mcfr_varptr[meltix]);
      return NULL;
    } /*end markggc */ ;
  memset (&meltfram__, 0, sizeof (meltfram__));
/* declstructinit plain routine meltrout_69_WARMELTmiHOOKS_AT_EXIT_FIRST nbval 3*/
  meltfram__.mcfr_nbvar = 3 /*nbval */ ;
  meltfram__.mcfr_clos = meltclosp_;
  meltfram__.mcfr_prev = (struct melt_callframe_st *) melt_topframe;
  melt_topframe = (struct melt_callframe_st *) &meltfram__;
  melt_trace_start ("AT_EXIT_FIRST", meltcallcount);
/*getargs*/

  /*getarg#0 */
  MELT_LOCATION ("warmelt-hooks.melt:2108:/ getarg");
 /*_.FUN__V2*/ meltfptr[1] = (melt_ptr_t) meltfirstargp_;
  ;
  goto meltlab_endgetargs;
meltlab_endgetargs:;
/*body*/
/*^block*/
/*anyblock*/
  {

    MELT_LOCATION ("warmelt-hooks.melt:2111:/ quasiblock");


    /*^getslot */
    {
      melt_ptr_t slot = NULL, obj = NULL;
      obj =
	(melt_ptr_t) (( /*!FINAL_EXIT_DELAYED_QUEUE */ meltfrout->
		       tabval[0])) /*=obj*/ ;
      melt_object_get_field (slot, obj, 2, "DELQU_FIRST");
  /*_.FIRSTLIST__V3*/ meltfptr[2] = slot;
    };
    ;

    MELT_CHECK_SIGNAL ();
    ;
 /*_#IS_CLOSURE__L1*/ meltfnum[0] =
      (melt_magic_discr ((melt_ptr_t) ( /*_.FUN__V2*/ meltfptr[1])) ==
       MELTOBMAG_CLOSURE);;
    MELT_LOCATION ("warmelt-hooks.melt:2113:/ cond");
    /*cond */ if ( /*_#IS_CLOSURE__L1*/ meltfnum[0])	/*then */
      {
	/*^cond.then */
	/*^block */
	/*anyblock */
	{


	  {
	    /*^locexp */
	    meltgc_append_list ((melt_ptr_t)
				( /*_.FIRSTLIST__V3*/ meltfptr[2]),
				(melt_ptr_t) ( /*_.FUN__V2*/ meltfptr[1]));
	  }
	  ;
	  /*epilog */
	}
	;
      }				/*noelse */
    ;

    MELT_LOCATION ("warmelt-hooks.melt:2111:/ clear");
	   /*clear *//*_.FIRSTLIST__V3*/ meltfptr[2] = 0;
    /*^clear */
	   /*clear *//*_#IS_CLOSURE__L1*/ meltfnum[0] = 0;
    /*epilog */
  }

  ;
  goto meltlabend_rout;
meltlabend_rout:
  melt_trace_end ("AT_EXIT_FIRST", meltcallcount);
  MELT_TRACE_EXIT_LOCATION ();
  melt_blocklevel_signals =
    current_blocklevel_signals_meltrout_69_WARMELTmiHOOKS_AT_EXIT_FIRST_melt;
  melt_topframe = (struct melt_callframe_st *) meltfram__.mcfr_prev;
  return (melt_ptr_t) ( /*noretval */ NULL);
#undef meltcallcount
#undef meltfram__
#undef MELTFRAM_NBVARNUM
#undef MELTFRAM_NBVARPTR
}				/*end meltrout_69_WARMELTmiHOOKS_AT_EXIT_FIRST */





melt_ptr_t MELT_MODULE_VISIBILITY
meltrout_70_WARMELTmiHOOKS_AT_EXIT_LAST (meltclosure_ptr_t meltclosp_,
					 melt_ptr_t meltfirstargp_,
					 const melt_argdescr_cell_t
					 meltxargdescr_[],
					 union meltparam_un * meltxargtab_,
					 const melt_argdescr_cell_t
					 meltxresdescr_[],
					 union meltparam_un * meltxrestab_)
{
  long current_blocklevel_signals_meltrout_70_WARMELTmiHOOKS_AT_EXIT_LAST_melt
    = melt_blocklevel_signals;

#if MELT_HAVE_DEBUG
  static long melt_call_counter__;
  long melt_thiscallcounter__ ATTRIBUTE_UNUSED = ++melt_call_counter__;
#undef meltcallcount
#define meltcallcount melt_thiscallcounter__
#else
#undef meltcallcount
#define meltcallcount 0L
#endif

  struct meltframe_meltrout_70_WARMELTmiHOOKS_AT_EXIT_LAST_st
  {
    int mcfr_nbvar;
#if MELT_HAVE_DEBUG
    const char *mcfr_flocs;
#else				/*!MELT_HAVE_DEBUG */
    const char *mcfr_unusedflocs;
#endif				/*MELT_HAVE_DEBUG */
    struct meltclosure_st *mcfr_clos;
    struct excepth_melt_st *mcfr_exh;
    struct melt_callframe_st *mcfr_prev;
#undef MELTFRAM_NBVARPTR
#undef MELTFRAM_NBVARNUM
#define MELTFRAM_NBVARPTR 3
    melt_ptr_t mcfr_varptr[3];
#define MELTFRAM_NBVARNUM 1
    long mcfr_varnum[1];
/*others*/
    long _spare_;
  }
   *meltframptr_ = 0, meltfram__;	/*declfrastruct */
#define meltframe meltfram__
  /*meltrout_70_WARMELTmiHOOKS_AT_EXIT_LAST is not initial declstructinit */
  if (MELT_UNLIKELY (meltxargdescr_ == MELTPAR_MARKGGC))
    {				/*mark for ggc */
      int meltix;
      meltframptr_ =
	(struct meltframe_meltrout_70_WARMELTmiHOOKS_AT_EXIT_LAST_st *)
	meltfirstargp_;
      /* use arguments meltrout_70_WARMELTmiHOOKS_AT_EXIT_LAST output_curframe_declstruct_init */
      (void) meltclosp_;
      (void) meltfirstargp_;
      (void) meltxargdescr_;
      (void) meltxargtab_;
      (void) meltxresdescr_;
      (void) meltxrestab_;
      /* marking closure */
      gt_ggc_mx_melt_un (meltframptr_->mcfr_clos);
      for (meltix = 0; meltix < 3; meltix++)
	if (meltframptr_->mcfr_varptr[meltix])
	  gt_ggc_mx_melt_un (meltframptr_->mcfr_varptr[meltix]);
      return NULL;
    } /*end markggc */ ;
  memset (&meltfram__, 0, sizeof (meltfram__));
/* declstructinit plain routine meltrout_70_WARMELTmiHOOKS_AT_EXIT_LAST nbval 3*/
  meltfram__.mcfr_nbvar = 3 /*nbval */ ;
  meltfram__.mcfr_clos = meltclosp_;
  meltfram__.mcfr_prev = (struct melt_callframe_st *) melt_topframe;
  melt_topframe = (struct melt_callframe_st *) &meltfram__;
  melt_trace_start ("AT_EXIT_LAST", meltcallcount);
/*getargs*/

  /*getarg#0 */
  MELT_LOCATION ("warmelt-hooks.melt:2116:/ getarg");
 /*_.FUN__V2*/ meltfptr[1] = (melt_ptr_t) meltfirstargp_;
  ;
  goto meltlab_endgetargs;
meltlab_endgetargs:;
/*body*/
/*^block*/
/*anyblock*/
  {

    MELT_LOCATION ("warmelt-hooks.melt:2119:/ quasiblock");


    /*^getslot */
    {
      melt_ptr_t slot = NULL, obj = NULL;
      obj =
	(melt_ptr_t) (( /*!FINAL_EXIT_DELAYED_QUEUE */ meltfrout->
		       tabval[0])) /*=obj*/ ;
      melt_object_get_field (slot, obj, 3, "DELQU_LAST");
  /*_.LASTLIST__V3*/ meltfptr[2] = slot;
    };
    ;

    MELT_CHECK_SIGNAL ();
    ;
 /*_#IS_CLOSURE__L1*/ meltfnum[0] =
      (melt_magic_discr ((melt_ptr_t) ( /*_.FUN__V2*/ meltfptr[1])) ==
       MELTOBMAG_CLOSURE);;
    MELT_LOCATION ("warmelt-hooks.melt:2121:/ cond");
    /*cond */ if ( /*_#IS_CLOSURE__L1*/ meltfnum[0])	/*then */
      {
	/*^cond.then */
	/*^block */
	/*anyblock */
	{


	  {
	    /*^locexp */
	    meltgc_append_list ((melt_ptr_t)
				( /*_.LASTLIST__V3*/ meltfptr[2]),
				(melt_ptr_t) ( /*_.FUN__V2*/ meltfptr[1]));
	  }
	  ;
	  /*epilog */
	}
	;
      }				/*noelse */
    ;

    MELT_LOCATION ("warmelt-hooks.melt:2119:/ clear");
	   /*clear *//*_.LASTLIST__V3*/ meltfptr[2] = 0;
    /*^clear */
	   /*clear *//*_#IS_CLOSURE__L1*/ meltfnum[0] = 0;
    /*epilog */
  }

  ;
  goto meltlabend_rout;
meltlabend_rout:
  melt_trace_end ("AT_EXIT_LAST", meltcallcount);
  MELT_TRACE_EXIT_LOCATION ();
  melt_blocklevel_signals =
    current_blocklevel_signals_meltrout_70_WARMELTmiHOOKS_AT_EXIT_LAST_melt;
  melt_topframe = (struct melt_callframe_st *) meltfram__.mcfr_prev;
  return (melt_ptr_t) ( /*noretval */ NULL);
#undef meltcallcount
#undef meltfram__
#undef MELTFRAM_NBVARNUM
#undef MELTFRAM_NBVARPTR
}				/*end meltrout_70_WARMELTmiHOOKS_AT_EXIT_LAST */



/**** end of warmelt-hooks+02.c ****/
