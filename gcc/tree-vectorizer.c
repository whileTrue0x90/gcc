/* Scalar evolution detector.
   Copyright (C) 2003 Free Software Foundation, Inc.
   Contributed by Dorit Naishlos <dorit@il.ibm.com>

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.  */

/* Loop Vectorization Pass.

   This pass tries to vectorize loops. This first implementation focuses on
   simple inner-most loops, with no conditional control flow, and a set of
   simple operations which vector form can be expressed using existing
   tree codes (PLUS, MULT etc).

   For example, the vectorizer transforms the following simple loop:

	short a[N]; short b[N]; short c[N]; int i;

  	for (i=0; i<N; i++){
    	  a[i] = b[i] + c[i];
  	}

   as if it was manually vectorized by rewriting the source code into:

  	typedef int __attribute__((mode(V8HI))) v8hi;
 	short a[N];  short b[N]; short c[N];   int i;
  	v8hi *pa = (v8hi*)a, *pb = (v8hi*)b, *pc = (v8hi*)c;
  	v8hi va, vb, vc;

  	for (i=0; i<N/8; i++){
    	  vb = pb[i];
    	  vc = pc[i];
    	  va = vb + vc;
    	  pa[i] = va;
  	}

   	The main entry to this pass is vectorize_loops(), in which for each
   the vectorizer applies a set of analyses on a given set of loops,
   followed by the actual vectorization transformation for the loops that
   had successfully passed the analysis phase.

   	Throughout this pass we make a distinction between two types of
   data: scalars (which are represented by SSA_NAMES), and data-refs. These
   are handled separately both by the analyzer and the loop-transformer.
   Currently, the vectorizer only supports simple data-refs which are
   limited to ARRAY_REFS that represent one dimentional arrays which base is
   an array (not a pointer), and have a simple (consecutive) access pattern.

   Analysis phase:
   ===============
   	The driver for the analysis phase is vect_analyze_loop_nest().
   which applies a set of loop analyses. Some of the analyses rely on the
   motonotic evolution analyzer developed by Sebastian Pop.

   	During the analysis phase the vectorizer records some information
   per stmt in a stmt_vec_info which is attached to each stmt in the loop,
   as well as general information about the loop as a whole, which is
   recorded in a loop_vec_info struct attached to each loop.

   Transformation phase:
   =====================
	The loop transformtaion phase scans all the stmts in the loop, and
   creates a vector stmt (or a sequence of stmts) for each scalar stmt S in
   the loop that needs to be vectorized. It insert the vector code sequence
   just before the scalar stmt S, and records a pointer to the vector code
   in STMT_VINFO_VEC_STMT (stmt_info) (where stmt_info is the stmt_vec_info
   struct that is attched to S). This pointer is used for the vectorization
   of following stmts which use the defs of stmt S. Stmt S is removed
   only if it has side effects (like changing memory). If stmt S does not
   have side effects, we currently rely on dead code elimination for
   removing it.

   	For example, say stmt S1 was vectorized into stmt VS1:

   VS1: vb = px[i];
   S1: 	b = x[i];    STMT_VINFO_VEC_STMT (stmt_info (S1)) = VS1
   S2:  a = b;

   To vectorize stmt S2, the vectorizer first finds the stmt that defines
   the operand 'b' (S1), and gets the relevant vector def 'vb' from the
   vector stmt VS1 pointed by STMT_VINFO_VEC_STMT (stmt_info (S1)). The
   resulting sequence would be:

   VS1: vb = px[i];
   S1: 	b = x[i];    	STMT_VINFO_VEC_STMT (stmt_info (S1)) = VS1
   VS2: va = vb;
   S2:  a = b;          STMT_VINFO_VEC_STMT (stmt_info (S2)) = VS2

   	Operands that are not SSA_NAMEs, are currently limited to array
   references appearing in load/store operations (like 'x[i]' in S1), and
   are handled differently.

   Target modelling:
   =================
   	Currently the only target specific information that is used is the
   size of the vector (in bytes) - "UNITS_PER_SIMD_WORD", and a target hook
   "vectype_for_scalar_type" that for a given (scalar) machine mode returns
   the vector machine_mode to be used. Targets that can support different
   sizes of vectors, for now will need to specify one value for
   "UNITS_PER_SIMD_WORD". More flexibility will be added in the future.

	Since we only vectorize operations which vector form can be
   expressed using existing tree codes, to verify that an operation is
   supported the vectorizer checks the relevant optab at the relevant
   machine_mode (e.g, add_optab->handlers[(int) V8HImode].insn_code).  If
   the value found is CODE_FOR_nothing, then there's no target support, and
   we can't vectorize the stmt. Otherwise - the stmt is transformed.


   For additional information on this project see:
   http://gcc.gnu.org/projects/tree-ssa/vectorization.html
*/

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "errors.h"
#include "ggc.h"
#include "tree.h"
#include "target.h"

#include "rtl.h"
#include "basic-block.h"
#include "diagnostic.h"
#include "tree-flow.h"
#include "tree-dump.h"
#include "timevar.h"
#include "cfgloop.h"
#include "tree-fold-const.h"
#include "expr.h"
#include "optabs.h"
#include "tree-chrec.h"
#include "tree-data-ref.h"
#include "tree-scalar-evolution.h"
#include "tree-vectorizer.h"

/* CHECKME: check for unnecessary include files.  */

static bitmap vars_to_rename;

/* Debugging dumps.  */
static FILE *dump_file;
static int dump_flags;

/* Main analysis functions */
static loop_vec_info vect_analyze_loop (struct loop *);
static loop_vec_info vect_analyze_loop_form (struct loop *);
static bool vect_analyze_data_refs (loop_vec_info);
static bool vect_mark_stmts_to_be_vectorized (loop_vec_info);
static bool vect_analyze_scalar_cycles (loop_vec_info);
static bool vect_analyze_data_ref_dependences (loop_vec_info);
static bool vect_analyze_data_ref_accesses (loop_vec_info);
static bool vect_analyze_operations (loop_vec_info);

/* Main code transformation fuctions.  */
static void vect_transform_loop (loop_vec_info);
static void vect_transform_loop_bound (loop_vec_info);
static bool vect_transform_stmt (tree, block_stmt_iterator *);
static tree vect_transform_load (tree, block_stmt_iterator *);
static tree vect_transform_store (tree, block_stmt_iterator *);
static tree vect_transform_binop (tree, block_stmt_iterator *);
static void vect_align_data_ref (tree, tree);

/* Utility functions for the analyses.  */
static bool vect_is_supportable_binop (tree);
static bool vect_is_supportable_store (tree);
static bool vect_is_supportable_load (tree);
static tree get_address_calculation_operands (stmt_vec_info);
static bool vect_is_simple_iv_evolution (tree, tree *, tree *);
static void vect_mark_relevant (varray_type, tree);
static bool vect_stmt_relevant_p (tree, loop_vec_info);
static tree vect_get_loop_niters (struct loop *, int *);
static bool vect_analyze_data_ref_access (struct data_reference *);
static bool vect_analyze_data_ref_dependence 
  			(struct data_reference *, struct data_reference *);

/* Utility functions for the code transformation.  */
static tree vect_create_destination_var (tree, tree);
static tree vect_create_data_ref (tree, tree, block_stmt_iterator *);
static tree vect_create_index_for_array_ref (tree, block_stmt_iterator *);
static tree get_vectype_for_scalar_type (tree);
static char *vect_get_name_for_new_var (tree);

/* General untility functions (CHECKME: where do they belong) */
static tree get_array_base (tree);

/* Main driver.  */
void vectorize_loops (tree fndecl, bitmap vars, struct loops *loops, 
			varray_type ev_info, enum tree_dump_index phase);

/* Utilities for creation and deletion of vec_info structs. */
loop_vec_info new_loop_vec_info (struct loop *loop);
void destroy_loop_vec_info (loop_vec_info);
stmt_vec_info new_stmt_vec_info (tree stmt);


/* Function new_stmt_vec_info.

   Create and initialize a new stmt_vec_info struct for STMT.
 */

stmt_vec_info
new_stmt_vec_info (tree stmt)
{
  stmt_vec_info res;
  res = (stmt_vec_info)xcalloc (1, sizeof (struct _stmt_vec_info));

  STMT_VINFO_TYPE (res) = undef_vec_info_type;
  STMT_VINFO_STMT (res) = stmt;
  STMT_VINFO_RELEVANT_P (res) = 0;
  STMT_VINFO_VECTYPE (res) = NULL;
  STMT_VINFO_VEC_STMT (res) = NULL;
  STMT_VINFO_DATA_REF (res) = NULL;

  return res;
}


/* Function new_loop_vec_info.

   Create and initialize a new loop_vec_info struct for LOOP, as well as
   stmt_vec_info structs for all the stmts in LOOP.
*/

loop_vec_info
new_loop_vec_info (struct loop *loop)
{
  loop_vec_info res;
  basic_block *bbs;
  block_stmt_iterator si;
  unsigned int i;

  res = (loop_vec_info)xcalloc (1, sizeof (struct _loop_vec_info));

  bbs = get_loop_body (loop);

  /* Create stmt_info for all stmts in the loop.  */
  for (i = 0; i < loop->num_nodes; i++)
    {
      basic_block bb = bbs[i];
      for (si = bsi_start (bb); !bsi_end_p (si); bsi_next (&si))
         {
           tree stmt = bsi_stmt (si);
           stmt_ann_t ann;

	   get_stmt_operands (stmt);
	   ann = stmt_ann (stmt);
           set_stmt_info (ann, new_stmt_vec_info (stmt));
         }
    }

  LOOP_VINFO_LOOP (res) = loop;
  LOOP_VINFO_BBS (res) = bbs;
  LOOP_VINFO_EXIT_COND (res) = NULL;
  LOOP_VINFO_NITERS (res) = -1;
  LOOP_VINFO_VECTORIZABLE_P (res) = 0;
  LOOP_VINFO_VECT_FACTOR (res) = 0;
  VARRAY_GENERIC_PTR_INIT (LOOP_VINFO_DATAREF_WRITES (res),
				20, "loop_write_datarefs");
  VARRAY_GENERIC_PTR_INIT (LOOP_VINFO_DATAREF_READS (res),
				20,  "loop_read_datarefs");
  return res;
}


/* Function destroy_loop_vec_info.
*/

