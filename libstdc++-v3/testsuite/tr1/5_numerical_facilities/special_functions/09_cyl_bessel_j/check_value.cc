// 2007-02-04  Edward Smith-Rowland <3dw4rd@verizon.net>
//
// Copyright (C) 2007 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING.  If not, write to the Free
// Software Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
// USA.

//  cyl_bessel_j


//  Compare against values generated by the GNU Scientific Library.
//  The GSL can be found on the web: http://www.gnu.org/software/gsl/

#include <tr1/cmath>
#if defined(__TEST_DEBUG)
#include <iostream>
#define VERIFY(A) \
if (!(A)) \
  { \
    std::cout << "line " << __LINE__ \
      << "  max_abs_frac = " << max_abs_frac \
      << std::endl; \
  }
#else
#include <testsuite_hooks.h>
#endif
#include "../testcase.h"


// Test data for nu=0.0000000000000000.
testcase_cyl_bessel_j<double> data001[] = {
  { 1.0000000000000000, 0.0000000000000000, 0.0000000000000000 },
  { -0.17759677131433835, 0.0000000000000000, 5.0000000000000000 },
  { -0.24593576445134832, 0.0000000000000000, 10.000000000000000 },
  { -0.014224472826780788, 0.0000000000000000, 15.000000000000000 },
  { 0.16702466434058319, 0.0000000000000000, 20.000000000000000 },
  { 0.096266783275958140, 0.0000000000000000, 25.000000000000000 },
  { -0.086367983581040184, 0.0000000000000000, 30.000000000000000 },
  { -0.12684568275631256, 0.0000000000000000, 35.000000000000000 },
  { 0.0073668905842372914, 0.0000000000000000, 40.000000000000000 },
  { 0.11581867067325630, 0.0000000000000000, 45.000000000000000 },
  { 0.055812327669251746, 0.0000000000000000, 50.000000000000000 },
  { -0.074548302648236808, 0.0000000000000000, 55.000000000000000 },
  { -0.091471804089061873, 0.0000000000000000, 60.000000000000000 },
  { 0.018687343227677920, 0.0000000000000000, 65.000000000000000 },
  { 0.094908726483013517, 0.0000000000000000, 70.000000000000000 },
  { 0.034643913805097029, 0.0000000000000000, 75.000000000000000 },
  { -0.069742165512210061, 0.0000000000000000, 80.000000000000000 },
  { -0.070940394796273301, 0.0000000000000000, 85.000000000000000 },
  { 0.026630016699969568, 0.0000000000000000, 90.000000000000000 },
  { 0.081811967783384149, 0.0000000000000000, 95.000000000000000 },
  { 0.019985850304223264, 0.0000000000000000, 100.00000000000000 },
};

// Test function for nu=0.0000000000000000.
template <typename Tp>
void test001()
{
  const Tp eps = std::numeric_limits<Tp>::epsilon();
  Tp max_abs_diff = -Tp(1);
  Tp max_abs_frac = -Tp(1);
  unsigned int num_datum = sizeof(data001)
                         / sizeof(testcase_cyl_bessel_j<double>);
  for (unsigned int i = 0; i < num_datum; ++i)
    {
      const Tp f = std::tr1::cyl_bessel_j(Tp(data001[i].nu), Tp(data001[i].x));
      const Tp f0 = data001[i].f0;
      const Tp diff = f - f0;
      if (std::abs(diff) > max_abs_diff)
        max_abs_diff = std::abs(diff);
      if (std::abs(f0) > Tp(10) * eps
       && std::abs(f) > Tp(10) * eps)
        {
          const Tp frac = diff / f0;
          if (std::abs(frac) > max_abs_frac)
            max_abs_frac = std::abs(frac);
        }
    }
  VERIFY(max_abs_frac < Tp(2.5000000000000014e-11));
}

