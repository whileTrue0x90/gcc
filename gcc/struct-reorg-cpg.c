#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "rtl.h"
#include "tree-inline.h"
#include "tree-flow.h"
#include "tree-flow-inline.h"
#include "tree-dump.h"
#include "langhooks.h"
#include "hashtab.h"
#include "toplev.h"
#include "flags.h"
#include "ggc.h"
#include "debug.h"
#include "target.h"
#include "cgraph.h"
#include "diagnostic.h"
#include "timevar.h"
#include "params.h"
#include "fibheap.h"
#include "c-common.h"
#include "params.h"
#include "intl.h"
#include "function.h"
#include "basic-block.h"
#include "tree-iterator.h"
#include "tree-pass.h"
#include "struct-reorg.h"
#include "math.h"

#define STRUCT_REORG_DISTANCE_THRESHOLD 256

static void reorder_fields_of_struct (struct data_structure *);
static void split_data_structure (struct data_structure *ds);

/* Updates CPG cell [f1,f2] with the new distance and count.  */
static void
add_cp_relation (cpg_t *cpg, int f1, int f2, gcov_type count, int dist)
{
  int i1, i2;
  struct cpg_cell *cell;

  if (count == 0)
    return;

  if (f1 < 0 || f2 < 0)
    return;

  i1 = MIN (f1,f2);
  i2 = MAX (f1,f2);

  if (! cpg->matrix[i1 + i2 * cpg->ds->num_fields])
    {
      cell = (struct cpg_cell *) xcalloc (1, sizeof (struct cpg_cell));
      cpg->matrix[i1 + i2 * cpg->ds->num_fields] = cell;
    }
  else
    cell = cpg->matrix[i1 + i2 * cpg->ds->num_fields];
  cell->distance
    = (cell->distance * cell->count + count*dist)/(cell->count + count);
  cell->count += count;
}

/* Add CP relation between NEXT and all the fields in LAF (Lately Accessed Fields) 
   until distance passes the threshold.  */
static void
update_cpg_for_lately_accessed_fields (cpg_t *cpg, struct bb_field_access *laf_start, 
				       struct bb_field_access *laf_end, int f_indx, 
				       gcov_type count)
{
  struct bb_field_access *laf;
  int distance_to_latest = 0;

  for (laf = laf_start; laf != laf_end; laf = laf->prev)
    {
      distance_to_latest += laf->distance_to_next;
      /* We want the minimum count over the patch between 
	 the two accesses.  */
      if (count > laf->count)
	count = laf->count;
      if (distance_to_latest > STRUCT_REORG_DISTANCE_THRESHOLD)
	break;
      add_cp_relation (cpg, laf->f_indx, f_indx,
           	       count, distance_to_latest);
    }
}

/* Given that BB_INDEX is the basic block index that closes a loop 
   while traversing the CFG, add CP relations for all the lately 
   accessed fields that came after that block was traversed.  */
static void
update_cpg_for_loop (cpg_t *cpg, struct bb_field_access *laf, int  bb_index)
{
  bool last_bb_found = false;
  struct bb_field_access *crr, *laf_last;

  for (crr = laf; crr; crr = crr->prev)
    {
      if (last_bb_found && laf->bb_index != bb_index)
	break;
      if (laf->bb_index == bb_index)
	last_bb_found = true;
    }
  laf_last = crr;
  for (crr = laf; crr != laf_last; crr = crr->prev)
   update_cpg_for_lately_accessed_fields (cpg, crr, laf_last, crr->f_indx, crr->count);
}

/* Go over the fields accesses inside the block BB (by traversing BBS_F_ACC_LISTS) of
   the given block, add CP relations to the given CPG, and return an updated list
   of lately accessed fields (creates a new one if its empty). The firs element 
   of the list is the latest accessed one.  This function also removes fields from the
   lately accessed list if their distance goes above the distance threshold.  */ 
static void 
update_cpg_for_bb (cpg_t *cpg, basic_block bb, sbitmap visited, 
		   struct bb_field_access *lately_accessed_fields, struct function *f)
{
  struct bb_field_access *crr, *laf;
  edge e;

  if ( bb == ENTRY_BLOCK_PTR_FOR_FUNCTION (f)
       || bb == EXIT_BLOCK_PTR_FOR_FUNCTION (f))
    return;

