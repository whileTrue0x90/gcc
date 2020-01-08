// { dg-do run { target c++11 } }
// { dg-options "-D__STDCPP_WANT_MATH_SPEC_FUNCS__" }
//
// Copyright (C) 2016-2020 Free Software Foundation, Inc.
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

//  ellint_1
//  Compare against values generated by the GNU Scientific Library.
//  The GSL can be found on the web: http://www.gnu.org/software/gsl/
#include <limits>
#include <cmath>
#if defined(__TEST_DEBUG)
#  include <iostream>
#  define VERIFY(A) \
  if (!(A)) \
    { \
      std::cout << "line " << __LINE__ \
	<< "  max_abs_frac = " << max_abs_frac \
	<< std::endl; \
    }
#else
#  include <testsuite_hooks.h>
#endif
#include <specfun_testcase.h>

// Test data for k=-0.90000000000000002.
// max(|f - f_Boost|): 8.8817841970012523e-16 at index 7
// max(|f - f_Boost| / |f_Boost|): 5.7842011620951154e-16
// mean(f - f_Boost): 5.8286708792820721e-17
// variance(f - f_Boost): 4.1942474344433133e-34
// stddev(f - f_Boost): 2.0479861900030756e-17
const testcase_ellint_1<double>
data001[10] =
{
  { 0.0000000000000000, -0.90000000000000002, 0.0000000000000000, 0.0 },
  { 0.17525427376115027, -0.90000000000000002, 0.17453292519943295, 0.0 },
  { 0.35492464591297446, -0.90000000000000002, 0.34906585039886590, 0.0 },
  { 0.54388221416157123, -0.90000000000000002, 0.52359877559829882, 0.0 },
  { 0.74797400423532512, -0.90000000000000002, 0.69813170079773179, 0.0 },
  { 0.97463898451966446, -0.90000000000000002, 0.87266462599716477, 0.0 },
  { 1.2334463254523438, -0.90000000000000002, 1.0471975511965976, 0.0 },
  { 1.5355247765594913, -0.90000000000000002, 1.2217304763960306, 0.0 },
  { 1.8882928567775126, -0.90000000000000002, 1.3962634015954636, 0.0 },
  { 2.2805491384227703, -0.90000000000000002, 1.5707963267948966, 0.0 },
};
const double toler001 = 2.5000000000000020e-13;

// Test data for k=-0.80000000000000004.
// max(|f - f_Boost|): 8.8817841970012523e-16 at index 8
// max(|f - f_Boost| / |f_Boost|): 5.2032670495747184e-16
// mean(f - f_Boost): 1.9428902930940239e-16
// variance(f - f_Boost): 2.7486111305082033e-32
// stddev(f - f_Boost): 1.6578935823834422e-16
const testcase_ellint_1<double>
data002[10] =
{
  { 0.0000000000000000, -0.80000000000000004, 0.0000000000000000, 0.0 },
  { 0.17510154241338899, -0.80000000000000004, 0.17453292519943295, 0.0 },
  { 0.35365068839779396, -0.80000000000000004, 0.34906585039886590, 0.0 },
  { 0.53926804409084550, -0.80000000000000004, 0.52359877559829882, 0.0 },
  { 0.73587926028070372, -0.80000000000000004, 0.69813170079773179, 0.0 },
  { 0.94770942970071170, -0.80000000000000004, 0.87266462599716477, 0.0 },
  { 1.1789022995388236, -0.80000000000000004, 1.0471975511965976, 0.0 },
  { 1.4323027881876012, -0.80000000000000004, 1.2217304763960306, 0.0 },
  { 1.7069629739121677, -0.80000000000000004, 1.3962634015954636, 0.0 },
  { 1.9953027776647294, -0.80000000000000004, 1.5707963267948966, 0.0 },
};
const double toler002 = 2.5000000000000020e-13;

