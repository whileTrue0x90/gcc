/* { dg-do compile } */

int
main ()
{
  int i, j;


#pragma acc parallel
  {
#pragma acc loop auto
    for (i = 0; i < 10; i++)
      { }
#pragma acc loop gang
    for (i = 0; i < 10; i++)
      { }
#pragma acc loop gang(static:5)
    for (i = 0; i < 10; i++)
      { }
#pragma acc loop gang(static:*)
    for (i = 0; i < 10; i++)
      { }
#pragma acc loop gang // { dg-error "gang, worker and vector may occur only once in a loop nest" }
    for (i = 0; i < 10; i++)
      {
#pragma acc loop vector
	for (j = 1; j < 10; j++)
	  { }
#pragma acc loop worker 
	for (j = 1; j < 10; j++)
	  { }
#pragma acc loop gang
	for (j = 1; j < 10; j++)
	  { }
      }
#pragma acc loop seq gang // { dg-error "incompatible use of clause" }
    for (i = 0; i < 10; i++)
      { }

#pragma acc loop worker
    for (i = 0; i < 10; i++)
      { }
#pragma acc loop worker // { dg-error "gang, worker and vector may occur only once in a loop nest" }
    for (i = 0; i < 10; i++)
      {
#pragma acc loop vector 
	for (j = 1; j < 10; j++)
	  { }
#pragma acc loop worker
	for (j = 1; j < 10; j++)
	  { }
#pragma acc loop gang
	for (j = 1; j < 10; j++)
	  { }
      }
#pragma acc loop seq worker // { dg-error "incompatible use of clause" }
    for (i = 0; i < 10; i++)
      { }
#pragma acc loop gang worker
    for (i = 0; i < 10; i++)
      { }

#pragma acc loop vector
    for (i = 0; i < 10; i++)
      { }
    for (i = 0; i < 10; i++)
      { }
#pragma acc loop vector // { dg-error "gang, worker and vector may occur only once in a loop nest" }
    for (i = 0; i < 10; i++)
      {
#pragma acc loop vector
	for (j = 1; j < 10; j++)
	  { }
#pragma acc loop worker
	for (j = 1; j < 10; j++)
	  { }
#pragma acc loop gang
	for (j = 1; j < 10; j++)
	  { }
      }
#pragma acc loop seq vector // { dg-error "incompatible use of clause" }
    for (i = 0; i < 10; i++)
      { }
#pragma acc loop gang vector
    for (i = 0; i < 10; i++)
      { }
#pragma acc loop worker vector
    for (i = 0; i < 10; i++)
      { }

#pragma acc loop auto
    for (i = 0; i < 10; i++)
      { }
#pragma acc loop seq auto // { dg-error "incompatible use of clause" }
    for (i = 0; i < 10; i++)
      { }
#pragma acc loop gang auto // { dg-error "incompatible use of clause" }
    for (i = 0; i < 10; i++)
      { }
#pragma acc loop worker auto // { dg-error "incompatible use of clause" }
    for (i = 0; i < 10; i++)
      { }
#pragma acc loop vector auto // { dg-error "incompatible use of clause" }
    for (i = 0; i < 10; i++)
      { }

  }

#pragma acc parallel loop auto
  for (i = 0; i < 10; i++)
    { }
#pragma acc parallel loop gang
  for (i = 0; i < 10; i++)
    { }
#pragma acc parallel loop gang(static:5)
  for (i = 0; i < 10; i++)
    { }
#pragma acc parallel loop gang(static:*)
  for (i = 0; i < 10; i++)
    { }

#pragma acc parallel loop seq gang // { dg-error "incompatible use of clause" "" { target c } }
  for (i = 0; i < 10; i++) // { dg-error "incompatible use of clause" "" { target c++ } }
    { }

#pragma acc parallel loop worker
  for (i = 0; i < 10; i++)
    { }

#pragma acc parallel loop seq worker // { dg-error "incompatible use of clause" "" { target c } }
  for (i = 0; i < 10; i++) // { dg-error "incompatible use of clause" "" { target c++ } }
    { }
#pragma acc parallel loop gang worker
  for (i = 0; i < 10; i++)
    { }

#pragma acc parallel loop vector
  for (i = 0; i < 10; i++)
    { }

#pragma acc parallel loop seq vector // { dg-error "incompatible use of clause" "" { target c } }
  for (i = 0; i < 10; i++) // { dg-error "incompatible use of clause" "" { target c++ } }
    { }
#pragma acc parallel loop gang vector
  for (i = 0; i < 10; i++)
    { }
#pragma acc parallel loop worker vector
  for (i = 0; i < 10; i++)
    { }

#pragma acc parallel loop auto
  for (i = 0; i < 10; i++)
    { }
#pragma acc parallel loop seq auto // { dg-error "incompatible use of clause" "" { target c } }
  for (i = 0; i < 10; i++) // { dg-error "incompatible use of clause" "" { target c++ } }
    { }
#pragma acc parallel loop gang auto // { dg-error "incompatible use of clause" "" { target c } }
  for (i = 0; i < 10; i++) // { dg-error "incompatible use of clause" "" { target c++ } }
    { }
#pragma acc parallel loop worker auto // { dg-error "incompatible use of clause" "" { target c } }
  for (i = 0; i < 10; i++) // { dg-error "incompatible use of clause" "" { target c++ } }
    { }
#pragma acc parallel loop vector auto // { dg-error "incompatible use of clause" "" { target c } }
  for (i = 0; i < 10; i++) // { dg-error "incompatible use of clause" "" { target c++ } }
    { }

  return 0;
}