void
destroy_loop_vec_info (loop_vec_info loop_vinfo)
{
  struct loop *loop;
  basic_block *bbs;
  int nbbs;
  block_stmt_iterator si;
  int j;

  if (!loop_vinfo)
    return;

  loop = LOOP_VINFO_LOOP(loop_vinfo);

  bbs = LOOP_VINFO_BBS(loop_vinfo);
  nbbs = loop->num_nodes;

  for (j = 0; j < nbbs; j++)
    {
      basic_block bb = bbs[j];
      for (si = bsi_start (bb); !bsi_end_p (si); bsi_next (&si))
        {
          tree stmt = bsi_stmt (si);
	  stmt_ann_t ann = stmt_ann (stmt);
          stmt_vec_info stmt_info = vinfo_for_stmt (stmt);
          free (stmt_info);
          set_stmt_info (ann, NULL);
        }
    }

    free (LOOP_VINFO_BBS (loop_vinfo));
    varray_clear (LOOP_VINFO_DATAREF_WRITES (loop_vinfo));
    varray_clear (LOOP_VINFO_DATAREF_READS (loop_vinfo));

    free (loop_vinfo);
}


/* Function vect_get_name_for_new_var.

   Return a name for a new variable.
   The current naming scheme appends the prefix "_vect_" to all the
   vectorizer generated variables, and uses the name of a corresponding
   scalar variable VAR if given.  
   
   CHECKME: alloca ok?
   CHECKME: naming scheme ok?
 */

static char *
vect_get_name_for_new_var (tree var)
{
  const char *name = NULL;
  const char *prefix = "_vect_";
  const char *prefix_var = "_vect_var";
  int prefix_len;
  char *vec_var_name;

  if (var)
    name = get_name (var);

  if (name)
    {
      prefix_len = strlen (prefix);
      vec_var_name = alloca (strlen (name) + prefix_len + 1);
      sprintf (vec_var_name, "%s%s", prefix, name);
    }
  else
    {
      prefix_len = strlen (prefix_var);
      vec_var_name = alloca (prefix_len + 1);
      sprintf (vec_var_name, "%s", prefix_var);
    }

  return vec_var_name;
}


/* POINTER_ARITHMETIC

   CHECKME: The RTL expander does not like an ARRAY_REF where the base is a
   poiner; Until this is fixed, instead of generating

   var = p[i]

   we generate the following pointer arithemtic sequence:

   1. T0 = (unsigned int)i
   2. T1 = T0 * N 	(N is the size of the vector in bytes)
   3. T2 = p + T1
   4. var = (*T2)
*/
#define POINTER_ARITHMETIC 1


/* Function create_index_for_array_ref.

   Create an offset/index to be used to access a memory location.
   Input:
   EXPR: the original (scalar) data reference. EXPR is expected to be an
   	 ARRAY_REF in a load or store STMT, of the form a[i];

   BSI: the block_stmt_iterator where STMT is. Any new stmts created by this
        function can be added here.

   Output:
   If POINTER_ARITHMETIC is defined, this functions returns an offset that
   will be added to a base pointer and used to refer to a memory location. 
   E.g., it will generate stms 1 and 2 above, and return T1.

   FORNOW: we are not trying to be efficient, and just creating the code
   sequence each time from scratch, even if the same offset can be reused.
   TODO: record the index in the array_ref_info or the stmt info and reuse 
   it.

   If POINTER_ARITHMETIC is undefined, this functions returns an index that
   will be usd to index an array, using a pointer as a base.

   FORNOW: We are only handling array accesses with step 1, so the same
   index as for the scalar access can be reused.

   CHECKME: consider using a new index with step = vectorization_factor. 
   This dependes on how we want to handle the loop bound.
*/

static tree
vect_create_index_for_array_ref (tree expr, block_stmt_iterator *bsi)
{
  tree idx;
  tree T0, T1, vec_stmt, mult_expr, new_temp;
  char *new_name;

  if (TREE_CODE (expr) != ARRAY_REF)
    return NULL;

  idx = TREE_OPERAND (expr, 1);

#ifndef POINTER_ARITHMETIC

  /* CHECKME: for now return the same index. May want to handle this
     differently in the future.  */
  return idx;


#else /* POINTER_ARITHMETIC */


  /* Given the array reference array[i], create
     'new_idx = (unsigned int)i * N'
     where N = UNITS_PER_SIMD_WORD

     FORNOW: We only handle loops in which all stmts operate on the same 
             data type; therefore, for all the array accesses in the loop, 
	     the following should hold:
	     UNITS_PER_WORD = 
	     		vectorization_factor * sizeof (data_type (array)).

     FORNOW: The access pattern of all arrays i nthe loop is step 1.
  */

  /*** create: unsigned int T0; ***/

  new_name = vect_get_name_for_new_var (idx);
  T0 = create_tmp_var (unsigned_intSI_type_node, new_name);
  add_referenced_tmp_var (T0);
  DBG_VECT2 (debug_generic_expr (T0));


  /*** create: T0 = (unsigned int)i; ***/

  vec_stmt = build (MODIFY_EXPR, unsigned_intSI_type_node, T0,
                        build1 (NOP_EXPR, unsigned_intSI_type_node, idx));
  new_temp = make_ssa_name (T0, vec_stmt);
  TREE_OPERAND (vec_stmt, 0) = new_temp;
  DBG_VECT2 (fprintf (stderr, "add new stmt: T0 = (unsigned int)i;\n"));
  DBG_VECT2 (debug_generic_expr (vec_stmt));
  bsi_insert_before (bsi, vec_stmt, BSI_SAME_STMT);


  /*** create: unsigned int T1; ***/

  new_name = vect_get_name_for_new_var (idx);
  T1 = create_tmp_var (unsigned_intSI_type_node, new_name);
  add_referenced_tmp_var (T1);
  DBG_VECT2 (debug_generic_expr (T1));


  /*** create: T1 = T0 * N; ***/

  mult_expr = build (MULT_EXPR, unsigned_intSI_type_node,
  			TREE_OPERAND (vec_stmt, 0),
                	build_int_2 (UNITS_PER_SIMD_WORD, 0));
  vec_stmt = build (MODIFY_EXPR, unsigned_intSI_type_node, T1, mult_expr);
  new_temp = make_ssa_name (T1, vec_stmt);
  TREE_OPERAND (vec_stmt, 0) = new_temp;
  DBG_VECT2 (fprintf (stderr, "add new stmt: T1 = T0 * N;\n"));
  DBG_VECT2 (debug_generic_expr (vec_stmt));
  bsi_insert_before (bsi, vec_stmt, BSI_SAME_STMT);

  return (TREE_OPERAND (vec_stmt, 0));

#endif /* POINTER_ARITHMETIC */
}


/* Function get_vectype_for_scalar_type
*/

static tree
get_vectype_for_scalar_type (tree scalar_type)
{
  tree vectype;
  enum machine_mode mode;
  int nbytes;
  int nunits;

  vectype = ((*targetm.vectype_for_scalar_type) (scalar_type));

  if (!vectype)
    return NULL;

  /* FORNOW: Only a single vector size per target is expected.
             Following is a sanity check to verify this assumption.  */

  mode = TYPE_MODE (scalar_type);
  nbytes = GET_MODE_SIZE (mode);
  nunits = UNITS_PER_SIMD_WORD / nbytes;

  if (nunits != GET_MODE_NUNITS (TYPE_MODE (vectype)))
    {
      DBG_VECT (fprintf (stderr, 
		"nbytes = %d, UNITS_PER_SIMD_WORD/nbytes = %d, nunits = %d\n",
		nbytes, nunits, GET_MODE_NUNITS (TYPE_MODE (vectype)))); 
      return NULL;
    }

  return vectype;
}


/* Function vect_align_data_ref

   Handle alignment of a memory accesses.

   FORNOW: Make sure the array is properly aligned. The vectorizer
           currently does not handle unaligned memory accesses.
           This restriction will be relaxed in the future.

   FORNOW: data_ref is an array_ref which alignment can be forced; i.e.,
             the base of the ARRAY_REF is not a pointer but an array.
             This restriction will be relaxed in the future.

   FORNOW: The array is being accessed starting at location 'init';
           We limit vectorization to cases in which init % NUNITS == 0
           (where NUNITS = GET_MODE_NUNITS (TYPE_MODE (vectype))).
           This restriction will be relaxed in the future.
 */

static void
vect_align_data_ref (tree ref, tree stmt)
{
  stmt_vec_info stmt_info = vinfo_for_stmt (stmt);
  tree vectype = STMT_VINFO_VECTYPE (stmt_info);
  tree array_base = get_array_base (ref);

  if (TREE_CODE (TREE_TYPE (array_base)) != ARRAY_TYPE)
    abort();

  if (TYPE_ALIGN (TREE_TYPE (array_base)) < TYPE_ALIGN (vectype))
    {
      /* CHECKME: is this the way to force the alignment of an array base?
      */
      DBG_VECT2 (fprintf (stderr,
      		"force alignment. before: scalar/vec type_align = %d/%d\n",
	     	TYPE_ALIGN (TREE_TYPE (array_base)), TYPE_ALIGN (vectype)));

      TYPE_ALIGN (TREE_TYPE (array_base)) = TYPE_ALIGN (vectype);
    }
}


/* Function vect_create_data_ref.

   Create a memory reference expression for vector access, to be used in a
   vector load/store stmt.

   Input:
   STMT: the stmt that references memory
         FORNOW: a load/store of the form 'var = a[i]'/'a[i] = var'.
   OP: the operand in STMT that is the memory referece
       FORNOW: and array_ref.
   BSI: the block_stmt_iterator where STMT is. Any new stmts created by this
        function can be added here.

   Output:
   1. Declare a new ptr to vector_type, and have it point to the array base.
      For example, for vector of type V8HI:
      v8hi *p0;
      p0 = (v8hi *)&a;

   3. If pointer arithmetic is defined, return '*(p0 + idx)'.
         where idx is the offset in bytes.
      Otherwise return the expression 'p0[idx]',
         where idx is the index used for the scalar expr.

   FORNOW: handle only simple array accesses (step 1).
 */

