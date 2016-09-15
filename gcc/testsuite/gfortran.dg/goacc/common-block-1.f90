! Test data clauses involving common blocks and common block data.
! Specifically, validate early matching errors.

subroutine subtest
  implicit none
  integer, parameter :: n = 10
  integer a(n), b(n), c, d(n), e
  real*4 x(n), y(n), z, w(n), v
  common /blockA/ a, c, x
  common /blockB/ b, y, z
  !$acc declare link(/blockA/, /blockB/, e, v)
end subroutine subtest

program test
  implicit none
  integer, parameter :: n = 10
  integer a(n), b(n), c, d(n), e
  real*4 x(n), y(n), z, w(n), v
  common /blockA/ a, c, x
  common /blockB/ b, y, z
  !$acc declare link(/blockA/, /blockB/, e, v)

  !$acc data copy(/blockA/, /blockB/, e, v)
  !$acc end data

  !$acc data copyin(/blockA/, /blockB/, e, v)
  !$acc end data

  !$acc data copyout(/blockA/, /blockB/, e, v)
  !$acc end data

  !$acc data create(/blockA/, /blockB/, e, v)
  !$acc end data

  !$acc data copyout(/blockA/, /blockB/, e, v)
  !$acc end data

  !$acc data pcopy(/blockA/, /blockB/, e, v)
  !$acc end data

  !$acc data pcopyin(/blockA/, /blockB/, e, v)
  !$acc end data

  !$acc data pcopyout(/blockA/, /blockB/, e, v)
  !$acc end data

  !$acc data pcreate(/blockA/, /blockB/, e, v)
  !$acc end data

  !$acc data pcopyout(/blockA/, /blockB/, e, v)
  !$acc end data

  !$acc data present(/blockA/, /blockB/, e, v) ! { dg-error "" }
  !$acc end data ! { dg-error "Unexpected ..ACC END DATA statement" }

  !$acc parallel private(/blockA/, /blockB/, e, v)
  !$acc end parallel

  !$acc parallel firstprivate(/blockA/, /blockB/, e, v)
  !$acc end parallel

  !$acc exit data delete(/blockA/, /blockB/, e, v)

  !$acc data deviceptr(/blockA/, /blockB/, e, v) ! { dg-error "Syntax error" }
  !$acc end data ! { dg-error "Unexpected ..ACC END DATA statement" }

  !$acc data deviceptr(/blockA/, /blockB/, e, v) ! { dg-error "Syntax error" }
  !$acc end data ! { dg-error "Unexpected ..ACC END DATA statement" }
end program test