  if (TEST_BIT (visited, bb->index))
    {
      update_cpg_for_loop (cpg, lately_accessed_fields, bb->index);
      return;
    } 
  SET_BIT (visited, bb->index);
  laf = lately_accessed_fields;

  for (crr = cpg->ds->bbs_f_acc_lists[bb->index]; crr; crr = crr->next)
    {
      struct bb_field_access *new = (struct bb_field_access *) 
				    xcalloc (1, sizeof (struct bb_field_access));

      new->f_indx = crr->f_indx;
      new->distance_to_next = crr->distance_to_next;
      new->count = bb->count;
      new->bb_index = bb->index;
      if (! laf) 
	{
	  laf = new; 
	  continue;
	}

      if (new->f_indx >= 0)
        update_cpg_for_lately_accessed_fields (cpg, laf, NULL, new->f_indx, bb->count); 

      new->prev = laf;
      laf->next = new;
      laf = new;
    } 
  for (e = bb->succ; e; e = e->succ_next)
    {
      for (crr = laf; crr != lately_accessed_fields; crr = crr->prev)
	crr->count = e->count;
      update_cpg_for_bb (cpg, e->dest, visited, laf, f);
    }
  RESET_BIT (visited, bb->index);
  
  /* Remove the access of the block from the list of lately accessed
     fields.  */
  for (crr = laf; crr != lately_accessed_fields; )
    {
      laf = crr->prev;
      free (crr);
      if (laf)
	laf->next = NULL;
      crr = laf;
    }
}

#if 0
/* Given a basic block we search for possibly immediate field accesses
   for each one of the outgoing arcs (on the CFG). Once an access is found
   we add a CP relation with the appropriate disntance and count.  The
   distance is the summation of all the disntances in the "empty" bbs in
   between the accesses plus the disntance at the end of the current bb
   plus the disntance at the beginning of the bb having the access in the
   other side.   */
static void
update_cpg_for_bb (cpg_t *cpg, edge e, sbitmap visited, cpg_cell_t cp,
		   int f1_indx, struct function *f)
{
  edge succ;
  struct bb_field_access *first_acc;

  if ( e->dest == ENTRY_BLOCK_PTR_FOR_FUNCTION (f)
       || e->dest == EXIT_BLOCK_PTR_FOR_FUNCTION (f)
       || TEST_BIT (visited, e->dest->index))
    return;

  first_acc = cpg->ds->bbs_f_acc_lists [e->dest->index];
  cp.distance += first_acc->distance_to_next;
  cp.count = MIN (e->count, cp.count);

  if (first_acc->next)
    {
      add_cp_relation (cpg, f1_indx, first_acc->next->f_indx,
                       cp.count, cp.distance);
      return;
    }

  SET_BIT (visited, e->dest->index);
  for (succ = e->dest->succ; succ; succ = succ->succ_next)
     update_cpg_for_bb (cpg, succ, visited, cp, f1_indx, f);
}
#endif

/* Build the Close Proximity Graph for a given data structure.  */
void
update_cpg_for_structure (struct data_structure *ds, struct function *f)
{
  edge e;
  sbitmap visited;
  cpg_t *cpg;

  if (!f)
    return;

  visited = sbitmap_alloc (n_basic_blocks_for_function (f));
  sbitmap_zero (visited);

  if (! ds->cpg)
    {
      cpg = (cpg_t *)xcalloc (1, sizeof (cpg_t));
      cpg->ds = ds;
      ds->cpg = cpg;
      cpg->matrix = (struct cpg_cell **)xcalloc (ds->num_fields
 					         * ds->num_fields,
					         sizeof (struct cpg_cell));
    }
  else
    cpg = ds->cpg;

  for (e = ENTRY_BLOCK_PTR_FOR_FUNCTION (f)->succ; e; e = e->succ_next)
    {
      update_cpg_for_bb (cpg, e->dest, visited, NULL, f);
    }
  sbitmap_free (visited);
}

/* Dump the Close Proximity Graph, we print the edges that connect the
   different fields.  In the following format:
	struct1 :
	  field1 <-- (dist, count) --> field2
          ...
 */