static tree
vect_create_data_ref (tree ref,
		      tree stmt,
		      block_stmt_iterator *bsi)
{
  tree data_ref;
  tree idx;
  tree base = get_array_base (ref);
  tree vec_stmt, T0;
  tree new_temp;
  stmt_vec_info stmt_info = vinfo_for_stmt (stmt);
  tree vectype = STMT_VINFO_VECTYPE (stmt_info);
  tree ptr_type;
  tree array_ptr;
  tree array_base;
  char *new_name;

  DBG_VECT2 (fprintf (stderr, "create array_ref of type:\n"));
  DBG_VECT2 (debug_generic_expr (vectype));

  vect_align_data_ref (ref, stmt);
  array_base = get_array_base (ref);

  /*** create: vectype *p;  ***/
  new_name = vect_get_name_for_new_var (array_base);
  ptr_type = build_pointer_type (vectype);
  array_ptr = create_tmp_var (ptr_type, new_name);
  add_referenced_tmp_var (array_ptr);

  DBG_VECT2 (debug_generic_expr (array_ptr));


  /*** create: p = (vectype *)&a; ***/
  DBG_VECT2 (fprintf (stderr, "create: p = (vectype *)&a;\n"));
  vec_stmt = build (MODIFY_EXPR, ptr_type, array_ptr,
                     build1 (NOP_EXPR, ptr_type,
			     build1 (ADDR_EXPR, ptr_type, base)));
  new_temp = make_ssa_name (array_ptr, vec_stmt);
  TREE_OPERAND (vec_stmt, 0) = new_temp;
  DBG_VECT2 (fprintf (stderr, "add new stmt: ptr = &array\n"));
  DBG_VECT2 (debug_generic_expr (vec_stmt));
  bsi_insert_before (bsi, vec_stmt, BSI_SAME_STMT);

  idx = vect_create_index_for_array_ref (ref, bsi);

#ifdef POINTER_ARITHMETIC

  /*** create: vectype *T0; ***/

  new_name = vect_get_name_for_new_var (array_base);
  T0 = create_tmp_var (ptr_type, new_name);
  add_referenced_tmp_var (T0);
#if 0
  /* FIXME: for may-alias computation.  */
  if (STMT_VINFO_TYPE (stmt_info) == store_vec_info_type)
    var_ann (T0)->is_dereferenced_store = 1;
  else if (STMT_VINFO_TYPE (stmt_info) == load_vec_info_type)
    var_ann (T0)->is_dereferenced_load = 1;
#endif
  bitmap_set_bit (vars_to_rename, var_ann (array_base)->uid);

  DBG_VECT2 (debug_generic_expr (T0));


  /*** create: T0 = idx + p; ***/

  DBG_VECT2 (fprintf (stderr, "create: T0 = idx + p\n"));
  vec_stmt = build (MODIFY_EXPR, ptr_type, T0,
              build (PLUS_EXPR, ptr_type, TREE_OPERAND (vec_stmt, 0), idx));
  new_temp = make_ssa_name (T0, vec_stmt);
  TREE_OPERAND (vec_stmt, 0) = new_temp;
  DBG_VECT2 (fprintf (stderr, "add new stmt: T0 = idx + p;\n"));
  DBG_VECT2 (debug_generic_expr (vec_stmt));
  bsi_insert_before (bsi, vec_stmt, BSI_SAME_STMT);


  /*** create dataref: '*T0' ***/

  DBG_VECT2 (fprintf (stderr, "create '*T0'\n"));
  data_ref = build1 (INDIRECT_REF, vectype, TREE_OPERAND (vec_stmt, 0));


#else /* POINTER_ARITHMETIC */


  /*** create data ref: 'p[idx]' ***/

  DBG_VECT2 (fprintf (stderr, "create p[idx]\n"));
  data_ref = build (ARRAY_REF, ptr_type, TREE_OPERAND (vec_stmt, 0), idx);


#endif /* POINTER_ARITHMETIC */

  DBG_VECT2 (debug_generic_expr (data_ref));

  return data_ref;
}


/* Function vect_create_destination_var

   Create a new teporary of type VECTYPE.
 */

static tree
vect_create_destination_var (tree scalar_dest, tree vectype)
{
  tree vec_dest;
  char *new_name;

  if (TREE_CODE (scalar_dest) != SSA_NAME)
    abort();

  new_name = vect_get_name_for_new_var (scalar_dest);
  vec_dest =
     create_tmp_var (vectype, new_name);
  add_referenced_tmp_var (vec_dest);

  /* FIXME: introduce new type.   */
  TYPE_ALIAS_SET (TREE_TYPE (vec_dest)) = TYPE_ALIAS_SET (TREE_TYPE (scalar_dest));
  DBG_VECT2 (debug_generic_expr (vec_dest));

  return vec_dest;
}


/* Function vect_transfom_binop.
*/

static tree
vect_transform_binop (tree stmt,
		      block_stmt_iterator *bsi ATTRIBUTE_UNUSED)
{
  tree vec_stmt;
  tree vec_stmt0, vec_stmt1;
  tree vec_dest;
  tree scalar_dest;
  tree operation;
  tree op0 = NULL, op1 = NULL;
  stmt_vec_info stmt_info0 = NULL, stmt_info1 = NULL;
  tree vec_oprnd0, vec_oprnd1;
  stmt_vec_info stmt_info = vinfo_for_stmt (stmt);
  tree vectype = STMT_VINFO_VECTYPE (stmt_info);
  enum tree_code code;
  tree new_temp;

  if (TREE_CODE (stmt) != MODIFY_EXPR)
    abort ();

  scalar_dest = TREE_OPERAND (stmt, 0);
  if (TREE_CODE (scalar_dest) != SSA_NAME)
    abort ();

  operation = TREE_OPERAND (stmt, 1);

  op0 = TREE_OPERAND (operation, 0);
  op1 = TREE_OPERAND (operation, 1);

  if (!op0 || TREE_CODE (op0) != SSA_NAME)
    abort ();

  if (!op1 || TREE_CODE (op1) != SSA_NAME)
    abort ();

  DBG_VECT2 (fprintf (stderr, "transform binop\n"));

  /** Handle def. **/

  vec_dest = vect_create_destination_var (scalar_dest, vectype);


  /** Handle uses - get the vectorized defs from the defining stmts.  **/

  /* FORNOW - we assume that the defining stmt is not a PHI node. This
              restriction will be relaxed in the future.
  */

  stmt_info0 = vinfo_for_stmt (SSA_NAME_DEF_STMT (op0));
  stmt_info1 = vinfo_for_stmt (SSA_NAME_DEF_STMT (op1));

  if (!stmt_info0 || !stmt_info1)
    abort ();

  vec_stmt0 = STMT_VINFO_VEC_STMT (stmt_info0);
  vec_stmt1 = STMT_VINFO_VEC_STMT (stmt_info1);

  if (!vec_stmt0 || !vec_stmt1)
    abort();

  DBG_VECT2 (fprintf (stderr, "defining vec stmts:\n"));
  DBG_VECT2 (debug_generic_expr (vec_stmt0));
  DBG_VECT2 (debug_generic_expr (vec_stmt1));

  /* CHECKME: any cases where the def we want is not TREE_OPERAND 0?  */
  vec_oprnd0 = TREE_OPERAND (vec_stmt0, 0);
  vec_oprnd1 = TREE_OPERAND (vec_stmt1, 0);


  /** arguments are ready. create the new vector stmt.  **/

  code = TREE_CODE (operation);
  vec_stmt = build (MODIFY_EXPR, vectype, vec_dest,
       build (code, vectype, vec_oprnd0, vec_oprnd1));
  new_temp = make_ssa_name (vec_dest, vec_stmt);
  TREE_OPERAND (vec_stmt, 0) = new_temp;

  return vec_stmt;
}


/* Function vect_transfom_store.  */

static tree
vect_transform_store (tree stmt,
		      block_stmt_iterator *bsi)
{
  tree scalar_dest;
  tree vec_stmt, vec_stmt1;
  tree data_ref;
  tree op;
  stmt_vec_info stmt_info1;
  tree vec_oprnd1;
  stmt_vec_info stmt_info = vinfo_for_stmt (stmt);
  tree vectype = STMT_VINFO_VECTYPE (stmt_info);
  tree def_stmt;

  if (TREE_CODE (stmt) != MODIFY_EXPR)
    abort();

  scalar_dest = TREE_OPERAND (stmt, 0);

  if (TREE_CODE (scalar_dest) != ARRAY_REF)
    abort();

  op = TREE_OPERAND (stmt, 1);

  if (TREE_CODE (op) != SSA_NAME)
    abort();

  DBG_VECT2 (fprintf (stderr, "build vstore\n"));

  /** Handle def.  **/
  data_ref = vect_create_data_ref (scalar_dest, stmt, bsi);
  if (!data_ref)
    abort ();


  /** Handle uses - get the vectorized def from the defining stmt.  **/
  def_stmt = SSA_NAME_DEF_STMT (op);

  /* FORNOW - we assume that the defining stmt is not a PHI node. This
              restriction will be relaxed in the future.
  */

  stmt_info1 = vinfo_for_stmt (def_stmt);
  if (!stmt_info1)
    abort ();

  vec_stmt1 = STMT_VINFO_VEC_STMT (stmt_info1);
  if (!vec_stmt1)
    abort ();

  /* CHECKME: any cases where the def we want is not TREE_OPERAND 0? */
  vec_oprnd1 = TREE_OPERAND (vec_stmt1, 0);

  /** arguments are ready. create the new vector stmt.  **/
  vec_stmt =
    build (MODIFY_EXPR, vectype, data_ref, vec_oprnd1);

  /* CHECKME: vect_set_vdefs_for_stmt? */

  return vec_stmt;
}


/* Function vect_transform_load.  */

static tree
vect_transform_load (tree stmt,
		     block_stmt_iterator *bsi)
{
  tree vec_stmt;
  tree scalar_dest;
  tree vec_dest = NULL;
  tree data_ref = NULL;
  tree op;
  stmt_vec_info stmt_info = vinfo_for_stmt (stmt);
  tree vectype = STMT_VINFO_VECTYPE (stmt_info);
  tree new_temp;

  DBG_VECT2 (fprintf (stderr, "vectorize load!\n"));

  if (TREE_CODE (stmt) != MODIFY_EXPR)
    abort ();

  scalar_dest = TREE_OPERAND (stmt, 0);
  if (TREE_CODE (scalar_dest) != SSA_NAME)
    abort ();

  op = TREE_OPERAND (stmt, 1);

  if (TREE_CODE (op) != ARRAY_REF)
    abort ();

  DBG_VECT2 (fprintf (stderr, "transform load\n"));

  /** Handle def.  **/
  vec_dest = vect_create_destination_var (scalar_dest, vectype);

  /** Handle use.  **/
  data_ref = vect_create_data_ref (op, stmt, bsi);

  if (!data_ref)
    abort ();

  DBG_VECT2 (fprintf (stderr, "build vload\n"));

  /** arguments are ready. create the new vector stmt.  **/
  vec_stmt =
    build (MODIFY_EXPR, vectype, vec_dest, data_ref);
  DBG_VECT2 (debug_generic_expr (vec_stmt));
  new_temp = make_ssa_name (vec_dest, vec_stmt);
  TREE_OPERAND (vec_stmt, 0) = new_temp;

  /* CHECKME: vect_set_vuses_for_stmt? */

  return vec_stmt;
}


/* Function vect_transform_stmt.
*/