// Test data for nu=0.33333333333333331.
testcase_cyl_bessel_j<double> data002[] = {
  { 0.0000000000000000, 0.33333333333333331, 0.0000000000000000 },
  { -0.30642046380026405, 0.33333333333333331, 5.0000000000000000 },
  { -0.18614516704869569, 0.33333333333333331, 10.000000000000000 },
  { 0.089740004221152581, 0.33333333333333331, 15.000000000000000 },
  { 0.17606058001293898, 0.33333333333333331, 20.000000000000000 },
  { 0.020097162141383202, 0.33333333333333331, 25.000000000000000 },
  { -0.13334053387426156, 0.33333333333333331, 30.000000000000000 },
  { -0.087118009397765442, 0.33333333333333331, 35.000000000000000 },
  { 0.069202942818858165, 0.33333333333333331, 40.000000000000000 },
  { 0.11387616964518317, 0.33333333333333331, 45.000000000000000 },
  { -0.00057226680771807741, 0.33333333333333331, 50.000000000000000 },
  { -0.10331600929280821, 0.33333333333333331, 55.000000000000000 },
  { -0.055618147270528023, 0.33333333333333331, 60.000000000000000 },
  { 0.064711954014113920, 0.33333333333333331, 65.000000000000000 },
  { 0.086879926462481619, 0.33333333333333331, 70.000000000000000 },
  { -0.012614484229891070, 0.33333333333333331, 75.000000000000000 },
  { -0.088199784400034537, 0.33333333333333331, 80.000000000000000 },
  { -0.036703611076564557, 0.33333333333333331, 85.000000000000000 },
  { 0.062916286828779533, 0.33333333333333331, 90.000000000000000 },
  { 0.069465244416806071, 0.33333333333333331, 95.000000000000000 },
  { -0.021271244853702295, 0.33333333333333331, 100.00000000000000 },
};

// Test function for nu=0.33333333333333331.
template <typename Tp>
void test002()
{
  const Tp eps = std::numeric_limits<Tp>::epsilon();
  Tp max_abs_diff = -Tp(1);
  Tp max_abs_frac = -Tp(1);
  unsigned int num_datum = sizeof(data002)
                         / sizeof(testcase_cyl_bessel_j<double>);
  for (unsigned int i = 0; i < num_datum; ++i)
    {
      const Tp f = std::tr1::cyl_bessel_j(Tp(data002[i].nu), Tp(data002[i].x));
      const Tp f0 = data002[i].f0;
      const Tp diff = f - f0;
      if (std::abs(diff) > max_abs_diff)
        max_abs_diff = std::abs(diff);
      if (std::abs(f0) > Tp(10) * eps
       && std::abs(f) > Tp(10) * eps)
        {
          const Tp frac = diff / f0;
          if (std::abs(frac) > max_abs_frac)
            max_abs_frac = std::abs(frac);
        }
    }
  VERIFY(max_abs_frac < Tp(2.5000000000000017e-10));
}

// Test data for nu=0.50000000000000000.
testcase_cyl_bessel_j<double> data003[] = {
  { 0.0000000000000000, 0.50000000000000000, 0.0000000000000000 },
  { -0.34216798479816180, 0.50000000000000000, 5.0000000000000000 },
  { -0.13726373575505049, 0.50000000000000000, 10.000000000000000 },
  { 0.13396768882243942, 0.50000000000000000, 15.000000000000000 },
  { 0.16288076385502981, 0.50000000000000000, 20.000000000000000 },
  { -0.021120283599650416, 0.50000000000000000, 25.000000000000000 },
  { -0.14392965337039987, 0.50000000000000000, 30.000000000000000 },
  { -0.057747757589458860, 0.50000000000000000, 35.000000000000000 },
  { 0.094000962389533579, 0.50000000000000000, 40.000000000000000 },
  { 0.10120783324271412, 0.50000000000000000, 45.000000000000000 },
  { -0.029605831888924662, 0.50000000000000000, 50.000000000000000 },
  { -0.10756039213265806, 0.50000000000000000, 55.000000000000000 },
  { -0.031397461182520445, 0.50000000000000000, 60.000000000000000 },
  { 0.081827430775628568, 0.50000000000000000, 65.000000000000000 },
  { 0.073802429539054637, 0.50000000000000000, 70.000000000000000 },
  { -0.035727009681702594, 0.50000000000000000, 75.000000000000000 },
  { -0.088661035811765446, 0.50000000000000000, 80.000000000000000 },
  { -0.015238065106312407, 0.50000000000000000, 85.000000000000000 },
  { 0.075189068550269425, 0.50000000000000000, 90.000000000000000 },
  { 0.055932643481494140, 0.50000000000000000, 95.000000000000000 },
  { -0.040402132716252113, 0.50000000000000000, 100.00000000000000 },
};