void
dump_cpg (FILE *dump_file, cpg_t *cpg)
{
  int i, j;
  bool first_time = true;
  tree struct_id = TYPE_NAME (cpg->ds->decl);

  if (! struct_id)
    return;

  if (TREE_CODE (struct_id) == TYPE_DECL)
    struct_id = DECL_NAME (struct_id);

  for (i = 0; i < cpg->ds->num_fields; i++)
    for (j = i; j < cpg->ds->num_fields; j++)
      {
        struct data_field_entry *f1 = &cpg->ds->fields[i];
        struct data_field_entry *f2 = &cpg->ds->fields[j];
	struct cpg_cell *cell = cpg->matrix [i + j*cpg->ds->num_fields];

	if (! cell)
          continue;
	if (first_time)
	  {
	    first_time = false;
	    if (struct_id)
	      fprintf (dump_file, "%s:\n", IDENTIFIER_POINTER (struct_id));
	  }
        fprintf (dump_file, "\t%s <-- (%d, ",
		 IDENTIFIER_POINTER (DECL_NAME (f1->decl)),
		 cell->distance);
        fprintf (dump_file, HOST_WIDEST_INT_PRINT_DEC, cell->count);
        fprintf (dump_file, ") --> %s\n",
		 IDENTIFIER_POINTER (DECL_NAME (f2->decl)));
      }
}

/* Frees the memory allocated for CPG.  */
void
free_cpg (cpg_t *cpg)
{
  free (cpg->matrix);
  free (cpg);
}

/* Stage 2 (profile based clustering):
   Given a DATA_STRUCTURE with the following data initialized properly:
   DECL, NUM_FIELDS, FIELDS, ALLOC_SITES. The FIELDS array contains entry
   for each one of the fields of the structure, a field could be an
   atomic type or a complex type. In any case it must contain all the
   fields of the data structure. When stage 1 has a complex field it can
   choose to refer to it as a complete one field or a separated fields
   and allocate entry for each one of its fields. The later case means
   that stage 2 can separate fields of that sub-structure from each other.
   An example when this is not possible is when the complex field address
   is taken - In such a case stage 1 must add one entry for the complex
   field.
   This function performs the algorithm for profile based cache aware
   data reorganization and represents its results in STRUCT_CLUSTERING
   field of the given DATA_STRUCTURE STR.  This function should
   be called for each one of the structure that stage 1 has found relevant
   for clustering or field reordering. If only field reordering is
   applicable then the parameter REORDERING_ONLY should be true, other
   wise we assume that any clustering of the structure is legal.
   If the parameter REORDERING_ONLY is true no clustering will be
   performed and only a cache aware field reordering will be made.
   return false in unexpected failure, true otherwise.

   EXAMPLE:
        struct s1 {
          struct s11
          { int f11, f12, f13} f1;
          int f2, f3,f4;
          struct s2 *f5;
        };
        struct s2 {
          int f1, f2, f3, f2;
        }

   Stage 1 can construct one of two possible arrays for FIELDS:
   {f1,f2,f3,f4,f5} or {f11,f12,f13,f2,f3,f4,f5}
   In the first case fields f11, f12, f13 will always be adjacent.
   In the later case the preferred clustering may separate them.

   CURRENT STATUS: we just build the cpg and print it to the dump file.
*/
bool
cache_aware_data_reorganization (ATTRIBUTE_UNUSED struct data_structure *ds,
				 ATTRIBUTE_UNUSED bool reordering_only)
{
  sbitmap ones = sbitmap_alloc (ds->num_fields);

  sbitmap_ones (ones);
  if (! reordering_only)
    split_data_structure (ds);
  if (! ds->struct_clustering 
      || sbitmap_a_and_b_cg (ones, ones, 
			     ds->struct_clustering->fields_in_cluster))
    reorder_fields_of_struct (ds);
  return true;
}


/* Following code implements field reordering algorithm based on the
   WCP heuristic.  */

static HOST_WIDE_INT
field_size_in_bytes (tree decl)
{
  HOST_WIDE_INT f_size = int_size_in_bytes (TREE_TYPE (decl));

  /* We shouldn't be handling this structure if its fields doesn't
     have a known size at compile time.  */
  if (f_size < 0)
    abort ();
  return f_size;
}

struct field_order {
  int f_indx;
  struct field_order *next, *prev;
};

static gcov_type 
cp_relation (struct data_structure *ds, int f1, int f2)
{
  
  int i1, i2;
  cpg_cell_t *cp;

  i1 = MIN (f1, f2);
  i2 = MAX (f1, f2);

  cp = ds->cpg->matrix [i1 + i2 * ds->num_fields]; 
  if (!cp || cp->distance > STRUCT_REORG_DISTANCE_THRESHOLD)
    return 0;
  else
    return cp->count;
}

