! PR libfortran/19678 and PR libfortran/19679
! { dg-do run { target fd_truncate } }
      integer i, j
      
      open (10,status='scratch')
      write (10,'(2A)') '1', achar(13)
      rewind (10)
      read (10,*) i
      if (i .ne. 1) call abort
      close (10)

      open (10,status='scratch')
      write (10,'(2A)') '   1', achar(13)
      write (10,'(2A)') '   2', achar(13)
      rewind (10)
      read (10,'(I4)') i
      read (10,'(I5)') j
      if ((i .ne. 1) .or. (j .ne. 2)) call abort
      end