// Test function for nu=0.50000000000000000.
template <typename Tp>
void test003()
{
  const Tp eps = std::numeric_limits<Tp>::epsilon();
  Tp max_abs_diff = -Tp(1);
  Tp max_abs_frac = -Tp(1);
  unsigned int num_datum = sizeof(data003)
                         / sizeof(testcase_cyl_bessel_j<double>);
  for (unsigned int i = 0; i < num_datum; ++i)
    {
      const Tp f = std::tr1::cyl_bessel_j(Tp(data003[i].nu), Tp(data003[i].x));
      const Tp f0 = data003[i].f0;
      const Tp diff = f - f0;
      if (std::abs(diff) > max_abs_diff)
        max_abs_diff = std::abs(diff);
      if (std::abs(f0) > Tp(10) * eps
       && std::abs(f) > Tp(10) * eps)
        {
          const Tp frac = diff / f0;
          if (std::abs(frac) > max_abs_frac)
            max_abs_frac = std::abs(frac);
        }
    }
  VERIFY(max_abs_frac < Tp(2.5000000000000014e-11));
}

// Test data for nu=0.66666666666666663.
testcase_cyl_bessel_j<double> data004[] = {
  { 0.0000000000000000, 0.66666666666666663, 0.0000000000000000 },
  { -0.35712533549168879, 0.66666666666666663, 5.0000000000000000 },
  { -0.080149603304315822, 0.66666666666666663, 10.000000000000000 },
  { 0.16918875175798076, 0.66666666666666663, 15.000000000000000 },
  { 0.13904826122116526, 0.66666666666666663, 20.000000000000000 },
  { -0.060770629698497579, 0.66666666666666663, 25.000000000000000 },
  { -0.14489851974205059, 0.66666666666666663, 30.000000000000000 },
  { -0.024604880159644467, 0.66666666666666663, 35.000000000000000 },
  { 0.11243936464912015, 0.66666666666666663, 40.000000000000000 },
  { 0.081776275512525379, 0.66666666666666663, 45.000000000000000 },
  { -0.056589908749367770, 0.66666666666666663, 50.000000000000000 },
  { -0.10455814523765933, 0.66666666666666663, 55.000000000000000 },
  { -0.0051030148548608109, 0.66666666666666663, 60.000000000000000 },
  { 0.093398227061639250, 0.66666666666666663, 65.000000000000000 },
  { 0.055763883611864899, 0.66666666666666663, 70.000000000000000 },
  { -0.056395322915757343, 0.66666666666666663, 75.000000000000000 },
  { -0.083131347805783087, 0.66666666666666663, 80.000000000000000 },
  { 0.0072315397874096309, 0.66666666666666663, 85.000000000000000 },
  { 0.082362798520905264, 0.66666666666666663, 90.000000000000000 },
  { 0.038630504403446168, 0.66666666666666663, 95.000000000000000 },
  { -0.056778819380529706, 0.66666666666666663, 100.00000000000000 },
};

// Test function for nu=0.66666666666666663.
template <typename Tp>
void test004()
{
  const Tp eps = std::numeric_limits<Tp>::epsilon();
  Tp max_abs_diff = -Tp(1);
  Tp max_abs_frac = -Tp(1);
  unsigned int num_datum = sizeof(data004)
                         / sizeof(testcase_cyl_bessel_j<double>);
  for (unsigned int i = 0; i < num_datum; ++i)
    {
      const Tp f = std::tr1::cyl_bessel_j(Tp(data004[i].nu), Tp(data004[i].x));
      const Tp f0 = data004[i].f0;
      const Tp diff = f - f0;
      if (std::abs(diff) > max_abs_diff)
        max_abs_diff = std::abs(diff);
      if (std::abs(f0) > Tp(10) * eps
       && std::abs(f) > Tp(10) * eps)
        {
          const Tp frac = diff / f0;
          if (std::abs(frac) > max_abs_frac)
            max_abs_frac = std::abs(frac);
        }
    }
  VERIFY(max_abs_frac < Tp(1.0000000000000006e-10));
}

