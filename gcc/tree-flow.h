/* {{{ Data and Control Flow Analysis for Trees.
   Copyright (C) 2001 Free Software Foundation, Inc.
   Contributed by Diego Novillo <dnovillo@redhat.com>

This file is part of GNU CC.

GNU CC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU CC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU CC; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

/* }}} */

#ifndef _TREE_FLOW_H
#define _TREE_FLOW_H 1

/* {{{ Types of references.  */

enum varref_type { VARDEF, VARUSE, VARPHI };

union varref_def;

/* }}} */

/* {{{ Common features of every variable reference.  */

struct varref_common
{
  /* Base symbol.  */
  tree sym;

  /* Statement containing the reference.  */
  tree stmt;

  /* Sub-tree containing the reference.  */
  tree expr;

  /* Basic block containing the reference.  */
  basic_block bb;

  /* Reference type.  */
  enum varref_type type;
};

/* }}} */

/* {{{ Variable definitions.  */

struct vardef
{
  struct varref_common common;

  /* Immediate uses for this definition.  */
  varray_type imm_uses;

  /* Saved definition chain.  */
  union varref_def *save_chain;
};

#define VARDEF_IMM_USES(r) (r)->def.imm_uses
#define VARDEF_SAVE_CHAIN(r) (r)->def.save_chain

/* }}} */

/* {{{ Variable uses.  */

struct varuse
{
  struct varref_common common;

  /* Definition chain.  */
  union varref_def *chain;
};

#define VARUSE_CHAIN(r) (r)->use.chain

/* }}} */

/* {{{ PHI terms.  */

struct varphi
{
  struct varref_common common;

  /* PHI arguments.  */
  varray_type phi_chain;
};

#define VARPHI_CHAIN(r) (r)->phi.phi_chain

/* }}} */

/* {{{ Generic variable reference structure.  */

union varref_def
{
  struct varref_common common;
  struct vardef def;
  struct varuse use;
  struct varphi phi;
};

typedef union varref_def *varref;

#define VARREF_TYPE(r) (r)->common.type
#define VARREF_BB(r) (r)->common.bb
#define VARREF_EXPR(r) (r)->common.expr
#define VARREF_STMT(r) (r)->common.stmt
#define VARREF_SYM(r) (r)->common.sym

/* }}} */

/* {{{ Tree annotations stored in tree_common.aux.  */

struct tree_ann_def
{
  /* Basic block that contains this tree.  */
  basic_block bb;

  /* For _DECL trees, list of references made to this variable.  */
  varray_type refs;

  /* Most recent definition for this symbol.  Used when placing FUD
     chains.  */
  varref currdef;

  /* Immediately enclosing compound statement to which this tree belongs.  */
  tree compound_stmt;
};

typedef struct tree_ann_def *tree_ann;

#define TREE_ANN(NODE)		\
    ((tree_ann)((NODE)->common.aux))

#define BB_FOR_STMT(NODE)	\
    ((TREE_ANN (NODE)) ? TREE_ANN (NODE)->bb : NULL)

#define TREE_CURRDEF(NODE) 	\
    ((TREE_ANN (NODE)) ? TREE_ANN (NODE)->currdef : NULL)

#define TREE_REFS(NODE)		\
    ((TREE_ANN (NODE)) ? TREE_ANN (NODE)->refs : NULL)

#define TREE_COMPOUND_STMT(NODE)\
    ((TREE_ANN (NODE)) ? TREE_ANN (NODE)->compound_stmt : NULL)

/* }}} */

/* {{{ Block annotations stored in basic_block.aux.  */

struct bb_ann_def
{
  /* Control flow parent.  */
  basic_block parent;

  /* List of references made in this block.  */
  varray_type refs;
};

typedef struct bb_ann_def *bb_ann;

#define BB_ANN(BLOCK)		\
    ((bb_ann)((BLOCK)->aux))

#define BB_PARENT(BLOCK)		\
    ((BB_ANN (BLOCK)) ? BB_ANN (BLOCK)->parent : NULL)

#define BB_REFS(BLOCK)		\
    ((BB_ANN (BLOCK)) ? BB_ANN (BLOCK)->refs : NULL)

/* }}} */

/* {{{ Global declarations.  */

/* Array of all symbols referenced in the function.  */

extern varray_type referenced_symbols;


/* Nonzero to warn about code which is never reached.  */

extern int warn_notreached;

/* }}} */


/* {{{ Functions in tree-cfg.c  */

extern void tree_find_basic_blocks PARAMS ((tree));
extern int is_ctrl_stmt PARAMS ((tree));
extern int is_ctrl_altering_stmt PARAMS ((tree));
extern int is_loop_stmt PARAMS ((tree));
extern int stmt_ends_bb_p PARAMS ((tree));
extern int stmt_starts_bb_p PARAMS ((tree));
extern void delete_cfg PARAMS ((void));
extern bb_ann get_bb_ann PARAMS ((basic_block));
extern void tree_dump_bb PARAMS ((FILE *, const char *, basic_block, int));
extern void tree_debug_bb PARAMS ((basic_block));
extern void tree_dump_cfg PARAMS ((FILE *));
extern void tree_debug_cfg PARAMS ((void));
extern void tree_cfg2dot PARAMS ((FILE *));
extern basic_block loop_parent PARAMS ((basic_block));
extern basic_block latch_block PARAMS ((basic_block));
extern basic_block switch_parent PARAMS ((basic_block));
extern tree first_exec_stmt PARAMS ((tree));
extern int is_exec_stmt PARAMS ((tree));
extern void validate_loops PARAMS ((struct loops *));

/* }}} */

/* {{{ Functions in tree-dfa.c  */

extern void tree_find_varrefs PARAMS ((void));
extern tree_ann get_tree_ann PARAMS ((tree));
extern varref create_varref PARAMS ((tree, enum varref_type,
				     basic_block, tree, tree));
extern void debug_varref PARAMS ((varref));
extern void dump_varref PARAMS ((FILE *, const char *, varref, int, int));
extern void debug_varref_list PARAMS ((varray_type));
extern void dump_varref_list PARAMS ((FILE *, const char *, varray_type, int,
                                      int));

/* }}} */

/* {{{ Functions in tree-ssa.c  */

extern void tree_build_ssa PARAMS ((void));
extern void delete_ssa PARAMS ((void));

/* }}} */

#endif /* _TREE_FLOW_H  */