#if 0
static HOST_WIDE_INT
fields_layout_distance (struct data_structure *ds,
			struct field_order *f1, int n)
{
  int i;
  struct field_order *crr;
  HOST_WIDE_INT size = 0;

  for (i = 0, crr = f1; i < n; i++, crr = crr->next)
    {
      if (! crr)
        break;

      size += field_size_in_bytes (ds->fields[crr->f_indx].decl);
    }
  return size;
}
#endif

static gcov_type
field_wcp (struct data_structure *ds, struct field_order *first, int n,
           bool left)
{
  int i;
  HOST_WIDE_INT size = 0;
  struct field_order *crr = first;

  for (i = 0; i < n; i++)
    {
      crr = left ? first->next : first->prev;

      if (! crr)
	break;

      size += field_size_in_bytes (ds->fields[crr->f_indx].decl);
    }
  if (! crr)
    return 0;

  return cp_relation (ds, first->f_indx, crr->f_indx) / pow (size, i - 1);
}

#if 0
static gcov_type
data_structure_wcp (struct data_structure *ds, struct field_order *first)
{
  HOST_WIDE_INT remain_size;
  gcov_type ds_wcp = 0;
  struct field_order *left;
  int i,j;

  remain_size = CACHE_LINE_SIZE;
  for (j = 1, left = first; left; j++, left = left->next)
    for (i = 1; i < ds->num_fields - j; i++)
      ds_wcp += field_wcp (ds, left, i, true);
  return ds_wcp;

}
#endif

static gcov_type
wcp_left_right_contribution (struct data_structure *ds, int num_ordered,
			     struct field_order *first, int f, bool left)
{
  gcov_type ds_wcp = 0;
  struct field_order *new_f = xcalloc (1, sizeof (struct field_order));
  int i;

  new_f->f_indx = f;

  if (left)
    {
      new_f->next = first;
      first->prev = new_f;
    }
  else
    {
      new_f->prev = first;
      first->next = new_f;
    }

  for (i = 1; i <= num_ordered ; i++)
    ds_wcp += field_wcp (ds, new_f, i, left);

  free (new_f);

  if (left)
    first->prev = NULL;
  else
    first->next = NULL;

  return ds_wcp;
}

/* Find the field with the maximum "fanout" and start the reordering
   with it.  */
static int
find_max_fanout_field (struct data_structure *ds,
		       sbitmap not_ordered_fields)
{
  int i, j;
  int f_max_fanout = -1;
  gcov_type max_fanout = 0;

  for (i = 0; i < ds->num_fields; i++)
    {
      gcov_type cp = 0;

      if (! TEST_BIT (not_ordered_fields, i))
	continue;

      for (j = 0; j < ds->num_fields; j++)
	if ((j != i) && TEST_BIT (not_ordered_fields, j))
	  cp += cp_relation (ds, i, j);

      if (cp > max_fanout)
        {
          max_fanout = cp;
          f_max_fanout = i;
        }
    }

  return f_max_fanout;
}


/* Perform the field reordering algorithm to find the "best" reordering
   of the structure fields (definition).  */