// Test data for nu=1.0000000000000000.
testcase_cyl_bessel_j<double> data005[] = {
  { 0.0000000000000000, 1.0000000000000000, 0.0000000000000000 },
  { -0.32757913759146523, 1.0000000000000000, 5.0000000000000000 },
  { 0.043472746168861369, 1.0000000000000000, 10.000000000000000 },
  { 0.20510403861352278, 1.0000000000000000, 15.000000000000000 },
  { 0.066833124175850092, 1.0000000000000000, 20.000000000000000 },
  { -0.12535024958028987, 1.0000000000000000, 25.000000000000000 },
  { -0.11875106261662292, 1.0000000000000000, 30.000000000000000 },
  { 0.043990942179625556, 1.0000000000000000, 35.000000000000000 },
  { 0.12603831803758497, 1.0000000000000000, 40.000000000000000 },
  { 0.028348854376424548, 1.0000000000000000, 45.000000000000000 },
  { -0.097511828125175157, 1.0000000000000000, 50.000000000000000 },
  { -0.078250038308684655, 1.0000000000000000, 55.000000000000000 },
  { 0.046598383758166398, 1.0000000000000000, 60.000000000000000 },
  { 0.097330172226126929, 1.0000000000000000, 65.000000000000000 },
  { 0.0099877887848384625, 1.0000000000000000, 70.000000000000000 },
  { -0.085139995044829109, 1.0000000000000000, 75.000000000000000 },
  { -0.056057296675712610, 1.0000000000000000, 80.000000000000000 },
  { 0.049151460334891116, 1.0000000000000000, 85.000000000000000 },
  { 0.079925646708868064, 1.0000000000000000, 90.000000000000000 },
  { -0.0023925612997268684, 1.0000000000000000, 95.000000000000000 },
  { -0.077145352014112142, 1.0000000000000000, 100.00000000000000 },
};

// Test function for nu=1.0000000000000000.
template <typename Tp>
void test005()
{
  const Tp eps = std::numeric_limits<Tp>::epsilon();
  Tp max_abs_diff = -Tp(1);
  Tp max_abs_frac = -Tp(1);
  unsigned int num_datum = sizeof(data005)
                         / sizeof(testcase_cyl_bessel_j<double>);
  for (unsigned int i = 0; i < num_datum; ++i)
    {
      const Tp f = std::tr1::cyl_bessel_j(Tp(data005[i].nu), Tp(data005[i].x));
      const Tp f0 = data005[i].f0;
      const Tp diff = f - f0;
      if (std::abs(diff) > max_abs_diff)
        max_abs_diff = std::abs(diff);
      if (std::abs(f0) > Tp(10) * eps
       && std::abs(f) > Tp(10) * eps)
        {
          const Tp frac = diff / f0;
          if (std::abs(frac) > max_abs_frac)
            max_abs_frac = std::abs(frac);
        }
    }
  VERIFY(max_abs_frac < Tp(1.0000000000000006e-10));
}

// Test data for nu=2.0000000000000000.
testcase_cyl_bessel_j<double> data006[] = {
  { 0.0000000000000000, 2.0000000000000000, 0.0000000000000000 },
  { 0.046565116277751971, 2.0000000000000000, 5.0000000000000000 },
  { 0.25463031368512062, 2.0000000000000000, 10.000000000000000 },
  { 0.041571677975250479, 2.0000000000000000, 15.000000000000000 },
  { -0.16034135192299817, 2.0000000000000000, 20.000000000000000 },
  { -0.10629480324238134, 2.0000000000000000, 25.000000000000000 },
  { 0.078451246073265340, 2.0000000000000000, 30.000000000000000 },
  { 0.12935945088086262, 2.0000000000000000, 35.000000000000000 },
  { -0.0010649746823580893, 2.0000000000000000, 40.000000000000000 },
  { -0.11455872158985966, 2.0000000000000000, 45.000000000000000 },
  { -0.059712800794258801, 2.0000000000000000, 50.000000000000000 },
  { 0.071702846709739212, 2.0000000000000000, 55.000000000000000 },
  { 0.093025083547667448, 2.0000000000000000, 60.000000000000000 },
  { -0.015692568697643280, 2.0000000000000000, 65.000000000000000 },
  { -0.094623361089160987, 2.0000000000000000, 70.000000000000000 },
  { -0.036914313672959186, 2.0000000000000000, 75.000000000000000 },
  { 0.068340733095317227, 2.0000000000000000, 80.000000000000000 },
  { 0.072096899745329499, 2.0000000000000000, 85.000000000000000 },
  { -0.024853891217550262, 2.0000000000000000, 90.000000000000000 },
  { -0.081862337494957346, 2.0000000000000000, 95.000000000000000 },
  { -0.021528757344505392, 2.0000000000000000, 100.00000000000000 },
};

