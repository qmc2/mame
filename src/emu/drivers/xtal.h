// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    xtal.h

    Documentation and consistent naming for known existing crystals.

***************************************************************************

    PCB Crystal Reference
    ----------------------
       _____     ________
       |16 |    |o       |
       |MHZ|    |16.0MHZ |
       |___|    |________|
       |   |

    There is a finite list of _manufactured_ crystals. This file aims
    to document all of the known speeds that crystals were available in.
    Feel free to add to the list if you find another speed crystal on
    a PCB, but please DON'T GUESS!

    Remember that some very high frequencies (typically above 100MHz) are
    generated by a frequency multiplying IC from a lower frequency
    crystal.

    This is intended as a reference of existing parts to prevent incorrect
    clock speed measurements with digital frequency counters being used
    in drivers. When measuring clocks, remember that most parts have a
    tolerance so be sure to reference existing parts only and not just
    accept direct readings as 100% true.

    MAME doesn't yet support fractions in crystal frequencies. For example,
    XTAL_3_579545MHz should actually be 3579545.454545...Hz (39375000/11).
    This is no problem though: see above note about tolerance.

    (Thanks to Guru for starting this documentation.)

**************************************************************************/

#ifndef MAME_EMU_DRIVERS_XTAL_H
#define MAME_EMU_DRIVERS_XTAL_H

#pragma once