static void
reorder_fields_of_struct (struct data_structure *ds)
{
  int i, next_to_order, num_ordered = 0;
  sbitmap not_ordered_fields;
  struct field_order *order_left = NULL;
  struct field_order *order_right = NULL;
  struct field_order *crr;
  bool side = true;

  not_ordered_fields = sbitmap_alloc (ds->num_fields);
  sbitmap_ones (not_ordered_fields);

  /* Start witht the field with the maximum fanout, it has potential
     greater potential to contribute to the WCP of the ordered group
     so we want it int he middle.  */
  next_to_order = find_max_fanout_field (ds, not_ordered_fields);

  /* Loop while there is fields that aren't ordered yet.  */
  while ((sbitmap_first_set_bit (not_ordered_fields) >=0)
         && next_to_order >= 0)
    {
      gcov_type max_contrib = 0;
      gcov_type f_contrib;
      struct field_order *new_o;

      /* Now insert the NEXT_TO_ORDER field to the correct side in the
	 ordered list of fields.  */
      RESET_BIT (not_ordered_fields, next_to_order);
      new_o = (struct field_order *)
	      xcalloc (1, sizeof (struct field_order));
      new_o->f_indx = next_to_order;
      if (! order_left)
        order_left = order_right = new_o;
      else if (side)
        {
          order_left->prev = new_o;
          new_o->next = order_left;
          order_left = new_o;
        }
      else
        {
          order_right->next = new_o;
          new_o->prev = order_right;
          order_right = new_o;
        }
        num_ordered ++;

      /* Find the field that gives the maximum contribution (to the left
	 or to the right), take also into acount the field fanout.  */
      for ( i = 0; i < ds->num_fields; i++)
        {
	  if (! TEST_BIT (not_ordered_fields, i))
	    continue;
          f_contrib = wcp_left_right_contribution (ds, num_ordered,
                                                   order_left, i, true);
          if (f_contrib > max_contrib)
            {
              max_contrib = f_contrib;
              next_to_order = i;
              side = true;
            }
          f_contrib = wcp_left_right_contribution (ds, num_ordered,
                                                   order_right, i, false);
          if (f_contrib > max_contrib)
            {
              max_contrib = f_contrib;
              next_to_order = i;
              side = false;
            }
        }

      /* This case is possible when the already ordered fields,
	 have no relation with the remaining fields, start again with
	 the field with the highest fanout from the remaining not
	 ordered fields.  */
      if (! TEST_BIT (not_ordered_fields, next_to_order))
     	{
	  next_to_order = find_max_fanout_field (ds, not_ordered_fields);
	  if (next_to_order < 0 )
	    next_to_order = sbitmap_first_set_bit (not_ordered_fields);
	  side = true;
        }
    }
  /* Now update the DS with the calculated ordering and free the
     ordering list.  */
  if (! ds->struct_clustering)
    ds->struct_clustering = (struct field_cluster *)
			    xcalloc (1, sizeof (struct field_cluster));
  ds->struct_clustering->fields_order = (int *)xcalloc (ds->num_fields,
                                                        sizeof (int));
  crr = order_left;
  i = 0;
  while (crr)
    {
      if (i >= ds->num_fields)
	abort ();
      ds->struct_clustering->fields_order[i++] = crr->f_indx;
      if (crr->next)
        {
	  crr = crr->next;
	  free (crr->prev);
        }
      else
        {
          free (crr);
          break;
        }
    }
}

/* The total size of the group of fields in G.  */
static HOST_WIDE_INT
group_size_in_bytes (struct data_structure *ds, sbitmap g)
{
  HOST_WIDE_INT size_summ = 0;
  int i;

  for (i = 0; i < ds->num_fields; i++)
    {
      if (! TEST_BIT (g, i))
	continue;
      size_summ += field_size_in_bytes (ds->fields[i].decl);
    }

  return size_summ;
}

/* The average of the CP relation between field F and the fields in
   G.  */
static gcov_type 
average_cp_relation (struct data_structure *ds, sbitmap g, int f)
{
  int i, num = 0;
  gcov_type average_cp = 0;

  for (i = 0 ; i < ds->num_fields; i++)
    if (TEST_BIT (g, i))
      {
        average_cp = ((average_cp * num) + cp_relation (ds, i, f))/(num + 1);
	num++;
      }
  
  return average_cp;
}