static bool
vect_transform_stmt (tree stmt,
		     block_stmt_iterator *bsi)
{
  bool is_store = false;
  tree vec_stmt = NULL;
  stmt_vec_info stmt_info = vinfo_for_stmt (stmt);

  switch (STMT_VINFO_TYPE (stmt_info))
  {
  case binop_vec_info_type:
    vec_stmt = vect_transform_binop (stmt, bsi);
    break;

  case load_vec_info_type:
    vec_stmt = vect_transform_load (stmt, bsi);
    break;

  case store_vec_info_type:
    vec_stmt = vect_transform_store (stmt, bsi);
    is_store = true;
    break;

  default:
    DBG_VECT (fprintf (stderr, "stmt not supported\n"));
    abort();
  }

  DBG_VECT2 (fprintf (stderr, "add new stmt\n"));
  bsi_insert_before (bsi, vec_stmt, BSI_SAME_STMT);

  DBG_VECT2 (fprintf (stderr, "record vdef in vinfo\n"));
  STMT_VINFO_VEC_STMT (stmt_info) = vec_stmt;

  DBG_VECT2 (fprintf (stderr, "done with stmt\n"));
  DBG_VECT2 (debug_tree (vec_stmt));

  return is_store;
}


/* Function vect_transform_loop_bound
*/

static void
vect_transform_loop_bound (loop_vec_info loop_vinfo)
{
  struct loop *loop = LOOP_VINFO_LOOP(loop_vinfo);
  tree expr;
  tree test, op0, op1;
  tree access_fn;
  tree init, step;
  int old_N, vf;
  int init_val, step_val, new_N;
  int old_bound, new_bound;

  /* FORNOW: assuming the loop bound is known.  */
  if (!LOOP_VINFO_NITERS_KNOWN_P (loop_vinfo))
    abort ();

  old_N = LOOP_VINFO_NITERS (loop_vinfo);
  vf = LOOP_VINFO_VECT_FACTOR (loop_vinfo);

  /* FORNOW:
     assuming number-of-iterations divides by the vectorization factor.  */
  if (old_N % vf)
    abort ();

  expr = LOOP_VINFO_EXIT_COND (loop_vinfo);
  if (!expr)
    abort();

  /* FORNOW:
     expecting an exit condition of the form:
     i <= N
     where:
     - i has a simple iv evolution
     - N is a known constant
   */

   if (TREE_CODE (expr) != COND_EXPR)
     abort ();
   test = TREE_OPERAND (expr, 0);
   if (TREE_CODE (test) != LE_EXPR)
     abort ();

   op0 = TREE_OPERAND (test, 0);
   op1 = TREE_OPERAND (test, 1);

   if (TREE_CODE (op0) != SSA_NAME)
     abort ();

   DBG_VECT2 (fprintf (stderr, "transform loop bound: call monev analyzer!\n"));
   access_fn = iccp_determine_evolution_function (loop, op0);
   if (!access_fn)
     {
       DBG_VECT (fprintf (stderr, "No Access function."));
       abort ();
     }
   DBG_VECT2 (fprintf (stderr, "Access function of loop_exit cond var:\n"));
   DBG_VECT2 (debug_generic_expr (access_fn));

   if (!vect_is_simple_iv_evolution (access_fn, &init, &step))
     abort ();

   /* FORNOW: vectorization ir restricted to cases in which the loop iv
              evolution has constant step and bounds, such that the number
              of iterations can be determined.
    */

   if (TREE_CODE (op1) != INTEGER_CST ||
       TREE_CODE (init) != INTEGER_CST ||
       TREE_CODE (step) != INTEGER_CST)
     abort ();

   /* CHECKME */
   if (TREE_INT_CST_HIGH (op1) != 0 ||
       TREE_INT_CST_HIGH (init) != 0 ||
       TREE_INT_CST_HIGH (step) != 0)
     abort ();

   old_bound = TREE_INT_CST_LOW (op1);
   init_val = TREE_INT_CST_LOW (init);
   step_val = TREE_INT_CST_LOW (step);

   if (step_val == 0)
     abort ();

   /* Just a sanity check.
      CHECKME: revisit this computation when more general cases are allowed.
    */
   if (((old_bound - init_val + 1) / step_val) != old_N)
     abort();


   /* Calculate the number of iteratoins of the vectorized loop.  */

   new_N = old_N / vf;

   /* CHECKME: revisit this computation when more general cases are allowed.
    */
   new_bound = (new_N * step_val) + init_val - 1;

   DBG_VECT2 (fprintf (stderr,
   	      "old_bound %d, new_bound %d, old_niters %d, new_niters %d\n",
               old_bound, new_bound, old_N, new_N));

   TREE_INT_CST_LOW (op1) = new_bound;

   DBG_VECT2 (debug_generic_expr (op1));
   DBG_VECT2 (debug_generic_expr (expr));
}


/* Function vect_transform_loop.  */

static void
vect_transform_loop (loop_vec_info loop_vinfo)
{
  struct loop *loop = LOOP_VINFO_LOOP(loop_vinfo);
  basic_block *bbs = LOOP_VINFO_BBS(loop_vinfo);
  int nbbs = loop->num_nodes;
  int vectorization_factor = LOOP_VINFO_VECT_FACTOR(loop_vinfo);
  block_stmt_iterator si;
  int i;

  DBG_VECT (fprintf (stderr, "\n<<vec_transform_loop>>\n"));

  /* CHECKME: FORNOW the vectorizer supports only loops which body consist
     of one basic block + header. When the vectorizer will support more
     involved loop forms, the order by which the BBs are traversed need
     to be considered.
   */

  for (i = 0; i < nbbs; i++)
    {
      basic_block bb = bbs[i];

      for (si = bsi_start (bb); !bsi_end_p (si); )
        {
          tree stmt = bsi_stmt (si);
          stmt_vec_info stmt_info;
	  tree vectype;
          bool is_store;

          DBG_VECT2 (fprintf (stderr, "\n-----\nvectorizing statement:\n"));
          DBG_VECT2 (debug_generic_stmt (stmt));

          stmt_info = vinfo_for_stmt (stmt);
          if (!stmt_info)
            {
              DBG_VECT (fprintf (stderr, "no stmt info!\n"));
              abort ();
            }

          if (!STMT_VINFO_RELEVANT_P (stmt_info))
            {
              bsi_next (&si);
              continue;
            }

          /* FORNOW: Verify that all stmts operate on the same number of
	             units and no inner unrolling is necessary.  */
	  vectype = STMT_VINFO_VECTYPE (stmt_info);
          if (GET_MODE_NUNITS (TYPE_MODE (vectype)) != vectorization_factor)
            {
              DBG_VECT (fprintf (stderr,
	      			"nunits != vectorization factor\n"));
	      abort ();
            }

          /* -------- vectorize statement ------------ */
          DBG_VECT2 (fprintf (stderr, "vectorize statement.\n"));
          is_store = vect_transform_stmt (stmt, &si);

          if (is_store)
	    {
              /* free the attched stmt_vec_info and remove the stmt.  */
              stmt_ann_t ann = stmt_ann (stmt);
              free (stmt_info);
              set_stmt_info (ann, NULL);

	      bsi_remove (&si);
              continue;
            }

          bsi_next (&si);

        }  /* stmts in BB */
    } /* BBs in loop */


  vect_transform_loop_bound (loop_vinfo);
  DBG_VECT (fprintf (stderr, "\n<<Success! loop vectorized.>>\n"));
}


/* Function vect_is_supportable_binop.

   Verify that STMT performs a binary operation and can be vectorized.  */

static bool
vect_is_supportable_binop (tree stmt)
{
  stmt_vec_info stmt_info = vinfo_for_stmt (stmt);
  tree operation;
  enum tree_code code;
  tree op0, op1;
  enum machine_mode vec_mode;
  optab binoptab;
  tree vectype = STMT_VINFO_VECTYPE (stmt_info);

  /* Is binop? */

  if (TREE_CODE (stmt) != MODIFY_EXPR)
    return false;

  if (TREE_CODE (TREE_OPERAND (stmt, 0)) != SSA_NAME)
    return false;

  operation = TREE_OPERAND (stmt, 1);
  code = TREE_CODE (operation);

  switch (code)
  {
  case PLUS_EXPR:
    binoptab = add_optab;
    break;
  case MULT_EXPR:
    binoptab = smul_optab;
    break;
  case MINUS_EXPR:
    binoptab = sub_optab;
    break;
  default:
    return false;
  }

  op0 = TREE_OPERAND (operation, 0);
  op1 = TREE_OPERAND (operation, 1);

  if (!op0 || TREE_CODE (op0) != SSA_NAME)
    return NULL;

  if (!op1 || TREE_CODE (op1) != SSA_NAME)
    return NULL;

  /* Suppotable by target?  */

  if (!binoptab)
    return NULL;

  vec_mode = TYPE_MODE (vectype);

  if (binoptab->handlers[(int) vec_mode].insn_code == CODE_FOR_nothing)
    {
      DBG_VECT (fprintf (stderr, "op not supported by target\n"));
      return false;
    }

  /* FORNOW: Not considering the cost.  */

  STMT_VINFO_TYPE (stmt_info) = binop_vec_info_type;

  return true;
}


/* Function vect_is_supportable_store.

   Verify that STMT performs a store to memory operation,
   and can be vectorized.  */

static bool
vect_is_supportable_store (tree stmt)
{
  stmt_vec_info stmt_info = vinfo_for_stmt (stmt);
  tree scalar_dest;
  tree op;

  /* Is vectorizable store? */

  if (TREE_CODE (stmt) != MODIFY_EXPR)
    return false;

  scalar_dest = TREE_OPERAND (stmt, 0);

  if (TREE_CODE (scalar_dest) != ARRAY_REF)
    return false;

  op = TREE_OPERAND (stmt, 1);

  if (TREE_CODE (op) != SSA_NAME)
    return false;

  if (!STMT_VINFO_DATA_REF (stmt_info))
    return false;

  /* Previous analysis steps have already verified that the data ref is
     vectorizable (w.r.t data dependences, access pattern, etc).  */

  /* FORNOW: Not considering the cost.  */

  STMT_VINFO_TYPE (stmt_info) = store_vec_info_type;

  return true;
}


/* Function vect_is_supportable_load.

   Verify that STMT performs a load from memory operation,
   and can be vectorized.  */

static bool
vect_is_supportable_load (tree stmt)
{
  stmt_vec_info stmt_info = vinfo_for_stmt (stmt);
  tree scalar_dest;
  tree op;

  /* Is vectorizable load? */

  if (TREE_CODE (stmt) != MODIFY_EXPR)
    return false;

  scalar_dest = TREE_OPERAND (stmt, 0);
  if (TREE_CODE (scalar_dest) != SSA_NAME)
    return false;

  op = TREE_OPERAND (stmt, 1);

  if (TREE_CODE (op) != ARRAY_REF)
    return false;

  if (!STMT_VINFO_DATA_REF (stmt_info))
    return false;

  /* Previous analysis steps have already verified that the data ref is
     vectorizable (w.r.t data dependences, access pattern, etc).  */

  /* FORNOW: Not considering the cost.  */

  STMT_VINFO_TYPE (stmt_info) = load_vec_info_type;

  return true;
}


