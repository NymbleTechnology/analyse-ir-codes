/*
    @file analyse-ir-codes.h

    Copyright 2011, Paul Chambers. All Rights Reserved.
*/

#define STRING_HASH_STEP(hash, ch) ((hash * 33) ^ (ch))

typedef struct {
    const char *    myName;
    const char *    version;
    struct {
        const char *built, *expiries;
    } date;
    time_t   expiryTimestamp;
} tGlobals;
extern tGlobals globals;

typedef enum {
    kDeviceTupeUnknown = 0,
#define defineDeviceType(id,string)  id,
#include "devicetypemapping.h"
#undef  defineDeviceType
    kDeviceTypeMax
} tDeviceType;

/* indexed by tDeviceType */
extern const char *gDeviceTypeName[];

typedef enum {
    kBrandUnknown = 0,
#define defineBrand(id,string)  id,
#include "brandmapping.h"
#undef  defineBrand
    kBrandMax
} tBrand;

/* indexed by tBrand */
extern const char *gBrandName[];

typedef enum {
    kUnknownRepeat,
    kFullRepeat,
    kPartialRepeat,
    kRepeat,
    kToggleRepeat
} tRepeatType;

typedef enum {
#define defineRepeatStream(id,...)  k ## id ## RepeatStream,
#include "repeatstreammapping.h"
#undef defineRepeatStream
    kMaxRepeatStream
} tRepeatStream;

/*
    tPeriod is a poor man's fixed point type.
*/

typedef unsigned long tPeriod, tCount;

typedef struct {
    tPeriod period;
    tCount  count;
} tHistEntry;

/* variable size */
typedef struct
{
    tCount      count;
    tHistEntry  d[];

} tHistogram;

/* fixed size - used for temporary storage during analysis */
/* valid data will be way less than this */
#define MAX_HIST_SIZE   100
typedef struct
{
    tCount      count;
    tHistEntry  d[MAX_HIST_SIZE];
    
} tRawHistogram;

typedef unsigned int tReferenceHistogram[4];

typedef enum {
    kUnknown = 0,
    kMarkVaries,            /* older protocols, almost always Sony */
    kSpaceVaries,           /* very common */
    kBiphase,               /* the Philips protocols, usually */
    kPPM,                   /* unusual, often IR keyboards */
                    /*--- constants up to this point also match 'kAmbiguous' ---*/
    kAmbiguous,             /* pathological cases - could be any one of the preceeding encodings */
                    /*--- the following constants cannot match 'kAmbiguous' ---*/
    kMarkVariesExtended,    /* unlikely, but not impossible */
    kSpaceVariesExtended,   /* unusual, and ambiguous with corner cases of PPM */
    kBiphaseExtended        /* biphase with an invalid long symbol in the middle, e.g. RC-5x */
} tEncoding;

/* variable size version of this structure*/
typedef struct tIRStream
{
    tCount          count;
    unsigned long   period[];   /* NOT tPeriods - these are not scaled */

} tIRStream;

/* fixed size - used for temporary storage during import */
#define MAX_RAW_IR_COUNT 200
typedef struct tRawIRStream
{
    tCount          count;
    unsigned long   period[MAX_RAW_IR_COUNT];   /* NOT tPeriods - these are not scaled */
    
} tRawIRStream;

#define SYMBOL_ARRAY_SIZE 4
typedef struct
{
    enum { /* in order of increasing confidence */
        kListEnd,
        kFromDB,
        kMeasured,
        kFromSpec
    } confidence;

    const char   *name;
    
    tEncoding   encoding;

    int symbolCounts[SYMBOL_ARRAY_SIZE];

    unsigned long   carrierFreq;        /* 0 not set */

    struct {
        /* these are not tPeriods, to make the static initializers less cluttered */
        unsigned long mark;     /* zero if a valid symbol */
        unsigned long space;    /* zero if a valid symbol */
    } leading;

    unsigned long   duration;   /* sum of the periods */
    
    tReferenceHistogram mark, space;
    
    tRepeatStream   repeatStream;   /* only non-zero for codes with a fixed 'repeat' stream - i.e. NEC */

    int matched;    /* counter for number of successful matches */

} tReferenceFingerprint;

typedef struct
{
    tEncoding       encoding;

    unsigned int    symbolCount;

    unsigned long   carrierFreq;        /* 0 not set */
    
    tRepeatType     repeatType;

    struct {
        unsigned long mark;     /* zero if a valid symbol */
        unsigned long space;    /* zero if a valid symbol */
    } leading, trailing;

    unsigned long    duration;  /* sum of the periods */
    
    tHistogram      *mark, *space;
    
    tReferenceFingerprint *protocol;       /* from protocol template array */

} tFingerprint;


typedef struct tIRCode
{
    struct tIRCode *next, *nextA;
    struct tIRCodeSet *parent;

    unsigned int    lineNumber; /* in the input file. Useful for error reporting */

    tFingerprint    fingerprint;
    
    struct {
        char *label;
        /* int action; */
    } button;

    struct {
    tIRStream  *a;
    tIRStream  *b;
    } first, repeat;
    
} tIRCode;

typedef struct tIRCodeSet
{
    struct tIRCodeSet *next;

    unsigned int    id;
    tDeviceType     deviceType;
    tBrand          brand;

    tIRCode *irCodes, *lastIrCode;

} tIRCodeSet;

extern tIRCodeSet   *gIRCodeSets;
extern tIRCode      *gIRCodes;
