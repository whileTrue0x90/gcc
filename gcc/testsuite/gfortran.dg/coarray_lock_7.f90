! { dg-do compile }
! { dg-options "-fdump-tree-original -fcoarray=lib" }
!
use iso_fortran_env
implicit none

type(lock_type) :: one[*]
type(lock_type) :: two(5,5)[*]
type(lock_type), allocatable :: three[:]
type(lock_type), allocatable :: four(:)[:]
integer :: ii
logical :: ll

allocate(three[*], stat=ii)
allocate(four(7)[*], stat=ii)

lock(one)
unlock(one)

lock(two(3,3), stat=ii)
unlock(two(2,3), stat=ii)

lock(three[4], acquired_lock=ll)
unlock(three[7], stat=ii)

lock(four(1)[6], acquired_lock=ll, stat=ii)
unlock(four(2)[7])
end

! { dg-final { scan-tree-dump-times "one = \\(void \\* \\* restrict\\) _gfortran_caf_register \\(1, 2, \\(void \\* \\*\\) &caf_token.., 0B, 0B, 0\\);" 1 "original" } }
! { dg-final { scan-tree-dump-times "two = \\(void \\*\\\[25\\\] \\* restrict\\) _gfortran_caf_register \\(25, 2, \\(void \\* \\*\\) &caf_token.., 0B, 0B, 0\\);" 1 "original" } }
! { dg-final { scan-tree-dump-times "three.base_addr = \\(void \\* restrict\\) _gfortran_caf_register \\(1, 3, &three.token, &stat.., 0B, 0\\);" 1 "original" } }
! { dg-final { scan-tree-dump-times "four.base_addr = \\(void \\* restrict\\) _gfortran_caf_register \\(7, 3, &four.token, &stat.., 0B, 0\\);" 1 "original" } }

! { dg-final { scan-tree-dump-times "_gfortran_caf_lock \\(caf_token.., 0, 0, 0B, 0B, 0B, 0\\);" 1 "original" } }
! { dg-final { scan-tree-dump-times "_gfortran_caf_unlock \\(caf_token.., 0, 0, 0B, 0B, 0\\);" 1 "original" } }

! { dg-final { scan-tree-dump-times "_gfortran_caf_lock \\(caf_token.., \\(3 - \\(integer\\(kind=4\\)\\) parm...dim\\\[0\\\].lower_bound\\) \\+ \\(integer\\(kind=4\\)\\) MAX_EXPR <\\(\\(\\(parm...dim\\\[0\\\].lower_bound \\+ parm...dim\\\[0\\\].extent\\) \\+ -1\\) - parm...dim\\\[0\\\].lower_bound\\) \\+ 1, 0> \\* \\(3 - \\(integer\\(kind=4\\)\\) parm...dim\\\[1\\\].lower_bound\\), 0, 0B, &ii, 0B, 0\\);|_gfortran_caf_lock \\(caf_token.1, \\(3 - parm...dim\\\[0\\\].lower_bound\\) \\+ MAX_EXPR <\\(\\(\\(parm...dim\\\[0\\\].lower_bound \\+ parm...dim\\\[0\\\].extent\\) \\+ -1\\) - parm...dim\\\[0\\\].lower_bound\\) \\+ 1, 0> \\* \\(3 - parm...dim\\\[1\\\].lower_bound\\), 0, 0B, &ii, 0B, 0\\);" 1 "original" } }
! { dg-final { scan-tree-dump-times "_gfortran_caf_unlock \\(caf_token.., \\(2 - \\(integer\\(kind=4\\)\\) parm...dim\\\[0\\\].lower_bound\\) \\+ \\(integer\\(kind=4\\)\\) MAX_EXPR <\\(\\(\\(parm...dim\\\[0\\\].lower_bound \\+ parm...dim\\\[0\\\].extent\\) \\+ -1\\) - parm...dim\\\[0\\\].lower_bound\\) \\+ 1, 0> \\* \\(3 - \\(integer\\(kind=4\\)\\) parm...dim\\\[1\\\].lower_bound\\), 0, &ii, 0B, 0\\);|_gfortran_caf_unlock \\(caf_token.., \\(2 - parm...dim\\\[0\\\].lower_bound\\) \\+ MAX_EXPR <\\(\\(\\(parm...dim\\\[0\\\].lower_bound \\+ parm...dim\\\[0\\\].extent\\) \\+ -1\\) - parm...dim\\\[0\\\].lower_bound\\) \\+ 1, 0> \\* \\(3 - parm...dim\\\[1\\\].lower_bound\\), 0, &ii, 0B, 0\\);" 1 "original" } }

! { dg-final { scan-tree-dump-times "_gfortran_caf_lock \\(three.token, 0, 5 - \\(integer\\(kind=4\\)\\) three.dim\\\[0\\\].lower_bound, &acquired.8, 0B, 0B, 0\\);|_gfortran_caf_lock \\(three.token, 0, 5 - three.dim\\\[0\\\].lower_bound, &acquired.., 0B, 0B, 0\\);" 1 "original" } }
! { dg-final { scan-tree-dump-times "_gfortran_caf_unlock \\(three.token, 0, 8 - \\(integer\\(kind=4\\)\\) three.dim\\\[0\\\].lower_bound, &ii, 0B, 0\\);|_gfortran_caf_unlock \\(three.token, 0, 8 - three.dim\\\[0\\\].lower_bound, &ii, 0B, 0\\);" 1 "original" } }

! { dg-final { scan-tree-dump-times "_gfortran_caf_lock \\(four.token, 1 - \\(integer\\(kind=4\\)\\) four.dim\\\[0\\\].lower_bound, 7 - \\(integer\\(kind=4\\)\\) four.dim\\\[1\\\].lower_bound, &acquired.., &ii, 0B, 0\\);|_gfortran_caf_lock \\(four.token, 1 - four.dim\\\[0\\\].lower_bound, 7 - four.dim\\\[1\\\].lower_bound, &acquired.., &ii, 0B, 0\\);" 1 "original" } }
! { dg-final { scan-tree-dump-times "_gfortran_caf_unlock \\(four.token, 2 - \\(integer\\(kind=4\\)\\) four.dim\\\[0\\\].lower_bound, 8 - \\(integer\\(kind=4\\)\\) four.dim\\\[1\\\].lower_bound, 0B, 0B, 0\\);|_gfortran_caf_unlock \\(four.token, 2 - four.dim\\\[0\\\].lower_bound, 8 - four.dim\\\[1\\\].lower_bound, 0B, 0B, 0\\);" 1 "original" } }

