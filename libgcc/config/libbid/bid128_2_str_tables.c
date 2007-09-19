/* Copyright (C) 2007  Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2, or (at your option) any later
version.

In addition to the permissions in the GNU General Public License, the
Free Software Foundation gives you unlimited permission to link the
compiled version of this file into combinations with other programs,
and to distribute those combinations without any restriction coming
from the use of this file.  (The General Public License restrictions
do apply in other respects; for example, they cover modification of
the file, and distribution when not linked into a combine
executable.)

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING.  If not, write to the Free
Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301, USA.  */

#include "bid_internal.h"

UINT64 __bid_Twoto60_m_10to18 = 152921504606846976LL;
UINT64 __bid_Twoto60 = 0x1000000000000000LL;
UINT64 __bid_Inv_Tento9 = 2305843009LL;        /* floor(2^61/10^9) */
UINT32 __bid_Twoto30_m_10to9 = 73741824;
UINT32 __bid_Tento9 = 1000000000;
UINT32 __bid_Tento6 = 1000000;
UINT32 __bid_Tento3 = 1000;

char __bid_midi_tbl[1000][3] = {
  "000", "001", "002", "003", "004", "005", "006", "007", "008", "009",
  "010", "011", "012", "013", "014", "015", "016", "017", "018", "019",
  "020", "021", "022", "023", "024", "025", "026", "027", "028", "029",
  "030", "031", "032", "033", "034", "035", "036", "037", "038", "039",
  "040", "041", "042", "043", "044", "045", "046", "047", "048", "049",
  "050", "051", "052", "053", "054", "055", "056", "057", "058", "059",
  "060", "061", "062", "063", "064", "065", "066", "067", "068", "069",
  "070", "071", "072", "073", "074", "075", "076", "077", "078", "079",
  "080", "081", "082", "083", "084", "085", "086", "087", "088", "089",
  "090", "091", "092", "093", "094", "095", "096", "097", "098", "099",
  "100", "101", "102", "103", "104", "105", "106", "107", "108", "109",
  "110", "111", "112", "113", "114", "115", "116", "117", "118", "119",
  "120", "121", "122", "123", "124", "125", "126", "127", "128", "129",
  "130", "131", "132", "133", "134", "135", "136", "137", "138", "139",
  "140", "141", "142", "143", "144", "145", "146", "147", "148", "149",
  "150", "151", "152", "153", "154", "155", "156", "157", "158", "159",
  "160", "161", "162", "163", "164", "165", "166", "167", "168", "169",
  "170", "171", "172", "173", "174", "175", "176", "177", "178", "179",
  "180", "181", "182", "183", "184", "185", "186", "187", "188", "189",
  "190", "191", "192", "193", "194", "195", "196", "197", "198", "199",
  "200", "201", "202", "203", "204", "205", "206", "207", "208", "209",
  "210", "211", "212", "213", "214", "215", "216", "217", "218", "219",
  "220", "221", "222", "223", "224", "225", "226", "227", "228", "229",
  "230", "231", "232", "233", "234", "235", "236", "237", "238", "239",
  "240", "241", "242", "243", "244", "245", "246", "247", "248", "249",
  "250", "251", "252", "253", "254", "255", "256", "257", "258", "259",
  "260", "261", "262", "263", "264", "265", "266", "267", "268", "269",
  "270", "271", "272", "273", "274", "275", "276", "277", "278", "279",
  "280", "281", "282", "283", "284", "285", "286", "287", "288", "289",
  "290", "291", "292", "293", "294", "295", "296", "297", "298", "299",
  "300", "301", "302", "303", "304", "305", "306", "307", "308", "309",
  "310", "311", "312", "313", "314", "315", "316", "317", "318", "319",
  "320", "321", "322", "323", "324", "325", "326", "327", "328", "329",
  "330", "331", "332", "333", "334", "335", "336", "337", "338", "339",
  "340", "341", "342", "343", "344", "345", "346", "347", "348", "349",
  "350", "351", "352", "353", "354", "355", "356", "357", "358", "359",
  "360", "361", "362", "363", "364", "365", "366", "367", "368", "369",
  "370", "371", "372", "373", "374", "375", "376", "377", "378", "379",
  "380", "381", "382", "383", "384", "385", "386", "387", "388", "389",
  "390", "391", "392", "393", "394", "395", "396", "397", "398", "399",
  "400", "401", "402", "403", "404", "405", "406", "407", "408", "409",
  "410", "411", "412", "413", "414", "415", "416", "417", "418", "419",
  "420", "421", "422", "423", "424", "425", "426", "427", "428", "429",
  "430", "431", "432", "433", "434", "435", "436", "437", "438", "439",
  "440", "441", "442", "443", "444", "445", "446", "447", "448", "449",
  "450", "451", "452", "453", "454", "455", "456", "457", "458", "459",
  "460", "461", "462", "463", "464", "465", "466", "467", "468", "469",
  "470", "471", "472", "473", "474", "475", "476", "477", "478", "479",
  "480", "481", "482", "483", "484", "485", "486", "487", "488", "489",
  "490", "491", "492", "493", "494", "495", "496", "497", "498", "499",
  "500", "501", "502", "503", "504", "505", "506", "507", "508", "509",
  "510", "511", "512", "513", "514", "515", "516", "517", "518", "519",
  "520", "521", "522", "523", "524", "525", "526", "527", "528", "529",
  "530", "531", "532", "533", "534", "535", "536", "537", "538", "539",
  "540", "541", "542", "543", "544", "545", "546", "547", "548", "549",
  "550", "551", "552", "553", "554", "555", "556", "557", "558", "559",
  "560", "561", "562", "563", "564", "565", "566", "567", "568", "569",
  "570", "571", "572", "573", "574", "575", "576", "577", "578", "579",
  "580", "581", "582", "583", "584", "585", "586", "587", "588", "589",
  "590", "591", "592", "593", "594", "595", "596", "597", "598", "599",
  "600", "601", "602", "603", "604", "605", "606", "607", "608", "609",
  "610", "611", "612", "613", "614", "615", "616", "617", "618", "619",
  "620", "621", "622", "623", "624", "625", "626", "627", "628", "629",
  "630", "631", "632", "633", "634", "635", "636", "637", "638", "639",
  "640", "641", "642", "643", "644", "645", "646", "647", "648", "649",
  "650", "651", "652", "653", "654", "655", "656", "657", "658", "659",
  "660", "661", "662", "663", "664", "665", "666", "667", "668", "669",
  "670", "671", "672", "673", "674", "675", "676", "677", "678", "679",
  "680", "681", "682", "683", "684", "685", "686", "687", "688", "689",
  "690", "691", "692", "693", "694", "695", "696", "697", "698", "699",
  "700", "701", "702", "703", "704", "705", "706", "707", "708", "709",
  "710", "711", "712", "713", "714", "715", "716", "717", "718", "719",
  "720", "721", "722", "723", "724", "725", "726", "727", "728", "729",
  "730", "731", "732", "733", "734", "735", "736", "737", "738", "739",
  "740", "741", "742", "743", "744", "745", "746", "747", "748", "749",
  "750", "751", "752", "753", "754", "755", "756", "757", "758", "759",
  "760", "761", "762", "763", "764", "765", "766", "767", "768", "769",
  "770", "771", "772", "773", "774", "775", "776", "777", "778", "779",
  "780", "781", "782", "783", "784", "785", "786", "787", "788", "789",
  "790", "791", "792", "793", "794", "795", "796", "797", "798", "799",
  "800", "801", "802", "803", "804", "805", "806", "807", "808", "809",
  "810", "811", "812", "813", "814", "815", "816", "817", "818", "819",
  "820", "821", "822", "823", "824", "825", "826", "827", "828", "829",
  "830", "831", "832", "833", "834", "835", "836", "837", "838", "839",
  "840", "841", "842", "843", "844", "845", "846", "847", "848", "849",
  "850", "851", "852", "853", "854", "855", "856", "857", "858", "859",
  "860", "861", "862", "863", "864", "865", "866", "867", "868", "869",
  "870", "871", "872", "873", "874", "875", "876", "877", "878", "879",
  "880", "881", "882", "883", "884", "885", "886", "887", "888", "889",
  "890", "891", "892", "893", "894", "895", "896", "897", "898", "899",
  "900", "901", "902", "903", "904", "905", "906", "907", "908", "909",
  "910", "911", "912", "913", "914", "915", "916", "917", "918", "919",
  "920", "921", "922", "923", "924", "925", "926", "927", "928", "929",
  "930", "931", "932", "933", "934", "935", "936", "937", "938", "939",
  "940", "941", "942", "943", "944", "945", "946", "947", "948", "949",
  "950", "951", "952", "953", "954", "955", "956", "957", "958", "959",
  "960", "961", "962", "963", "964", "965", "966", "967", "968", "969",
  "970", "971", "972", "973", "974", "975", "976", "977", "978", "979",
  "980", "981", "982", "983", "984", "985", "986", "987", "988", "989",
  "990", "991", "992", "993", "994", "995", "996", "997", "998", "999"
};