// Test data for k=-0.69999999999999996.
// max(|f - f_Boost|): 8.8817841970012523e-16 at index 8
// max(|f - f_Boost| / |f_Boost|): 5.5425633303580569e-16
// mean(f - f_Boost): 3.3306690738754695e-17
// variance(f - f_Boost): 2.8136647641852830e-32
// stddev(f - f_Boost): 1.6773982127644239e-16
const testcase_ellint_1<double>
data003[10] =
{
  { 0.0000000000000000, -0.69999999999999996, 0.0000000000000000, 0.0 },
  { 0.17496737466916723, -0.69999999999999996, 0.17453292519943295, 0.0 },
  { 0.35254687535677931, -0.69999999999999996, 0.34906585039886590, 0.0 },
  { 0.53536740275997130, -0.69999999999999996, 0.52359877559829882, 0.0 },
  { 0.72603797651684454, -0.69999999999999996, 0.69813170079773179, 0.0 },
  { 0.92698296348313447, -0.69999999999999996, 0.87266462599716477, 0.0 },
  { 1.1400447527693316, -0.69999999999999996, 1.0471975511965976, 0.0 },
  { 1.3657668117194071, -0.69999999999999996, 1.2217304763960306, 0.0 },
  { 1.6024686895959162, -0.69999999999999996, 1.3962634015954636, 0.0 },
  { 1.8456939983747234, -0.69999999999999996, 1.5707963267948966, 0.0 },
};
const double toler003 = 2.5000000000000020e-13;

// Test data for k=-0.59999999999999998.
// max(|f - f_Boost|): 4.4408920985006262e-16 at index 7
// max(|f - f_Boost| / |f_Boost|): 3.3664899092028927e-16
// mean(f - f_Boost): 5.2735593669694933e-17
// variance(f - f_Boost): 3.4333862218458872e-34
// stddev(f - f_Boost): 1.8529398861932589e-17
const testcase_ellint_1<double>
data004[10] =
{
  { 0.0000000000000000, -0.59999999999999998, 0.0000000000000000, 0.0 },
  { 0.17485154362988359, -0.59999999999999998, 0.17453292519943295, 0.0 },
  { 0.35160509865544320, -0.59999999999999998, 0.34906585039886590, 0.0 },
  { 0.53210652578446138, -0.59999999999999998, 0.52359877559829882, 0.0 },
  { 0.71805304664485659, -0.59999999999999998, 0.69813170079773179, 0.0 },
  { 0.91082759030195981, -0.59999999999999998, 0.87266462599716477, 0.0 },
  { 1.1112333229323361, -0.59999999999999998, 1.0471975511965976, 0.0 },
  { 1.3191461190365270, -0.59999999999999998, 1.2217304763960306, 0.0 },
  { 1.5332022105084779, -0.59999999999999998, 1.3962634015954636, 0.0 },
  { 1.7507538029157523, -0.59999999999999998, 1.5707963267948966, 0.0 },
};
const double toler004 = 2.5000000000000020e-13;

// Test data for k=-0.50000000000000000.
// max(|f - f_Boost|): 4.4408920985006262e-16 at index 7
// max(|f - f_Boost| / |f_Boost|): 3.4551389361831220e-16
// mean(f - f_Boost): -5.8286708792820721e-17
// variance(f - f_Boost): 4.1942474344433133e-34
// stddev(f - f_Boost): 2.0479861900030756e-17
const testcase_ellint_1<double>
data005[10] =
{
  { 0.0000000000000000, -0.50000000000000000, 0.0000000000000000, 0.0 },
  { 0.17475385514035785, -0.50000000000000000, 0.17453292519943295, 0.0 },
  { 0.35081868470101579, -0.50000000000000000, 0.34906585039886590, 0.0 },
  { 0.52942862705190574, -0.50000000000000000, 0.52359877559829882, 0.0 },
  { 0.71164727562630326, -0.50000000000000000, 0.69813170079773179, 0.0 },
  { 0.89824523594227768, -0.50000000000000000, 0.87266462599716477, 0.0 },
  { 1.0895506700518853, -0.50000000000000000, 1.0471975511965976, 0.0 },
  { 1.2853005857432933, -0.50000000000000000, 1.2217304763960306, 0.0 },
  { 1.4845545520549488, -0.50000000000000000, 1.3962634015954636, 0.0 },
  { 1.6857503548125961, -0.50000000000000000, 1.5707963267948966, 0.0 },
};
const double toler005 = 2.5000000000000020e-13;

