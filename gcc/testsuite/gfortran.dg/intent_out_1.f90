! { dg-do compile }
! Tests the fix for PRs 18578, 18579 and their repeats 20857 and 20885.
! Contributed by Paul Thomas  <pault@gcc@gnu.org>
  real, parameter :: a =42.0
  real :: b
  call foo(b + 2.0)    ! { dg-error "must be definable" }
  call foo(a)          ! { dg-error "must be definable" }
  call bar(b + 2.0)    ! { dg-error "must be definable" }
  call bar(a)          ! { dg-error "must be definable" }
contains
  subroutine foo(a)
    real, intent(out) :: a
    a = 0.0
  end subroutine foo
  subroutine bar(a)
    real, intent(INout) :: a
    a = 0.0
  end subroutine bar
end