/* Function vect_analyze_operations.

   Scan the loop stmts and make sure they are all vectorizable.  */

static bool
vect_analyze_operations (loop_vec_info loop_vinfo)
{
  struct loop *loop = LOOP_VINFO_LOOP(loop_vinfo);
  basic_block *bbs = LOOP_VINFO_BBS(loop_vinfo);
  int nbbs = loop->num_nodes;
  block_stmt_iterator si;
  int vectorization_factor = 0;
  int i;
  bool ok;

  DBG_VECT (fprintf (stderr, "\n<<vect_analyze_operations>>\n"));

  for (i = 0; i < nbbs; i++)
    {
      basic_block bb = bbs[i];

      for (si = bsi_start (bb); !bsi_end_p (si); bsi_next (&si))
        {
          tree stmt = bsi_stmt (si);
          int nunits;
          stmt_vec_info stmt_info = vinfo_for_stmt (stmt);
	  tree vectype;
          dataflow_t df;
  	  int j, num_uses;

          DBG_VECT2 (fprintf (stderr, "\n-------\nexamining statement:\n"));
          DBG_VECT2 (debug_generic_stmt (stmt));

          if (!stmt_info)
            {
              DBG_VECT (fprintf (stderr, "no stmt info?\n"));
              DBG_VECT (debug_generic_stmt (stmt));
              abort();
            }

          /* skip stmts which do not need to be vectorized.
	     this is expected to include:
             - the COND_EXPR which is the loop exit condition
             - any LABEL_EXPRs in the loop
             - computations that are used only for array indexing or loop 
	       control
           */

          if (! STMT_VINFO_RELEVANT_P (stmt_info))
            {
              DBG_VECT2 (fprintf (stderr, "irrelevant\n"));
              continue;
	    }

          if (TREE_CODE (stmt) != MODIFY_EXPR)
            {
              DBG_VECT (fprintf (stderr, "not a MODIFY_EXPR\n"));
              DBG_VECT (debug_generic_stmt (stmt));
              return false;
            }

          if (VECTOR_MODE_P (TYPE_MODE (TREE_TYPE (stmt))))
            {
              DBG_VECT (fprintf (stderr, "vector stmt in loop!\n"));
              DBG_VECT (debug_generic_stmt (stmt));
              return false;
            }

          vectype = get_vectype_for_scalar_type (TREE_TYPE (stmt));
          if (! vectype)
	    {
              DBG_VECT (fprintf (stderr, "no vectype for stmt.\n"));
              DBG_VECT (debug_generic_stmt (stmt));
              return false;
            }

          STMT_VINFO_VECTYPE (stmt_info) = vectype;

          ok = (vect_is_supportable_binop (stmt)
	    	|| vect_is_supportable_load (stmt)
	    	|| vect_is_supportable_store (stmt));

          if (!ok)
            {
              DBG_VECT (fprintf (stderr, "stmt not supported.\n"));
              DBG_VECT (debug_generic_stmt (stmt));
              return false;
            }

          /* FORNOW: Make sure that the def of this stmt is not used out
	     side the loop. This restriction will be relaxed in the future.
           */

  	  df = get_immediate_uses (stmt);
  	  num_uses = num_immediate_uses (df);
  	  for (j = 0; j < num_uses; j++)
    	    {
      	      tree use = immediate_use (df, j);
      	      int use_depth = bb_for_stmt (use)->loop_father->depth;
      	      int loop_depth = loop->depth;
              /* CHECKME: better check if bb belongs to the loop?  */
      	      if (use_depth < loop_depth)
            	{
              	  DBG_VECT (fprintf (stderr,"def used out of loop:\n"));
              	  DBG_VECT (debug_generic_stmt (use));
              	  return false;
            	}
    	    }

 	  nunits = GET_MODE_NUNITS (TYPE_MODE (vectype));
	  DBG_VECT2 (fprintf (stderr, "nunits = %d\n", nunits));

          if (vectorization_factor)
	    {
	      /* FORNOW: don't allow mixed units. 
                         This restriction will be relaxed in the future.  */
	      if (nunits != vectorization_factor)
                {
		  DBG_VECT (fprintf (stderr, "mixed types unsupported.\n"));
                  DBG_VECT (debug_generic_stmt (stmt));
                  return false;
                }
	    }
          else
	    vectorization_factor = nunits;
        }
    }

  /* TODO: Analayze cost. Decide if worth while to vectorize.  */

  LOOP_VINFO_VECT_FACTOR (loop_vinfo) = vectorization_factor;

  /* FORNOW: handle only cases where the loop bound divides by the
     vectorization factor. */

  DBG_VECT2 (fprintf (stderr, "vectorization_factor = %d, niters = %d\n",
   		      vectorization_factor,LOOP_VINFO_NITERS (loop_vinfo)));

  if (!LOOP_VINFO_NITERS_KNOWN_P (loop_vinfo) ||
      LOOP_VINFO_NITERS (loop_vinfo) % vectorization_factor != 0)
    {
      DBG_VECT (fprintf (stderr,
      		"loop bound unknown or doesn't divide by %d\n", 
		vectorization_factor));
      return false;
    }

  return true;
}


/* Function get_address_calculation_operands

   If the STMT represented by STMT_INFO has a data-reference,
   return the operands in STMT that are used for the address calculation of
   the data reference.
   For example, in a data_ref 'a[i_1]', the operand 'i_1' will be returned.

   FORNOW: expecting ref to be a one dimentional ARRAY_REF.
           (i.e, only one operand is returned.
*/

static tree
get_address_calculation_operands (stmt_vec_info stmt_info)
{
  struct data_reference *dr;
  tree index_op;
  tree ref;

  if (!stmt_info)
    return NULL;

  dr = STMT_VINFO_DATA_REF (stmt_info);

  if (!dr)
    return NULL;

  ref = DR_REF (dr);
  DBG_VECT2 (fprintf (stderr, "stmt has a data ref\n"));

  /* FORNOW: handling only one dimentional arrays.  */
  if (TREE_CODE (ref) != ARRAY_REF ||
      TREE_CODE (TREE_OPERAND (ref, 0)) == ARRAY_REF)
    {
      DBG_VECT (fprintf (stderr, "unexpected form of data ref:\n"));
      DBG_VECT (debug_generic_expr (ref));
      return NULL;
    }

  index_op = TREE_OPERAND (ref, 1);
  DBG_VECT2 (debug_generic_expr (index_op));

  return index_op;
}


/* Function vect_is_simple_iv_evolution.

   FORNOW: A simple evolution of an induction variables in the loop is
   considered a polynomial evolution with step 1.
*/

static bool
vect_is_simple_iv_evolution (tree access_fn, tree *init, tree *step)
{
   tree init_expr;
   tree step_expr;

   if (!evolution_function_is_affine_multivariate_p (access_fn))
     return false;

   if (TREE_CODE (access_fn) != POLYNOMIAL_CHREC)
     return false;

   step_expr = CHREC_RIGHT (access_fn);
   init_expr = CHREC_LEFT (access_fn);

   DBG_VECT2 (fprintf (stderr, "step:\n"));
   DBG_VECT2 (debug_generic_expr (step_expr));
   DBG_VECT2 (fprintf (stderr, "init:\n"));
   DBG_VECT2 (debug_generic_expr (init_expr));

   *init = init_expr;
   *step = step_expr;

   if (TREE_CODE (step_expr) != INTEGER_CST)
     return false;

   if (!integer_onep (step_expr))
     return false;

   return true;
}


/* Function vect_analyze_scalar_cycles.

   Examine the cross iteration def-use cycles of scalar variables, by 
   analyzing the loop (scalar) PHIs; verify that the cross iteration def-use 
   cycles that they represent do not impede vectorization.

   FORNOW: Reduction as in the following loop, is not supported yet:
              loop1:
              for (i=0; i<N; i++)
                 sum += a[i];
   	   The cross-iteration cycle corresponding to variable 'sum' will be
	   considered too complicated and will impede vectorization.

   FORNOW: Induction as in the following loop, is not supported yet:
              loop2:
              for (i=0; i<N; i++)
                 a[i] = i;

           However, the following loop *is* vectorizable:
              loop3:
              for (i=0; i<N; i++)
                 a[i] = b[i];

           In both loops there exists a def-use cycle for the variable i:
              loop: i_2 = PHI (i_0, i_1)
                    a[i_2] = ...;
                    i_1 = i_2 + 1;
                    GOTO loop;

           The evolution of the above cycle is considered simple enough,
	   however, we also check that the cycle does not need to be 
	   vectorized, i.e - we check that the variable that this cycle 
	   defines is only used for array indexing or in stmts that do not 
	   need to be vectorized. This is not the case in loop2, but it 
	   *is* the case in loop3.
 */

static bool
vect_analyze_scalar_cycles (loop_vec_info loop_vinfo)
{
  tree phi;
  struct loop *loop = LOOP_VINFO_LOOP(loop_vinfo);
  basic_block bb = loop->header;
  dataflow_t df;
  int num_uses;
  tree dummy;

  DBG_VECT (fprintf (stderr, "\n<<vect_analyze_scalar_evolutions>>\n"));

  for (phi = phi_nodes (bb); phi; phi = TREE_CHAIN (phi))
    {
      int i;
      tree access_fn = NULL;

      DBG_VECT2 (fprintf (stderr,"Analyze phi\n"));
      DBG_VECT2 (debug_generic_expr (phi));

      /* Skip virtual phi's. The data dependences that are associated with
         virtual defs/uses (i.e., memory accesses) are analyzed elsewhere.  
       */

      /* CHECKME: correct way to check for a virtual phi?  */

      if (!is_gimple_reg (SSA_NAME_VAR (PHI_RESULT (phi))))
	{
          DBG_VECT2 (fprintf (stderr,"virtual phi. skip.\n"));
          continue;
        }

      /* Analyze the evolution function. */

      /* FORNOW: The only scalar cross-iteration cycles that we allow are
         those of the loop induction variable;
         Furthermore, if that induction variable is used in an operation 
	 that needs to be vectorized (i.e, is not solely used to index 
	 arrays and check the exit condition) - we do not support its 
	 vectorization Yet.
       */

      /* 1. Verify that it is an IV with a simple enough access pattern. */

      DBG_VECT2 (fprintf (stderr, "analyze cycles: call monev analyzer!\n"));
      access_fn = 
      	      iccp_determine_evolution_function (loop, PHI_RESULT (phi));
      if (! access_fn)
        {
          DBG_VECT (fprintf (stderr, "No Access function."));
          return false;
        }

      DBG_VECT2 (fprintf (stderr, "Access function of PHI: "));
      DBG_VECT2 (debug_generic_expr (access_fn));

      if (! vect_is_simple_iv_evolution (access_fn, &dummy, &dummy))
        {
          DBG_VECT (fprintf (stderr,"unsupported cross iter cycle.\n"));
          return false;
        }

      /* 2. Verify that this variable is only used in stmts that do not need
            to be vectorized.  */

      df = get_immediate_uses (phi);
      num_uses = num_immediate_uses (df);
      for (i = 0; i < num_uses; i++)
        {
          tree use = immediate_use (df, i);
          stmt_vec_info stmt_info = vinfo_for_stmt (use);
          tree index_op = get_address_calculation_operands(stmt_info);

          if (stmt_info &&
	      STMT_VINFO_RELEVANT_P (stmt_info) &&
	      (!index_op || PHI_RESULT (phi) != index_op))
            {

              DBG_VECT (fprintf (stderr,
	         "induction var needs to be vectorized. Unsupported.\n"));
              DBG_VECT (debug_generic_expr (use));
              return false;
            }
        }
    }

  return true;
}