// Test data for k=-0.39999999999999991.
// max(|f - f_Boost|): 4.4408920985006262e-16 at index 9
// max(|f - f_Boost| / |f_Boost|): 3.1423314994346225e-16
// mean(f - f_Boost): -6.9388939039072284e-17
// variance(f - f_Boost): 1.7333369499485123e-32
// stddev(f - f_Boost): 1.3165625507162629e-16
const testcase_ellint_1<double>
data006[10] =
{
  { 0.0000000000000000, -0.39999999999999991, 0.0000000000000000, 0.0 },
  { 0.17467414669441528, -0.39999999999999991, 0.17453292519943295, 0.0 },
  { 0.35018222772483443, -0.39999999999999991, 0.34906585039886590, 0.0 },
  { 0.52729015917508748, -0.39999999999999991, 0.52359877559829882, 0.0 },
  { 0.70662374407341244, -0.39999999999999991, 0.69813170079773179, 0.0 },
  { 0.88859210497602159, -0.39999999999999991, 0.87266462599716477, 0.0 },
  { 1.0733136290471381, -0.39999999999999991, 1.0471975511965976, 0.0 },
  { 1.2605612170157066, -0.39999999999999991, 1.2217304763960306, 0.0 },
  { 1.4497513956433439, -0.39999999999999991, 1.3962634015954636, 0.0 },
  { 1.6399998658645112, -0.39999999999999991, 1.5707963267948966, 0.0 },
};
const double toler006 = 2.5000000000000020e-13;

// Test data for k=-0.29999999999999993.
// max(|f - f_Boost|): 6.6613381477509392e-16 at index 9
// max(|f - f_Boost| / |f_Boost|): 4.2241249691539529e-16
// mean(f - f_Boost): -8.3266726846886741e-17
// variance(f - f_Boost): 4.1942474344433135e-32
// stddev(f - f_Boost): 2.0479861900030756e-16
const testcase_ellint_1<double>
data007[10] =
{
  { 0.0000000000000000, -0.29999999999999993, 0.0000000000000000, 0.0 },
  { 0.17461228653000099, -0.29999999999999993, 0.17453292519943295, 0.0 },
  { 0.34969146102798421, -0.29999999999999993, 0.34906585039886590, 0.0 },
  { 0.52565822873726309, -0.29999999999999993, 0.52359877559829882, 0.0 },
  { 0.70284226512408543, -0.29999999999999993, 0.69813170079773179, 0.0 },
  { 0.88144139195111171, -0.29999999999999993, 0.87266462599716477, 0.0 },
  { 1.0614897067260520, -0.29999999999999993, 1.0471975511965976, 0.0 },
  { 1.2428416824174220, -0.29999999999999993, 1.2217304763960306, 0.0 },
  { 1.4251795877015929, -0.29999999999999993, 1.3962634015954636, 0.0 },
  { 1.6080486199305126, -0.29999999999999993, 1.5707963267948966, 0.0 },
};
const double toler007 = 2.5000000000000020e-13;

// Test data for k=-0.19999999999999996.
// max(|f - f_Boost|): 4.4408920985006262e-16 at index 6
// max(|f - f_Boost| / |f_Boost|): 4.2156475739151676e-16
// mean(f - f_Boost): -9.7144514654701197e-17
// variance(f - f_Boost): 1.1650687317898094e-33
// stddev(f - f_Boost): 3.4133103166717924e-17
const testcase_ellint_1<double>
data008[10] =
{
  { 0.0000000000000000, -0.19999999999999996, 0.0000000000000000, 0.0 },
  { 0.17456817290292806, -0.19999999999999996, 0.17453292519943295, 0.0 },
  { 0.34934315932086796, -0.19999999999999996, 0.34906585039886590, 0.0 },
  { 0.52450880529443988, -0.19999999999999996, 0.52359877559829882, 0.0 },
  { 0.70020491009844887, -0.19999999999999996, 0.69813170079773179, 0.0 },
  { 0.87651006649967977, -0.19999999999999996, 0.87266462599716477, 0.0 },
  { 1.0534305870298994, -0.19999999999999996, 1.0471975511965976, 0.0 },
  { 1.2308975521670789, -0.19999999999999996, 1.2217304763960306, 0.0 },
  { 1.4087733584990738, -0.19999999999999996, 1.3962634015954636, 0.0 },
  { 1.5868678474541662, -0.19999999999999996, 1.5707963267948966, 0.0 },
};
const double toler008 = 2.5000000000000020e-13;