// Test function for nu=2.0000000000000000.
template <typename Tp>
void test006()
{
  const Tp eps = std::numeric_limits<Tp>::epsilon();
  Tp max_abs_diff = -Tp(1);
  Tp max_abs_frac = -Tp(1);
  unsigned int num_datum = sizeof(data006)
                         / sizeof(testcase_cyl_bessel_j<double>);
  for (unsigned int i = 0; i < num_datum; ++i)
    {
      const Tp f = std::tr1::cyl_bessel_j(Tp(data006[i].nu), Tp(data006[i].x));
      const Tp f0 = data006[i].f0;
      const Tp diff = f - f0;
      if (std::abs(diff) > max_abs_diff)
        max_abs_diff = std::abs(diff);
      if (std::abs(f0) > Tp(10) * eps
       && std::abs(f) > Tp(10) * eps)
        {
          const Tp frac = diff / f0;
          if (std::abs(frac) > max_abs_frac)
            max_abs_frac = std::abs(frac);
        }
    }
  VERIFY(max_abs_frac < Tp(1.0000000000000006e-10));
}

// Test data for nu=5.0000000000000000.
testcase_cyl_bessel_j<double> data007[] = {
  { 0.0000000000000000, 5.0000000000000000, 0.0000000000000000 },
  { 0.26114054612017007, 5.0000000000000000, 5.0000000000000000 },
  { -0.23406152818679365, 5.0000000000000000, 10.000000000000000 },
  { 0.13045613456502966, 5.0000000000000000, 15.000000000000000 },
  { 0.15116976798239504, 5.0000000000000000, 20.000000000000000 },
  { -0.066007995398423044, 5.0000000000000000, 25.000000000000000 },
  { -0.14324029551207709, 5.0000000000000000, 30.000000000000000 },
  { -0.0015053072953907080, 5.0000000000000000, 35.000000000000000 },
  { 0.12257346597711774, 5.0000000000000000, 40.000000000000000 },
  { 0.057984499200954144, 5.0000000000000000, 45.000000000000000 },
  { -0.081400247696569658, 5.0000000000000000, 50.000000000000000 },
  { -0.092569895786432710, 5.0000000000000000, 55.000000000000000 },
  { 0.027454744228344184, 5.0000000000000000, 60.000000000000000 },
  { 0.099110527701539039, 5.0000000000000000, 65.000000000000000 },
  { 0.026058129823895274, 5.0000000000000000, 70.000000000000000 },
  { -0.078523977013751398, 5.0000000000000000, 75.000000000000000 },
  { -0.065862349140031654, 5.0000000000000000, 80.000000000000000 },
  { 0.038669072284680923, 5.0000000000000000, 85.000000000000000 },
  { 0.082759319528415129, 5.0000000000000000, 90.000000000000000 },
  { 0.0079423372702472905, 5.0000000000000000, 95.000000000000000 },
  { -0.074195736964513911, 5.0000000000000000, 100.00000000000000 },
};

// Test function for nu=5.0000000000000000.
template <typename Tp>
void test007()
{
  const Tp eps = std::numeric_limits<Tp>::epsilon();
  Tp max_abs_diff = -Tp(1);
  Tp max_abs_frac = -Tp(1);
  unsigned int num_datum = sizeof(data007)
                         / sizeof(testcase_cyl_bessel_j<double>);
  for (unsigned int i = 0; i < num_datum; ++i)
    {
      const Tp f = std::tr1::cyl_bessel_j(Tp(data007[i].nu), Tp(data007[i].x));
      const Tp f0 = data007[i].f0;
      const Tp diff = f - f0;
      if (std::abs(diff) > max_abs_diff)
        max_abs_diff = std::abs(diff);
      if (std::abs(f0) > Tp(10) * eps
       && std::abs(f) > Tp(10) * eps)
        {
          const Tp frac = diff / f0;
          if (std::abs(frac) > max_abs_frac)
            max_abs_frac = std::abs(frac);
        }
    }
  VERIFY(max_abs_frac < Tp(5.0000000000000028e-11));
}