enum
{
/*
    Name                = Frequency     Examples
    ------------------    ------------  ------------------------------------------------------------ */
	XTAL_32_768kHz      = 32768,        /* 32.768kHz, used to drive RTC chips */
	XTAL_1MHz           = 1000000,      /* Used to drive OKI M6295 chips */
	XTAL_1_2944MHz      = 1294400,      /* BBN BitGraph PSG */
	XTAL_1_75MHz        = 1750000,      /* RCA CDP1861 */
	XTAL_1_8432MHz      = 1843200,      /* Bondwell 12/14 */
	XTAL_1_9968MHz      = 1996800,      /* NEC PC-98xx */
	XTAL_2MHz           = 2000000,
	XTAL_2_01216MHz     = 2012160,      /* Cidelsa Draco sound board */
	XTAL_2_097152MHz    = 2097152,      /* Icatel 1995 - Brazilian public payphone */
	XTAL_2_4576MHz      = 2457600,      /* Atari ST MFP, NEC PC-98xx */
	XTAL_2_5MHz         = 2500000,      /* Janken Man units */
	XTAL_3MHz           = 3000000,      /* Probably only used to drive 68705 or similar MCUs on 80's Taito PCBs */
	XTAL_3_072MHz       = 3072000,      /* INS 8520 input clock rate */
	XTAL_3_12MHz        = 3120000,      /* SP0250 clock on Gottlieb games */
	XTAL_3_5MHz         = 3500000,      /* Reported by Commodore 65 document, true xtal unchecked on PCB */
	XTAL_3_52128MHz     = 3521280,      /* RCA COSMAC VIP */
	XTAL_3_57864MHz     = 3578640,      /* Atari Portfolio PCD3311T */
	XTAL_3_579545MHz    = 3579545,      /* NTSC color subcarrier, extremely common, used on 100's of PCBs (Keytronic custom part #48-300-010 is equivalent) */
	XTAL_3_6864MHz      = 3686400,      /* CPS3 */
	XTAL_4MHz           = 4000000,
	XTAL_4_028MHz       = 4028000,      /* Sony SMC-777 */
	XTAL_4_096MHz       = 4096000,      /* Used to drive OKI M9810 chips */
	XTAL_4_194304Mhz    = 4194304,      /* Used to drive MC146818 / Nintendo Game Boy */
	XTAL_4_224MHz       = 4224000,      /* Used to drive OKI M6295 chips, usually with /4 divider */
	XTAL_4_41MHz        = 4410000,      /* Pioneer PR-8210 ldplayer */
	XTAL_4_43361MHz     = 4433610,      /* Cidelsa Draco */
	XTAL_4_433619MHz    = 4433619,      /* PAL color subcarrier (technically 4.43361875mhz)*/
	XTAL_4_608MHz       = 4608000,      /* Luxor ABC-77 keyboard (Keytronic custom part #48-300-107 is equivalent) */
	XTAL_4_9152MHz      = 4915200,
	XTAL_5MHz           = 5000000,      /* Mutant Night */
	XTAL_5_0688MHz      = 5068800,      /* Xerox 820, DEC VK100 */
	XTAL_5_46MHz        = 5460000,      /* ec1840 and ec1841 keyboard */
	XTAL_5_626MHz       = 5626000,      /* RCA CDP1869 PAL dot clock */
	XTAL_5_67MHz        = 5670000,      /* RCA CDP1869 NTSC dot clock */
	XTAL_5_7143MHz      = 5714300,      /* Cidelsa Destroyer */
	XTAL_5_911MHz       = 5911000,      /* Philips Videopac Plus G7400 */
	XTAL_5_9904MHz      = 5990400,      /* Luxor ABC 800 keyboard (Keytronic custom part #48-300-008 is equivalent) */
	XTAL_6MHz           = 6000000,      /* American Poker II, Taito SJ System */
	XTAL_6_144MHz       = 6144000,      /* Used on Alpha Denshi early 80's games sound board, Casio FP-200 and Namco System 16 */
	XTAL_6_5MHz         = 6500000,      /* Jupiter Ace */
	XTAL_6_9MHz         = 6900000,      /* BBN BitGraph CPU */
	XTAL_7MHz           = 7000000,      /* Jaleco Mega System PCBs */
	XTAL_7_15909MHz     = 7159090,      /* Blood Bros (2x NTSC subcarrier) */
	XTAL_7_3728MHz      = 7372800,
	XTAL_7_8643MHz      = 7864300,      /* Used on InterFlip games as video clock */
	XTAL_8MHz           = 8000000,      /* Extremely common, used on 100's of PCBs */
	XTAL_8_2MHz         = 8200000,      /* Universal Mr. Do - Model 8021 PCB */
	XTAL_8_388MHz       = 8388000,      /* Nintendo Game Boy Color */
	XTAL_8_4672MHz      = 8467200,      /* Subsino's Ying Hua Lian */
	XTAL_8_664MHz       = 8664000,      /* Touchmaster */
	XTAL_8_7MHz         = 8700000,      /* Tandberg TDV 2324 */
	XTAL_8_867236MHz    = 8867236,      /* RCA CDP1869 PAL color clock (~2x PAL subcarrier) */
	XTAL_8_867238MHz    = 8867238,      /* ETI-660 (~2x PAL subcarrier) */
	XTAL_8_945MHz       = 8945000,      /* Hit Me */
	XTAL_9_216MHz       = 9216000,      /* Conitec PROF-180X */
	XTAL_9_828MHz       = 9828000,      /* Universal PCBs */
	XTAL_9_8304MHz      = 9830400,      /* Epson PX-8 */
	XTAL_9_987MHz       = 9987000,      /* Crazy Balloon */
	XTAL_10MHz          = 10000000,
	XTAL_10_245MHz      = 10245000,     /* PES Speech box */
	XTAL_10_595MHz      = 10595000,     /* Mad Alien */
	XTAL_10_6875MHz     = 10687500,     /* BBC Bridge Companion */
	XTAL_10_69425MHz    = 10694250,     /* Xerox 820 */
	XTAL_10_730MHz      = 10730000,     /* Ruleta RE-900 VDP Clock */
	XTAL_10_733MHz      = 10733000,     /* The Fairyland Story */
	XTAL_10_738635MHz   = 10738635,     /* TMS9918 family (3x NTSC subcarrier) */
	XTAL_10_816MHz      = 10816000,     /* Universal 1979-1980 (Cosmic Alien, etc) */
	XTAL_11MHz          = 11000000,     /* Mario I8039 sound */
	XTAL_11_0592MHz     = 11059200,     /* Lethal Justice, Ruleta RE-900, DEC VT220 */
	XTAL_11_2MHz        = 11200000,     /* New York, New York */
	XTAL_11_289MHz      = 11289000,     /* Vanguard */
	XTAL_11_4MHz        = 11400000,     /* HP 9845 */
	XTAL_11_6688MHz     = 11668800,     /* Gameplan pixel clock */
	XTAL_11_8MHz        = 11800000,     /* IBM PC Music Feature Card */
	XTAL_11_9808MHz     = 11980800,     /* Luxor ABC 80 */
	XTAL_12MHz          = 12000000,     /* Extremely common, used on 100's of PCBs */
	XTAL_12_096MHz      = 12096000,     /* Some early 80's Atari games */
	XTAL_12_288MHz      = 12288000,     /* Sega Model 3 digital audio board */
	XTAL_12_432MHz      = 12432000,     /* Kaneko Fly Boy/Fast Freddie Hardware */
	XTAL_12_4725MHz     = 12472500,     /* Bonanza's Mini Boy 7 */
	XTAL_12_48MHz       = 12480000,     /* TRS-80 Model II */
	XTAL_12_5MHz        = 12500000,     /* Red Alert audio board */
	XTAL_12_8MHz        = 12800000,     /* Cave CV1000 */
	XTAL_12_9792MHz     = 12979200,     /* Exidy 440 */
	XTAL_13_3MHz        = 13300000,     /* BMC bowling */
	XTAL_13_33056MHz    = 13330560,     /* Taito L */
	XTAL_13_4MHz        = 13400000,     /* TNK3, Ikari Warriors h/w */
	XTAL_13_4952MHz     = 13495200,     /* Used on Shadow Force pcb and maybe other Technos pcbs? */
	XTAL_14MHz          = 14000000,
	XTAL_14_112MHz      = 14112000,     /* Timex/Sinclair TS2068 */
	XTAL_14_314MHz      = 14314000,     /* Taito TTL Board  */
	XTAL_14_31818MHz    = 14318181,     /* Extremely common, used on 100's of PCBs (4x NTSC subcarrier) */
	XTAL_14_705882MHz   = 14705882,     /* Aleck64 */
	XTAL_14_7456MHz     = 14745600,     /* Namco System 12 & System Super 22/23 for JVS */
	XTAL_15MHz          = 15000000,     /* Sinclair QL, Amusco Poker */
	XTAL_15_36MHz       = 15360000,     /* Visual 1050 */
	XTAL_15_4MHz        = 15400000,     /* DVK KSM */
	XTAL_15_468MHz      = 15468480,     /* Bank Panic h/w, Sega G80 */
	XTAL_15_8976MHz     = 15897600,     /* IAI Swyft */
	XTAL_16MHz          = 16000000,     /* Extremely common, used on 100's of PCBs */
	XTAL_16_384MHz      = 16384000,
	XTAL_16_5888MHz     = 16588800,     /* SM 7238 */
	XTAL_16_67MHz       = 16670000,
	XTAL_16_777216MHz   = 16777216,     /* Nintendo Game Boy Advance */
	XTAL_16_9344MHz     = 16934400,     /* Usually used to drive 90's Yamaha OPL/FM chips (44100 * 384) */
	XTAL_17_36MHz       = 17360000,     /* OMTI Series 10 SCSI controller */
	XTAL_17_73447MHz    = 17734470,     /* (~4x PAL subcarrier) */
	XTAL_17_734472MHz   = 17734472,     /* actually ~4x PAL subcarrier */
	XTAL_17_9712MHz     = 17971200,
	XTAL_18MHz          = 18000000,     /* S.A.R, Ikari Warriors 3 */
	XTAL_18_432MHz      = 18432000,     /* Extremely common, used on 100's of PCBs (48000 * 384) */
	XTAL_18_720MHz      = 18720000,     /* Nokia MikroMikko 1 */
	XTAL_19_6MHz        = 19600000,     /* Universal Mr. Do - Model 8021 PCB */
	XTAL_19_6608MHz     = 19660800,     /* Euro League (bootleg), labeled as "UKI 19.6608 20PF" */
	XTAL_19_923MHz      = 19923000,     /* Cinematronics vectors */
	XTAL_19_968MHz      = 19968000,     /* Used mostly by some Taito games */
	XTAL_20MHz          = 20000000,
	XTAL_20_16MHz       = 20160000,     /* Nintendo 8080 */
	XTAL_20_079MHz      = 20790000,     /* Blockade-hardware Gremlin games */
	XTAL_20_625MHz      = 20625000,     /* SM 7238 */
	XTAL_21MHz          = 21000000,     /* Lock-On pixel clock */
	XTAL_21_3MHz        = 21300000,
	XTAL_21_4772MHz     = 21477272,     /* BMC bowling, some Data East 90's games, Vtech Socrates; (6x NTSC subcarrier) */
	XTAL_22MHz          = 22000000,
	XTAL_22_3210MHz     = 22321000,     /* Apple LaserWriter II NT */
	XTAL_22_1184MHz     = 22118400,     /* Amusco Poker */
	XTAL_23_9616MHz     = 23961600,     /* Osborne 4 (Vixen) */
	XTAL_24MHz          = 24000000,     /* Mario, 80's Data East games, 80's Konami games */
	XTAL_24_0734MHz     = 24073400,     /* DEC Rainbow 100 */
	XTAL_24_576MHz      = 24576000,     /* Pole Position h/w, Model 3 CPU board */
	XTAL_24_8832MHz     = 24883200,     /* DEC VT100 */
	XTAL_25MHz          = 25000000,     /* Namco System 22, Taito GNET, Dogyuun h/w */
	XTAL_25_1748MHz     = 25174800,     /* Sega S16A, S16B */
	XTAL_25_39836MHz    = 25398360,     /* Tandberg TDV 2324 */
	XTAL_25_447MHz      = 25447000,     /* Namco EVA3A (Funcube2) */
	XTAL_25_590906MHz   = 25590906,     /* Atari Jaguar NTSC */
	XTAL_25_593900MHz   = 25593900,     /* Atari Jaguar PAL */
	XTAL_26_601712MHz   = 26601712,     /* Astro Corp.'s Show Hand, PAL Vtech/Yeno Socrates (6x PAL subcarrier) */
	XTAL_26_66666MHz    = 26666666,     /* Irem M92 but most use 27MHz */
	XTAL_26_686MHz      = 26686000,     /* Typically used on 90's Taito PCBs to drive the custom chips */
	XTAL_27MHz          = 27000000,     /* Some Banpresto games macrossp, Irem M92 and 90's Toaplan games */
	XTAL_27_164MHz      = 27164000,     /* Typically used on 90's Taito PCBs to drive the custom chips */
	XTAL_27_2109MHz     = 27210900,     /* LA Girl */
	XTAL_28MHz          = 28000000,
	XTAL_28_322MHz      = 28322000,     /* Saitek RISC 2500, Mephisto Montreux */
	XTAL_28_37516MHz    = 28375160,     /* Amiga PAL systems */
	XTAL_28_48MHz       = 28480000,     /* Chromatics CGC-7900 */
	XTAL_28_63636MHz    = 28636363,     /* Later Leland games and Atari GT, Amiga NTSC, Raiden2 h/w (8x NTSC subcarrier)*/
	XTAL_28_64MHz       = 28640000,     /* Fukki FG-1c AI AM-2 PCB */
	XTAL_28_7MHz        = 28700000,
	XTAL_29_4912MHz     = 29491200,     /* Xerox Alto-II system clock (tagged 29.4MHz in the schematics) */
	XTAL_30MHz          = 30000000,     /* Impera Magic Card */
	XTAL_30_4761MHz     = 30476100,     /* Taito JC */
	XTAL_30_8MHz        = 30800000,     /* 15IE-00-013 */
	XTAL_32MHz          = 32000000,
	XTAL_32_22MHz       = 32220000,     /* Typically used on 90's Data East PCBs (close to 9x NTSC subcarrier which is 32.215905Mhz*/
	XTAL_32_5304MHz     = 32530400,     /* Seta 2 */
	XTAL_33MHz          = 33000000,     /* Sega Model 3 video board */
	XTAL_33_264MHz      = 33264000,     /* Hazeltine 1500 terminal */
	XTAL_33_333MHz      = 33333000,     /* Sega Model 3 CPU board, Vegas */
	XTAL_33_833MHz      = 33833000,
	XTAL_33_8688MHz     = 33868800,     /* Usually used to drive 90's Yamaha OPL/FM chips with /2 divider */
	XTAL_36MHz          = 36000000,     /* Sega Model 1 video board */
	XTAL_38_76922MHz    = 38769220,     /* Namco System 21 video board */
	XTAL_40MHz          = 40000000,
	XTAL_42MHz          = 42000000,     /* BMC A-00211 - Popo Bear */
	XTAL_42_9545MHz     = 42954545,     /* CPS3 (12x NTSC subcarrier)*/
	XTAL_44_1MHz        = 44100000,     /* Subsino's Bishou Jan */
	XTAL_45MHz          = 45000000,     /* Eolith with Hyperstone CPUs */
	XTAL_45_158MHz      = 45158000,     /* Sega Model 2A video board, Model 3 CPU board */
	XTAL_45_6192Mhz     = 45619200,     /* DEC VK100 */
	XTAL_48MHz          = 48000000,     /* Williams/Midway Y/Z-unit system / SSV board */
	XTAL_48_384MHz      = 48384000,     /* Namco NB-1 */
	XTAL_48_66MHz       = 48660000,     /* Zaxxon */
	XTAL_49_152MHz      = 49152000,     /* Used on some Namco PCBs, Baraduke h/w, System 21, Super System 22  */
	XTAL_50MHz          = 50000000,     /* Williams/Midway T/W/V-unit system */
	XTAL_50_113MHz      = 50113000,     /* Namco NA-1 (14x NTSC subcarrier)*/
	XTAL_52MHz          = 52000000,     /* Cojag */
	XTAL_52_832MHz      = 52832000,     /* Wang PC TIG video controller */
	XTAL_53_693175MHz   = 53693175,     /* PSX-based h/w, Sony ZN1-2-based (15x NTSC subcarrier) */
	XTAL_54MHz          = 54000000,     /* Taito JC */
	XTAL_55MHz          = 55000000,     /* Eolith Vega */
	XTAL_57_2727MHz     = 57272727,     /* Psikyo SH2 with /2 divider (16x NTSC subcarrier)*/
	XTAL_60MHz          = 60000000,
	XTAL_61_44MHz       = 61440000,     /* dkong */
	XTAL_64MHz          = 64000000,     /* BattleToads */
	XTAL_66_6667MHz     = 66666700,     /* Later Midway games */
	XTAL_67_7376MHz     = 67737600,     /* PSX-based h/w, Sony ZN1-2-based */
	XTAL_72MHz          = 72000000,     /* Aristocrat MKV */
	XTAL_72_576MHz      = 72576000,     /* Centipede, Millipede, Missile Command, Let's Go Bowling "Multipede" */
	XTAL_73_728MHz      = 73728000,     /* Ms. Pac-Man/Galaga 20th Anniversary */
	XTAL_100MHz         = 100000000,    /* PSX-based Namco System 12, Vegas, Sony ZN1-2-based */
	XTAL_101_4912MHz    = 101491200,    /* PSX-based Namco System 10 */
	XTAL_200MHz         = 200000000,    /* Base SH4 CPU (Naomi, Hikaru etc.) */

/* Resonators (There are probably more) */