/* Function get_array_base.  */

static tree
get_array_base (tree expr)
{
  tree expr1;
  if (TREE_CODE (expr) != ARRAY_REF)
    abort();

  expr1 = TREE_OPERAND (expr, 0);
  while (TREE_CODE (expr1) == ARRAY_REF)
    expr1 = TREE_OPERAND (expr1, 0);

  return expr1;
}


/* Function vect_analyze_data_ref_dependence
*/

static bool
vect_analyze_data_ref_dependence (struct data_reference *dra,
				  struct data_reference *drb)
{
  /* FORNOW: use most trivial and conservative test.  */

  /* CHECKME: this test holds only if the array base is not a pointer.
              This had been verified by analyze_data_refs.
	      This restriction will be relaxed in the future.  */

  if (! vec_array_base_name_differ_p (dra, drb))
    {
      DBG_VECT (fprintf (stderr, 
		"vect_analyze_data_ref_dependence: same base\n"));
      return false;
    }

  return true;
}


/* Function vect_analyze_data_ref_dependences.

   Examine all the data references in the loop, and make sure there do not
   exist any data dependences between them.

   FORNOW: We do not contruct a data dependence graph and try to deal with
           dependences, but fail at the first data dependence that we 
	   encounter.

   FORNOW: We only handle array references.

   FORNOW: We apply a trivial conservative dependence test.
*/

static bool
vect_analyze_data_ref_dependences (loop_vec_info loop_vinfo)
{
  unsigned int i, j;
  varray_type loop_write_refs = LOOP_VINFO_DATAREF_WRITES (loop_vinfo);
  varray_type loop_read_refs = LOOP_VINFO_DATAREF_READS (loop_vinfo);

  /* examine store-store (output) dependences */
  DBG_VECT2 (fprintf (stderr, "compare all store-store pairs\n"));
  for (i = 0; i < VARRAY_ACTIVE_SIZE (loop_write_refs); i++)
    {
      for (j = i + 1; j < VARRAY_ACTIVE_SIZE (loop_write_refs); j++)
        {
          struct data_reference *dra = VARRAY_GENERIC_PTR (loop_write_refs, i);
          struct data_reference *drb = VARRAY_GENERIC_PTR (loop_write_refs, j);
	  bool ok = vect_analyze_data_ref_dependence (dra, drb);
          if (!ok)
            return false;
        }
    }

  /* examine load-store (true/anti) dependences */
  DBG_VECT2 (fprintf (stderr, "compare all load-store pairs\n"));
  for (i = 0; i < VARRAY_ACTIVE_SIZE (loop_read_refs); i++)
    {
      for (j = 0; j < VARRAY_ACTIVE_SIZE (loop_write_refs); j++)
        {
          struct data_reference *dra = VARRAY_GENERIC_PTR (loop_read_refs, i);
          struct data_reference *drb = VARRAY_GENERIC_PTR (loop_write_refs, j);
          bool ok = vect_analyze_data_ref_dependence (dra, drb);
          if (!ok)
            return false;
        }
    }

  return true;
}


/* Function vect_analyze_data_ref_access.
*/

static bool
vect_analyze_data_ref_access (struct data_reference *dr)
{
  varray_type access_fns = DR_ACCESS_FNS (dr);
  tree stmt = DR_EXPR (dr);
  tree vectype;
  tree access_fn;
  tree init, step;
  int init_val;

  /* FORNOW: handle only one dimentional arrays.
  	     This restriction will be relaxed in the future. */
  if (VARRAY_ACTIVE_SIZE (access_fns) != 1)
    {
      DBG_VECT (fprintf (stderr, "multi dimentional array reference.\n"));
      return false;
    }
  access_fn = DR_ACCESS_FN (dr, 0);

  if (!vect_is_simple_iv_evolution (access_fn, &init, &step))
    {
      DBG_VECT (fprintf (stderr, "too complicated access function\n"));
      DBG_VECT (debug_generic_expr (access_fn));
      return false;
    }

  /* FORNOW: In order to simplify the handling of alignment, in addition
             to the above we also make sure that the first location
             at which the array is accessed ('init') is on an 'NUNITS'
             boundary, since we are also making sure that the array base
             is aligned. This restiction will be relaxed in the future.
  */
  if (TREE_CODE (init) != INTEGER_CST)
    {
      DBG_VECT (fprintf (stderr, "init not INTEGER_CST\n"));
      return false;
    }

  /* CHECKME */
  if (TREE_INT_CST_HIGH (init) != 0)
    {
      DBG_VECT (fprintf (stderr, "init CST_HIGH != 0\n"));
      return false;
    }

  init_val = TREE_INT_CST_LOW (init);

  vectype = get_vectype_for_scalar_type (TREE_TYPE (stmt));
  if (! vectype)
    {
      DBG_VECT (fprintf (stderr, "no vectype for stmt.\n"));
      DBG_VECT (debug_generic_expr (stmt));
      return false;
    }

  if (init_val % GET_MODE_NUNITS (TYPE_MODE (vectype)))
    {
      DBG_VECT (fprintf (stderr, "first access not aligned.\n"));
      return false;
    }

  return true;
}


/* Function vect_analyze_data_ref_accesses.

   Analyze the access pattern of all the data references in the loop.

   FORNOW: the only access pattern that is considered vectorizable is a 
   	   simple step 1 (consecutive) access.

   FORNOW: handle only one dimentioanl arrays.
 */

static bool
vect_analyze_data_ref_accesses (loop_vec_info loop_vinfo)
{
  unsigned int i;
  varray_type loop_write_datarefs = LOOP_VINFO_DATAREF_WRITES (loop_vinfo);
  varray_type loop_read_datarefs = LOOP_VINFO_DATAREF_READS (loop_vinfo);

  DBG_VECT (fprintf (stderr, "\n<<vect_analyze_data_ref_accesses>>\n"));

  for (i = 0; i < VARRAY_ACTIVE_SIZE (loop_write_datarefs); i++)
    {
      struct data_reference *dr = VARRAY_GENERIC_PTR (loop_write_datarefs, i);
      bool ok = vect_analyze_data_ref_access (dr);
      if (!ok)
        return false;
    }

  for (i = 0; i < VARRAY_ACTIVE_SIZE (loop_read_datarefs); i++)
    {
      struct data_reference *dr = VARRAY_GENERIC_PTR (loop_read_datarefs, i);
      bool ok = vect_analyze_data_ref_access (dr);
      if (!ok)
        return false;
    }

  return true;
}


/* Function vect_analyze_data_refs.

   Find all the data references in the loop.

   FORNOW: Handle only one dimentional ARRAY_REFs which base is really an
           array (not a pointer)
 */

static bool
vect_analyze_data_refs (loop_vec_info loop_vinfo)
{
  struct loop *loop = LOOP_VINFO_LOOP (loop_vinfo);
  basic_block *bbs = LOOP_VINFO_BBS (loop_vinfo);
  int nbbs = loop->num_nodes;
  block_stmt_iterator si;
  int j;
  struct data_reference *dr;

  DBG_VECT (fprintf (stderr, "\n<<vect_analyze_data_refs>>\n"));

  for (j = 0; j < nbbs; j++)
    {
      basic_block bb = bbs[j];
      for (si = bsi_start (bb); !bsi_end_p (si); bsi_next (&si))
        {
          tree stmt = bsi_stmt (si);
          stmt_vec_info stmt_info = vinfo_for_stmt (stmt);
	  vdef_optype vdefs = STMT_VDEF_OPS (stmt);
	  vuse_optype vuses = STMT_VUSE_OPS (stmt);
          varray_type datarefs = NULL;
          int nvuses = 0, nvdefs = 0;
          tree ref = NULL;
 
          /* CHECKME: Relying on the fact that there exists a data-ref 
                      in stmt, if and only if it has vuses/vdefs
           */
            
          if (!vuses && !vdefs)
            continue;

          if (vuses)
            nvuses = NUM_VUSES (vuses);
          if (vdefs)
            nvdefs = NUM_VDEFS (vdefs);

          if (nvuses + nvdefs != 1)
            {
              /* CHECKME: multiple vdefs/vuses in a GIMPLE stmt are
                 assumed to indicate a non vectorizable stmt (e.g, ASM,
		 CALL_EXPR) or the presence of an aliasing problem. The
                 first case is ruled out durint vect_analyze_operations;
                 As for the second case, currently the vuses/vdefs are
                 meaningless as they are too conservative. We therefore
                 ignore them.  */

              DBG_VECT2 (fprintf (stderr,"unexpected multiple vops\n"));
              DBG_VECT2 (debug_generic_stmt (stmt));
              /* return false; */
            }

          if (TREE_CODE (stmt) != MODIFY_EXPR)
            {
              /* CHECKME: a vdef/vuse in a GIMPLE stmt is assumed to
  	                  appear only in a MODIFY_EXPR.  */

              DBG_VECT (fprintf (stderr,"unexpected vops in stmt\n"));
              DBG_VECT (debug_generic_stmt (stmt));
              return false;
            }

          if (vuses)
            {
              if (TREE_CODE (TREE_OPERAND (stmt, 1)) == ARRAY_REF)
                {
                  ref = TREE_OPERAND (stmt, 1);
                  datarefs = LOOP_VINFO_DATAREF_READS (loop_vinfo);
		}
            }

          if (vdefs)
            {
              if (TREE_CODE (TREE_OPERAND (stmt, 0)) == ARRAY_REF)
                {
                  ref = TREE_OPERAND (stmt, 0);
                  datarefs = LOOP_VINFO_DATAREF_WRITES (loop_vinfo);
		}
            }

          if (!ref)
            {
              /* A different type of data reference (pointer?, struct?)
                 FORNOW: Do not attempt to handle.  */

              DBG_VECT (fprintf (stderr,"unhandled non-array data ref\n"));
              DBG_VECT (debug_generic_stmt (stmt));
              return false;
            }

          dr = vec_analyze_array (loop, stmt, ref);

          /* FORNOW: make sure that the array is one dimentional.
	             This restriction will be relaxed in the future.
	   */
          if (TREE_CODE (TREE_OPERAND (ref, 0)) == ARRAY_REF)
            {
              DBG_VECT (fprintf (stderr,"unhandled 2D-array data ref\n"));
              DBG_VECT (debug_generic_stmt (stmt));
              return false;
            }

          /* FORNOW: make sure that the base of the array ref is really
             an array (not a pointer) which alignment can be forced
             (because we do not handle misalignment yet).

	     CHECKME: correct check?
	   */
          if (TREE_CODE (TREE_TYPE (TREE_OPERAND (ref, 0))) != ARRAY_TYPE)
            {
              DBG_VECT (fprintf (stderr,
	      				"unhandled ptr-based array ref\n"));
              DBG_VECT (debug_generic_stmt (stmt));
              return false;
            }

          /* In addition to the above, we also check that the first
	     location in the array that is being accessed is aligned
	     (in analyze_data_ref_accesses).  */

          VARRAY_PUSH_GENERIC_PTR (datarefs, dr);
          STMT_VINFO_DATA_REF (stmt_info) = dr;
        }
    }

    return true;
}