// Test data for nu=10.000000000000000.
testcase_cyl_bessel_j<double> data008[] = {
  { 0.0000000000000000, 10.000000000000000, 0.0000000000000000 },
  { 0.0014678026473104744, 10.000000000000000, 5.0000000000000000 },
  { 0.20748610663335865, 10.000000000000000, 10.000000000000000 },
  { -0.090071811047659045, 10.000000000000000, 15.000000000000000 },
  { 0.18648255802394512, 10.000000000000000, 20.000000000000000 },
  { -0.075179843948523270, 10.000000000000000, 25.000000000000000 },
  { -0.12987689399858882, 10.000000000000000, 30.000000000000000 },
  { 0.063546391343962852, 10.000000000000000, 35.000000000000000 },
  { 0.11938336278226093, 10.000000000000000, 40.000000000000000 },
  { -0.026971402475010734, 10.000000000000000, 45.000000000000000 },
  { -0.11384784914946940, 10.000000000000000, 50.000000000000000 },
  { -0.015773790303746010, 10.000000000000000, 55.000000000000000 },
  { 0.097177143328071106, 10.000000000000000, 60.000000000000000 },
  { 0.054617389951112157, 10.000000000000000, 65.000000000000000 },
  { -0.065870338561951874, 10.000000000000000, 70.000000000000000 },
  { -0.080417867891894437, 10.000000000000000, 75.000000000000000 },
  { 0.024043850978184754, 10.000000000000000, 80.000000000000000 },
  { 0.086824832700067869, 10.000000000000000, 85.000000000000000 },
  { 0.019554748856312278, 10.000000000000000, 90.000000000000000 },
  { -0.072341598669443757, 10.000000000000000, 95.000000000000000 },
  { -0.054732176935472103, 10.000000000000000, 100.00000000000000 },
};

// Test function for nu=10.000000000000000.
template <typename Tp>
void test008()
{
  const Tp eps = std::numeric_limits<Tp>::epsilon();
  Tp max_abs_diff = -Tp(1);
  Tp max_abs_frac = -Tp(1);
  unsigned int num_datum = sizeof(data008)
                         / sizeof(testcase_cyl_bessel_j<double>);
  for (unsigned int i = 0; i < num_datum; ++i)
    {
      const Tp f = std::tr1::cyl_bessel_j(Tp(data008[i].nu), Tp(data008[i].x));
      const Tp f0 = data008[i].f0;
      const Tp diff = f - f0;
      if (std::abs(diff) > max_abs_diff)
        max_abs_diff = std::abs(diff);
      if (std::abs(f0) > Tp(10) * eps
       && std::abs(f) > Tp(10) * eps)
        {
          const Tp frac = diff / f0;
          if (std::abs(frac) > max_abs_frac)
            max_abs_frac = std::abs(frac);
        }
    }
  VERIFY(max_abs_frac < Tp(2.5000000000000014e-11));
}

