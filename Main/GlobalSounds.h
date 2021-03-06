#ifndef __XACT_PROJECT_GLOBALSOUNDS__
#define __XACT_PROJECT_GLOBALSOUNDS__

typedef enum
{
    XACT_VARIABLE_NUMCUEINSTANCES = 0,
    XACT_VARIABLE_ATTACKTIME = 1,
    XACT_VARIABLE_RELEASETIME = 2,
    XACT_VARIABLE_ORIENTATIONANGLE = 3,
    XACT_VARIABLE_DOPPLERPITCHSCALAR = 4,
    XACT_VARIABLE_SPEEDOFSOUND = 5,
    XACT_VARIABLE_DISTANCE = 6,
} XACT_VARIABLE;

#define XACT_VARIABLE_COUNT 7

typedef enum
{
    XACT_WAVEBANK_CAR_HORN3 = 0,
    XACT_WAVEBANK_CAR_HORN1 = 1,
    XACT_WAVEBANK_CAR_HORN2 = 2,
    XACT_WAVEBANK_CAR_BEEPBEP = 3,
    XACT_WAVEBANK_CAR_CLOP = 4,
    XACT_WAVEBANK_CAR_WINDOWSXPERROR = 5,
    XACT_WAVEBANK_CAR_ATTACK1 = 6,
    XACT_WAVEBANK_CAR_GODLIKE = 7,
    XACT_WAVEBANK_CAR_DOH = 8,
    XACT_WAVEBANK_CAR_DOMINATING = 9,
    XACT_WAVEBANK_CAR_LOSTMATCH = 10,
    XACT_WAVEBANK_CAR_RAMPAGE = 11,
    XACT_WAVEBANK_CAR_SUCKER = 12,
    XACT_WAVEBANK_CAR_UNSTOPPABLE = 13,
    XACT_WAVEBANK_CAR_YEEHAW = 14,
    XACT_WAVEBANK_CAR_YOUSUCK = 15,
    XACT_WAVEBANK_CAR_1 = 16,
    XACT_WAVEBANK_CAR_2 = 17,
    XACT_WAVEBANK_CAR_3 = 18,
    XACT_WAVEBANK_CAR_4 = 19,
    XACT_WAVEBANK_CAR_5 = 20,
    XACT_WAVEBANK_CAR_6 = 21,
    XACT_WAVEBANK_CAR_7 = 22,
    XACT_WAVEBANK_CAR_8 = 23,
    XACT_WAVEBANK_CAR_9 = 24,
    XACT_WAVEBANK_CAR_10 = 25,
    XACT_WAVEBANK_CAR_WINNERCONGRAT = 26,
    XACT_WAVEBANK_CAR_PREPARE = 27,
    XACT_WAVEBANK_CAR_30SEC = 28,
    XACT_WAVEBANK_CAR_1MIN = 29,
    XACT_WAVEBANK_CAR_3MIN = 30,
    XACT_WAVEBANK_CAR_5MIN = 31,
} XACT_WAVEBANK_CAR;

#define XACT_WAVEBANK_CAR_ENTRY_COUNT 32

typedef enum
{
    XACT_CUE_CAR_CARHORN1 = 0,
    XACT_CUE_CAR_CARHORN2 = 1,
    XACT_CUE_CAR_CARHORN3 = 2,
    XACT_CUE_CAR_MOUSECLICK = 3,
    XACT_CUE_CAR_ERROR = 4,
    XACT_CUE_CAR_ATTACK = 5,
    XACT_CUE_CAR_GODLIKE = 6,
    XACT_CUE_CAR_HOMMERDOH = 7,
    XACT_CUE_CAR_DOMINATING = 8,
    XACT_CUE_CAR_LOSTMATCH = 9,
    XACT_CUE_CAR_PREPARE = 10,
    XACT_CUE_CAR_WINNERCONGRAT = 11,
    XACT_CUE_CAR_YOUSUCK = 12,
    XACT_CUE_CAR_YOUHAW = 13,
    XACT_CUE_CAR_SUCKER = 14,
    XACT_CUE_CAR_UNSTOPABLE = 15,
    XACT_CUE_CAR_RAMPAGE = 16,
    XACT_CUE_CAR_COUNTDOWN1 = 17,
    XACT_CUE_CAR_COUNTDOWN2 = 18,
    XACT_CUE_CAR_COUNTDOWN3 = 19,
    XACT_CUE_CAR_COUNTDOWN4 = 20,
    XACT_CUE_CAR_COUNTDOWN5 = 21,
    XACT_CUE_CAR_COUNTDOWN6 = 22,
    XACT_CUE_CAR_COUNTDOWN7 = 23,
    XACT_CUE_CAR_COUNTDOWN8 = 24,
    XACT_CUE_CAR_COUNTDOWN9 = 25,
    XACT_CUE_CAR_COUNTDOWN10 = 26,
    XACT_CUE_CAR_COUNTDOWN1MIN = 27,
    XACT_CUE_CAR_COUNTDOWN30SEC = 28,
    XACT_CUE_CAR_COUNTDOWN3MIN = 29,
    XACT_CUE_CAR_COUNTDOWN5MIN = 30,
    XACT_CUE_CAR_CARHORNRANDOM = 31,
    XACT_CUE_CAR_GOAL = 32,
} XACT_CUE_CAR;

#define XACT_CUE_CAR_ENTRY_COUNT 33

#endif // __XACT_PROJECT_GLOBALSOUNDS__
