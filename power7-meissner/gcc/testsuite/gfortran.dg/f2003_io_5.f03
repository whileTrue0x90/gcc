! { dg-do run { target fd_truncate } }
! Test case prepared by Jerry DeLisle  <jvdelisle@gcc.gnu.org>
! Test of decimal="comma" in namelist and complex
integer :: i
real :: a(10) = [ (i*1.3, i=1,10) ]
real :: b(10)
complex :: c
character(34) :: complex
namelist /nm/ a

open(99,file="mynml",form="formatted",decimal="point",status="replace")
write(99,nml=nm,decimal="comma")
a = 5.55
rewind(99)
read(99,nml=nm,decimal="comma")
if (any (a /= [ (i*1.3, i=1,10) ])) call abort
close(99, status="delete")

c = (3.123,4.456)
write(complex,*,decimal="comma") c
if (complex.ne." (  3,1229999    ;  4,4559999    )") call abort
c = (0.0, 0.0)
read(complex,*,decimal="comma") c
if (complex.ne." (  3,1229999    ;  4,4559999    )") call abort

end