// Test data for nu=20.000000000000000.
testcase_cyl_bessel_j<double> data009[] = {
  { 0.0000000000000000, 20.000000000000000, 0.0000000000000000 },
  { 2.7703300521289426e-11, 20.000000000000000, 5.0000000000000000 },
  { 1.1513369247813403e-05, 20.000000000000000, 10.000000000000000 },
  { 0.0073602340792234882, 20.000000000000000, 15.000000000000000 },
  { 0.16474777377532657, 20.000000000000000, 20.000000000000000 },
  { 0.051994049228303287, 20.000000000000000, 25.000000000000000 },
  { 0.0048310199934041105, 20.000000000000000, 30.000000000000000 },
  { -0.10927417397178038, 20.000000000000000, 35.000000000000000 },
  { 0.12779393355084886, 20.000000000000000, 40.000000000000000 },
  { 0.0047633437900312841, 20.000000000000000, 45.000000000000000 },
  { -0.11670435275957974, 20.000000000000000, 50.000000000000000 },
  { 0.025389204574566695, 20.000000000000000, 55.000000000000000 },
  { 0.10266020557876331, 20.000000000000000, 60.000000000000000 },
  { -0.023138582263434168, 20.000000000000000, 65.000000000000000 },
  { -0.096058573489952323, 20.000000000000000, 70.000000000000000 },
  { 0.0068961047221522270, 20.000000000000000, 75.000000000000000 },
  { 0.090565405489918357, 20.000000000000000, 80.000000000000000 },
  { 0.015985497599497155, 20.000000000000000, 85.000000000000000 },
  { -0.080345344044422506, 20.000000000000000, 90.000000000000000 },
  { -0.040253075701614051, 20.000000000000000, 95.000000000000000 },
  { 0.062217458498338679, 20.000000000000000, 100.00000000000000 },
};

// Test function for nu=20.000000000000000.
template <typename Tp>
void test009()
{
  const Tp eps = std::numeric_limits<Tp>::epsilon();
  Tp max_abs_diff = -Tp(1);
  Tp max_abs_frac = -Tp(1);
  unsigned int num_datum = sizeof(data009)
                         / sizeof(testcase_cyl_bessel_j<double>);
  for (unsigned int i = 0; i < num_datum; ++i)
    {
      const Tp f = std::tr1::cyl_bessel_j(Tp(data009[i].nu), Tp(data009[i].x));
      const Tp f0 = data009[i].f0;
      const Tp diff = f - f0;
      if (std::abs(diff) > max_abs_diff)
        max_abs_diff = std::abs(diff);
      if (std::abs(f0) > Tp(10) * eps
       && std::abs(f) > Tp(10) * eps)
        {
          const Tp frac = diff / f0;
          if (std::abs(frac) > max_abs_frac)
            max_abs_frac = std::abs(frac);
        }
    }
  VERIFY(max_abs_frac < Tp(1.0000000000000006e-10));
}

// Test data for nu=50.000000000000000.
testcase_cyl_bessel_j<double> data010[] = {
  { 0.0000000000000000, 50.000000000000000, 0.0000000000000000 },
  { 2.2942476159525415e-45, 50.000000000000000, 5.0000000000000000 },
  { 1.7845136078715964e-30, 50.000000000000000, 10.000000000000000 },
  { 6.1060519495338733e-22, 50.000000000000000, 15.000000000000000 },
  { 4.4510392847006872e-16, 50.000000000000000, 20.000000000000000 },
  { 9.7561594280229727e-12, 50.000000000000000, 25.000000000000000 },
  { 2.0581656631564181e-08, 50.000000000000000, 30.000000000000000 },
  { 7.6069951699272926e-06, 50.000000000000000, 35.000000000000000 },
  { 0.00068185243531768255, 50.000000000000000, 40.000000000000000 },
  { 0.017284343240791228, 50.000000000000000, 45.000000000000000 },
  { 0.12140902189761522, 50.000000000000000, 50.000000000000000 },
  { 0.13594720957176004, 50.000000000000000, 55.000000000000000 },
  { -0.13798273148535209, 50.000000000000000, 60.000000000000000 },
  { 0.12116217746619408, 50.000000000000000, 65.000000000000000 },
  { -0.11394866738787141, 50.000000000000000, 70.000000000000000 },
  { 0.094076799581573417, 50.000000000000000, 75.000000000000000 },
  { -0.039457764590251236, 50.000000000000000, 80.000000000000000 },
  { -0.040412060734136369, 50.000000000000000, 85.000000000000000 },
  { 0.090802099838032252, 50.000000000000000, 90.000000000000000 },
  { -0.055979156267280269, 50.000000000000000, 95.000000000000000 },
  { -0.038698339728525460, 50.000000000000000, 100.00000000000000 },
};

