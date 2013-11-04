! { dg-do run }
!
! Uncovered in fixing PR fortran/58793
!
! Contributed by Tobias Burnus  <burnus@gcc.gnu.org>
!
! Barfed on the hollerith argument
!
program test
  logical l
  call up("abc", l)
  if (l) call abort
  call up(3habc, l) ! { dg-warning "Legacy Extension" }
  if (.not. l) call abort
contains
  subroutine up(x, l)
    class(*) :: x
    logical l
    select type(x)
     type is (character(*))
      l = .false.
     class default
      l = .true.
    end select
  end subroutine
end program test