// Test data for k=-0.099999999999999978.
// max(|f - f_Boost|): 4.4408920985006262e-16 at index 8
// max(|f - f_Boost| / |f_Boost|): 3.1735566504509645e-16
// mean(f - f_Boost): -3.6082248300317589e-17
// variance(f - f_Boost): 8.2258607846939269e-33
// stddev(f - f_Boost): 9.0696531271564778e-17
const testcase_ellint_1<double>
data009[10] =
{
  { 0.0000000000000000, -0.099999999999999978, 0.0000000000000000, 0.0 },
  { 0.17454173353063662, -0.099999999999999978, 0.17453292519943295, 0.0 },
  { 0.34913506721468096, -0.099999999999999978, 0.34906585039886590, 0.0 },
  { 0.52382550016538953, -0.099999999999999978, 0.52359877559829882, 0.0 },
  { 0.69864700854177020, -0.099999999999999978, 0.69813170079773179, 0.0 },
  { 0.87361792586964859, -0.099999999999999978, 0.87266462599716477, 0.0 },
  { 1.0487386319621685, -0.099999999999999978, 1.0471975511965976, 0.0 },
  { 1.2239913752078759, -0.099999999999999978, 1.2217304763960306, 0.0 },
  { 1.3993423113684051, -0.099999999999999978, 1.3962634015954636, 0.0 },
  { 1.5747455615173558, -0.099999999999999978, 1.5707963267948966, 0.0 },
};
const double toler009 = 2.5000000000000020e-13;

// Test data for k=0.0000000000000000.
// max(|f - f_Boost|): 2.2204460492503131e-16 at index 7
// max(|f - f_Boost| / |f_Boost|): 2.1203697876423447e-16
// mean(f - f_Boost): -1.9428902930940238e-17
// variance(f - f_Boost): 4.6602749271592373e-35
// stddev(f - f_Boost): 6.8266206333435850e-18
const testcase_ellint_1<double>
data010[10] =
{
  { 0.0000000000000000, 0.0000000000000000, 0.0000000000000000, 0.0 },
  { 0.17453292519943295, 0.0000000000000000, 0.17453292519943295, 0.0 },
  { 0.34906585039886590, 0.0000000000000000, 0.34906585039886590, 0.0 },
  { 0.52359877559829882, 0.0000000000000000, 0.52359877559829882, 0.0 },
  { 0.69813170079773179, 0.0000000000000000, 0.69813170079773179, 0.0 },
  { 0.87266462599716477, 0.0000000000000000, 0.87266462599716477, 0.0 },
  { 1.0471975511965976, 0.0000000000000000, 1.0471975511965976, 0.0 },
  { 1.2217304763960306, 0.0000000000000000, 1.2217304763960306, 0.0 },
  { 1.3962634015954636, 0.0000000000000000, 1.3962634015954636, 0.0 },
  { 1.5707963267948966, 0.0000000000000000, 1.5707963267948966, 0.0 },
};
const double toler010 = 2.5000000000000020e-13;

// Test data for k=0.10000000000000009.
// max(|f - f_Boost|): 4.4408920985006262e-16 at index 8
// max(|f - f_Boost| / |f_Boost|): 3.1735566504509645e-16
// mean(f - f_Boost): -5.8286708792820721e-17
// variance(f - f_Boost): 4.1942474344433133e-34
// stddev(f - f_Boost): 2.0479861900030756e-17
const testcase_ellint_1<double>
data011[10] =
{
  { 0.0000000000000000, 0.10000000000000009, 0.0000000000000000, 0.0 },
  { 0.17454173353063662, 0.10000000000000009, 0.17453292519943295, 0.0 },
  { 0.34913506721468096, 0.10000000000000009, 0.34906585039886590, 0.0 },
  { 0.52382550016538953, 0.10000000000000009, 0.52359877559829882, 0.0 },
  { 0.69864700854177020, 0.10000000000000009, 0.69813170079773179, 0.0 },
  { 0.87361792586964859, 0.10000000000000009, 0.87266462599716477, 0.0 },
  { 1.0487386319621685, 0.10000000000000009, 1.0471975511965976, 0.0 },
  { 1.2239913752078759, 0.10000000000000009, 1.2217304763960306, 0.0 },
  { 1.3993423113684051, 0.10000000000000009, 1.3962634015954636, 0.0 },
  { 1.5747455615173560, 0.10000000000000009, 1.5707963267948966, 0.0 },
};
const double toler011 = 2.5000000000000020e-13;

