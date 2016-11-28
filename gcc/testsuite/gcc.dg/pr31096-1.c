/* PR middle-end/31096 */
/* { dg-do compile } */
/* { dg-options "-O2 -fdump-tree-optimized" } */

#define zero(name, op) \
int name (int a, int b) \
{ return a * 0 op b * 0; }

zero(zeq, ==) zero(zne, !=) zero(zlt, <)
zero(zgt, >)  zero(zge, >=) zero(zle, <=)

#define unsign_pos(name, op) \
int name (unsigned a, unsigned b) \
{ return a * 4 op b * 4; }

unsign_pos(upeq, ==) unsign_pos(upne, !=) unsign_pos(uplt, <)
unsign_pos(upgt, >)  unsign_pos(upge, >=) unsign_pos(uple, <=)

#define unsign_neg(name, op) \
int name (unsigned a, unsigned b) \
{ return a * -2 op b * -2; }

unsign_neg(uneq, ==) unsign_neg(unne, !=) unsign_neg(unlt, <)
unsign_neg(ungt, >)  unsign_neg(unge, >=) unsign_neg(unle, <=)

#define float(name, op) \
int name (float a, float b) \
{ return a * 5 op b * 5; }

float(feq, ==) float(fne, !=) float(flt, <)
float(fgt, >)  float(fge, >=) float(fle, <=)

#define float_val(name, op) \
int name (int a, int b) \
{ return a * 54.0 op b * 54.0; }

float_val(fveq, ==) float_val(fvne, !=) float_val(fvlt, <)
float_val(fvgt, >)  float_val(fvge, >=) float_val(fvle, <=)

#define vec(name, op) \
int name (int a, int b) \
{ int c[10]; return a * c[1] op b * c[1]; }

vec(veq, ==) vec(vne, !=) vec(vlt, <)
vec(vgt, >)  vec(vge, >=) vec(vle, <=)

/* { dg-final { scan-tree-dump-times "\\(D\\) \\* 4" 24 "optimized" } } */
/* { dg-final { scan-tree-dump-times "\\(D\\) \\* 4294967294" 12 "optimized" } } */
/* { dg-final { scan-tree-dump-times "\\(D\\) \\* 5\\.0e\\+0" 12 "optimized" } } */
/* { dg-final { scan-tree-dump-times "\\* 5\\.4e\\+1" 12 "optimized" } } */
/* { dg-final { scan-tree-dump-times "\\(D\\) \\* c\\\$1_8\\(D\\)" 12 "optimized" } } */