UINT64 __bid_mod10_18_tbl[9][128] = {
  // 2^59 = 576460752303423488, A and B breakdown, where data = A*10^18 + B 

  { 
  0LL, 0LL, 0LL, 576460752303423488LL,        
      //  0*2^59,  1*2^59
  1LL, 152921504606846976LL, 1LL, 729382256910270464LL,        
      //  2*2^59,  3*2^59
  2LL, 305843009213693952LL, 2LL, 882303761517117440LL,        
      //  4*2^59,  5*2^59
  3LL, 458764513820540928LL, 4LL, 35225266123964416LL,        
      //  6*2^59,  7*2^59
  4LL, 611686018427387904LL, 5LL, 188146770730811392LL,        
      //  8*2^59,  9*2^59
  5LL, 764607523034234880LL, 6LL, 341068275337658368LL,        
  // 10*2^59, 11*2^59
  6LL, 917529027641081856LL, 7LL, 493989779944505344LL,        
      // 12*2^59, 13*2^59
  8LL, 70450532247928832LL, 8LL, 646911284551352320LL,        
      // 14*2^59, 15*2^59
  9LL, 223372036854775808LL, 9LL, 799832789158199296LL,        
      // 16*2^59, 17*2^59
  10LL, 376293541461622784LL, 10LL, 952754293765046272LL,        
      // 18*2^59, 19*2^59
  11LL, 529215046068469760LL, 12LL, 105675798371893248LL,        
      // 20*2^59, 21*2^59
  12LL, 682136550675316736LL, 13LL, 258597302978740224LL,        
      // 22*2^59, 23*2^59
  13LL, 835058055282163712LL, 14LL, 411518807585587200LL,        
      // 24*2^59, 25*2^59
  14LL, 987979559889010688LL, 15LL, 564440312192434176LL,        
      // 26*2^59, 27*2^59
  16LL, 140901064495857664LL, 16LL, 717361816799281152LL,        
      // 28*2^59, 29*2^59
  17LL, 293822569102704640LL, 17LL, 870283321406128128LL,        
      // 30*2^59, 31*2^59
  18LL, 446744073709551616LL, 19LL, 23204826012975104LL,        
      // 32*2^59, 33*2^59
  19LL, 599665578316398592LL, 20LL, 176126330619822080LL,        
      // 34*2^59, 35*2^59
  20LL, 752587082923245568LL, 21LL, 329047835226669056LL,        
      // 36*2^59, 37*2^59
  21LL, 905508587530092544LL, 22LL, 481969339833516032LL,        
      // 38*2^59, 39*2^59
  23LL, 58430092136939520LL, 23LL, 634890844440363008LL,        
      // 40*2^59, 41*2^59
  24LL, 211351596743786496LL, 24LL, 787812349047209984LL,        
      // 42*2^59, 43*2^59
  25LL, 364273101350633472LL, 25LL, 940733853654056960LL,        
      // 44*2^59, 45*2^59
  26LL, 517194605957480448LL, 27LL, 93655358260903936LL,        
      // 46*2^59, 47*2^59
  27LL, 670116110564327424LL, 28LL, 246576862867750912LL,        
      // 48*2^59, 49*2^59
  28LL, 823037615171174400LL, 29LL, 399498367474597888LL,        
      // 50*2^59, 51*2^59
  29LL, 975959119778021376LL, 30LL, 552419872081444864LL,        
      // 52*2^59, 53*2^59
  31LL, 128880624384868352LL, 31LL, 705341376688291840LL,        
      // 54*2^59, 55*2^59
  32LL, 281802128991715328LL, 32LL, 858262881295138816LL,        
      // 56*2^59, 57*2^59
  33LL, 434723633598562304LL, 34LL, 11184385901985792LL,        
      // 58*2^59, 59*2^59
  34LL, 587645138205409280LL, 35LL, 164105890508832768LL,        
      // 60*2^59, 61*2^59
  35LL, 740566642812256256LL, 36LL, 317027395115679744LL,        
      // 62*2^59, 63*2^59
  },

  {
  // 2^65 = 36*10^18 + 893488147419103232
  0LL, 0LL, 36LL, 893488147419103232LL,        
      //  0*2^65,  1*2^65
  73LL, 786976294838206464LL, 110LL, 680464442257309696LL,        
      //  2*2^65,  3*2^65
  147LL, 573952589676412928LL, 184LL, 467440737095516160LL,        
      //  4*2^65,  5*2^65
  221LL, 360928884514619392LL, 258LL, 254417031933722624LL,        
      //  6*2^65,  7*2^65
  295LL, 147905179352825856LL, 332LL, 41393326771929088LL,        
      //  8*2^65,  9*2^65
  368LL, 934881474191032320LL, 405LL, 828369621610135552LL,        
      //  0*2^65,  1*2^65
  442LL, 721857769029238784LL, 479LL, 615345916448342016LL,        
      //  2*2^65,  3*2^65
  516LL, 508834063867445248LL, 553LL, 402322211286548480LL,        
      //  4*2^65,  5*2^65
  590LL, 295810358705651712LL, 627LL, 189298506124754944LL,        
      //  6*2^65,  7*2^65
  664LL, 82786653543858176LL, 700LL, 976274800962961408LL,        
      //  8*2^65,  9*2^65
  737LL, 869762948382064640LL, 774LL, 763251095801167872LL,        
      //  0*2^65,  1*2^65
  811LL, 656739243220271104LL, 848LL, 550227390639374336LL,        
      //  2*2^65,  3*2^65
  885LL, 443715538058477568LL, 922LL, 337203685477580800LL,        
      //  4*2^65,  5*2^65
  959LL, 230691832896684032LL, 996LL, 124179980315787264LL,        
      //  6*2^65,  7*2^65
  1033LL, 17668127734890496LL, 1069LL, 911156275153993728LL,        
      //  8*2^65,  9*2^65
  1106LL, 804644422573096960LL, 1143LL, 698132569992200192LL,        
      //  0*2^65,  1*2^65
  1180LL, 591620717411303424LL, 1217LL, 485108864830406656LL,        
      //  2*2^65,  3*2^65
  1254LL, 378597012249509888LL, 1291LL, 272085159668613120LL,        
      //  4*2^65,  5*2^65
  1328LL, 165573307087716352LL, 1365LL, 59061454506819584LL,        
      //  6*2^65,  7*2^65
  1401LL, 952549601925922816LL, 1438LL, 846037749345026048LL,        
      //  8*2^65,  9*2^65
  1475LL, 739525896764129280LL, 1512LL, 633014044183232512LL,        
      //  0*2^65,  1*2^65
  1549LL, 526502191602335744LL, 1586LL, 419990339021438976LL,        
      //  2*2^65,  3*2^65
  1623LL, 313478486440542208LL, 1660LL, 206966633859645440LL,        
      //  4*2^65,  5*2^65
  1697LL, 100454781278748672LL, 1733LL, 993942928697851904LL,        
      //  6*2^65,  7*2^65
  1770LL, 887431076116955136LL, 1807LL, 780919223536058368LL,        
      //  8*2^65,  9*2^65
  1844LL, 674407370955161600LL, 1881LL, 567895518374264832LL,        
      //  0*2^65,  1*2^65
  1918LL, 461383665793368064LL, 1955LL, 354871813212471296LL,        
      //  2*2^65,  3*2^65
  1992LL, 248359960631574528LL, 2029LL, 141848108050677760LL,        
      //  4*2^65,  5*2^65
  2066LL, 35336255469780992LL, 2102LL, 928824402888884224LL,        
      //  6*2^65,  7*2^65
  2139LL, 822312550307987456LL, 2176LL, 715800697727090688LL,        
      //  8*2^65,  9*2^65
  2213LL, 609288845146193920LL, 2250LL, 502776992565297152LL,        
      //  0*2^65,  1*2^65
  2287LL, 396265139984400384LL, 2324LL, 289753287403503616LL,        
      //  2*2^65,  3*2^65
  },

  {
  0LL, 0LL, 2361LL, 183241434822606848LL,
  4722LL, 366482869645213696LL, 7083LL, 549724304467820544LL,
  9444LL, 732965739290427392LL, 11805LL, 916207174113034240LL,
  14167LL, 99448608935641088LL, 16528LL, 282690043758247936LL,
  18889LL, 465931478580854784LL, 21250LL, 649172913403461632LL,
  23611LL, 832414348226068480LL, 25973LL, 15655783048675328LL,
  28334LL, 198897217871282176LL, 30695LL, 382138652693889024LL,
  33056LL, 565380087516495872LL, 35417LL, 748621522339102720LL,
  37778LL, 931862957161709568LL, 40140LL, 115104391984316416LL,
  42501LL, 298345826806923264LL, 44862LL, 481587261629530112LL,
  47223LL, 664828696452136960LL, 49584LL, 848070131274743808LL,
  51946LL, 31311566097350656LL, 54307LL, 214553000919957504LL,
  56668LL, 397794435742564352LL, 59029LL, 581035870565171200LL,
  61390LL, 764277305387778048LL, 63751LL, 947518740210384896LL,
  66113LL, 130760175032991744LL, 68474LL, 314001609855598592LL,
  70835LL, 497243044678205440LL, 73196LL, 680484479500812288LL,
  75557LL, 863725914323419136LL, 77919LL, 46967349146025984LL,
  80280LL, 230208783968632832LL, 82641LL, 413450218791239680LL,
  85002LL, 596691653613846528LL, 87363LL, 779933088436453376LL,
  89724LL, 963174523259060224LL, 92086LL, 146415958081667072LL,
  94447LL, 329657392904273920LL, 96808LL, 512898827726880768LL,
  99169LL, 696140262549487616LL, 101530LL, 879381697372094464LL,
  103892LL, 62623132194701312LL, 106253LL, 245864567017308160LL,
  108614LL, 429106001839915008LL, 110975LL, 612347436662521856LL,
  113336LL, 795588871485128704LL, 115697LL, 978830306307735552LL,
  118059LL, 162071741130342400LL, 120420LL, 345313175952949248LL,
  122781LL, 528554610775556096LL, 125142LL, 711796045598162944LL,
  127503LL, 895037480420769792LL, 129865LL, 78278915243376640LL,
  132226LL, 261520350065983488LL, 134587LL, 444761784888590336LL,
  136948LL, 628003219711197184LL, 139309LL, 811244654533804032LL,
  141670LL, 994486089356410880LL, 144032LL, 177727524179017728LL,
  146393LL, 360968959001624576LL, 148754LL, 544210393824231424LL,
  },

  {
  0LL, 0LL, 151115LL, 727451828646838272LL,
  302231LL, 454903657293676544LL, 453347LL, 182355485940514816LL,
  604462LL, 909807314587353088LL, 755578LL, 637259143234191360LL,
  906694LL, 364710971881029632LL, 1057810LL, 92162800527867904LL,
  1208925LL, 819614629174706176LL, 1360041LL, 547066457821544448LL,
  1511157LL, 274518286468382720LL, 1662273LL, 1970115115220992LL,
  1813388LL, 729421943762059264LL, 1964504LL, 456873772408897536LL,
  2115620LL, 184325601055735808LL, 2266735LL, 911777429702574080LL,
  2417851LL, 639229258349412352LL, 2568967LL, 366681086996250624LL,
  2720083LL, 94132915643088896LL, 2871198LL, 821584744289927168LL,
  3022314LL, 549036572936765440LL, 3173430LL, 276488401583603712LL,
  3324546LL, 3940230230441984LL, 3475661LL, 731392058877280256LL,
  3626777LL, 458843887524118528LL, 3777893LL, 186295716170956800LL,
  3929008LL, 913747544817795072LL, 4080124LL, 641199373464633344LL,
  4231240LL, 368651202111471616LL, 4382356LL, 96103030758309888LL,
  4533471LL, 823554859405148160LL, 4684587LL, 551006688051986432LL,
  4835703LL, 278458516698824704LL, 4986819LL, 5910345345662976LL,
  5137934LL, 733362173992501248LL, 5289050LL, 460814002639339520LL,
  5440166LL, 188265831286177792LL, 5591281LL, 915717659933016064LL,
  5742397LL, 643169488579854336LL, 5893513LL, 370621317226692608LL,
  6044629LL, 98073145873530880LL, 6195744LL, 825524974520369152LL,
  6346860LL, 552976803167207424LL, 6497976LL, 280428631814045696LL,
  6649092LL, 7880460460883968LL, 6800207LL, 735332289107722240LL,
  6951323LL, 462784117754560512LL, 7102439LL, 190235946401398784LL,
  7253554LL, 917687775048237056LL, 7404670LL, 645139603695075328LL,
  7555786LL, 372591432341913600LL, 7706902LL, 100043260988751872LL,
  7858017LL, 827495089635590144LL, 8009133LL, 554946918282428416LL,
  8160249LL, 282398746929266688LL, 8311365LL, 9850575576104960LL,
  8462480LL, 737302404222943232LL, 8613596LL, 464754232869781504LL,
  8764712LL, 192206061516619776LL, 8915827LL, 919657890163458048LL,
  9066943LL, 647109718810296320LL, 9218059LL, 374561547457134592LL,
  9369175LL, 102013376103972864LL, 9520290LL, 829465204750811136LL,
  },

  {
  0LL, 0LL, 9671406LL, 556917033397649408LL,
  19342813LL, 113834066795298816LL, 29014219LL, 670751100192948224LL,
  38685626LL, 227668133590597632LL, 48357032LL, 784585166988247040LL,
  58028439LL, 341502200385896448LL, 67699845LL, 898419233783545856LL,
  77371252LL, 455336267181195264LL, 87042659LL, 12253300578844672LL,
  96714065LL, 569170333976494080LL, 106385472LL, 126087367374143488LL,
  116056878LL, 683004400771792896LL, 125728285LL, 239921434169442304LL,
  135399691LL, 796838467567091712LL, 145071098LL, 353755500964741120LL,
  154742504LL, 910672534362390528LL, 164413911LL, 467589567760039936LL,
  174085318LL, 24506601157689344LL, 183756724LL, 581423634555338752LL,
  193428131LL, 138340667952988160LL, 203099537LL, 695257701350637568LL,
  212770944LL, 252174734748286976LL, 222442350LL, 809091768145936384LL,
  232113757LL, 366008801543585792LL, 241785163LL, 922925834941235200LL,
  251456570LL, 479842868338884608LL, 261127977LL, 36759901736534016LL,
  270799383LL, 593676935134183424LL, 280470790LL, 150593968531832832LL,
  290142196LL, 707511001929482240LL, 299813603LL, 264428035327131648LL,
  309485009LL, 821345068724781056LL, 319156416LL, 378262102122430464LL,
  328827822LL, 935179135520079872LL, 338499229LL, 492096168917729280LL,
  348170636LL, 49013202315378688LL, 357842042LL, 605930235713028096LL,
  367513449LL, 162847269110677504LL, 377184855LL, 719764302508326912LL,
  386856262LL, 276681335905976320LL, 396527668LL, 833598369303625728LL,
  406199075LL, 390515402701275136LL, 415870481LL, 947432436098924544LL,
  425541888LL, 504349469496573952LL, 435213295LL, 61266502894223360LL,
  444884701LL, 618183536291872768LL, 454556108LL, 175100569689522176LL,
  464227514LL, 732017603087171584LL, 473898921LL, 288934636484820992LL,
  483570327LL, 845851669882470400LL, 493241734LL, 402768703280119808LL,
  502913140LL, 959685736677769216LL, 512584547LL, 516602770075418624LL,
  522255954LL, 73519803473068032LL, 531927360LL, 630436836870717440LL,
  541598767LL, 187353870268366848LL, 551270173LL, 744270903666016256LL,
  560941580LL, 301187937063665664LL, 570612986LL, 858104970461315072LL,
  580284393LL, 415022003858964480LL, 589955799LL, 971939037256613888LL,
  599627206LL, 528856070654263296LL, 609298613LL, 85773104051912704LL,
  },

  {
  0LL, 0LL, 618970019LL, 642690137449562112LL,
  1237940039LL, 285380274899124224LL, 1856910058LL, 928070412348686336LL, 
  2475880078LL, 570760549798248448LL, 3094850098LL, 213450687247810560LL, 
  3713820117LL, 856140824697372672LL, 4332790137LL, 498830962146934784LL, 
  4951760157LL, 141521099596496896LL, 5570730176LL, 784211237046059008LL,
  6189700196LL, 426901374495621120LL, 6808670216LL, 69591511945183232LL,
  7427640235LL, 712281649394745344LL, 8046610255LL, 354971786844307456LL,
  8665580274LL, 997661924293869568LL, 9284550294LL, 640352061743431680LL,
  9903520314LL, 283042199192993792LL, 10522490333LL, 925732336642555904LL,
  11141460353LL, 568422474092118016LL, 11760430373LL, 211112611541680128LL,
  12379400392LL, 853802748991242240LL, 12998370412LL, 496492886440804352LL,
  13617340432LL, 139183023890366464LL, 14236310451LL, 781873161339928576LL,
  14855280471LL, 424563298789490688LL, 15474250491LL, 67253436239052800LL,
  16093220510LL, 709943573688614912LL, 16712190530LL, 352633711138177024LL,
  17331160549LL, 995323848587739136LL, 17950130569LL, 638013986037301248LL,
  18569100589LL, 280704123486863360LL, 19188070608LL, 923394260936425472LL,
  19807040628LL, 566084398385987584LL, 20426010648LL, 208774535835549696LL,
  21044980667LL, 851464673285111808LL, 21663950687LL, 494154810734673920LL,
  22282920707LL, 136844948184236032LL, 22901890726LL, 779535085633798144LL,
  23520860746LL, 422225223083360256LL, 24139830766LL, 64915360532922368LL,
  24758800785LL, 707605497982484480LL, 25377770805LL, 350295635432046592LL,
  25996740824LL, 992985772881608704LL, 26615710844LL, 635675910331170816LL,
  27234680864LL, 278366047780732928LL, 27853650883LL, 921056185230295040LL,
  28472620903LL, 563746322679857152LL, 29091590923LL, 206436460129419264LL,
  29710560942LL, 849126597578981376LL, 30329530962LL, 491816735028543488LL,
  30948500982LL, 134506872478105600LL, 31567471001LL, 777197009927667712LL,
  32186441021LL, 419887147377229824LL, 32805411041LL, 62577284826791936LL,
  33424381060LL, 705267422276354048LL, 34043351080LL, 347957559725916160LL,
  34662321099LL, 990647697175478272LL, 35281291119LL, 633337834625040384LL,
  35900261139LL, 276027972074602496LL, 36519231158LL, 918718109524164608LL,
  37138201178LL, 561408246973726720LL, 37757171198LL, 204098384423288832LL,
  38376141217LL, 846788521872850944LL, 38995111237LL, 489478659322413056LL,
  },

  {
  0LL, 0LL, 39614081257LL, 132168796771975168LL, 
  79228162514LL, 264337593543950336LL, 118842243771LL, 396506390315925504LL,
  158456325028LL, 528675187087900672LL, 198070406285LL, 660843983859875840LL,
  237684487542LL, 793012780631851008LL, 277298568799LL, 925181577403826176LL,
  316912650057LL, 57350374175801344LL, 356526731314LL, 189519170947776512LL,
  396140812571LL, 321687967719751680LL, 435754893828LL, 453856764491726848LL,
  475368975085LL, 586025561263702016LL, 514983056342LL, 718194358035677184LL,
  554597137599LL, 850363154807652352LL, 594211218856LL, 982531951579627520LL,
  633825300114LL, 114700748351602688LL, 673439381371LL, 246869545123577856LL,
  713053462628LL, 379038341895553024LL, 752667543885LL, 511207138667528192LL,
  792281625142LL, 643375935439503360LL, 831895706399LL, 775544732211478528LL,
  871509787656LL, 907713528983453696LL, 911123868914LL, 39882325755428864LL,
  950737950171LL, 172051122527404032LL, 990352031428LL, 304219919299379200LL,
  1029966112685LL, 436388716071354368LL, 1069580193942LL, 568557512843329536LL,
  1109194275199LL, 700726309615304704LL, 1148808356456LL, 832895106387279872LL,
  1188422437713LL, 965063903159255040LL, 1228036518971LL, 97232699931230208LL,
  1267650600228LL, 229401496703205376LL, 1307264681485LL, 361570293475180544LL,
  1346878762742LL, 493739090247155712LL, 1386492843999LL, 625907887019130880LL,
  1426106925256LL, 758076683791106048LL, 1465721006513LL, 890245480563081216LL,
  1505335087771LL, 22414277335056384LL, 1544949169028LL, 154583074107031552LL,
  1584563250285LL, 286751870879006720LL, 1624177331542LL, 418920667650981888LL,
  1663791412799LL, 551089464422957056LL, 1703405494056LL, 683258261194932224LL,
  1743019575313LL, 815427057966907392LL, 1782633656570LL, 947595854738882560LL,
  1822247737828LL, 79764651510857728LL, 1861861819085LL, 211933448282832896LL,
  1901475900342LL, 344102245054808064LL, 1941089981599LL, 476271041826783232LL,
  1980704062856LL, 608439838598758400LL, 2020318144113LL, 740608635370733568LL,
  2059932225370LL, 872777432142708736LL, 2099546306628LL, 4946228914683904LL,
  2139160387885LL, 137115025686659072LL, 2178774469142LL, 269283822458634240LL,
  2218388550399LL, 401452619230609408LL, 2258002631656LL, 533621416002584576LL,
  2297616712913LL, 665790212774559744LL, 2337230794170LL, 797959009546534912LL,
  2376844875427LL, 930127806318510080LL, 2416458956685LL, 62296603090485248LL,
  2456073037942LL, 194465399862460416LL, 2495687119199LL, 326634196634435584LL,
  },

  {
  0LL, 0LL, 2535301200456LL, 458802993406410752LL,
  5070602400912LL, 917605986812821504LL, 7605903601369LL, 376408980219232256LL,
  10141204801825LL, 835211973625643008LL, 12676506002282LL, 294014967032053760LL,
  15211807202738LL, 752817960438464512LL, 17747108403195LL, 211620953844875264LL,
  20282409603651LL, 670423947251286016LL, 22817710804108LL, 129226940657696768LL,
  25353012004564LL, 588029934064107520LL, 27888313205021LL, 46832927470518272LL,
  30423614405477LL, 505635920876929024LL, 32958915605933LL, 964438914283339776LL,
  35494216806390LL, 423241907689750528LL, 38029518006846LL, 882044901096161280LL, 40564819207303LL, 340847894502572032LL, 43100120407759LL, 799650887908982784LL,
  45635421608216LL, 258453881315393536LL, 48170722808672LL, 717256874721804288LL,
  50706024009129LL, 176059868128215040LL, 53241325209585LL, 634862861534625792LL,
  55776626410042LL, 93665854941036544LL, 58311927610498LL, 552468848347447296LL,
  60847228810955LL, 11271841753858048LL, 63382530011411LL, 470074835160268800LL,
  65917831211867LL, 928877828566679552LL, 68453132412324LL, 387680821973090304LL,
  70988433612780LL, 846483815379501056LL, 73523734813237LL, 305286808785911808LL,
  76059036013693LL, 764089802192322560LL, 78594337214150LL, 222892795598733312LL,
  81129638414606LL, 681695789005144064LL, 83664939615063LL, 140498782411554816LL,
  86200240815519LL, 599301775817965568LL, 88735542015976LL, 58104769224376320LL,
  91270843216432LL, 516907762630787072LL, 93806144416888LL, 975710756037197824LL,
  96341445617345LL, 434513749443608576LL, 98876746817801LL, 893316742850019328LL,
  101412048018258LL, 352119736256430080LL, 103947349218714LL, 810922729662840832LL,
  106482650419171LL, 269725723069251584LL, 109017951619627LL, 728528716475662336LL,
  111553252820084LL, 187331709882073088LL, 114088554020540LL, 646134703288483840LL,
  116623855220997LL, 104937696694894592LL, 119159156421453LL, 563740690101305344LL,
  121694457621910LL, 22543683507716096LL, 124229758822366LL, 481346676914126848LL,
  126765060022822LL, 940149670320537600LL, 129300361223279LL, 398952663726948352LL,
  131835662423735LL, 857755657133359104LL, 134370963624192LL, 316558650539769856LL,
  136906264824648LL, 775361643946180608LL, 139441566025105LL, 234164637352591360LL,
  141976867225561LL, 692967630759002112LL, 144512168426018LL, 151770624165412864LL,
  147047469626474LL, 610573617571823616LL, 149582770826931LL, 69376610978234368LL,
  152118072027387LL, 528179604384645120LL, 154653373227843LL, 986982597791055872LL,
  157188674428300LL, 445785591197466624LL, 159723975628756LL, 904588584603877376LL,
  },

  {
  0LL, 0LL, 162259276829213LL, 363391578010288128LL,
  324518553658426LL, 726783156020576256LL, 486777830487640LL, 90174734030864384LL,
  649037107316853LL, 453566312041152512LL, 811296384146066LL, 816957890051440640LL,
  973555660975280LL, 180349468061728768LL, 1135814937804493LL, 543741046072016896LL,
  1298074214633706LL, 907132624082305024LL, 1460333491462920LL, 270524202092593152LL,
  1622592768292133LL, 633915780102881280LL, 1784852045121346LL, 997307358113169408LL,
  1947111321950560LL, 360698936123457536LL, 2109370598779773LL, 724090514133745664LL,
  2271629875608987LL, 87482092144033792LL, 2433889152438200LL, 450873670154321920LL,
  2596148429267413LL, 814265248164610048LL, 2758407706096627LL, 177656826174898176LL,
  2920666982925840LL, 541048404185186304LL, 3082926259755053LL, 904439982195474432LL,
  3245185536584267LL, 267831560205762560LL, 3407444813413480LL, 631223138216050688LL,
  3569704090242693LL, 994614716226338816LL, 3731963367071907LL, 358006294236626944LL,
  3894222643901120LL, 721397872246915072LL, 4056481920730334LL, 84789450257203200LL,
  4218741197559547LL, 448181028267491328LL, 4381000474388760LL, 811572606277779456LL,
  4543259751217974LL, 174964184288067584LL, 4705519028047187LL, 538355762298355712LL,
  4867778304876400LL, 901747340308643840LL, 5030037581705614LL, 265138918318931968LL,
  5192296858534827LL, 628530496329220096LL, 5354556135364040LL, 991922074339508224LL,
  5516815412193254LL, 355313652349796352LL, 5679074689022467LL, 718705230360084480LL,
  5841333965851681LL, 82096808370372608LL, 6003593242680894LL, 445488386380660736LL,
  6165852519510107LL, 808879964390948864LL, 6328111796339321LL, 172271542401236992LL,
  6490371073168534LL, 535663120411525120LL, 6652630349997747LL, 899054698421813248LL,
  6814889626826961LL, 262446276432101376LL, 6977148903656174LL, 625837854442389504LL,
  7139408180485387LL, 989229432452677632LL, 7301667457314601LL, 352621010462965760LL,
  7463926734143814LL, 716012588473253888LL, 7626186010973028LL, 79404166483542016LL,
  7788445287802241LL, 442795744493830144LL, 7950704564631454LL, 806187322504118272LL,
  8112963841460668LL, 169578900514406400LL, 8275223118289881LL, 532970478524694528LL,
  8437482395119094LL, 896362056534982656LL, 8599741671948308LL, 259753634545270784LL,
  8762000948777521LL, 623145212555558912LL, 8924260225606734LL, 986536790565847040LL,
  9086519502435948LL, 349928368576135168LL, 9248778779265161LL, 713319946586423296LL,
  9411038056094375LL, 76711524596711424LL, 9573297332923588LL, 440103102606999552LL,
  9735556609752801LL, 803494680617287680LL, 9897815886582015LL, 166886258627575808LL,
  10060075163411228LL, 530277836637863936LL, 10222334440240441LL, 893669414648152064LL
  }
};
