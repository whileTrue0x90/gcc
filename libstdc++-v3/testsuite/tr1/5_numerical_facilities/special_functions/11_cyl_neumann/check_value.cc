// 2007-02-04  Edward Smith-Rowland <3dw4rd@verizon.net>
//
// Copyright (C) 2007-2014 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING3.  If not see
// <http://www.gnu.org/licenses/>.

//  cyl_neumann


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
testcase_cyl_neumann<double> data001[] = {
  { -0.30851762524903376, 0.0000000000000000, 5.0000000000000000 },
  { 0.055671167283599395, 0.0000000000000000, 10.000000000000000 },
  { 0.20546429603891825, 0.0000000000000000, 15.000000000000000 },
  { 0.062640596809383955, 0.0000000000000000, 20.000000000000000 },
  { -0.12724943226800620, 0.0000000000000000, 25.000000000000000 },
  { -0.11729573168666411, 0.0000000000000000, 30.000000000000000 },
  { 0.045797987195155640, 0.0000000000000000, 35.000000000000000 },
  { 0.12593641705826095, 0.0000000000000000, 40.000000000000000 },
  { 0.027060469763313322, 0.0000000000000000, 45.000000000000000 },
  { -0.098064995470077104, 0.0000000000000000, 50.000000000000000 },
  { -0.077569178730412622, 0.0000000000000000, 55.000000000000000 },
  { 0.047358952209449412, 0.0000000000000000, 60.000000000000000 },
  { 0.097183557740181933, 0.0000000000000000, 65.000000000000000 },
  { 0.0093096664589410131, 0.0000000000000000, 70.000000000000000 },
  { -0.085369047647775642, 0.0000000000000000, 75.000000000000000 },
  { -0.055620339089769981, 0.0000000000000000, 80.000000000000000 },
  { 0.049567884951494258, 0.0000000000000000, 85.000000000000000 },
  { 0.079776475854877765, 0.0000000000000000, 90.000000000000000 },
  { -0.0028230995861232323, 0.0000000000000000, 95.000000000000000 },
  { -0.077244313365083112, 0.0000000000000000, 100.00000000000000 },
};

