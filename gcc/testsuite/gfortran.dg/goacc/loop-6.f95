! { dg-do compile }
! { dg-additional-options "-fmax-errors=100" }

! TODO: nested kernels are allowed in 2.0

program test
  implicit none
  integer :: i, j

  !$acc parallel
    !$acc loop auto
    DO i = 1,10
    ENDDO
    !$acc loop gang
    DO i = 1,10
    ENDDO
    !$acc loop gang(static:5)
    DO i = 1,10
    ENDDO
    !$acc loop gang(static:*)
    DO i = 1,10
    ENDDO
    !$acc loop gang
    DO i = 1,10
      !$acc loop vector
      DO j = 1,10
      ENDDO
      !$acc loop worker
      DO j = 1,10
      ENDDO
    ENDDO

    !$acc loop worker
    DO i = 1,10
    ENDDO
    !$acc loop worker
    DO i = 1,10
      !$acc loop vector
      DO j = 1,10
      ENDDO
    ENDDO
    !$acc loop gang worker
    DO i = 1,10
    ENDDO

    !$acc loop vector
    DO i = 1,10
    ENDDO
    !$acc loop vector(5) ! { dg-error "no arguments allowed to gang" }
    DO i = 1,10
    ENDDO
    !$acc loop vector(length:5) ! { dg-error "no arguments allowed to gang" }
    DO i = 1,10
    ENDDO
    !$acc loop vector
    DO i = 1,10
    ENDDO
    !$acc loop gang vector
    DO i = 1,10
    ENDDO
    !$acc loop worker vector
    DO i = 1,10
    ENDDO

    !$acc loop auto
    DO i = 1,10
    ENDDO
  !$acc end parallel

  !$acc parallel loop vector
  DO i = 1,10
  ENDDO
  !$acc parallel loop vector(5) ! { dg-error "no arguments allowed to gang" }
  DO i = 1,10
  ENDDO
  !$acc parallel loop vector(length:5) ! { dg-error "no arguments allowed to gang" }
  DO i = 1,10
  ENDDO
end