/* The structure splitting algorithm
  Input:
    a group of fields G and its close proximity graph CPG(G).
  Output:
    a division of G into sub-groups g1 , g2 ,  , gn
  Definitions:
  1.  max_size - is the upper threshold on the sub-group size.
  2.  g[0] - the group of cold fields or fields that cannot gain from
             splitting to different groups.
  The Algorithm:
  1.  g[0] = {f ? G | access count (f) < Cold threshold access count }
  2.  G = G - g[0]
  3.  i=1
  4.  While G is no empty do
  5.    find the fields couple (fa, fb), such that CPG(fa, fb) is the highest.
  6.    if CPGcount( fa, fb) < access count threshold then
  7.        finish.
  8.    if size(fa) + size(fb) > max_size then
  9.      mark the edge (a,b) as not relevant.
  10.     continue.
  11.   endif
  12.   g[i] = {fa, fb}
  13.   find f in G such that the average CP(f,f`) where,
  14.   f` is in g[i] is the maximum and size(f) + size(g[i]) < max_size.
  15.   if average CP (f, g[i]) < access count threshold then
  16.     i = i + 1
  17.     continue.
  18.   endif
  11.   G = G - { f }
  12.   gi = gi + { f }
  13. end while.
*/
static void 
split_data_structure (struct data_structure *ds)
{
  int i, j;
  int max_size = STRUCT_REORG_CACHE_LINE_SIZE / 2; 
  int dynamic_threshold_n_updates = 0;
  gcov_type cold_field_threshold = 0;
  sbitmap remaining_fields = sbitmap_alloc (ds->num_fields); 
  struct field_cluster *crr_cluster, *cold_cluster;
  
  sbitmap_ones (remaining_fields);
  /* Find the cold field threshold.  */
  for (i = 0; i < ds->num_fields; i++)
    for (j = i; j < ds->num_fields; j++)
      if (cp_relation (ds, i, j) > cold_field_threshold)
        cold_field_threshold = cp_relation (ds, i, j);
  cold_field_threshold /= COLD_FIELD_RATIO;

  /* Build g[0] - cold group.  */
  cold_cluster = (struct field_cluster *) xcalloc (1, sizeof (struct field_cluster));
  ds->struct_clustering = cold_cluster;
  cold_cluster->fields_in_cluster = sbitmap_alloc (ds->num_fields);

  /* Add to the cold group all the fields that have access counts less than the threshold.  */
#if 0
  for (i = 0 ; i < ds->num_fields; i++)
    if (ds->fields[i].count < cold_field_threshold)
      {
	SET_BIT (cold_cluster->fields_in_cluster, i);
	RESET_BIT (remaining_fields, i);
      }
#endif 

  crr_cluster = cold_cluster;
  while (sbitmap_first_set_bit (remaining_fields) >= 0)
    {
      gcov_type max_cp = 0;
      int max_i = -1; 
      int max_j = -1;

      /* Find the hotest CP relation in the CPG from the remaining fields.  */
      for (i = 0; i < ds->num_fields; i++)
	for (j = i; j < ds->num_fields; j++)
	  if (TEST_BIT (remaining_fields, i) && TEST_BIT (remaining_fields, j))
	    {
	      gcov_type cp = cp_relation (ds, i, j);
	      HOST_WIDE_INT size = field_size_in_bytes (ds->fields[i].decl) 
				  + field_size_in_bytes (ds->fields[j].decl);  
	      if (cp > max_cp && size <= max_size) 
		{
		  max_cp = cp;
		  max_i = i;
		  max_j = j;
		}
	    }
      /* If the maximum CP is less thanthe threshold, consider all the remianing
	 fields as cold.  */
      
      while (max_cp < cold_field_threshold)
	{
	  if (++dynamic_threshold_n_updates > STRUCT_REORG_DYNAMIC_THRESHOLD)
	    break;
	  cold_field_threshold = cold_field_threshold / COLD_FIELD_RATIO; 
	}
      if (max_cp < cold_field_threshold)
	{
	  sbitmap_a_or_b (cold_cluster->fields_in_cluster, cold_cluster->fields_in_cluster, 
			  remaining_fields);
	  break;
	}

      /* Create a new cluster that contains MAX_I, MAX_J fields.  */
      ds->struct_clustering = (struct field_cluster *) xcalloc (1, sizeof (struct field_cluster));
      ds->struct_clustering->sibling = crr_cluster;
      crr_cluster = ds->struct_clustering;
      crr_cluster->fields_in_cluster = sbitmap_alloc (ds->num_fields);
      SET_BIT (crr_cluster->fields_in_cluster, max_i);
      RESET_BIT (remaining_fields, max_i);
      SET_BIT (crr_cluster->fields_in_cluster, max_j);
      RESET_BIT (remaining_fields, max_j);

      while (sbitmap_first_set_bit (remaining_fields) >= 0)
	{
	  gcov_type average_cp;
	  HOST_WIDE_INT f_size, g_size;

	  max_cp = 0;
	  for (i = 0; i < ds->num_fields; i++)
	    {
	      if (! TEST_BIT (remaining_fields, i))
		continue;
	      f_size = field_size_in_bytes (ds->fields[i].decl);
	      g_size = group_size_in_bytes (ds, crr_cluster->fields_in_cluster);
	      if ((g_size + f_size) > max_size)
		continue;
	      average_cp = average_cp_relation (ds, crr_cluster->fields_in_cluster, i);
	      average_cp = (average_cp * g_size)/(g_size + f_size);
	      if (average_cp > max_cp)
		{
		  max_i = i;
		  max_cp = average_cp;
		}
	    }
	  if (max_cp < cold_field_threshold)
	    break;
	  SET_BIT (crr_cluster->fields_in_cluster, max_i);
	  RESET_BIT (remaining_fields, max_i);
	}
    } 
}

