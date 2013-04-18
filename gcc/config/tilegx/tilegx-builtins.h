/* Enum for builtin intrinsics for TILE-Gx.
   Copyright (C) 2011-2013 Free Software Foundation, Inc.
   Contributed by Walter Lee (walt@tilera.com)

   This file is part of GCC.

   GCC is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 3, or (at your
   option) any later version.

   GCC is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with GCC; see the file COPYING3.  If not see
   <http://www.gnu.org/licenses/>.  */

#ifndef GCC_TILEGX_BUILTINS_H
#define GCC_TILEGX_BUILTINS_H

enum tilegx_builtin
{
  TILEGX_INSN_ADD,
  TILEGX_INSN_ADDX,
  TILEGX_INSN_ADDXSC,
  TILEGX_INSN_AND,
  TILEGX_INSN_BFEXTS,
  TILEGX_INSN_BFEXTU,
  TILEGX_INSN_BFINS,
  TILEGX_INSN_CLZ,
  TILEGX_INSN_CMOVEQZ,
  TILEGX_INSN_CMOVNEZ,
  TILEGX_INSN_CMPEQ,
  TILEGX_INSN_CMPEXCH,
  TILEGX_INSN_CMPEXCH4,
  TILEGX_INSN_CMPLES,
  TILEGX_INSN_CMPLEU,
  TILEGX_INSN_CMPLTS,
  TILEGX_INSN_CMPLTU,
  TILEGX_INSN_CMPNE,
  TILEGX_INSN_CMUL,
  TILEGX_INSN_CMULA,
  TILEGX_INSN_CMULAF,
  TILEGX_INSN_CMULF,
  TILEGX_INSN_CMULFR,
  TILEGX_INSN_CMULH,
  TILEGX_INSN_CMULHR,
  TILEGX_INSN_CRC32_32,
  TILEGX_INSN_CRC32_8,
  TILEGX_INSN_CTZ,
  TILEGX_INSN_DBLALIGN,
  TILEGX_INSN_DBLALIGN2,
  TILEGX_INSN_DBLALIGN4,
  TILEGX_INSN_DBLALIGN6,
  TILEGX_INSN_DRAIN,
  TILEGX_INSN_DTLBPR,
  TILEGX_INSN_EXCH,
  TILEGX_INSN_EXCH4,
  TILEGX_INSN_FDOUBLE_ADD_FLAGS,
  TILEGX_INSN_FDOUBLE_ADDSUB,
  TILEGX_INSN_FDOUBLE_MUL_FLAGS,
  TILEGX_INSN_FDOUBLE_PACK1,
  TILEGX_INSN_FDOUBLE_PACK2,
  TILEGX_INSN_FDOUBLE_SUB_FLAGS,
  TILEGX_INSN_FDOUBLE_UNPACK_MAX,
  TILEGX_INSN_FDOUBLE_UNPACK_MIN,
  TILEGX_INSN_FETCHADD,
  TILEGX_INSN_FETCHADD4,
  TILEGX_INSN_FETCHADDGEZ,
  TILEGX_INSN_FETCHADDGEZ4,
  TILEGX_INSN_FETCHAND,
  TILEGX_INSN_FETCHAND4,
  TILEGX_INSN_FETCHOR,
  TILEGX_INSN_FETCHOR4,
  TILEGX_INSN_FINV,
  TILEGX_INSN_FLUSH,
  TILEGX_INSN_FLUSHWB,
  TILEGX_INSN_FNOP,
  TILEGX_INSN_FSINGLE_ADD1,
  TILEGX_INSN_FSINGLE_ADDSUB2,
  TILEGX_INSN_FSINGLE_MUL1,
  TILEGX_INSN_FSINGLE_MUL2,
  TILEGX_INSN_FSINGLE_PACK1,
  TILEGX_INSN_FSINGLE_PACK2,
  TILEGX_INSN_FSINGLE_SUB1,
  TILEGX_INSN_ICOH,
  TILEGX_INSN_ILL,
  TILEGX_INSN_INFO,
  TILEGX_INSN_INFOL,
  TILEGX_INSN_INV,
  TILEGX_INSN_LD,
  TILEGX_INSN_LD1S,
  TILEGX_INSN_LD1U,
  TILEGX_INSN_LD2S,
  TILEGX_INSN_LD2U,
  TILEGX_INSN_LD4S,
  TILEGX_INSN_LD4U,
  TILEGX_INSN_LDNA,
  TILEGX_INSN_LDNT,
  TILEGX_INSN_LDNT1S,
  TILEGX_INSN_LDNT1U,
  TILEGX_INSN_LDNT2S,
  TILEGX_INSN_LDNT2U,
  TILEGX_INSN_LDNT4S,
  TILEGX_INSN_LDNT4U,
  TILEGX_INSN_LD_L2,
  TILEGX_INSN_LD1S_L2,
  TILEGX_INSN_LD1U_L2,
  TILEGX_INSN_LD2S_L2,
  TILEGX_INSN_LD2U_L2,
  TILEGX_INSN_LD4S_L2,
  TILEGX_INSN_LD4U_L2,
  TILEGX_INSN_LDNA_L2,
  TILEGX_INSN_LDNT_L2,
  TILEGX_INSN_LDNT1S_L2,
  TILEGX_INSN_LDNT1U_L2,
  TILEGX_INSN_LDNT2S_L2,
  TILEGX_INSN_LDNT2U_L2,
  TILEGX_INSN_LDNT4S_L2,
  TILEGX_INSN_LDNT4U_L2,
  TILEGX_INSN_LD_MISS,
  TILEGX_INSN_LD1S_MISS,
  TILEGX_INSN_LD1U_MISS,
  TILEGX_INSN_LD2S_MISS,
  TILEGX_INSN_LD2U_MISS,
  TILEGX_INSN_LD4S_MISS,
  TILEGX_INSN_LD4U_MISS,
  TILEGX_INSN_LDNA_MISS,
  TILEGX_INSN_LDNT_MISS,
  TILEGX_INSN_LDNT1S_MISS,
  TILEGX_INSN_LDNT1U_MISS,
  TILEGX_INSN_LDNT2S_MISS,
  TILEGX_INSN_LDNT2U_MISS,
  TILEGX_INSN_LDNT4S_MISS,
  TILEGX_INSN_LDNT4U_MISS,
  TILEGX_INSN_LNK,
  TILEGX_INSN_MF,
  TILEGX_INSN_MFSPR,
  TILEGX_INSN_MM,
  TILEGX_INSN_MNZ,
  TILEGX_INSN_MOVE,
  TILEGX_INSN_MTSPR,
  TILEGX_INSN_MUL_HS_HS,
  TILEGX_INSN_MUL_HS_HU,
  TILEGX_INSN_MUL_HS_LS,
  TILEGX_INSN_MUL_HS_LU,
  TILEGX_INSN_MUL_HU_HU,
  TILEGX_INSN_MUL_HU_LS,
  TILEGX_INSN_MUL_HU_LU,
  TILEGX_INSN_MUL_LS_LS,
  TILEGX_INSN_MUL_LS_LU,
  TILEGX_INSN_MUL_LU_LU,
  TILEGX_INSN_MULA_HS_HS,
  TILEGX_INSN_MULA_HS_HU,
  TILEGX_INSN_MULA_HS_LS,
  TILEGX_INSN_MULA_HS_LU,
  TILEGX_INSN_MULA_HU_HU,
  TILEGX_INSN_MULA_HU_LS,
  TILEGX_INSN_MULA_HU_LU,
  TILEGX_INSN_MULA_LS_LS,
  TILEGX_INSN_MULA_LS_LU,
  TILEGX_INSN_MULA_LU_LU,
  TILEGX_INSN_MULAX,
  TILEGX_INSN_MULX,
  TILEGX_INSN_MZ,
  TILEGX_INSN_NAP,
  TILEGX_INSN_NOP,
  TILEGX_INSN_NOR,
  TILEGX_INSN_OR,
  TILEGX_INSN_PCNT,
  TILEGX_INSN_PREFETCH_L1,
  TILEGX_INSN_PREFETCH_L1_FAULT,
  TILEGX_INSN_PREFETCH_L2,
  TILEGX_INSN_PREFETCH_L2_FAULT,
  TILEGX_INSN_PREFETCH_L3,
  TILEGX_INSN_PREFETCH_L3_FAULT,
  TILEGX_INSN_REVBITS,
  TILEGX_INSN_REVBYTES,
  TILEGX_INSN_ROTL,
  TILEGX_INSN_SHL,
  TILEGX_INSN_SHL16INSLI,
  TILEGX_INSN_SHL1ADD,
  TILEGX_INSN_SHL1ADDX,
  TILEGX_INSN_SHL2ADD,
  TILEGX_INSN_SHL2ADDX,
  TILEGX_INSN_SHL3ADD,
  TILEGX_INSN_SHL3ADDX,
  TILEGX_INSN_SHLX,
  TILEGX_INSN_SHRS,
  TILEGX_INSN_SHRU,
  TILEGX_INSN_SHRUX,
  TILEGX_INSN_SHUFFLEBYTES,
  TILEGX_INSN_SHUFFLEBYTES1,
  TILEGX_INSN_ST,
  TILEGX_INSN_ST1,
  TILEGX_INSN_ST2,
  TILEGX_INSN_ST4,
  TILEGX_INSN_STNT,
  TILEGX_INSN_STNT1,
  TILEGX_INSN_STNT2,
  TILEGX_INSN_STNT4,
  TILEGX_INSN_SUB,
  TILEGX_INSN_SUBX,
  TILEGX_INSN_SUBXSC,
  TILEGX_INSN_TBLIDXB0,
  TILEGX_INSN_TBLIDXB1,
  TILEGX_INSN_TBLIDXB2,
  TILEGX_INSN_TBLIDXB3,
  TILEGX_INSN_V1ADD,
  TILEGX_INSN_V1ADDI,
  TILEGX_INSN_V1ADDUC,
  TILEGX_INSN_V1ADIFFU,
  TILEGX_INSN_V1AVGU,
  TILEGX_INSN_V1CMPEQ,
  TILEGX_INSN_V1CMPEQI,
  TILEGX_INSN_V1CMPLES,
  TILEGX_INSN_V1CMPLEU,
  TILEGX_INSN_V1CMPLTS,
  TILEGX_INSN_V1CMPLTSI,
  TILEGX_INSN_V1CMPLTU,
  TILEGX_INSN_V1CMPLTUI,
  TILEGX_INSN_V1CMPNE,
  TILEGX_INSN_V1DDOTPU,
  TILEGX_INSN_V1DDOTPUA,
  TILEGX_INSN_V1DDOTPUS,
  TILEGX_INSN_V1DDOTPUSA,
  TILEGX_INSN_V1DOTP,
  TILEGX_INSN_V1DOTPA,
  TILEGX_INSN_V1DOTPU,
  TILEGX_INSN_V1DOTPUA,
  TILEGX_INSN_V1DOTPUS,
  TILEGX_INSN_V1DOTPUSA,
  TILEGX_INSN_V1INT_H,
  TILEGX_INSN_V1INT_L,
  TILEGX_INSN_V1MAXU,
  TILEGX_INSN_V1MAXUI,
  TILEGX_INSN_V1MINU,
  TILEGX_INSN_V1MINUI,
  TILEGX_INSN_V1MNZ,
  TILEGX_INSN_V1MULTU,
  TILEGX_INSN_V1MULU,
  TILEGX_INSN_V1MULUS,
  TILEGX_INSN_V1MZ,
  TILEGX_INSN_V1SADAU,
  TILEGX_INSN_V1SADU,
  TILEGX_INSN_V1SHL,
  TILEGX_INSN_V1SHLI,
  TILEGX_INSN_V1SHRS,
  TILEGX_INSN_V1SHRSI,
  TILEGX_INSN_V1SHRU,
  TILEGX_INSN_V1SHRUI,
  TILEGX_INSN_V1SUB,
  TILEGX_INSN_V1SUBUC,
  TILEGX_INSN_V2ADD,
  TILEGX_INSN_V2ADDI,
  TILEGX_INSN_V2ADDSC,
  TILEGX_INSN_V2ADIFFS,
  TILEGX_INSN_V2AVGS,
  TILEGX_INSN_V2CMPEQ,
  TILEGX_INSN_V2CMPEQI,
  TILEGX_INSN_V2CMPLES,
  TILEGX_INSN_V2CMPLEU,
  TILEGX_INSN_V2CMPLTS,
  TILEGX_INSN_V2CMPLTSI,
  TILEGX_INSN_V2CMPLTU,
  TILEGX_INSN_V2CMPLTUI,
  TILEGX_INSN_V2CMPNE,
  TILEGX_INSN_V2DOTP,
  TILEGX_INSN_V2DOTPA,
  TILEGX_INSN_V2INT_H,
  TILEGX_INSN_V2INT_L,
  TILEGX_INSN_V2MAXS,
  TILEGX_INSN_V2MAXSI,
  TILEGX_INSN_V2MINS,
  TILEGX_INSN_V2MINSI,
  TILEGX_INSN_V2MNZ,
  TILEGX_INSN_V2MULFSC,
  TILEGX_INSN_V2MULS,
  TILEGX_INSN_V2MULTS,
  TILEGX_INSN_V2MZ,
  TILEGX_INSN_V2PACKH,
  TILEGX_INSN_V2PACKL,
  TILEGX_INSN_V2PACKUC,
  TILEGX_INSN_V2SADAS,
  TILEGX_INSN_V2SADAU,
  TILEGX_INSN_V2SADS,
  TILEGX_INSN_V2SADU,
  TILEGX_INSN_V2SHL,
  TILEGX_INSN_V2SHLI,
  TILEGX_INSN_V2SHLSC,
  TILEGX_INSN_V2SHRS,
  TILEGX_INSN_V2SHRSI,
  TILEGX_INSN_V2SHRU,
  TILEGX_INSN_V2SHRUI,
  TILEGX_INSN_V2SUB,
  TILEGX_INSN_V2SUBSC,
  TILEGX_INSN_V4ADD,
  TILEGX_INSN_V4ADDSC,
  TILEGX_INSN_V4INT_H,
  TILEGX_INSN_V4INT_L,
  TILEGX_INSN_V4PACKSC,
  TILEGX_INSN_V4SHL,
  TILEGX_INSN_V4SHLSC,
  TILEGX_INSN_V4SHRS,
  TILEGX_INSN_V4SHRU,
  TILEGX_INSN_V4SUB,
  TILEGX_INSN_V4SUBSC,
  TILEGX_INSN_WH64,
  TILEGX_INSN_XOR,
  TILEGX_NETWORK_BARRIER,
  TILEGX_IDN0_RECEIVE,
  TILEGX_IDN1_RECEIVE,
  TILEGX_IDN_SEND,
  TILEGX_UDN0_RECEIVE,
  TILEGX_UDN1_RECEIVE,
  TILEGX_UDN2_RECEIVE,
  TILEGX_UDN3_RECEIVE,
  TILEGX_UDN_SEND,
  TILEGX_BUILTIN_max
};

#endif /* !GCC_TILEGX_BUILTINS_H */
