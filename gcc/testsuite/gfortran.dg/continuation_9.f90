! { dg-do compile }
! { dg-options "-std=f95" }
  &  
&
 &
end
! { dg-warning "not allowed by itself in line 3" "" {target "*-*-*"} 0 }
! { dg-warning "not allowed by itself in line 4" "" {target "*-*-*"} 0 }
! { dg-warning "not allowed by itself in line 5" "" {target "*-*-*"} 0 }