	XTAL_384kHz         = 384000,       /* Commonly used for driving OKI MSM5205 */
	XTAL_400kHz         = 400000,       /* OKI MSM5205 on Great Swordman h/w */
	XTAL_430kHz         = 430000,
	XTAL_455kHz         = 455000,       /* OKI MSM5205 on Gladiator h/w */
	XTAL_512kHz         = 512000,       /* Toshiba TC8830F */
	XTAL_600kHz         = 600000,
	XTAL_640kHz         = 640000,       /* NEC UPD7759, Texas Instruments Speech Chips @ 8khz */
	XTAL_960kHz         = 960000,       /* Xerox Notetaker Keyboard UART */
	XTAL_1_056MHz       = 1056000       /* OKI M6295 on Trio The Punch h/w */
};


/*

For further reference:

A search at http://search.digikey.com/scripts/DkSearch/dksus.dll?Cat=852333;keywords=cry
reveals the following shipping frequencies as of 1/1/2008:

20kHz
25.600kHz
26.667kHz
28kHz

30kHz
30.720kHz
30.76kHz
31.2kHz
31.25kHz
31.5kHz
32.000kHz
32.56kHz
32.768kHz
32.919kHz
34kHz
36kHz
38kHz
38.4kHz
39.500kHz

40kHz
44.100kHz
46.604kHz
46.6084kHz

50kHz
59.787kHz

60.000kHz
60.002kHz
60.005kHz
65.535kHz
65.536kHz
69kHz

70kHz
71kHz
72kHz
73kHz
74kHz
74.3kHz
74.4kHz
75kHz
76kHz
76.79kHz
76.8kHz
76.81kHz
77kHz
77.204kHz
77.287kHz
77.500kHz
77.503kHz
77.504kHz
78kHz
79kHz

83kHz

96kHz
96.006kHz

100kHz
111kHz
117.72kHz
120kHz
120.8475kHz
125kHz
131.072kHz
149.475kHz
153.600kHz

200kHz

307.2kHz

1.000MHz
1.8432MHz

2.000MHz
2.048MHz
2.097152MHz
2.4576MHz
2.5MHz
2.560MHz
2.949120MHz

3.000MHz
3.276MHz
3.2768MHz
3.579MHz
3.579545MHz
3.640MHz
3.6864MHz
3.700MHz
3.859MHz
3.93216MHz

4.000MHz
4.032MHz
4.096MHz
4.09625MHz
4.194MHz
4.194304MHz
4.332MHz
4.433MHz
4.433616MHz
4.433618MHz
4.433619MHz
4.74687MHz
4.800MHz
4.8970MHz
4.90625MHz
4.915MHz
4.9152MHz

5.000MHz
5.0688MHz
5.120MHz
5.185MHz
5.223438MHz
5.5MHz
5.5296MHz
5.9904MHz

6.000MHz
6.14MHz
6.144MHz
6.1760MHz
6.400 MHz
6.49830MHz
6.5MHz
6.5536MHz
6.612813MHz
6.7458MHz
6.757MHz
6.76438MHz

7.1505MHz
7.15909 MHz
7.2MHz
7.3728MHz
7.68MHz
7.94888MHz

8.000MHz
8.000156MHz
8.192MHz
8.388608MHz
8.432MHz
8.5MHz
8.6432MHz

9.000MHz
9.216MHz
9.509375MHz
9.545MHz
9.6MHz
9.7941MHz
9.830MHz
9.8304MHz
9.84375MHz
9.8438MHz

10.000MHz
10.240MHz
10.245MHz
10.6244MHz
10.738635MHz
10.73865MHz

11.000MHz
11.046MHz
11.0592MHz
11.228MHz
11.2896MHz
11.520MHz
11.981350MHz

12.000MHz
12.000393MHz
12.096MHz
12.1875MHz
12.288MHz
12.352MHz
12.500MHz
12.688MHz
12.800MHz
12.96MHz

13.000MHz
13.0625MHz
13.225MHz
13.2256MHz
13.500MHz
13.5168MHz
13.56MHz
13.605MHz
13.824MHz
13.94916MHz

14.00MHz
14.318MHz
14.31818MHz
14.3359MHz
14.3594MHz
14.4MHz
14.5MHz
14.69MHz
14.7456MHz
14.850MHz

15MHz
15.360MHz

16.000MHz
16.000312MHz
16.128MHz
16.257MHz
16.3676MHz
16.368MHz
16.384MHz
16.576MHz
16.6660MHz
16.667MHz
16.670MHz
16.800MHz
16.934MHz
16.9344MHz

17.734475MHz

18.000MHz
18.432MHz
18.869MHz

19.200MHz
19.440MHz
19.660MHz
19.6608MHz
19.68MHz
19.800MHz

20.000MHz
20.35625MHz
20.3563MHz
20.480MHz

21.47727MHz

22.000MHz
22.118MHz
22.1184MHz
22.400MHz
22.5MHz
22.5792MHz
22.6278MHz

23MHz
23.2643MHz
23.5MHz
23.5122MHz
23.592MHz

24.000MHz
24.00014MHz
24.5MHz
24.545454 MHz
24.5535MHz
24.576MHz
24.704MHz
24.7456MHz

25.000MHz
25MHz
25.175MHz
25.2235MHz
25.4563MHz
25.5MHz

26.000MHz
26.45125MHz
26.4513MHz
26.5MHz
26.5971MHz
26.800MHz

27.000MHz
27.1344MHz
27.3067MHz
27.4688MHz

28.000MHz
28.224MHz
28.259375MHz
28.2594MHz
28.322MHz
28.375MHz
28.5938MHz
28.636MHz
28.6363MHz
28.63636MHz

29.4912MHz
29.498928MHz
29.500MHz

30.000MHz
32.000MHz
32.514MHz
32.768MHz
33.000MHz
33.333MHz
33.3333MHz
33.8688MHz
35.2512MHz
35.3280MHz
36.000MHz
38.000MHz
38.00053MHz
38.400MHz
38.880MHz
39MHz

40.000MHz
40.320MHz
40.960 MHz
42.000MHz
44.000MHz
44.2368MHz
44.545MHz
44.736MHz
44.800MHz
44.900MHz
45.000MHz
46.000MHz
48.000MHz
49.152MHz
49.86MHz

50.000MHz
53.125MHz
55.000MHz

60.000MHz
64.000MHz
66.000MHz
66.666MHz
66.6666MHz

73.66979MHz
75.957292MHz
76.121875MHz

80.000MHz

100.00MHz

*/

#endif // MAME_EMU_DRIVERS_XTAL_H
