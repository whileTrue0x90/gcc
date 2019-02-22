! { dg-do compile }
! { dg-options "-O2 -Wuninitialized" }
! PR 67679 - this used to cause an undefined warning for
! variables generated by the compiler.

subroutine s(h, Gmin, r)

   implicit none
   real, intent(in) ::  Gmin(3), h(3)
   integer, intent(inout) :: r

   integer :: x_min(3), x_max(3), k, iStat
   logical, dimension(:), allocatable :: check

   do k = 1,1
      x_min(k) = int(Gmin(k)*h(k))
      x_max(k) = int(Gmin(k)*h(k))
   end do

   allocate(check(x_min(1):x_max(1)),stat=iStat)

   check(:) = .false.

   do k = x_min(1),x_max(1)
            r = r + 1
   end do

end
