! { dg-do run }
! { dg-options "-std=legacy" }
! PR38439 I/O PD edit descriptor inconsistency
! Test case prepared by Jerry DeLisle <jvdelisle@gcc.gnu.org>
      character(len=25) :: str
      character(len=132) :: msg, line
      str = '(1pd24.15e6)'
      line = "initial string"
      x = 555.25
      
      write (line,str,iostat=istat, iomsg=msg) 1.0d0, 1.234
      if (istat.ne.0) call abort
      if (line.ne."   1.000000000000000D+001.E+00") call abort
      
      write (line,'(1pd24.15e6)',iostat=istat, iomsg=msg) 1.0d0, 1.234 ! { dg-warning "Period required" }
      if (istat.ne.0) call abort
      if (line.ne."   1.000000000000000D+001.E+00") call abort

      str = '(1pd0.15)'
      write (line,str,iostat=istat, iomsg=msg) 1.0d0
      if (istat.ne.5006 .or. msg(1:15).ne."Positive width ") call abort
      read (*,str,iostat=istat, iomsg=msg) x
      if (istat.ne.5006 .or. msg(1:15).ne."Positive width ") call abort
      if (x.ne.555.25) call abort
      
      write (line,'(1pd24.15e11.3)') 1.0d0, 1.234
      if (line.ne."   1.000000000000000D+00  1.234E+00") call abort
      
      end
