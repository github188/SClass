#include "Stdafx.h"
#include "MyMemory.h"
#include "IO/AndroidDB.h"
#include "Text/MyString.h"

IO::AndroidDB::AndroidInfo IO::AndroidDB::androids[] = {
//	androidId           brandName      modelName                modelNum        cpuName
//	{"8S89H_E3",        "",            "",                      "",             ""},
	{"A75B",            "Evercoss",    "A75B",                  "A75B",         "MT6735"},
	{"ALE-CL00",        "Huawei",      "P8 Lite",               "ALE-CL00",     "MSM8939"},
	{"ALP-AL00",        "Huawei",      "Mate 10",               "ALP-AL00",     "Kirin 970"},
	{"ALP-L29",         "Huawei",      "Mate 10 Dual SIM",      "ALP-L29",      "Kirin 970"},
	{"ANE-LX2",         "Huawei",      "P20 Lite",              "ANE-LX2",      "Kirin 659"},
	{"ASK-AL00x",       "Huawei",      "Honor Play 3 Dual SIM", "ASK-AL00x",    "Kirin 710"},
	{"ASUS_I001DC",     "ASUS",        "ROG PHONE 2",           "ASUS_I001DC",  "SM8150"},
	{"ASUS_X00QD",      "ASUS",        "ZenFone 5 (ZE620KL)",   "ASUS_X00QD",   "SDM636"},
	{"ASUS_X00RD",      "ASUS",        "ZenFone Live (ZA550AL)","ASUS_X00RD",   "MSM8917"},
	{"ASUS_X018D",      "ASUS",        "Zenfone Max Plus (ZB570TL)","ASUS_X018D","MT6750T"},
	{"ASUS_Z00AD",      "ASUS",        "ZenFone 2 (ZE551ML)",   "ASUS_Z00AD",   "Intel Z3580"},
	{"ASUS_Z012DA",     "ASUS",        "ZenFone 3 (ZE552KL)",   "ASUS_Z012DA",  "MSM8953"},
	{"BKL-AL20",        "Huawei",      "Honor V10",             "BKL-AL20",     "Kirin 970"},
	{"CHC-U01",         "Huawei",      "G Play Mini",           "CHC-U01",      "Kirin 620"},
	{"CLT-L29",         "Huawei",      "P20 Pro",               "CLT-L29",      "Kirin 970"},
	{"CPH1611",         "OPPO",        "R9s Plus",              "CPH1611",      "MSM8976"},
	{"CPH1909",         "OPPO",        "A5s Dual-SIM",          "CPH1909",      "MT6765"},
	{"CPH1941",         "OPPO",        "A9 2020 Dual-SIM",      "CPH1941",      "SM6125"},
	{"CPH1969",         "OPPO",        "F11 Pro",               "CPH1969",      "MT6771"},
	{"Che1-CL10",       "Huawei",      "Honor 4X",              "Che1-CL10",    "MSM8916"},
	{"Che2-TL00",       "Huawei",      "Honor 4X",              "Che2-TL00",    "Kirin 620"},
	{"Coolpad Y82-520", "Coolpad",     "Y82-520",               "Y82-520",      "MSM8916"},
	{"E2303",           "SONY",        "Xperia M4 Aqua",        "E2303",        "MSM8939"},
	{"E6633",           "SONY",        "Xperia Z5 Dual",        "E6633",        "MSM8994"},
	{"ELE-L29",         "Huawei",      "P30 Dual SIM",          "ELE-L29",      "Kirin 980"},
	{"EML-AL00",        "Huawei",      "P20 Dual SIM",          "EML-AL00",     "Kirin 970"},
	{"EML-L29",         "Huawei",      "P20",                   "EML-L29",      "Kirin 970"},
	{"EVA-L19",         "Huawei",      "P9",                    "EVA-L19",      "Kirin 955"},
	{"EVR-L29",         "Huawei",      "Mate 20 X Dual SIM",    "EVR-L29",      "Kirin 980"},
	{"FIG-LX2",         "Huawei",      "Y7s",                   "FIG-LX2",      "Kirin 659"},
	{"FLA-LX2",         "Huawei",      "Y9",                    "FLA-LX2",      "Kirin 659"},
	{"FRD-AL00",        "Huawei",      "Honor 8 Dual SIM",      "FRD-AL00",     "Kirin 950"},
	{"G3116",           "SONY",        "Xperia XA1 Dual",       "G3116",        "MT6757"},
	{"G8142",           "SONY",        "Xperia XZ Premium Dual","G8142",        "MSM8998"},
	{"G8232",           "SONY",        "Xperia XZs",            "G8232",        "MSM8996"},
	{"GCE x86 Phone",   "Unknown",     "Android Simulator",     "",             ""},
	{"GIONEE S10CL",    "Gionee",      "S10C",                  "S10CL",        "MSM8920"},
	{"GN5005",          "Gionee",      "GN5005",                "GN5005",       "MT6737"},
	{"H4133",           "SONY",        "Xperia XA2 Dual",       "H4133",        "SDM630"},
	{"H4233",           "SONY",        "Xperia XA2 Ultra Dual", "H4233",        "SDM630"},
	{"HMA-AL00",        "Huawei",      "Mate 20 Dual Slot",     "HMA-AL00",     "Kirin 980"},
	{"HMA-L09",         "Huawei",      "Mate 20",               "HMA-L09",      "Kirin 980"},
	{"HMA-L29",         "Huawei",      "Mate 20 Dual SIM",      "HMA-L29",      "Kirin 980"},
	{"HMA-TL00",        "Huawei",      "Mate 20 Dual Slot",     "HMA-TL00",     "Kirin 980"},
	{"HTC 2Q7A100",     "HTC",         "U19e",                  "2Q7A100",      "SDM710"},
	{"HTC 10 evo",      "HTC",         "10 evo",                "10 evo",       "MSM8994"},
	{"HTC One X9 dual sim","HTC",      "HTC One X9 dual sim",   "X9 dual sim",  "MT6795"},
	{"HTC U12 life",    "HTC",         "U12 life",              "U12 life",     "SDM636"},
	{"HTC_U-2u",        "HTC",         "U Play (U-2u)",         "U-2u",         "MT6755"},
	{"HTC_U-3u",        "HTC",         "U11",                   "U-3u",         "MSM8998"},
	{"IM-A860K",        "Pantech",     "Vega No 6",             "IM-A860K",     "APQ8064"},
	{"INE-LX2",         "Huawei",      "nova 3i",               "INE-LX2",      "Kirin 710"},
	{"J9110",           "SONY",        "Xperia 1 Dual SIM",     "J9110",        "SM8150"},
	{"JMM-AL10",        "Huawei",      "Honor V9 Play",         "JMM-AL10",     "MT6750"},
	{"LDN-AL00",        "Huawei",      "Enjoy 8 Dual SIM",      "LDN-AL00",     "MSM8937"},
	{"LDN-L21",         "Huawei",      "Y7 2018",               "LDN-L01",      "MSM8937"},
	{"LDN-LX1",         "Huawei",      "Y7 Prime 2018 Dual",    "LDN-LX1",      "MSM8940"},
	{"LDN-LX2",         "Huawei",      "Y7 Prime 2018",         "LDN-LX2",      "MSM8937"},
	{"LG-D500",         "LG",          "Optimus F6",            "LG-D500",      "MSM8930"},
	{"LG-D802",         "LG",          "G2",                    "LG-D802",      "MSM8974-AA"},
	{"LG-D838",         "LG",          "G Pro 2",               "LG-D838",      "MSM8974-AA"},
	{"LG-H860",         "LG",          "G5",                    "LG-H860",      "MSM8996"},
	{"LG-H870DS",       "LG",          "G6 Dual SIM",           "LG-H870DS",    "MSM8996"},
	{"LG-H930",         "LG",          "V30",                   "LG-H930",      "MSM8998"},
	{"LG-H961N",        "LG",          "V10",                   "LG-H961N",     "MSM8992"},
	{"LG-H990",         "LG",          "V20",                   "LG-H990",      "MSM8996"},
	{"LG-K535",         "LG",          "Stylus 2 Plus",         "LG-K535",      "MSM8937"},
	{"LG-M700",         "LG",          "Q6",                    "LG-M700",      "MSM8940"},
	{"LIO-L29",         "Huawei",      "Mate 30 Pro",           "LIO-L29",      "Kirin 990 4G"},
	{"LM-G710",         "LG",          "G7 ThinQ",              "LM-G710",      "SDM845"},
	{"LYA-L09",         "Huawei",      "Mate 20 Pro",           "LYA-L09",      "Kirin 980"},
	{"LYA-L29",         "Huawei",      "Mate 20 Pro Dual SIM",  "LYA-L29",      "Kirin 980"},
	{"MAR-LX2",         "Huawei",      "P30 Lite Dual SIM",     "MAR-LX2",      "Kirin 710"},
	{"MHA-L29",         "Huawei",      "Mate 9",                "MHA-L29",      "Kirin 960"},
	{"MI 5",            "Xiaomi",      "Mi 5",                  "MI5",          "MSM8996"},
	{"MI 8",            "Xiaomi",      "Mi 8",                  "M1803E1A",     "SDM845"},
	{"MI MAX 2",        "Xiaomi",      "Mi Max 2",              "MDE40",        "MSM8953"},
	{"MI MAX 3",        "Xiaomi",      "Mi Max 3",              "M1804E4A",     "SDM636"},
	{"MLA-AL10",        "Huawei",      "nova Plus",             "MLA-AL10",     "MSM8953"},
	{"MStar Android TV","MStar",       "Android TV",            "Android TV",   ""},
	{"Mi 9T",           "Xiaomi",      "Mi 9T",                 "M1903F10G",    "SM7150-AA"},
	{"Mi A1",           "Xiaomi",      "Mi A1",                 "MDG2",         "MSM8953"},
	{"Mi A3",           "Xiaomi",      "Mi A3",                 "M1906F9SH",    "SM6125"},
	{"Mi MIX 2",        "Xiaomi",      "Mi MIX 2",              "MDE5",         "MSM8998"},
	{"MiTV4-ANSM0",     "Xiaomi",      "Mi TV 4S",              "MiTV4-ANSM0",  "Amlogic T962"},
	{"Moto E (4) Plus", "Moto",        "E4 Plus",               "XT176",        "MT6737"},
	{"Nexus 5",         "LG",          "Nexus 5",               "LG-D821",      "MSM8974-AA"},
	{"Nexus 5X",        "LG",          "Nexus 5X",              "LG-H791",      "MSM8992"},
	{"Nokia 7 plus",    "Nokia",       "7 plus",                "TA-1041",      "SDM660"},
	{"Nokia 8.1",       "Nokia",       "8.1",                   "TA-1119",      "SDM710"},
	{"Nokia 9",         "Nokia",       "9",                     "TA-1094",      "SDM845"},
	{"ONEPLUS A5000",   "OnePlus",     "5",                     "A5000",        "MSM8998"},
	{"ONEPLUS A5010",   "OnePlus",     "5T",                    "A5010",        "MSM8998"},
	{"ONEPLUS A6010",   "OnePlus",     "6T",                    "A6010",        "SDM845"},
	{"OPPO R9s",        "OPPO"  ,      "R9s",                   "R9s",          "MSM8953"},
	{"OPPO R11",        "OPPO",        "R11",                   "R11",          "SDM660"},
	{"PAR-TL20",        "Huawei",      "nova 3",                "PAR-TL20",     "Kirin 970"},
	{"POCOPHONE F1",    "Xiaomi",      "POCOPHONE F1",          "M1805E10A",    "SDM845"},
	{"Pixel 2 XL",      "Google",      "Pixel 2 XL",            "Pixel 2 XL",   "MSM8998"},
	{"R7Plusm",         "OPPO",        "R7 Plus (Mobile)",      "R7Plusm",      "MSM8939"},
	{"RNE-L22",         "Huawei",      "nova 2i",               "RNE-L22",      "Kirin 659"},
	{"RVL-AL09",        "Huawei",      "Honor Note 10",         "RNE-L22",      "Kirin 970"},
	{"Redmi 4A",        "Xiaomi",      "Redmi 4A",              "Redmi 4A",     "MSM8917"},
	{"Redmi 5",         "Xiaomi",      "Redmi 5",               "MDG1",         "SDM450"},
	{"Redmi 5 Plus",    "Xiaomi",      "Redmi 5 Plus",          "MEG7",         "MSM8953"},
	{"Redmi Note 4",    "Xiaomi",      "Redmi Note 4",          "Redmi Note 4", "MSM8953"},
	{"Redmi Note 5" ,   "Xiaomi",      "Redmi Note 5",          "M1803E7SG",    "SDM636"},
	{"Redmi Note 5A",   "Xiaomi",      "Redmi Note 5A",         "MDI6S",        "MSM8940"},
	{"Redmi Note 7",    "Xiaomi",      "Redmi Note 7",          "M1901F7G",     "SDM660"},
	{"Redmi Note 8 Pro","Xiaomi",      "Redmi Note 8 Pro",      "M1906G7I",     "MT6785V"},
	{"Redmi Note 8T",   "Xiaomi",      "Redmi Note 8T",         "Redmi Note 8T","SM6125"},
	{"S41",             "Cat",         "S41",                   "S41",          "MT6757"},
	{"SM-A202F",        "Samsung",     "Galaxy A20e",           "SM-A202F",     "Exynos7884"},
	{"SM-A205GN",       "Samsung",     "Galaxy A20",            "SM-A205GN",    "Exynos7884"},
	{"SM-A405FN",       "Samsung",     "Galaxy A40",            "SM-A405FN",    "Exynos7904"},
	{"SM-A520F",        "Samsung",     "Galaxy A5",             "SM-A520F",     "Exynos7880"},
	{"SM-A530F",        "Samsung",     "Galaxy A8",             "SM-A530F",     "Exynos7885"},
	{"SM-A600FN",       "Samsung",     "Galaxy A6",             "SM-A600FN",    "Exynos7870"},
	{"SM-A705FN",       "Samsung",     "Galaxy A70",            "SM-A705FN",    "SM6150"},
	{"SM-A705MN",       "Samsung",     "Galaxy A70",            "SM-A705MN",    "SM6150"},
	{"SM-A730F",        "Samsung",     "Galaxy A8+",            "SM-A730F",     "Exynos7885"},
	{"SM-A920F",        "Samsung",     "Galaxy A9s",            "SM-A920F",     "SDM660"},
	{"SM-A3050",        "Samsung",     "Galaxy A40s",           "SM-A3050",     "Exynos7885"},
	{"SM-A5070",        "Samsung",     "Galaxy A50s Dual SIM",  "SM-A5070",     "Exynos9611"},
	{"SM-A6060",        "Samsung",     "Galaxy A60",            "SM-A6060",     "SM6150"},
	{"SM-A7050",        "Samsung",     "Galaxy A70",            "SM-A7050",     "SM6150"},
	{"SM-A8050",        "Samsung",     "Galaxy A80",            "SM-A8050",     "SM7150-AB"},
	{"SM-A9000",        "Samsung",     "Galaxy A9",             "SM-A9000",     "MSM8976"},
	{"SM-A9200",        "Samsung",     "Galaxy A9s",            "SM-A9200",     "SDM660"},
	{"SM-C5010",        "Samsung",     "Galaxy C5 Pro",         "SM-C5010",     "MSM8953Pro"},
	{"SM-C7000",        "Samsung",     "Galaxy C7",             "SM-C7000",     "MSM8953"},
	{"SM-C7010",        "Samsung",     "Galaxy C7 Pro",         "SM-C7010",     "MSM8953Pro"},
	{"SM-C7100",        "Samsung",     "Galaxy C8",             "SM-C7100",     "MT6757CD"},
	{"SM-C9000",        "Samsung",     "Galaxy C9 Pro",         "SM-C9000",     "MSM8976-PRO"},
	{"SM-G610F",        "Samsung",     "Galaxy J7 Prime",       "SM-G610F",     "Exynos7870"},
	{"SM-G610Y",        "Samsung",     "Galaxy J7 Prime",       "SM-G610Y",     "Exynos7870"},
	{"SM-G920V",        "Samsung",     "Galaxy S6 (Verizon)",   "SM-G920V",     "Exynos7420"},
	{"SM-G930F",        "Samsung",     "Galaxy S7",             "SM-G930F",     "Exynos8890"},
	{"SM-G955F",        "Samsung",     "Galaxy S8+",            "SM-G955F",     "Exynos8895"},
	{"SM-G975U",        "Samsung",     "Galaxy S10+",           "SM-G975U",     "SM8150"},
	{"SM-G6100",        "Samsung",     "Galaxy J7 Prime",       "SM-G6100",     "MSM8953"},
	{"SM-G9287",        "Samsung",     "Galaxy S6 edge+",       "SM-G9287",     "Exynos7420"},
	{"SM-G9350",        "Samsung",     "Galaxy S7 edge",        "SM-G9350",     "MSM8996"},
	{"SM-G9550",        "Samsung",     "Galaxy S8+",            "SM-G9550",     "MSM8998"},
	{"SM-G9600",        "Samsung",     "Galaxy S9 Duos",        "SM-G9600",     "SDM845"},
	{"SM-G9650",        "Samsung",     "Galaxy S9+",            "SM-G9650",     "SDM845"},
	{"SM-G9700",        "Samsung",     "Galaxy S10e",           "SM-G9700",     "SM8150"},
	{"SM-G9730",        "Samsung",     "Galaxy S10",            "SM-G9730",     "SM8150"},
	{"SM-G9750",        "Samsung",     "Galaxy S10+",           "SM-G9750",     "SM8150"},
	{"SM-J120M",        "Samsung",     "Galaxy J1",             "SM-J120M",     "SC9830"},
	{"SM-J330FN",       "Samsung",     "Galaxy J3",             "SM-J330FN",    "Exynos7570"},
	{"SM-J415GN",       "Samsung",     "Galaxy J4+",            "SM-J415GN",    "MSM8917"},
	{"SM-J600G",        "Samsung",     "Galaxy J6",             "SM-J600G",     "Exynos7870"},
	{"SM-J710GN",       "Samsung",     "Galaxy J7",             "SM-J710GN",    "Exynos7870"},
	{"SM-J730GM",       "Samsung",     "Galaxy J7 Pro Duos",    "SM-J730GM",    "Exynos7870"},
	{"SM-J5108",        "Samsung",     "Galaxy J5 2016",        "SM-J5108",     "MSM8916"},
	{"SM-N910U",        "Samsung",     "Galaxy Note 4",         "SM-N910U",     "Exynos5433"},
	{"SM-N960F",        "Samsung",     "Galaxy Note9",          "SM-N960F",     "Exynos9810"},
	{"SM-N975F",        "Samsung",     "Galaxy Note10+",        "SM-N975F",     "Exynos9825"},
	{"SM-N9005",        "Samsung",     "Galaxy Note 3",         "SM-N9005",     "MSM8974-AA"},
	{"SM-N9200",        "Samsung",     "Galaxy Note 5",         "SM-N9200",     "Exynos7420"},
	{"SM-N9500",        "Samsung",     "Galaxy Note8",          "SM-N9500",     "MSM8998"},
	{"SM-N9600",        "Samsung",     "Galaxy Note9",          "SM-N9600",     "SDM845"},
	{"SM-N9700",        "Samsung",     "Galaxy Note10",         "SM-N9700",     "SM8150"},
	{"SM-N9750",        "Samsung",     "Galaxy Note10+",        "SM-N9750",     "SM8150"},
	{"SM-P205",         "Samsung",     "Galaxy Tab A 8.0",      "SM-P205",      "Exynos7904"},
	{"SM-T580",         "Samsung",     "Galaxy Tab A 10.1 WiFi","SM-T580",      "Exynos7870"},
	{"SM-T813",         "Samsung",     "Galaxy Tab S2 9.7 WiFi","SM-T813",      "MSM8976"},
	{"SM801",           "Smartisan",   "T2",                    "SM801",        "MSM8992"},
	{"STF-AL00",        "Huawei",      "Honor 9",               "STF-AL00",     "Kirin 960"},
	{"STK-L22",         "Huawei",      "Y9 Prime",              "STK-L22",      "Kirin 710F"},
	{"STK-LX3",         "Huawei",      "Y9 Prime",              "STK-LX3",      "Kirin 710F"},
	{"SUGAR Y12s",      "SUGAR",       "Y12s",                  "SUGAR-Y12s",   "MT6739WW"},
	{"TA-1003",         "Nokia",       "Nokia 6 Dual SIM",      "TA-1003",      "MSM8937"},
	{"TRT-TL10",        "Huawei",      "Y7 Prime",              "TRT-TL10",     "MSM8940"},
	{"VCE-L22",         "Huawei",      "nova 4",                "VCE-L22",      "Kirin 970"},
	{"VIE-L29",         "Huawei",      "P9 Plus Dual SIM",      "VIE-L29",      "Kirin 955"},
	{"VKY-L29",         "Huawei",      "P10 Plus Dual SIM",     "VKY-L29",      "Kirin 960"},
	{"VNS-L31",         "Huawei",      "P9 lite",               "VNS-L31",      "Kirin 650"},
	{"VOG-L29",         "Huawei",      "P30 Pro Dual SIM",      "VOG-L29",      "Kirin 980"},
	{"WAS-TL10",        "Huawei",      "P10 Lite",              "WAS-TL10",     "Kirin 658"},
	{"YAL-L21",         "Huawei",      "Honor 20 Dual SIM",     "YAL-L21",      "Kirin 980"},
	{"ZTE BLADE A512",  "ZTE",         "Blade A512",            "A512",         "MSM8917"},
	{"vivo 1801",       "vivo",        "Y71i",                  "1801",         "MSM8917"},
	{"vivo 1807",       "vivo",        "Y95",                   "1807",         "SDM439"},
	{"vivo 1811",       "vivo",        "Y91",                   "1811",         "SDM439"},
	{"vivo 1901",       "vivo",        "Y17",                   "1901",         "MT6765"},
	{"vivo 1904",       "vivo",        "Y12",                   "1904",         "MT6762"},
	{"vivo 1909",       "vivo",        "V17 Pro",               "1909",         "SM6150"},
};

const IO::AndroidDB::AndroidInfo *IO::AndroidDB::GetAndroidInfo(const UTF8Char *androidId)
{
	OSInt i = 0;
	OSInt j = (sizeof(androids) / sizeof(androids[0])) - 1;
	OSInt k;
	OSInt l;

	while (i <= j)
	{
		k = (i + j) >> 1;
		l = Text::StrCompare(androidId, (const UTF8Char*)androids[k].androidId);
		if (l > 0)
		{
			i = k + 1;
		}
		else if (l < 0)
		{
			j = k - 1;
		}
		else
		{
			return &androids[k];
		}
	}
	return 0;
}