/* Utility functions used by vect_mark_stmts_to_be_vectorized.
   Implementation inspired by tree-ssa-dce.c.  */

/* Function vect_mark_relevant.

   Mark STMT as "relevant for vectorization" and add it to WORKLIST.
 */

static void
vect_mark_relevant (varray_type worklist, tree stmt)
{
  stmt_vec_info stmt_info;

  DBG_VECT2 (fprintf (stderr, "mark relevant.\n"));
  DBG_VECT2 (debug_generic_expr (stmt));

  if (TREE_CODE (stmt) == PHI_NODE)
    {
      VARRAY_PUSH_TREE (worklist, stmt);
      return;
    }

  stmt_info = vinfo_for_stmt (stmt);

  if (!stmt_info)
    {
      DBG_VECT (fprintf (stderr, "mark relevant: no stmt info!!\n"));
      DBG_VECT (debug_generic_expr (stmt));
      return;
    }

  if (STMT_VINFO_RELEVANT_P (stmt_info))
    {
      DBG_VECT2 (fprintf (stderr, "already marked relevant.\n"));
      return;
    }

  STMT_VINFO_RELEVANT_P (stmt_info) = 1;
  VARRAY_PUSH_TREE (worklist, stmt);
}


/* Function vect_stmt_relevant_p.

   Return true if STMT in loop that is represented by LOOP_VINFO is
   "relevant for vectorization".

   A stmt is considered "relevant for vectorization" if:
   - it has uses outside the loop.
   - it has vdefs (it alters memory).
   - control stmts in the loop (except for the exit condition).

   CHECKME: what other side effects would the vectorizer allow?
 */

static bool
vect_stmt_relevant_p (tree stmt, loop_vec_info loop_vinfo)
{
  vdef_optype vdefs;
  struct loop *loop = LOOP_VINFO_LOOP (loop_vinfo);
  int i;
  dataflow_t df;
  int num_uses;

  /* cond stmt other than loop exit cond.  */
  if (is_ctrl_stmt (stmt)
      && (stmt != LOOP_VINFO_EXIT_COND (loop_vinfo)))
    return true;

  /* changing memory.  */
  get_stmt_operands (stmt);
  vdefs = STMT_VDEF_OPS (stmt);
  if (vdefs)
    {
      DBG_VECT2 (fprintf (stderr,"vec_stmt_relevant_p: stmt has vdefs:\n"));
      DBG_VECT2 (debug_generic_stmt (stmt));
      return true;
    }

  /* uses outside the loop.  */
  df = get_immediate_uses (stmt);
  num_uses = num_immediate_uses (df);
  for (i = 0; i < num_uses; i++)
    {
      tree use = immediate_use (df, i);
      int use_depth = bb_for_stmt (use)->loop_father->depth;
      int loop_depth = loop->depth;
      /* CHECKME: better check if bb belongs to the loop?  */
      if (use_depth < loop_depth)
        {
          DBG_VECT2 (fprintf (stderr,
	  		"vec_stmt_relevant_p: used out of loop:\n"));
          DBG_VECT2 (debug_generic_stmt (use));
          return true;
        }
    }

  return false;
}


/* Function vect_mark_stmts_to_be_vectorized.

   Not all stmts in the loop need to be vectorized. For example:

     for i...
       for j...
   1.    T0 = i + j
   2.  	 T1 = a[T0]

   3.    j = j + 1

   Stmt 1 and 3 do not need to be vectorized, because loopo control and
   addressing of vectorized data-refs are handled differently.

   This pass detects such stmts.
 */

static bool
vect_mark_stmts_to_be_vectorized (loop_vec_info loop_vinfo)
{
  varray_type worklist;
  struct loop *loop = LOOP_VINFO_LOOP (loop_vinfo);
  basic_block *bbs = LOOP_VINFO_BBS (loop_vinfo);
  unsigned int nbbs = loop->num_nodes;
  block_stmt_iterator si;
  tree stmt;
  stmt_ann_t ann;
  unsigned int i;
  int j;
  use_optype use_ops;
  stmt_vec_info stmt_info;

  DBG_VECT (fprintf (stderr, "\n<<vect_mark_stmts_to_be_vectorized>>\n"));
  VARRAY_TREE_INIT (worklist, 64, "work list");

  /* 1. Init worklist.  */

  for (i = 0; i < nbbs; i++)
    {
      basic_block bb = bbs[i];
      for (si = bsi_start (bb); !bsi_end_p (si); bsi_next (&si))
        {
          stmt = bsi_stmt (si);

 	  DBG_VECT2 (fprintf (stderr, "init: stmt relevant?\n"));
	  DBG_VECT2 (debug_generic_stmt (stmt));

	  stmt_info = vinfo_for_stmt (stmt);
  	  STMT_VINFO_RELEVANT_P (stmt_info) = 0;

	  if (vect_stmt_relevant_p (stmt, loop_vinfo))
	    vect_mark_relevant (worklist, stmt);
	}
    }


  /* 2. Process_worklist */

  while (VARRAY_ACTIVE_SIZE (worklist) > 0)
    {
      tree index_op;
      stmt = VARRAY_TOP_TREE (worklist);
      VARRAY_POP (worklist);

      DBG_VECT2 (fprintf (stderr, "worklist: examine stmt:\n"));
      DBG_VECT2 (debug_generic_stmt (stmt));

      /* Examine the USES in this statement. Mark all the statements which 
         feed this statement's uses as "relevant", unless the USE is used as 
	 an array index.
       */

      if (TREE_CODE (stmt) == PHI_NODE)
        {
          /* follow the def-use chain inside the loop */
          for (j = 0; j < PHI_NUM_ARGS (stmt); j++)
            {
              tree arg = PHI_ARG_DEF (stmt, j);
              if (TREE_CODE (arg) == SSA_NAME)
                {
                  tree def_stmt = NULL_TREE;
		  basic_block bb;

	          if (TREE_CODE (arg) == SSA_NAME)
                    def_stmt = SSA_NAME_DEF_STMT (arg);
                  if (def_stmt == NULL_TREE || 
		      TREE_CODE (def_stmt) == NOP_EXPR)
                    {
          	      DBG_VECT2 (fprintf (stderr, "\nworklist: no def_stmt!\n"));
  		      varray_clear (worklist);
		      return false; 	
                    }
          	  DBG_VECT2 (fprintf (stderr, "\nworklist: def_stmt:\n"));
          	  DBG_VECT2 (debug_generic_expr (def_stmt));
		  bb = bb_for_stmt (def_stmt);
		  if (flow_bb_inside_loop_p (loop, bb))
                    vect_mark_relevant (worklist, def_stmt);
                }
            }

          continue;
        }

      get_stmt_operands (stmt);
      ann = stmt_ann (stmt);
      use_ops = USE_OPS (ann);

      stmt_info = vinfo_for_stmt (stmt);

      /* FORNOW: expecting only one such operands.
                 should be extended to support multi-dimentional arrays.  */
      index_op = get_address_calculation_operands (stmt_info);

      for (i = 0; i < NUM_USES (use_ops); i++)
        {
          tree use = USE_OP (use_ops, i);
          DBG_VECT2 (fprintf (stderr, "\nworklist: examine use %d:\n",i));
          DBG_VECT2 (debug_generic_expr (use));

          if (use != index_op)
	    {
              tree def_stmt = NULL_TREE;
	      basic_block bb;

	      if (TREE_CODE (use) == SSA_NAME)
	        def_stmt = SSA_NAME_DEF_STMT (use);
   	      if (def_stmt == NULL_TREE || 
		  TREE_CODE (def_stmt) == NOP_EXPR)
                {
                  DBG_VECT2 (fprintf (stderr, "\nworklist: no def_stmt!\n"));
  	          varray_clear (worklist);
		  return false; 	
                }
              DBG_VECT2 (fprintf (stderr, "\nworklist: def_stmt:\n"));
              DBG_VECT2 (debug_generic_expr (def_stmt));
	      bb = bb_for_stmt (def_stmt);
	      if (flow_bb_inside_loop_p (loop, bb))
                vect_mark_relevant (worklist, def_stmt);
            }
        }

    } /* while worklist */

  varray_clear (worklist);
  return true;
}


/* Function vect_get_loop_niters.

   Determine How many iterations the loop is excuted.

   FORNOW: Handling a simple limited set of loop forms. In the future - use 
           a more general implementation.
*/

