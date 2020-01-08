// { dg-do run { target c++11 } }
// { dg-require-cstdint "" }
// { dg-require-little-endian "" }
//
// 2008-11-24  Edward M. Smith-Rowland <3dw4rd@verizon.net>
// 2012-08-28  Ulrich Drepper  <drepper@gmail.com>, adapted for SFMT
//
// Copyright (C) 2008-2020 Free Software Foundation, Inc.
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

#include <iostream>
#include <ext/random>
#include <testsuite_hooks.h>


template<class SFMT>
void run_test(std::initializer_list<typename SFMT::result_type> vals)
{
  typedef typename SFMT::result_type result_type;
  SFMT e;

  e.seed(sizeof(result_type) == 4 ? 1234 : 4321);
  e.discard(990);
  bool success = true;
  for (auto i : vals)
    {
      result_type r = e();
      success &= r == i;
      std::cout << r << " vs " << i << std::endl;
    }
  VERIFY( success );
}


void
test01()
{
  __gnu_cxx::sfmt19937 e;

  VERIFY( e.min() == 0 );
  VERIFY( e.max() == std::numeric_limits<uint32_t>::max() );

  run_test<__gnu_cxx::sfmt607>({ UINT32_C(1318548553), UINT32_C(1985957974),
	UINT32_C(1367744196), UINT32_C(3463392791), UINT32_C(2780736231),
	UINT32_C(3894488561), UINT32_C(3157036262), UINT32_C(3491812767),
	UINT32_C(1724574180), UINT32_C(3645035493) });

  run_test<__gnu_cxx::sfmt607_64>({UINT64_C(15510024334182072935),
	UINT64_C(5793753331747412752), UINT64_C(16198353238554625740),
	UINT64_C(2233208824926016498), UINT64_C(3566091399820823780),
	UINT64_C(16608268514591292798), UINT64_C(10684941689666043359),
	UINT64_C(12463424292910456802), UINT64_C(5902567440240131366),
	UINT64_C(7228030834036501150) });

  run_test<__gnu_cxx::sfmt1279>({ UINT32_C(66657331), UINT32_C(637106837),
	UINT32_C(406927341), UINT32_C(3964420203), UINT32_C(2127134160),
	UINT32_C(1327235047), UINT32_C(227339400), UINT32_C(97109542),
	UINT32_C(1814799261), UINT32_C(340888197) });

  run_test<__gnu_cxx::sfmt1279_64>({ UINT64_C(16431921382083697129),
	UINT64_C(3107599092104940900), UINT64_C(4055245506102959965),
	UINT64_C(16096064917153424198), UINT64_C(14429331498726837109),
	UINT64_C(9539664361920633782), UINT64_C(1435296568185387099),
	UINT64_C(15922567183295047131), UINT64_C(641988285517426228),
	UINT64_C(15936274870984512675) });

  run_test<__gnu_cxx::sfmt2281>({ UINT32_C(2662391944), UINT32_C(1176696104),
	UINT32_C(3587947451), UINT32_C(4098993357), UINT32_C(3140998698),
	UINT32_C(870759742), UINT32_C(623529127), UINT32_C(3458807285),
	UINT32_C(3341615957), UINT32_C(195614711) });

  run_test<__gnu_cxx::sfmt2281_64>({ UINT64_C(16747191622237074632),
	UINT64_C(15804170396401370381), UINT64_C(3395175278324920203),
	UINT64_C(1541877340159274442), UINT64_C(14176322102994316687),
	UINT64_C(5130618305074712143), UINT64_C(6769693652413407081),
	UINT64_C(17733765687477661079), UINT64_C(5189766940360047353),
	UINT64_C(1333654688569723389) });

  run_test<__gnu_cxx::sfmt4253>({ UINT32_C(90342133), UINT32_C(1083987943),
	UINT32_C(1785481425), UINT32_C(1921212667), UINT32_C(3164342992),
	UINT32_C(1489324569), UINT32_C(603530523), UINT32_C(952851722),
	UINT32_C(2380944844), UINT32_C(3335854133) });

  run_test<__gnu_cxx::sfmt4253_64>({ UINT64_C(11570915401962514263),
	UINT64_C(206693220452528225), UINT64_C(16553299974633247759),
	UINT64_C(1069562842508952901), UINT64_C(7203975672387749585),
	UINT64_C(7552781925224963166), UINT64_C(16865729458807008705),
	UINT64_C(7848963629493506078), UINT64_C(9282397173969292817),
	UINT64_C(10738488504584559289) });

  run_test<__gnu_cxx::sfmt11213>({ UINT32_C(2072997009), UINT32_C(1332330347),
	UINT32_C(179681555), UINT32_C(2315290438), UINT32_C(2429393974),
	UINT32_C(509881964), UINT32_C(3807607878), UINT32_C(3055319970),
	UINT32_C(671840881), UINT32_C(3477325874) });

  run_test<__gnu_cxx::sfmt11213_64>({ UINT64_C(373867573626408653),
	UINT64_C(4732829340233638861), UINT64_C(16174630176505735656),
	UINT64_C(10063018133994900869), UINT64_C(17308645173308419196),
	UINT64_C(11091353816581371951), UINT64_C(15078420471318089727),
	UINT64_C(17965717592743818706), UINT64_C(12301543162252389155),
	UINT64_C(1724943167823308511) });

  run_test<__gnu_cxx::sfmt19937>({ UINT32_C(4002809368), UINT32_C(421169044),
	UINT32_C(1112642589), UINT32_C(3076213779), UINT32_C(3387033971),
	UINT32_C(2499610950), UINT32_C(3057240914), UINT32_C(1662679783),
	UINT32_C(461224431), UINT32_C(1168395933) });

  run_test<__gnu_cxx::sfmt19937_64>({ UINT64_C(8032857516355555296),
	UINT64_C(14023605983059313116), UINT64_C(1032336061815461376),
	UINT64_C(9840995337876562612), UINT64_C(9869256223029203587),
	UINT64_C(12227975697177267636), UINT64_C(12728115115844186033),
	UINT64_C(7752058479783205470), UINT64_C(729733219713393087),
	UINT64_C(12954017801239007622) });

  run_test<__gnu_cxx::sfmt44497>({ UINT32_C(1483092082), UINT32_C(1895679637),
	UINT32_C(9122740), UINT32_C(635864575), UINT32_C(320732971),
	UINT32_C(4253159584), UINT32_C(30097521), UINT32_C(839233316),
	UINT32_C(1431693534), UINT32_C(645981752) });

  run_test<__gnu_cxx::sfmt44497_64>({ UINT64_C(6246103978016445638),
	UINT64_C(4198275826138953222), UINT64_C(12473679170573289212),
	UINT64_C(14745709982748360209), UINT64_C(3630790792408208113),
	UINT64_C(4195294399578350499), UINT64_C(3742595698794327253),
	UINT64_C(17388385867517445933), UINT64_C(4261866397667814669),
	UINT64_C(17394085161690598095) });

  run_test<__gnu_cxx::sfmt86243>({ UINT32_C(3910985535), UINT32_C(100094501),
	UINT32_C(3120362616), UINT32_C(1854432382), UINT32_C(314688154),
	UINT32_C(522122712), UINT32_C(3026095676), UINT32_C(3681962735),
	UINT32_C(1851548627), UINT32_C(2153846465) });

  run_test<__gnu_cxx::sfmt86243_64>({ UINT64_C(250135615696586029),
	UINT64_C(4836277875486422184), UINT64_C(12389320296183057446),
	UINT64_C(7983028875884559442), UINT64_C(10079555227308335361),
	UINT64_C(14829333386540244214), UINT64_C(12159744972103351172),
	UINT64_C(4932579842314286356), UINT64_C(5200375244476537050),
	UINT64_C(11795681221121010641) });

  run_test<__gnu_cxx::sfmt132049>({ UINT32_C(1551023420), UINT32_C(1462317554),
	UINT32_C(2882528449), UINT32_C(1136299843), UINT32_C(292840589),
	UINT32_C(1307775247), UINT32_C(463274356), UINT32_C(1430357686),
	UINT32_C(3907607055), UINT32_C(3462509184) });

  run_test<__gnu_cxx::sfmt132049_64>({ UINT64_C(649482638765113922),
	UINT64_C(14205859353699897918), UINT64_C(14077261854908137257),
	UINT64_C(9564785861212212042), UINT64_C(7310747921257808846),
	UINT64_C(13759009477111470372), UINT64_C(11942123860149328831),
	UINT64_C(12868386070200572127), UINT64_C(18348617059674004332),
	UINT64_C(4233208019331956061) });

  run_test<__gnu_cxx::sfmt216091>({ UINT32_C(4171954654), UINT32_C(2938491210),
	UINT32_C(1356393891), UINT32_C(3558249995), UINT32_C(3711769979),
	UINT32_C(3434953144), UINT32_C(1601628304), UINT32_C(2187495640),
	UINT32_C(1762169715), UINT32_C(2141213778) });

  run_test<__gnu_cxx::sfmt216091_64>({ UINT64_C(11322404276387828766),
	UINT64_C(9653391575000195546), UINT64_C(1767839622905368464),
	UINT64_C(1690838241348740821), UINT64_C(817628268513271254),
	UINT64_C(15111277786569319196), UINT64_C(15817118810543358764),
	UINT64_C(5639001693408668083), UINT64_C(9959854003669400568),
	UINT64_C(13675983279642398887) });
}

int main()
{
  test01();
  return 0;
}