// Test function for nu=50.000000000000000.
template <typename Tp>
void test010()
{
  const Tp eps = std::numeric_limits<Tp>::epsilon();
  Tp max_abs_diff = -Tp(1);
  Tp max_abs_frac = -Tp(1);
  unsigned int num_datum = sizeof(data010)
                         / sizeof(testcase_cyl_bessel_j<double>);
  for (unsigned int i = 0; i < num_datum; ++i)
    {
      const Tp f = std::tr1::cyl_bessel_j(Tp(data010[i].nu), Tp(data010[i].x));
      const Tp f0 = data010[i].f0;
      const Tp diff = f - f0;
      if (std::abs(diff) > max_abs_diff)
        max_abs_diff = std::abs(diff);
      if (std::abs(f0) > Tp(10) * eps
       && std::abs(f) > Tp(10) * eps)
        {
          const Tp frac = diff / f0;
          if (std::abs(frac) > max_abs_frac)
            max_abs_frac = std::abs(frac);
        }
    }
  VERIFY(max_abs_frac < Tp(1.0000000000000006e-11));
}

// Test data for nu=100.00000000000000.
testcase_cyl_bessel_j<double> data011[] = {
  { 0.0000000000000000, 100.00000000000000, 0.0000000000000000 },
  { 6.2677893955418763e-119, 100.00000000000000, 5.0000000000000000 },
  { 6.5973160641553816e-89, 100.00000000000000, 10.000000000000000 },
  { 1.9660095611249536e-71, 100.00000000000000, 15.000000000000000 },
  { 3.9617550943362524e-59, 100.00000000000000, 20.000000000000000 },
  { 1.1064482655301687e-49, 100.00000000000000, 25.000000000000000 },
  { 4.5788015281752354e-42, 100.00000000000000, 30.000000000000000 },
  { 9.9210206714732606e-36, 100.00000000000000, 35.000000000000000 },
  { 2.3866062996027414e-30, 100.00000000000000, 40.000000000000000 },
  { 1.0329791804565538e-25, 100.00000000000000, 45.000000000000000 },
  { 1.1159273690838340e-21, 100.00000000000000, 50.000000000000000 },
  { 3.7899753451900682e-18, 100.00000000000000, 55.000000000000000 },
  { 4.7832744078781205e-15, 100.00000000000000, 60.000000000000000 },
  { 2.5375564579490517e-12, 100.00000000000000, 65.000000000000000 },
  { 6.1982452141641260e-10, 100.00000000000000, 70.000000000000000 },
  { 7.4479005905904457e-08, 100.00000000000000, 75.000000000000000 },
  { 4.6065530648234948e-06, 100.00000000000000, 80.000000000000000 },
  { 0.00015043869999501605, 100.00000000000000, 85.000000000000000 },
  { 0.0026021305819963472, 100.00000000000000, 90.000000000000000 },
  { 0.023150768009428162, 100.00000000000000, 95.000000000000000 },
  { 0.096366673295861571, 100.00000000000000, 100.00000000000000 },
};

// Test function for nu=100.00000000000000.
template <typename Tp>
void test011()
{
  const Tp eps = std::numeric_limits<Tp>::epsilon();
  Tp max_abs_diff = -Tp(1);
  Tp max_abs_frac = -Tp(1);
  unsigned int num_datum = sizeof(data011)
                         / sizeof(testcase_cyl_bessel_j<double>);
  for (unsigned int i = 0; i < num_datum; ++i)
    {
      const Tp f = std::tr1::cyl_bessel_j(Tp(data011[i].nu), Tp(data011[i].x));
      const Tp f0 = data011[i].f0;
      const Tp diff = f - f0;
      if (std::abs(diff) > max_abs_diff)
        max_abs_diff = std::abs(diff);
      if (std::abs(f0) > Tp(10) * eps
       && std::abs(f) > Tp(10) * eps)
        {
          const Tp frac = diff / f0;
          if (std::abs(frac) > max_abs_frac)
            max_abs_frac = std::abs(frac);
        }
    }
  VERIFY(max_abs_frac < Tp(1.0000000000000006e-11));
}

int main(int, char**)
{
  test001<double>();
  test002<double>();
  test003<double>();
  test004<double>();
  test005<double>();
  test006<double>();
  test007<double>();
  test008<double>();
  test009<double>();
  test010<double>();
  test011<double>();
  return 0;
}