// Test data for k=0.20000000000000018.
// max(|f - f_Boost|): 4.4408920985006262e-16 at index 6
// max(|f - f_Boost| / |f_Boost|): 4.2156475739151676e-16
// mean(f - f_Boost): -9.7144514654701197e-17
// variance(f - f_Boost): 1.1650687317898094e-33
// stddev(f - f_Boost): 3.4133103166717924e-17
const testcase_ellint_1<double>
data012[10] =
{
  { 0.0000000000000000, 0.20000000000000018, 0.0000000000000000, 0.0 },
  { 0.17456817290292806, 0.20000000000000018, 0.17453292519943295, 0.0 },
  { 0.34934315932086796, 0.20000000000000018, 0.34906585039886590, 0.0 },
  { 0.52450880529443988, 0.20000000000000018, 0.52359877559829882, 0.0 },
  { 0.70020491009844887, 0.20000000000000018, 0.69813170079773179, 0.0 },
  { 0.87651006649967977, 0.20000000000000018, 0.87266462599716477, 0.0 },
  { 1.0534305870298994, 0.20000000000000018, 1.0471975511965976, 0.0 },
  { 1.2308975521670789, 0.20000000000000018, 1.2217304763960306, 0.0 },
  { 1.4087733584990738, 0.20000000000000018, 1.3962634015954636, 0.0 },
  { 1.5868678474541662, 0.20000000000000018, 1.5707963267948966, 0.0 },
};
const double toler012 = 2.5000000000000020e-13;

// Test data for k=0.30000000000000004.
// max(|f - f_Boost|): 8.8817841970012523e-16 at index 9
// max(|f - f_Boost| / |f_Boost|): 5.5233306300061082e-16
// mean(f - f_Boost): -1.0547118733938987e-16
// variance(f - f_Boost): 7.5633408838247182e-32
// stddev(f - f_Boost): 2.7501528837184157e-16
const testcase_ellint_1<double>
data013[10] =
{
  { 0.0000000000000000, 0.30000000000000004, 0.0000000000000000, 0.0 },
  { 0.17461228653000099, 0.30000000000000004, 0.17453292519943295, 0.0 },
  { 0.34969146102798421, 0.30000000000000004, 0.34906585039886590, 0.0 },
  { 0.52565822873726309, 0.30000000000000004, 0.52359877559829882, 0.0 },
  { 0.70284226512408543, 0.30000000000000004, 0.69813170079773179, 0.0 },
  { 0.88144139195111171, 0.30000000000000004, 0.87266462599716477, 0.0 },
  { 1.0614897067260520, 0.30000000000000004, 1.0471975511965976, 0.0 },
  { 1.2428416824174220, 0.30000000000000004, 1.2217304763960306, 0.0 },
  { 1.4251795877015929, 0.30000000000000004, 1.3962634015954636, 0.0 },
  { 1.6080486199305128, 0.30000000000000004, 1.5707963267948966, 0.0 },
};
const double toler013 = 2.5000000000000020e-13;

// Test data for k=0.40000000000000013.
// max(|f - f_Boost|): 4.4408920985006262e-16 at index 9
// max(|f - f_Boost| / |f_Boost|): 3.1423314994346225e-16
// mean(f - f_Boost): -4.7184478546569152e-17
// variance(f - f_Boost): 1.9448563670505968e-32
// stddev(f - f_Boost): 1.3945810722401896e-16
const testcase_ellint_1<double>
data014[10] =
{
  { 0.0000000000000000, 0.40000000000000013, 0.0000000000000000, 0.0 },
  { 0.17467414669441528, 0.40000000000000013, 0.17453292519943295, 0.0 },
  { 0.35018222772483443, 0.40000000000000013, 0.34906585039886590, 0.0 },
  { 0.52729015917508748, 0.40000000000000013, 0.52359877559829882, 0.0 },
  { 0.70662374407341244, 0.40000000000000013, 0.69813170079773179, 0.0 },
  { 0.88859210497602159, 0.40000000000000013, 0.87266462599716477, 0.0 },
  { 1.0733136290471381, 0.40000000000000013, 1.0471975511965976, 0.0 },
  { 1.2605612170157066, 0.40000000000000013, 1.2217304763960306, 0.0 },
  { 1.4497513956433439, 0.40000000000000013, 1.3962634015954636, 0.0 },
  { 1.6399998658645112, 0.40000000000000013, 1.5707963267948966, 0.0 },
};
const double toler014 = 2.5000000000000020e-13;