// Test function for nu=0.0000000000000000.
template <typename Tp>
void test001()
{
  const Tp eps = std::numeric_limits<Tp>::epsilon();
  Tp max_abs_diff = -Tp(1);
  Tp max_abs_frac = -Tp(1);
  unsigned int num_datum = sizeof(data001)
                         / sizeof(testcase_cyl_neumann<double>);
  for (unsigned int i = 0; i < num_datum; ++i)
    {
      const Tp f = std::tr1::cyl_neumann(Tp(data001[i].nu), Tp(data001[i].x));
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
  VERIFY(max_abs_frac < Tp(1.0000000000000006e-10));
}

// Test data for nu=0.33333333333333331.
testcase_cyl_neumann<double> data002[] = {
  { -0.18192321129343830, 0.33333333333333331, 5.0000000000000000 },
  { 0.17020111788268769, 0.33333333333333331, 10.000000000000000 },
  { 0.18540507541540799, 0.33333333333333331, 15.000000000000000 },
  { -0.028777707635715091, 0.33333333333333331, 20.000000000000000 },
  { -0.15829741864944166, 0.33333333333333331, 25.000000000000000 },
  { -0.058645772316705216, 0.33333333333333331, 30.000000000000000 },
  { 0.10294930308870620, 0.33333333333333331, 35.000000000000000 },
  { 0.10547870367098920, 0.33333333333333331, 40.000000000000000 },
  { -0.034334228816010864, 0.33333333333333331, 45.000000000000000 },
  { -0.11283489933031278, 0.33333333333333331, 50.000000000000000 },
  { -0.030007358986895123, 0.33333333333333331, 55.000000000000000 },
  { 0.086699173295718093, 0.33333333333333331, 60.000000000000000 },
  { 0.074875579668878672, 0.33333333333333331, 65.000000000000000 },
  { -0.039323246374552645, 0.33333333333333331, 70.000000000000000 },
  { -0.091263539574475222, 0.33333333333333331, 75.000000000000000 },
  { -0.013358849535984282, 0.33333333333333331, 80.000000000000000 },
  { 0.078373575537830184, 0.33333333333333331, 85.000000000000000 },
  { 0.055812482883955974, 0.33333333333333331, 90.000000000000000 },
  { -0.043310380106990579, 0.33333333333333331, 95.000000000000000 },
  { -0.076900504962136587, 0.33333333333333331, 100.00000000000000 },
};

// Test function for nu=0.33333333333333331.
template <typename Tp>
void test002()
{
  const Tp eps = std::numeric_limits<Tp>::epsilon();
  Tp max_abs_diff = -Tp(1);
  Tp max_abs_frac = -Tp(1);
  unsigned int num_datum = sizeof(data002)
                         / sizeof(testcase_cyl_neumann<double>);
  for (unsigned int i = 0; i < num_datum; ++i)
    {
      const Tp f = std::tr1::cyl_neumann(Tp(data002[i].nu), Tp(data002[i].x));
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
  VERIFY(max_abs_frac < Tp(2.5000000000000014e-11));
}

// Test data for nu=0.50000000000000000.
testcase_cyl_neumann<double> data003[] = {
  { -0.10121770918510843, 0.50000000000000000, 5.0000000000000000 },
  { 0.21170886633139813, 0.50000000000000000, 10.000000000000000 },
  { 0.15650551590730860, 0.50000000000000000, 15.000000000000000 },
  { -0.072806904785061841, 0.50000000000000000, 20.000000000000000 },
  { -0.15817308404205055, 0.50000000000000000, 25.000000000000000 },
  { -0.022470290598831121, 0.50000000000000000, 30.000000000000000 },
  { 0.12187835265849536, 0.50000000000000000, 35.000000000000000 },
  { 0.084138655676395432, 0.50000000000000000, 40.000000000000000 },
  { -0.062482641933003132, 0.50000000000000000, 45.000000000000000 },
  { -0.10888475635053953, 0.50000000000000000, 50.000000000000000 },
  { -0.0023805454010948804, 0.50000000000000000, 55.000000000000000 },
  { 0.098104683735037904, 0.50000000000000000, 60.000000000000000 },
  { 0.055663470218594434, 0.50000000000000000, 65.000000000000000 },
  { -0.060396767883824809, 0.50000000000000000, 70.000000000000000 },
  { -0.084922578922046868, 0.50000000000000000, 75.000000000000000 },
  { 0.0098472271924441215, 0.50000000000000000, 80.000000000000000 },
  { 0.085190643574343639, 0.50000000000000000, 85.000000000000000 },
  { 0.037684970437156261, 0.50000000000000000, 90.000000000000000 },
  { -0.059772904856097479, 0.50000000000000000, 95.000000000000000 },
  { -0.068803091468728053, 0.50000000000000000, 100.00000000000000 },
};

// Test function for nu=0.50000000000000000.
template <typename Tp>
void test003()
{
  const Tp eps = std::numeric_limits<Tp>::epsilon();
  Tp max_abs_diff = -Tp(1);
  Tp max_abs_frac = -Tp(1);
  unsigned int num_datum = sizeof(data003)
                         / sizeof(testcase_cyl_neumann<double>);
  for (unsigned int i = 0; i < num_datum; ++i)
    {
      const Tp f = std::tr1::cyl_neumann(Tp(data003[i].nu), Tp(data003[i].x));
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
  VERIFY(max_abs_frac < Tp(5.0000000000000028e-11));
}

// Test data for nu=0.66666666666666663.
testcase_cyl_neumann<double> data004[] = {
  { -0.016050662643389616, 0.66666666666666663, 5.0000000000000000 },
  { 0.23937232657540730, 0.66666666666666663, 10.000000000000000 },
  { 0.11762106604241242, 0.66666666666666663, 15.000000000000000 },
  { -0.11182254014899563, 0.66666666666666663, 20.000000000000000 },
  { -0.14756582982938804, 0.66666666666666663, 25.000000000000000 },
  { 0.015078692908077665, 0.66666666666666663, 30.000000000000000 },
  { 0.13260911815705798, 0.66666666666666663, 35.000000000000000 },
  { 0.057217565989652795, 0.66666666666666663, 40.000000000000000 },
  { -0.086373755152382048, 0.66666666666666663, 45.000000000000000 },
  { -0.097624139208051630, 0.66666666666666663, 50.000000000000000 },
  { 0.025354902147023434, 0.66666666666666663, 55.000000000000000 },
  { 0.10288136476351209, 0.66666666666666663, 60.000000000000000 },
  { 0.032728379560128203, 0.66666666666666663, 65.000000000000000 },
  { -0.077363672735747777, 0.66666666666666663, 70.000000000000000 },
  { -0.072855870458293975, 0.66666666666666663, 75.000000000000000 },
  { 0.032358106046953494, 0.66666666666666663, 80.000000000000000 },
  { 0.086240651537394228, 0.66666666666666663, 85.000000000000000 },
  { 0.017029601697285159, 0.66666666666666663, 90.000000000000000 },
  { -0.072173520560584709, 0.66666666666666663, 95.000000000000000 },
  { -0.056057339204073985, 0.66666666666666663, 100.00000000000000 },
};

// Test function for nu=0.66666666666666663.
template <typename Tp>
void test004()
{
  const Tp eps = std::numeric_limits<Tp>::epsilon();
  Tp max_abs_diff = -Tp(1);
  Tp max_abs_frac = -Tp(1);
  unsigned int num_datum = sizeof(data004)
                         / sizeof(testcase_cyl_neumann<double>);
  for (unsigned int i = 0; i < num_datum; ++i)
    {
      const Tp f = std::tr1::cyl_neumann(Tp(data004[i].nu), Tp(data004[i].x));
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
  VERIFY(max_abs_frac < Tp(2.5000000000000014e-11));
}

// Test data for nu=1.0000000000000000.
testcase_cyl_neumann<double> data005[] = {
  { 0.14786314339122689, 1.0000000000000000, 5.0000000000000000 },
  { 0.24901542420695388, 1.0000000000000000, 10.000000000000000 },
  { 0.021073628036873546, 1.0000000000000000, 15.000000000000000 },
  { -0.16551161436252118, 1.0000000000000000, 20.000000000000000 },
  { -0.098829964783237384, 1.0000000000000000, 25.000000000000000 },
  { 0.084425570661747149, 1.0000000000000000, 30.000000000000000 },
  { 0.12751273354559012, 1.0000000000000000, 35.000000000000000 },
  { -0.0057935058215496330, 1.0000000000000000, 40.000000000000000 },
  { -0.11552517964639945, 1.0000000000000000, 45.000000000000000 },
  { -0.056795668562014713, 1.0000000000000000, 50.000000000000000 },
  { 0.073846265432577940, 1.0000000000000000, 55.000000000000000 },
  { 0.091869609369866906, 1.0000000000000000, 60.000000000000000 },
  { -0.017940374275377303, 1.0000000000000000, 65.000000000000000 },
  { -0.094844652625716244, 1.0000000000000000, 70.000000000000000 },
  { -0.035213785160580456, 1.0000000000000000, 75.000000000000000 },
  { 0.069395913784588051, 1.0000000000000000, 80.000000000000000 },
  { 0.071233187582749782, 1.0000000000000000, 85.000000000000000 },
  { -0.026187238607768282, 1.0000000000000000, 90.000000000000000 },
  { -0.081827958724501229, 1.0000000000000000, 95.000000000000000 },
  { -0.020372312002759942, 1.0000000000000000, 100.00000000000000 },
};

// Test function for nu=1.0000000000000000.
template <typename Tp>
void test005()
{
  const Tp eps = std::numeric_limits<Tp>::epsilon();
  Tp max_abs_diff = -Tp(1);
  Tp max_abs_frac = -Tp(1);
  unsigned int num_datum = sizeof(data005)
                         / sizeof(testcase_cyl_neumann<double>);
  for (unsigned int i = 0; i < num_datum; ++i)
    {
      const Tp f = std::tr1::cyl_neumann(Tp(data005[i].nu), Tp(data005[i].x));
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
  VERIFY(max_abs_frac < Tp(2.5000000000000014e-11));
}

// Test data for nu=2.0000000000000000.
testcase_cyl_neumann<double> data006[] = {
  { 0.36766288260552449, 2.0000000000000000, 5.0000000000000000 },
  { -0.0058680824422086154, 2.0000000000000000, 10.000000000000000 },
  { -0.20265447896733510, 2.0000000000000000, 15.000000000000000 },
  { -0.079191758245636068, 2.0000000000000000, 20.000000000000000 },
  { 0.11934303508534720, 2.0000000000000000, 25.000000000000000 },
  { 0.12292410306411393, 2.0000000000000000, 30.000000000000000 },
  { -0.038511545278264774, 2.0000000000000000, 35.000000000000000 },
  { -0.12622609234933843, 2.0000000000000000, 40.000000000000000 },
  { -0.032194922192042189, 2.0000000000000000, 45.000000000000000 },
  { 0.095793168727596509, 2.0000000000000000, 50.000000000000000 },
  { 0.080254497473415454, 2.0000000000000000, 55.000000000000000 },
  { -0.044296631897120513, 2.0000000000000000, 60.000000000000000 },
  { -0.097735569256347382, 2.0000000000000000, 65.000000000000000 },
  { -0.012019513676818619, 2.0000000000000000, 70.000000000000000 },
  { 0.084430013376826832, 2.0000000000000000, 75.000000000000000 },
  { 0.057355236934384685, 2.0000000000000000, 80.000000000000000 },
  { -0.047891809949547205, 2.0000000000000000, 85.000000000000000 },
  { -0.080358414490605948, 2.0000000000000000, 90.000000000000000 },
  { 0.0011004057182389959, 2.0000000000000000, 95.000000000000000 },
  { 0.076836867125027908, 2.0000000000000000, 100.00000000000000 },
};

// Test function for nu=2.0000000000000000.
template <typename Tp>
void test006()
{
  const Tp eps = std::numeric_limits<Tp>::epsilon();
  Tp max_abs_diff = -Tp(1);
  Tp max_abs_frac = -Tp(1);
  unsigned int num_datum = sizeof(data006)
                         / sizeof(testcase_cyl_neumann<double>);
  for (unsigned int i = 0; i < num_datum; ++i)
    {
      const Tp f = std::tr1::cyl_neumann(Tp(data006[i].nu), Tp(data006[i].x));
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
  VERIFY(max_abs_frac < Tp(2.5000000000000017e-10));
}

// Test data for nu=5.0000000000000000.
testcase_cyl_neumann<double> data007[] = {
  { -0.45369482249110188, 5.0000000000000000, 5.0000000000000000 },
  { 0.13540304768936232, 5.0000000000000000, 10.000000000000000 },
  { 0.16717271575940021, 5.0000000000000000, 15.000000000000000 },
  { -0.10003576788953225, 5.0000000000000000, 20.000000000000000 },
  { -0.14705799311372267, 5.0000000000000000, 25.000000000000000 },
  { 0.031627359289264322, 5.0000000000000000, 30.000000000000000 },
  { 0.13554781474770031, 5.0000000000000000, 35.000000000000000 },
  { 0.031869448780850372, 5.0000000000000000, 40.000000000000000 },
  { -0.10426932700176872, 5.0000000000000000, 45.000000000000000 },
  { -0.078548413913081608, 5.0000000000000000, 50.000000000000000 },
  { 0.055257033062858382, 5.0000000000000000, 55.000000000000000 },
  { 0.099464632840450901, 5.0000000000000000, 60.000000000000000 },
  { 0.00023860469499600970, 5.0000000000000000, 65.000000000000000 },
  { -0.091861802216406066, 5.0000000000000000, 70.000000000000000 },
  { -0.048383671296970077, 5.0000000000000000, 75.000000000000000 },
  { 0.060293667104896330, 5.0000000000000000, 80.000000000000000 },
  { 0.077506166682734010, 5.0000000000000000, 85.000000000000000 },
  { -0.015338764062239803, 5.0000000000000000, 90.000000000000000 },
  { -0.081531504045514375, 5.0000000000000000, 95.000000000000000 },
  { -0.029480196281662041, 5.0000000000000000, 100.00000000000000 },
};

// Test function for nu=5.0000000000000000.
template <typename Tp>
void test007()
{
  const Tp eps = std::numeric_limits<Tp>::epsilon();
  Tp max_abs_diff = -Tp(1);
  Tp max_abs_frac = -Tp(1);
  unsigned int num_datum = sizeof(data007)
                         / sizeof(testcase_cyl_neumann<double>);
  for (unsigned int i = 0; i < num_datum; ++i)
    {
      const Tp f = std::tr1::cyl_neumann(Tp(data007[i].nu), Tp(data007[i].x));
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
  VERIFY(max_abs_frac < Tp(2.5000000000000013e-09));
}

// Test data for nu=10.000000000000000.
testcase_cyl_neumann<double> data008[] = {
  { -25.129110095610098, 10.000000000000000, 5.0000000000000000 },
  { -0.35981415218340279, 10.000000000000000, 10.000000000000000 },
  { 0.21997141360195582, 10.000000000000000, 15.000000000000000 },
  { -0.043894653515658202, 10.000000000000000, 20.000000000000000 },
  { -0.14871839049980651, 10.000000000000000, 25.000000000000000 },
  { 0.075056702122397012, 10.000000000000000, 30.000000000000000 },
  { 0.12222473135000553, 10.000000000000000, 35.000000000000000 },
  { -0.046723877232677867, 10.000000000000000, 40.000000000000000 },
  { -0.11739339009322178, 10.000000000000000, 45.000000000000000 },
  { 0.0057238971820535740, 10.000000000000000, 50.000000000000000 },
  { 0.10733910125831635, 10.000000000000000, 55.000000000000000 },
  { 0.036290350559545506, 10.000000000000000, 60.000000000000000 },
  { -0.083239127691715639, 10.000000000000000, 65.000000000000000 },
  { -0.069639384138314872, 10.000000000000000, 70.000000000000000 },
  { 0.045798335061325038, 10.000000000000000, 75.000000000000000 },
  { 0.086269195064844428, 10.000000000000000, 80.000000000000000 },
  { -0.0018234674126248629, 10.000000000000000, 85.000000000000000 },
  { -0.082067762371231298, 10.000000000000000, 90.000000000000000 },
  { -0.038798074754578075, 10.000000000000000, 95.000000000000000 },
  { 0.058331574236414815, 10.000000000000000, 100.00000000000000 },
};

// Test function for nu=10.000000000000000.
template <typename Tp>
void test008()
{
  const Tp eps = std::numeric_limits<Tp>::epsilon();
  Tp max_abs_diff = -Tp(1);
  Tp max_abs_frac = -Tp(1);
  unsigned int num_datum = sizeof(data008)
                         / sizeof(testcase_cyl_neumann<double>);
  for (unsigned int i = 0; i < num_datum; ++i)
    {
      const Tp f = std::tr1::cyl_neumann(Tp(data008[i].nu), Tp(data008[i].x));
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
  VERIFY(max_abs_frac < Tp(2.5000000000000017e-10));
}

// Test data for nu=20.000000000000000.
testcase_cyl_neumann<double> data009[] = {
  { -593396529.69143212, 20.000000000000000, 5.0000000000000000 },
  { -1597.4838482696264, 20.000000000000000, 10.000000000000000 },
  { -3.3087330924737630, 20.000000000000000, 15.000000000000000 },
  { -0.28548945860020319, 20.000000000000000, 20.000000000000000 },
  { 0.19804074776289243, 20.000000000000000, 25.000000000000000 },
  { -0.16848153948742675, 20.000000000000000, 30.000000000000000 },
  { 0.10102784152594022, 20.000000000000000, 35.000000000000000 },
  { 0.045161820565805928, 20.000000000000000, 40.000000000000000 },
  { -0.12556489308015448, 20.000000000000000, 45.000000000000000 },
  { 0.016442633948115841, 20.000000000000000, 50.000000000000000 },
  { 0.10853448778255187, 20.000000000000000, 55.000000000000000 },
  { -0.026721408520664677, 20.000000000000000, 60.000000000000000 },
  { -0.098780425256324203, 20.000000000000000, 65.000000000000000 },
  { 0.016201957786018205, 20.000000000000000, 70.000000000000000 },
  { 0.093591198265063735, 20.000000000000000, 75.000000000000000 },
  { 0.0040484400737295740, 20.000000000000000, 80.000000000000000 },
  { -0.086314929459920503, 20.000000000000000, 85.000000000000000 },
  { -0.028274110097231495, 20.000000000000000, 90.000000000000000 },
  { 0.072349520791638755, 20.000000000000000, 95.000000000000000 },
  { 0.051247973076188565, 20.000000000000000, 100.00000000000000 },
};

// Test function for nu=20.000000000000000.
template <typename Tp>
void test009()
{
  const Tp eps = std::numeric_limits<Tp>::epsilon();
  Tp max_abs_diff = -Tp(1);
  Tp max_abs_frac = -Tp(1);
  unsigned int num_datum = sizeof(data009)
                         / sizeof(testcase_cyl_neumann<double>);
  for (unsigned int i = 0; i < num_datum; ++i)
    {
      const Tp f = std::tr1::cyl_neumann(Tp(data009[i].nu), Tp(data009[i].x));
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
testcase_cyl_neumann<double> data010[] = {
  { -2.7888370175838943e+42, 50.000000000000000, 5.0000000000000000 },
  { -3.6410665018007421e+27, 50.000000000000000, 10.000000000000000 },
  { -1.0929732912175405e+19, 50.000000000000000, 15.000000000000000 },
  { -15606426801663.732, 50.000000000000000, 20.000000000000000 },
  { -753573251.44662631, 50.000000000000000, 25.000000000000000 },
  { -386759.32602734747, 50.000000000000000, 30.000000000000000 },
  { -1172.8690492895341, 50.000000000000000, 35.000000000000000 },
  { -15.615608873419953, 50.000000000000000, 40.000000000000000 },
  { -0.87058346204176951, 50.000000000000000, 45.000000000000000 },
  { -0.21031655464397736, 50.000000000000000, 50.000000000000000 },
  { 0.093048240412999375, 50.000000000000000, 55.000000000000000 },
  { 0.0086417699626745066, 50.000000000000000, 60.000000000000000 },
  { -0.025019788459221974, 50.000000000000000, 65.000000000000000 },
  { -0.0014815155191908913, 50.000000000000000, 70.000000000000000 },
  { 0.050335774732164155, 50.000000000000000, 75.000000000000000 },
  { -0.092924250967987204, 50.000000000000000, 80.000000000000000 },
  { 0.087332463030205670, 50.000000000000000, 85.000000000000000 },
  { -0.016164237701651891, 50.000000000000000, 90.000000000000000 },
  { -0.068897613820457920, 50.000000000000000, 95.000000000000000 },
  { 0.076505263944802962, 50.000000000000000, 100.00000000000000 },
};

// Test function for nu=50.000000000000000.
template <typename Tp>
void test010()
{
  const Tp eps = std::numeric_limits<Tp>::epsilon();
  Tp max_abs_diff = -Tp(1);
  Tp max_abs_frac = -Tp(1);
  unsigned int num_datum = sizeof(data010)
                         / sizeof(testcase_cyl_neumann<double>);
  for (unsigned int i = 0; i < num_datum; ++i)
    {
      const Tp f = std::tr1::cyl_neumann(Tp(data010[i].nu), Tp(data010[i].x));
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
  VERIFY(max_abs_frac < Tp(2.5000000000000017e-10));
}

// Test data for nu=100.00000000000000.
testcase_cyl_neumann<double> data011[] = {
  { -5.0848639160196196e+115, 100.00000000000000, 5.0000000000000000 },
  { -4.8491482711800245e+85, 100.00000000000000, 10.000000000000000 },
  { -1.6375955323195320e+68, 100.00000000000000, 15.000000000000000 },
  { -8.2002648144679137e+55, 100.00000000000000, 20.000000000000000 },
  { -2.9712216432562373e+46, 100.00000000000000, 25.000000000000000 },
  { -7.2875284708240766e+38, 100.00000000000000, 30.000000000000000 },
  { -3.4251079902108953e+32, 100.00000000000000, 35.000000000000000 },
  { -1.4552439438101799e+27, 100.00000000000000, 40.000000000000000 },
  { -3.4506612476220073e+22, 100.00000000000000, 45.000000000000000 },
  { -3.2938001882025948e+18, 100.00000000000000, 50.000000000000000 },
  { -1005686182055527.4, 100.00000000000000, 55.000000000000000 },
  { -831892881402.11377, 100.00000000000000, 60.000000000000000 },
  { -1650863778.0598330, 100.00000000000000, 65.000000000000000 },
  { -7192614.1976097804, 100.00000000000000, 70.000000000000000 },
  { -64639.072261231602, 100.00000000000000, 75.000000000000000 },
  { -1152.5905185698464, 100.00000000000000, 80.000000000000000 },
  { -40.250761402102000, 100.00000000000000, 85.000000000000000 },
  { -2.8307771387185459, 100.00000000000000, 90.000000000000000 },
  { -0.45762200495904559, 100.00000000000000, 95.000000000000000 },
  { -0.16692141141757649, 100.00000000000000, 100.00000000000000 },
};

// Test function for nu=100.00000000000000.
template <typename Tp>
void test011()
{
  const Tp eps = std::numeric_limits<Tp>::epsilon();
  Tp max_abs_diff = -Tp(1);
  Tp max_abs_frac = -Tp(1);
  unsigned int num_datum = sizeof(data011)
                         / sizeof(testcase_cyl_neumann<double>);
  for (unsigned int i = 0; i < num_datum; ++i)
    {
      const Tp f = std::tr1::cyl_neumann(Tp(data011[i].nu), Tp(data011[i].x));
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
