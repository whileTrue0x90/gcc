! { dg-do run }
! { dg-options "-std=f2008 -fall-intrinsics -fcheck=bounds" }

! PR fortran/29785
! Check pointer rank remapping at runtime.

! Contributed by Daniel Kraft, d@domob.eu.

PROGRAM main
  IMPLICIT NONE
  INTEGER, TARGET :: arr(12), basem(3, 4)
  INTEGER, POINTER :: vec(:), mat(:, :)
  INTEGER :: i

  arr = (/ (i, i = 1, 12) /)
  basem = RESHAPE (arr, SHAPE (basem))

  ! We need not necessarily change the rank...
  vec(2_1:5) => arr(1_1:12_1:2_1)
  IF (LBOUND (vec, 1) /= 2 .OR. UBOUND (vec, 1) /= 5) CALL abort ()
  IF (ANY (vec /= (/ 1, 3, 5, 7 /))) CALL abort ()
  IF (vec(2) /= 1 .OR. vec(5) /= 7) CALL abort ()

  ! ...but it is of course the more interesting.  Also try remapping a pointer.
  vec => arr(1:12:2)
  mat(1:3, 1:2) => vec
  IF (ANY (LBOUND (mat) /= (/ 1, 1 /) .OR. UBOUND (mat) /= (/ 3, 2 /))) &
    CALL abort ()
  IF (ANY (mat /= RESHAPE (arr(1:12:2), SHAPE (mat)))) CALL abort ()
  IF (mat(1, 1) /= 1 .OR. mat(1, 2) /= 7) CALL abort ()

  ! Remap with target of rank > 1.
  vec(1:12_1) => basem
  IF (LBOUND (vec, 1) /= 1 .OR. UBOUND (vec, 1) /= 12) CALL abort ()
  IF (ANY (vec /= arr)) CALL abort ()
  IF (vec(1) /= 1 .OR. vec(5) /= 5 .OR. vec(12) /= 12) CALL abort ()
END PROGRAM main