// Test data for k=0.50000000000000000.
// max(|f - f_Boost|): 4.4408920985006262e-16 at index 7
// max(|f - f_Boost| / |f_Boost|): 3.4551389361831220e-16
// mean(f - f_Boost): -5.8286708792820721e-17
// variance(f - f_Boost): 4.1942474344433133e-34
// stddev(f - f_Boost): 2.0479861900030756e-17
const testcase_ellint_1<double>
data015[10] =
{
  { 0.0000000000000000, 0.50000000000000000, 0.0000000000000000, 0.0 },
  { 0.17475385514035785, 0.50000000000000000, 0.17453292519943295, 0.0 },
  { 0.35081868470101579, 0.50000000000000000, 0.34906585039886590, 0.0 },
  { 0.52942862705190574, 0.50000000000000000, 0.52359877559829882, 0.0 },
  { 0.71164727562630326, 0.50000000000000000, 0.69813170079773179, 0.0 },
  { 0.89824523594227768, 0.50000000000000000, 0.87266462599716477, 0.0 },
  { 1.0895506700518853, 0.50000000000000000, 1.0471975511965976, 0.0 },
  { 1.2853005857432933, 0.50000000000000000, 1.2217304763960306, 0.0 },
  { 1.4845545520549488, 0.50000000000000000, 1.3962634015954636, 0.0 },
  { 1.6857503548125961, 0.50000000000000000, 1.5707963267948966, 0.0 },
};
const double toler015 = 2.5000000000000020e-13;

// Test data for k=0.60000000000000009.
// max(|f - f_Boost|): 4.4408920985006262e-16 at index 7
// max(|f - f_Boost| / |f_Boost|): 3.3664899092028927e-16
// mean(f - f_Boost): 7.4940054162198071e-17
// variance(f - f_Boost): 2.6715739327327140e-33
// stddev(f - f_Boost): 5.1687270509601433e-17
const testcase_ellint_1<double>
data016[10] =
{
  { 0.0000000000000000, 0.60000000000000009, 0.0000000000000000, 0.0 },
  { 0.17485154362988359, 0.60000000000000009, 0.17453292519943295, 0.0 },
  { 0.35160509865544320, 0.60000000000000009, 0.34906585039886590, 0.0 },
  { 0.53210652578446138, 0.60000000000000009, 0.52359877559829882, 0.0 },
  { 0.71805304664485659, 0.60000000000000009, 0.69813170079773179, 0.0 },
  { 0.91082759030195981, 0.60000000000000009, 0.87266462599716477, 0.0 },
  { 1.1112333229323361, 0.60000000000000009, 1.0471975511965976, 0.0 },
  { 1.3191461190365270, 0.60000000000000009, 1.2217304763960306, 0.0 },
  { 1.5332022105084779, 0.60000000000000009, 1.3962634015954636, 0.0 },
  { 1.7507538029157526, 0.60000000000000009, 1.5707963267948966, 0.0 },
};
const double toler016 = 2.5000000000000020e-13;

// Test data for k=0.70000000000000018.
// max(|f - f_Boost|): 6.6613381477509392e-16 at index 8
// max(|f - f_Boost| / |f_Boost|): 4.1569224977685422e-16
// mean(f - f_Boost): -1.1102230246251566e-17
// variance(f - f_Boost): 2.3145398087213714e-32
// stddev(f - f_Boost): 1.5213611697165703e-16
const testcase_ellint_1<double>
data017[10] =
{
  { 0.0000000000000000, 0.70000000000000018, 0.0000000000000000, 0.0 },
  { 0.17496737466916723, 0.70000000000000018, 0.17453292519943295, 0.0 },
  { 0.35254687535677931, 0.70000000000000018, 0.34906585039886590, 0.0 },
  { 0.53536740275997130, 0.70000000000000018, 0.52359877559829882, 0.0 },
  { 0.72603797651684454, 0.70000000000000018, 0.69813170079773179, 0.0 },
  { 0.92698296348313447, 0.70000000000000018, 0.87266462599716477, 0.0 },
  { 1.1400447527693318, 0.70000000000000018, 1.0471975511965976, 0.0 },
  { 1.3657668117194073, 0.70000000000000018, 1.2217304763960306, 0.0 },
  { 1.6024686895959164, 0.70000000000000018, 1.3962634015954636, 0.0 },
  { 1.8456939983747236, 0.70000000000000018, 1.5707963267948966, 0.0 },
};
const double toler017 = 2.5000000000000020e-13;