static tree
vect_get_loop_niters (struct loop *loop, int *number_of_iterations)
{
  edge exit;
  basic_block exit_bb;
  tree expr, test, op0, op1;
  tree access_fn;
  tree init, step;
  int N, niters, init_val, step_val;
  tree loop_cond;

  DBG_VECT (fprintf (stderr, "\n<<get_loop_niters>>\n"));

  /* Inspired by tree-scalar-evolution.c:get_loop_exit_condition(): */

  if (!loop->exit_edges)
    {
      DBG_VECT2 (fprintf (stderr, "get_loop_niters: no exit edges.\n"));
      return NULL;
    }

  exit = loop->exit_edges[0];
  exit_bb = exit->src;
  expr = last_stmt (exit_bb);

  loop_cond = expr;

  /* Make sure that exit condition is simple enough  */

  /* FORNOW:
     expecting an exit condition of the form
     i <= N
     where:
     - i has a simple iv evolution
     - N is a known constant
   */

  if (TREE_CODE (expr) != COND_EXPR)
    {
      DBG_VECT2 (fprintf (stderr, "get_loop_niters: last not cond.\n"));
      return NULL;
    }

  test = TREE_OPERAND (expr, 0);

  if (TREE_CODE (test) != LE_EXPR)
    return NULL;

  op0 = TREE_OPERAND (test, 0);
  op1 = TREE_OPERAND (test, 1);

  /* check that the evolution of op0 is simple enough */

  if (TREE_CODE (op0) != SSA_NAME)
    {
      DBG_VECT2 (fprintf (stderr, "get_loop_niters: not LE.\n"));
      return NULL;
    }

  DBG_VECT2 (fprintf (stderr, "get_loop_niters: call monev analyzer!\n"));
  access_fn = iccp_determine_evolution_function (loop, op0);
  if (!access_fn)
    {
      DBG_VECT (fprintf (stderr, "No Access function."));
      return NULL;
    }
  DBG_VECT2 (fprintf (stderr, "Access function of loop_exit cond var:\n"));
  DBG_VECT2 (debug_generic_expr (access_fn));

  if (!vect_is_simple_iv_evolution (access_fn, &init, &step))
    {
      DBG_VECT (fprintf (stderr,"unsupported loop iv.\n"));
      return NULL;
    }
  DBG_VECT2 (debug_generic_expr (init));
  DBG_VECT2 (debug_generic_expr (step));

  /* FORNOW: Make sure that loop bound is known.  */
  DBG_VECT2 (debug_generic_expr (op1));

  if (TREE_CODE (op1) != INTEGER_CST ||
      TREE_CODE (init) != INTEGER_CST ||
      TREE_CODE (step) != INTEGER_CST)
    {
      DBG_VECT (fprintf (stderr, "init, step or bound not INTEGER_CST\n"));
      return NULL;
    }

  /* CHECKME */
  if (TREE_INT_CST_HIGH (op1) != 0 ||
      TREE_INT_CST_HIGH (init) != 0 ||
      TREE_INT_CST_HIGH (step) != 0)
    {
      DBG_VECT (fprintf (stderr, "init, step or bound CST_HIGH != 0\n"));
      return NULL;
    }

  N = TREE_INT_CST_LOW (op1);
  init_val = TREE_INT_CST_LOW (init);
  step_val = TREE_INT_CST_LOW (step);

  /* CHECKME: this is probably a redundant check.  */
  if (step_val == 0)
    return NULL;

  /* CHECKME: revisit this computations where more general cases are
              supported.  */
  niters = (N - init_val + 1) / step_val;

  *number_of_iterations =  (niters < 0 ?  0 : niters);

  return loop_cond;
}


/* Function vect_analyze_loop_form.

   FORNOW: Verify the following restrictions:

   - it's an inner-most loop
   - number of BBs = 2 (which are the loop header and the latch)
   - the loop has a pre header
   - the loop has a single entry and exit
   - the loop exit condition is simple enough
*/

static loop_vec_info
vect_analyze_loop_form (struct loop *loop)
{
  loop_vec_info loop_vinfo;
  tree loop_cond;
  int number_of_iterations = -1;

  DBG_VECT (fprintf (stderr, "\n<<vect_analyze_loop_form>>\n"));
  if (loop->level > 1         /* FORNOW: inner-most loop (CHECKME)  */
      || loop->num_exits > 1
      || loop->num_entries > 1
      || loop->num_nodes != 2 /* FORNOW */
      || !loop->pre_header
      || !loop->header
      || !loop->latch)
    {
      DBG_VECT (fprintf (stderr,
 	    "loop_analyzer: bad loop form (entry/exit, nbbs, level...)\n"));
      DBG_VECT (flow_loop_dump (loop, stderr, NULL, 1));
      return NULL;
    }

  loop_cond = vect_get_loop_niters (loop, &number_of_iterations);
  if (!loop_cond)
    {
      DBG_VECT (fprintf (stderr, "Complicated exit condition.\n"));
      return NULL;
    }

  if (number_of_iterations < 0)
    {
      DBG_VECT (fprintf (stderr, "Can't determine num iters\n"));
      return NULL;
    }

  loop_vinfo = new_loop_vec_info (loop);

  LOOP_VINFO_EXIT_COND (loop_vinfo) = loop_cond;
  LOOP_VINFO_NITERS (loop_vinfo) = number_of_iterations;

  return loop_vinfo;
}


/* Function vect_analyze_loop.

   Apply a set of analyses on LOOP, and create a loop_vec_info struct
   for it. The different analyses will record information in the
   loop_vec_info struct.  */

static loop_vec_info
vect_analyze_loop (struct loop *loop)
{
   bool ok;
   loop_vec_info loop_vinfo;

   DBG_VECT (fprintf (stderr, "\n\n\n<<<<<<< analyze_loop_nest >>>>>>>\n"));

   /* Check the CFG characteristics of the loop (nesting, entry/exit, etc.
    */

   loop_vinfo = vect_analyze_loop_form (loop);
   if (!loop_vinfo)
     {
       DBG_VECT (fprintf (stderr, "loop_analyzer: bad loop form.\n"));
       return NULL;
     }


   /* Find all data references in the loop (which correspond to vdefs/vuses)
      and analyze their evolution in the loop.

      FORNOW: Handle only simple, one-dimentional, array references, which
              alignment can be forced.
    */

   ok = vect_analyze_data_refs (loop_vinfo);
   if (!ok)
     {
       DBG_VECT (fprintf (stderr, "loop_analyzer: bad data references.\n"));
       destroy_loop_vec_info (loop_vinfo);
       return NULL;
     }


   /* Data-flow analysis to detect stmts that do not need to be vectorized.
    */

   ok = vect_mark_stmts_to_be_vectorized (loop_vinfo);
   if (!ok)
     {
       DBG_VECT (fprintf (stderr, "loop_analyzer: unexpected pattern.\n"));
       destroy_loop_vec_info (loop_vinfo);
       return NULL;
     }


   /* Check that all cross-iteration scalar data-flow cycles are OK.
      Cross-iteration cycles caused by virtual phis are analyzed seperately.
   */

   ok = vect_analyze_scalar_cycles (loop_vinfo);
   if (!ok)
     {
       DBG_VECT (fprintf (stderr, "loop_analyzer: bad scalar cycle.\n"));
       destroy_loop_vec_info (loop_vinfo);
       return NULL;
     }


   /* Analyze data dependences between the data-refs in the loop.
      FORNOW: We do not contruct a data dependence graph and try to deal
              with dependences, but fail at the first data dependence that
	      we encounter.  */

   ok = vect_analyze_data_ref_dependences (loop_vinfo);

   /* TODO: May want to generate run time pointer aliasing checks and
            loop versioning.  */

   /* TODO: May want to perform loop transformations to break dependence
            cycles.  */

   if (!ok)
     {
       DBG_VECT (fprintf (stderr, "loop_analyzer: bad data dependence.\n"));
       destroy_loop_vec_info (loop_vinfo);
       return NULL;
     }


   /* Analyze the access patterns of the data-refs in the loop (consecutive,
      complex, stc). FORNOW: Only handle consecutive access pattern.  */

   ok = vect_analyze_data_ref_accesses (loop_vinfo);
   if (!ok)
     {
       DBG_VECT (fprintf (stderr, "loop_analyzer: bad data access.\n"));
       destroy_loop_vec_info (loop_vinfo);
       return NULL;
     }


   /* Scan all the operations in the loop and make sure they are 
      vectorizable.  */

   ok = vect_analyze_operations (loop_vinfo);
   if (!ok)
     {
       DBG_VECT (fprintf (stderr, "loop_analyzer: bad operations.\n"));
       destroy_loop_vec_info (loop_vinfo);
       return NULL;
     }

   /* TODO: May want to collapse conditional code and loop versioning.   */

   /* TODO: Alignment: May want to perform loop peeling and/or run time 
            tests and loop versioning.  */

   LOOP_VINFO_VECTORIZABLE_P (loop_vinfo) = 1;

   return loop_vinfo;
}


/* Function vectorize_loops.
   Entry Point to loop vectorization phase.  */

void
vectorize_loops (tree fndecl,
		 bitmap vars,
                 struct loops *loops,
		 varray_type ev_info ATTRIBUTE_UNUSED,
		 enum tree_dump_index phase)
{
   unsigned int i;
   unsigned int num_vectorized_loops = 0;

   /* Does the target support SIMD?  */
   /* FORNOW: until more sophisticated machine modelling is in place.  */
   if (!UNITS_PER_SIMD_WORD)
     {
       DBG_VECT (fprintf (stderr,
       		"vectorizer: target vector size is not defined.\n"));
       return;
     }

   DBG_VECT2 (dump_function_to_file (fndecl, stderr,
   				 ~(TDF_RAW | TDF_SLIM | TDF_LINENO)));

   timevar_push (TV_TREE_VECTORIZATION);

   /* Initialize debugging dumps.  */
   dump_file = dump_begin (phase, &dump_flags);

   DBG_VECT2 (fprintf (stderr, "loop vectorization phase\n"));

   vars_to_rename = vars;

   /*  ----------- Initializations.  -----------  */
   initialize_scalar_evolutions_analyzer ();

   /*  ----------- Analyze loops. -----------  */
   /* CHECKME */
  for (i = 1; i < loops->num; i++)
    {
      loop_vec_info loop_vinfo;
      struct loop *loop = loops->parray[i];

      flow_loop_scan (loop, LOOP_ALL);

      loop_vinfo = vect_analyze_loop (loop);
      loop->aux = loop_vinfo;

#ifndef ANALYZE_ALL_THEN_VECTORIZE_ALL
       if (!loop_vinfo ||
           !LOOP_VINFO_VECTORIZABLE_P (loop_vinfo))
	 continue;

       vect_transform_loop (loop_vinfo);
       num_vectorized_loops++;
#endif
    }

#ifdef ANALYZE_ALL_THEN_VECTORIZE_ALL
  for (i = 1; i < loops->num; i++)
    {
      struct loop *loop = loops->parray[i];
      loop_vec_info loop_vinfo = loop->aux;

      if (!loop_vinfo ||
	  !LOOP_VINFO_VECTORIZABLE_P (loop_vinfo))
        continue;

      vect_transform_loop (loop_vinfo);
      num_vectorized_loops++;
    }
#endif

   DBG_VEC (fprintf (stderr, "vectorized %d loops in function.\n",
				num_vectorized_loops));

   DBG_VECT2 (dump_function_to_file (fndecl, stderr,
   				 ~(TDF_RAW | TDF_SLIM | TDF_LINENO)));

   /*  ----------- Finialize. -----------  */

   for (i = 1; i < loops->num; i++)
    {
      struct loop *loop = loops->parray[i];
      loop_vec_info loop_vinfo = loop->aux;
      destroy_loop_vec_info (loop_vinfo);
    }

   finalize_scalar_evolutions_analyzer ();

   timevar_pop (TV_TREE_VECTORIZATION);

   /* Debugging dumps.  */
   if (dump_file)
    {
      dump_function_to_file (fndecl, dump_file, dump_flags);
      dump_end (phase, dump_file);
    }

}
