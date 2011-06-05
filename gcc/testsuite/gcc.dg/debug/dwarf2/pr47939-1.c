/* { dg-do compile } */
/* { dg-options "-save-temps -g -dA" } */

typedef struct _Harry { int dummy; } Harry_t;
Harry_t harry;

/* { dg-final { scan-assembler "DW_TAG_typedef\[^\\r\\n\]*\[\\r\\n\]+\[^\\r\\n\]*(DW_AT_name: \"Harry_t\"|\"Harry_t..\"\[^\\r\\n\]*DW_AT_name)" } } */
/* { dg-final { cleanup-saved-temps } } */