// Test data for k=0.80000000000000004.
// max(|f - f_Boost|): 8.8817841970012523e-16 at index 8
// max(|f - f_Boost| / |f_Boost|): 5.2032670495747184e-16
// mean(f - f_Boost): 1.9428902930940239e-16
// variance(f - f_Boost): 2.7486111305082033e-32
// stddev(f - f_Boost): 1.6578935823834422e-16
const testcase_ellint_1<double>
data018[10] =
{
  { 0.0000000000000000, 0.80000000000000004, 0.0000000000000000, 0.0 },
  { 0.17510154241338899, 0.80000000000000004, 0.17453292519943295, 0.0 },
  { 0.35365068839779396, 0.80000000000000004, 0.34906585039886590, 0.0 },
  { 0.53926804409084550, 0.80000000000000004, 0.52359877559829882, 0.0 },
  { 0.73587926028070372, 0.80000000000000004, 0.69813170079773179, 0.0 },
  { 0.94770942970071170, 0.80000000000000004, 0.87266462599716477, 0.0 },
  { 1.1789022995388236, 0.80000000000000004, 1.0471975511965976, 0.0 },
  { 1.4323027881876012, 0.80000000000000004, 1.2217304763960306, 0.0 },
  { 1.7069629739121677, 0.80000000000000004, 1.3962634015954636, 0.0 },
  { 1.9953027776647294, 0.80000000000000004, 1.5707963267948966, 0.0 },
};
const double toler018 = 2.5000000000000020e-13;

// Test data for k=0.90000000000000013.
// max(|f - f_Boost|): 8.8817841970012523e-16 at index 9
// max(|f - f_Boost| / |f_Boost|): 4.3381508715713360e-16
// mean(f - f_Boost): 1.4710455076283324e-16
// variance(f - f_Boost): 6.7801293731072419e-32
// stddev(f - f_Boost): 2.6038681558610532e-16
const testcase_ellint_1<double>
data019[10] =
{
  { 0.0000000000000000, 0.90000000000000013, 0.0000000000000000, 0.0 },
  { 0.17525427376115027, 0.90000000000000013, 0.17453292519943295, 0.0 },
  { 0.35492464591297446, 0.90000000000000013, 0.34906585039886590, 0.0 },
  { 0.54388221416157123, 0.90000000000000013, 0.52359877559829882, 0.0 },
  { 0.74797400423532512, 0.90000000000000013, 0.69813170079773179, 0.0 },
  { 0.97463898451966446, 0.90000000000000013, 0.87266462599716477, 0.0 },
  { 1.2334463254523440, 0.90000000000000013, 1.0471975511965976, 0.0 },
  { 1.5355247765594915, 0.90000000000000013, 1.2217304763960306, 0.0 },
  { 1.8882928567775128, 0.90000000000000013, 1.3962634015954636, 0.0 },
  { 2.2805491384227707, 0.90000000000000013, 1.5707963267948966, 0.0 },
};
const double toler019 = 2.5000000000000020e-13;

template<typename Ret, unsigned int Num>
  void
  test(const testcase_ellint_1<Ret> (&data)[Num], Ret toler)
  {
    bool test __attribute__((unused)) = true;
    const Ret eps = std::numeric_limits<Ret>::epsilon();
    Ret max_abs_diff = -Ret(1);
    Ret max_abs_frac = -Ret(1);
    unsigned int num_datum = Num;
    for (unsigned int i = 0; i < num_datum; ++i)
      {
	const Ret f = std::ellint_1(data[i].k, data[i].phi);
	const Ret f0 = data[i].f0;
	const Ret diff = f - f0;
	if (std::abs(diff) > max_abs_diff)
	  max_abs_diff = std::abs(diff);
	if (std::abs(f0) > Ret(10) * eps
	 && std::abs(f) > Ret(10) * eps)
	  {
	    const Ret frac = diff / f0;
	    if (std::abs(frac) > max_abs_frac)
	      max_abs_frac = std::abs(frac);
	  }
      }
    VERIFY(max_abs_frac < toler);
  }

int
main()
{
  test(data001, toler001);
  test(data002, toler002);
  test(data003, toler003);
  test(data004, toler004);
  test(data005, toler005);
  test(data006, toler006);
  test(data007, toler007);
  test(data008, toler008);
  test(data009, toler009);
  test(data010, toler010);
  test(data011, toler011);
  test(data012, toler012);
  test(data013, toler013);
  test(data014, toler014);
  test(data015, toler015);
  test(data016, toler016);
  test(data017, toler017);
  test(data018, toler018);
  test(data019, toler019);
  return 0;
}
