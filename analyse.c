/*
    @file analyse.c
    
    Algorithms to analyse IR codes

    Copyright 2011, Paul Chambers. All Rights Reserved.
*/

#include "common.h"

#include "analyse-ir-codes.h"
#include "analyse.h"


/* indexed by tDeviceType */
const char *gDeviceTypeName[] = 
{
    "unknown device",
#define defineDeviceType(id,string)  string,
#include "devicetypemapping.h"
#undef  defineDeviceType
    NULL
};

/* indexed by tBrand */
const char *gBrandName[] = 
{
    "unknown brand",
#define defineBrand(id,string)  string,
#include "brandmapping.h"
#undef  defineBrand
    NULL
};

/* have to do this the hard way - can't statically initialize flexible array members */
struct {
    tCount          count;
    unsigned long   period[8];   /* NOT tPeriods - these are not scaled */
} gRepeatStream[] = {
#define defineRepeatStream(id,...)  __VA_ARGS__,
#include "repeatstreammapping.h"
#undef defineRepeatStream
    {0}
};

tReferenceFingerprint gProtocol[] = 
{
    /* encoding, symbolCounts, carrierFreq, leading {mark, space}, durations, mark symbols, space symbols */

    { kFromSpec,  "NEC",             kSpaceVaries,       {32},           38000, {342,171}, 4104, {21},     {21,64}, kNECRepeatStream },
    { kFromSpec,  "Sony SIRCS",      kMarkVaries,        {12,15,20},     40000, {96},      1800, {24,48},  {24}     },
    { kFromSpec,  "Philips RC-5",    kBiphase,           {13},           36000, {0},       4445, {32,64},  {32,64}  },
    { kFromSpec,  "Philips RC-5 (a)", kAmbiguous,        {12},           36000, {0},       4445, {32,64},  {32,64}  }, /* RC-5 '0' digit */
    { kFromSpec,  "Philips RC-5e",   kBiphaseExtended,   {18,19},        36000, {0},       4445, {32,64},  {32,64}  },
    { kFromSpec,  "JVC (1)",         kSpaceVaries,       {16,32},        38000, {320,160}, 2195, {20},     {20,60}  },
    { kFromSpec,  "JVC (2)",         kSpaceVaries,       {16,32},        38000, {320,160}, 3373, {20},     {20,60}  },
    /* the following were measured */
    { kMeasured,  "Panasonic",       kSpaceVaries,       {48},           37000, {128,64},  4673, {16},     {16,48}  },
    { kMeasured,  "RCA",             kSpaceVaries,       {24},           57360, {229,229}, 3695, {29},     {57,114} },
    { kMeasured,  "Denon",           kSpaceVaries,       {15},           38000, {0},       2564, {10},     {30,70}  },
    /* the following I created by hand, from analysis of the database */
    { kFromDB,    "NEC-like (1)",    kSpaceVaries,       {32},           38000, {171,171}, 4104, {21},     {21,64}  },
    { kFromDB,    "NEC-Like (2)",    kSpaceVaries,       {32},           38000, {342,171}, 5893, {21},     {21,64}  },
    { kFromDB,    "NEC-Like (3)",    kSpaceVaries,       {28,40,42},     38000, {342,171}, 4104, {21},     {21,64}  },
    { kFromDB,    "Mitsubishi?",     kSpaceVaries,       {16},           33000, {0},       1822, {11},     {28,67}  },
    { kFromDB,    "Mystery2?",       kSpaceVaries,       {10},           36000, {0},        872, {11},     {26,64}  },
    { kFromDB,    "JVC?",            kSpaceVaries,       {16},           58800, {0,360},   3600, {26},     {97,163} },
    { kFromDB,    "Philips? (1)",    kBiphaseExtended,   {37},           37000, {98},      3886, {17,33,50}, {15,31} },
    { kFromDB,    "Philips? (2)",    kBiphase,           {22,23},        38000, {102},     4000, {18,34},  {16,32}  },
    { kFromDB,    "Philips? (3)",    kBiphaseExtended,   {19,20},        38000, {102},     4000, {18,34},  {16,32}  },
    { kFromDB,    "TCL (Philips?)",  kBiphase,           {13},           38400, {0},       4062, {16,32},  {16,32}  },
    { kFromDB,    "Fujitsu?",        kSpaceVaries,       {48},           38400, {132,59},  3904, {20},     {11,44}  }, /* set 100061,200033 */
    { kFromDB,    "Panasonic? (3)",  kSpaceVaries,       {48},           37000, {128,64},  3200, {16},     {16,48}  },
    { kFromDB,    "Panasonic? (4)",  kSpaceVaries,       {22},           56000, {196,196}, 5668, {50},     {45,140} },
    { kFromDB,    "Funai?",          kSpaceVaries,       {24},           38400, {136,136}, 4065, {35},     {31,100} },
    { kFromDB,    "Mystery1?",       kPPM,               {16,21,22},     40000, {0},       7138, {21},     {18,163,203} },
    { kFromDB,    "Pace?",           kSpaceVaries,       {17},           40000, {361},     4000, {23},     {86,178} },     /* set 200011, 200003 */
    { kFromDB,    "Samsung? (2)",    kPPM,               {38},           38400, {172},     4608, {10},     {20,657} },
    { kFromDB,    "Samsung? (1)",    kSpaceVaries,       {48},           38400, {96,71},   3724, {17},     {15,43}  },
    { kFromDB,    "Yamaha?",         kPPM,               {18},           38400, {323},     2550, {23},     {17,53,157} },  /* set 100090,100060,100014 - two part? */
    { kFromDB,    "Mystery3?",       kSpaceVaries,       {11},           48000, {0},       6000, {11},     {239,365} },
    { kFromDB,    "Mystery4?",       kSpaceVaries,       {17},           38400, {94},      4400, {47},     {44,106} },     /* set 200024 */
    { kFromDB,    "Zenith?",         kPPM,               {15},           40000, {0},       7110, {21},     {18,161,201} }, /* set 200008 */
    { kFromDB,    "Mystery5?",       kBiphase,           {11,12,13,14},  38400, {230},     1950, {23,46},  {22,45}  },     /* set 200006 */
    { kFromDB,    "Daewoo?",         kPPM,               {18},           38400, {308},     2291, {20},     {18,56,152} },  /* set 100093 - two part? */

    { kListEnd }
};

inline tPeriod toPeriod(unsigned long x) { return (tPeriod)(x*8+4); }
inline float periodToFloat( tPeriod x) { return (x/8.0); }

/**
    @param reference the reference value to match against
    @param value     the value to check
    @param threshold the limit within which the match is acceptable (in tenths of a percent)   
*/
int fuzzyMatch(unsigned long reference, unsigned long value, unsigned int threshold)
{
    return ( ((abs(reference - value) * 1000 * 2) / (reference + value)) <  threshold );
}

int durationsMatch(int durationA, int durationB)
{
    if ( (durationA * durationB) == 0 )
        return ((durationA + durationB) == 0);

    return fuzzyMatch( durationA, durationB, 100 );
}

int checkSymbolCount(tReferenceFingerprint *reference, int symbolCount)
{
    int i;

    for (i = 0; i < SYMBOL_ARRAY_SIZE; ++i)
    {
        if (reference->symbolCounts[i] == 0)
            break;

        if (reference->symbolCounts[i] == symbolCount)
            return 1;
    }
    return 0;
}

inline int checkEncoding( tEncoding referenceEncoding, tEncoding fingerprintEncoding )
{
    return (
        (fingerprintEncoding == referenceEncoding)
        || (fingerprintEncoding == kAmbiguous && referenceEncoding < kAmbiguous)
    );
}
    
tReferenceFingerprint *identifyProtocol(tFingerprint *fingerprint)
{
    tReferenceFingerprint   *result;
    int refCarrier;
    int fpCarrier, fpLeadMark, fpLeadSpace, fpDuration;
    
    /* scale appropriately, to avoid both overflow and loss-of-precision */
    fpCarrier = fingerprint->carrierFreq/100;
    fpLeadMark  = (fingerprint->leading.mark * 1000) / fpCarrier;
    fpLeadSpace = (fingerprint->leading.space * 1000) / fpCarrier;
    fpDuration  = (fingerprint->duration * 1000) / fpCarrier;
    
    result = &gProtocol[0];
    while (result->confidence != kListEnd)
    {
        refCarrier = result->carrierFreq/100;
        /* compare fingerprint with reference, see if they match */
        if ( checkEncoding( result->encoding, fingerprint->encoding )
          && checkSymbolCount(result, fingerprint->symbolCount)
          && durationsMatch((result->leading.mark*1000)/refCarrier,  fpLeadMark )
          && durationsMatch((result->leading.space*1000)/refCarrier, fpLeadSpace )
          && durationsMatch((result->duration*1000)/refCarrier, fpDuration)
        )
        {   /* we have a match */
            ++result->matched;
            return result;
        }
        ++result;
    }

    ++result->matched;
    return NULL;
}

void dumpFingerprintStats(void)
{
    tReferenceFingerprint   *protocol;
    int total = 0;
    
    logDebug(0, "--- table of fingerprints identified ---" );

    protocol = &gProtocol[0];
    while (protocol->confidence != kListEnd) {
        logprintf(DEBUG_LINE_PREFIX "    %4d codes identified as %s\n", protocol->matched, protocol->name );
        total += protocol->matched;
        ++protocol;
    } 
    total += protocol->matched;
    logprintf(DEBUG_LINE_PREFIX "    %4d codes not identified out of %d (%d%%)\n",
                protocol->matched, total, (protocol->matched * 100 / total) );
}


void dumpHistogram(tHistogram *hist)
{
    tCount i;
    char sep;

    if (hist == NULL) return;

    logprintf("%lu: ", hist->count);
    sep = ' ';
    for (i = 0; i < hist->count; ++i)
    {
        logprintf("%c%.3f:%lu", sep, periodToFloat(hist->d[i].period), hist->d[i].count );
        sep = ',';
    }
    logprintf("\n");
}

void dumpRepeatType(tIRCode *code)
{
    const char * rptStr;

    switch (code->fingerprint.repeatType)
    {
    case kFullRepeat:       rptStr = "full repeat"; break;
    case kPartialRepeat:    rptStr = "partial repeat"; break;
    case kRepeat:           rptStr = "repeat"; break;
    case kToggleRepeat:     rptStr = "toggle"; break;
    default:                rptStr = "unknown"; break;
    }
    logDebug(0,"repeat type: %s", rptStr);
}

void dumpFingerprint(tFingerprint *fingerprint)
{
    const char *encodingStr;

    switch (fingerprint->encoding)
    {
    case kMarkVaries:           encodingStr = "mark varies"; break;
    case kSpaceVaries:          encodingStr = "space varies"; break;
    case kBiphase:              encodingStr = "biphase"; break;
    case kPPM:                  encodingStr = "PPM"; break;
    case kAmbiguous:            encodingStr = "ambiguous"; break;
    case kMarkVariesExtended:   encodingStr = "mark varies extended"; break;
    case kSpaceVariesExtended:  encodingStr = "space varies extended"; break;
    case kBiphaseExtended:      encodingStr = "biphase extended"; break;
    default:                    encodingStr = "### unknown ###"; break;
    }

    logDebug(0, "encoding %s, symbol count %d", encodingStr, fingerprint->symbolCount);
    logDebug(0, "carrier %lu, duration %lu, leading: {%lu,%lu}, trailing: {%lu,%lu}",
                fingerprint->carrierFreq,
                fingerprint->duration,
                fingerprint->leading.mark,
                fingerprint->leading.space,
                fingerprint->trailing.mark,
                fingerprint->trailing.space );

    logprintf(DEBUG_LINE_PREFIX "mark  "); dumpHistogram(fingerprint->mark);
    logprintf(DEBUG_LINE_PREFIX "space "); dumpHistogram(fingerprint->space);
}

void dumpStream(tIRStream *stream)
{
    tCount i;
    char sep;
    unsigned long duration;

    if (stream == NULL)
    {
        logprintf("(empty)\n");
    }
    else
    {
        duration = 0;
        sep = ' ';
        for (i = 0; i < stream->count; ++i)
        {
            logprintf("%c%lu", sep, stream->period[i] );
            duration += stream->period[i];
            sep = (sep == '-')? ',' : '-';
        }

        logprintf(" (total %lu)\n", duration );
    }
}

void dumpIRStreams(tIRCode *code)
{
    logprintf(DEBUG_LINE_PREFIX "first/a:");
    dumpStream( code->first.a );

    if (code->first.b != NULL)
    {
        logprintf(DEBUG_LINE_PREFIX "first/b:");
        dumpStream( code->first.b );
    }
    if (code->repeat.a != NULL)
    {
        logprintf(DEBUG_LINE_PREFIX "repeat/a:");
        dumpStream( code->repeat.a );
    }
    if (code->repeat.b != NULL)
    {
        logprintf(DEBUG_LINE_PREFIX "repeat/b:");
        dumpStream( code->repeat.b );
    }
}

void dumpIRCode(tIRCode *code)
{
    dumpFingerprint( &code->fingerprint );

    dumpRepeatType( code );

    dumpIRStreams( code );
}

tHistogram *dupHistogram(tRawHistogram *raw)
{
    tHistogram *result;
    tCount i;

    if (raw == NULL || raw->count == 0)
        return NULL;
    
    result = calloc( 1, sizeof(tHistogram) + (raw->count * sizeof(tHistEntry)) );
    if (result != NULL)
    {
        result->count = raw->count;
        for (i = 0; i < raw->count; ++i)
        {
            result->d[i].period = raw->d[i].period;
            result->d[i].count  = raw->d[i].count;
        }
    }
    return result;
}

tHistogram *normalizeRawHistogram(tRawHistogram *hist)
{
    tCount i;
    int refPeriod;
    unsigned long   periodSum;
    tCount          periodCount;
    tRawHistogram   symbols;

    symbols.count = 0;
    i = 0;
    while (i < hist->count)
    {
        /* collapse runs of periods that differ by under 10% */
        refPeriod = hist->d[i].period;
        periodSum = 0;
        periodCount = 0;
        while ( (i < hist->count) && fuzzyMatch( refPeriod + 7, hist->d[i].period, 100) )
        {
            /* logprintf("[%d]=%d:%d,", i, hist->d[i].period, hist->d[i].count); */

            periodSum   += (hist->d[i].period * hist->d[i].count);
            periodCount += hist->d[i].count;

            refPeriod   = hist->d[i].period;
            ++i;
        }
        if (periodCount > 0)
        {
            symbols.d[symbols.count].period = periodSum / periodCount;
            symbols.d[symbols.count].count  = periodCount;
            
            if (symbols.count < MAX_HIST_SIZE)
            ++symbols.count;
        }

/*      logprintf("\n<%d> %d (%d/%d = %f)\n",
            symbols.count,
            (periodSum + periodCount/2) / periodCount,
            periodSum, periodCount,
            (float)periodSum/(float)periodCount ); */
    }
    return dupHistogram(&symbols);
}

void insertIntoSortedHistogram( tRawHistogram *hist, tPeriod period )
{
    unsigned int insert,insertAt;
    tCount j;

    /* default is to append, if the loop completes (i.e. period is the largest seen so far) */
    insert = 1;
    insertAt = hist->count;
    for (j = 0; j < hist->count; j++)
    {
        if (period == hist->d[j].period)
        {
            ++hist->d[j].count;
            insert = 0;     /* don't insert, we found an existing entry */
            break;          /* terminate loop, we're done */
        }
        else if (period < hist->d[j].period)
        { 
            /* tell the insertion code where to insert */
            insert = 1;
            insertAt = j;
            /* move the latter half of the array up by one, to make a hole */
            for (j = hist->count; j > insertAt; --j)
            {
                hist->d[j].period = hist->d[j-1].period;
                hist->d[j].count  = hist->d[j-1].count;
            }
            break;          /* terminate loop, following 'insert' code does the rest */
        }
    }
    if (insert) /* or append */
    {
        hist->d[insertAt].period = period;
        hist->d[insertAt].count = 1;
        if (hist->count < MAX_HIST_SIZE - 1)
            ++hist->count;
    }
}

/* returns the count from the matching histgram entry, or 0 if there isn't one */
int countFromHistogram(tHistogram *hist, int period)
{
    tCount j;

    for (j = 0; j < hist->count; j++)
    {
        if ( fuzzyMatch(hist->d[j].period, period, 100) )
            return hist->d[j].count;
    }
    return 0;
}

void incCountInHistogram(tHistogram *hist, int period)
{
    tCount j;

    for (j = 0; j < hist->count; j++)
    {
        if ( fuzzyMatch(hist->d[j].period, period, 100) )
        {
            ++hist->d[j].count;
            break;
        }
    }
}


void analyzeIRStream(tIRStream *stream, tFingerprint *fingerprint)
{
    tRawHistogram   mark, space, *rhist;
    tHistogram      *hist;
    tCount  i, last;
    int     period;

    /* build histograms of the periods in this IR stream */
    
    mark.count = 0;
    space.count = 0;
    rhist = &mark;
    last = (stream->count - 2);

    fingerprint->duration = stream->period[0];
    fingerprint->duration += stream->period[1];
    for (i = 2; i < last; ++i)
    {
        period = stream->period[i];
        fingerprint->duration += period;

        insertIntoSortedHistogram(rhist, toPeriod(period));

        /* dumpHistogram((tHistogram *)hist); */
        rhist = (rhist != &mark)? &mark : &space;
    }
    fingerprint->duration += stream->period[i++]; /* last mark */
    fingerprint->duration += stream->period[i];   /* add in the inter-code gap */

    fingerprint->mark  = normalizeRawHistogram(&mark);
    fingerprint->space = normalizeRawHistogram(&space);
    
    /* we now have a histogram of the 'meat' of the code, i.e. ignoring leading pair
       and trailing mark. Next determine if those are also valid symbols */

    fingerprint->leading.mark  = stream->period[0];
    if ( countFromHistogram( fingerprint->mark, toPeriod(fingerprint->leading.mark)) > 0 )
    {
        incCountInHistogram( fingerprint->mark, toPeriod(fingerprint->leading.mark));
        fingerprint->leading.mark = 0;
    }

    fingerprint->leading.space  = stream->period[1];
    if ( countFromHistogram( fingerprint->space, toPeriod(fingerprint->leading.space)) > 0 )
    {
        incCountInHistogram( fingerprint->space, toPeriod(fingerprint->leading.space));
        fingerprint->leading.space = 0;
    }

    fingerprint->trailing.mark  = stream->period[stream->count - 2];
    if ( countFromHistogram( fingerprint->mark, toPeriod(fingerprint->trailing.mark)) > 0 )
    {
        incCountInHistogram( fingerprint->mark, toPeriod(fingerprint->trailing.mark));
        fingerprint->trailing.mark = 0;
    }

    fingerprint->trailing.space  = stream->period[stream->count - 1];
    
    /* now try to determine the type of encoding */

    switch (fingerprint->mark->count)
    {
    case 1: /* bursts are constart width, space varies. Predominant method. */
        switch (fingerprint->space->count)
        {
        case 0:
            fingerprint->encoding = kUnknown;
            break;
        case 1:  /* Pathlogical case - both mark and space are a constant width. Encoding
                    could be kMarkVaries, kSpaceVaries, kBiphase or PPM, we can't tell. */
                 /* Typically this occurs when the code is either all zeros or all ones,
                    alternating ones and zeros for biphase, or identical repeated bit
                    groups for for PPM */
            fingerprint->encoding = kAmbiguous;
            break;
        case 2: /* The most common encoding method by far. */
            fingerprint->encoding = kSpaceVaries;
            break;
        case 3:
            /*  this is also ambiguous, a given PPM code could only use 3 symbols,
                and the longest one may only occur once.
                But PPM is unusal, so assume the common case */
            if (fingerprint->space->d[2].count == 1)
                fingerprint->encoding = kSpaceVariesExtended;
            else
                fingerprint->encoding = kPPM;   /* probably... */
            break;
        default:
            fingerprint->encoding = kPPM;
            break;
        }
        break;

    case 2: /* two widths of burst, either 'markvaries' or 'biphase' */
        switch (fingerprint->space->count)
        {
        case 1:
            /* could be ambiguous, if there's only one count of the larger mark
                RC-5 '0' digit, for example */
            if (fingerprint->mark->d[1].count == 1)
                fingerprint->encoding = kAmbiguous;
            else
                fingerprint->encoding = kMarkVaries;
            break;
        case 2:
            fingerprint->encoding = kBiphase;
            break;
        case 3:
            if (fingerprint->space->d[2].count == 1)
                fingerprint->encoding = kBiphaseExtended;
            else
                fingerprint->encoding = kUnknown;
            break;
        default:
            /* don't know what the heck this is */
            fingerprint->encoding = kUnknown;
            break;
        }
        break;

    case 3:
        if (fingerprint->mark->d[2].count == 1)
            fingerprint->encoding = kBiphaseExtended;
        else
            fingerprint->encoding = kUnknown;
        break;

    default: /* don't know what the heck this is, perhaps an analysis problem */
        fingerprint->encoding = kUnknown;
        break;
    }

    /* estimate the number of symbols, based on encoding */
    /* inc if the leading pair is not AGC, therefore a valid symbol (probably) */
    fingerprint->symbolCount = 0;
    hist = NULL;
    switch (fingerprint->encoding)
    {
    case kMarkVaries:
    case kMarkVariesExtended:
        hist = fingerprint->mark;
        break;

    case kAmbiguous: /* works for ambiguous cases of kSpaceVaries and kPPM. kBiphase only if all ones or zeros. */
    case kSpaceVaries:
    case kSpaceVariesExtended:
    case kPPM:
        hist = fingerprint->space;
        break;

    case kBiphase:
    case kBiphaseExtended:
        fingerprint->symbolCount =  fingerprint->mark->d[0].count;
        fingerprint->symbolCount += fingerprint->mark->d[1].count * 2;
        fingerprint->symbolCount += fingerprint->space->d[0].count;
        fingerprint->symbolCount += fingerprint->space->d[1].count * 2;
        fingerprint->symbolCount /= 2;
        break;

    default:
        break;
    }

    if (hist != NULL)
    {
        for (i = 0; i < hist->count; ++i)
        {
            fingerprint->symbolCount += hist->d[i].count;
        }
    }
}

unsigned long normalizeFromReference( unsigned long *period, tReferenceHistogram refhist)
{
    int i;
    for (i = 0; i < 4; ++i)
    {
        if (refhist[i] == 0)
            break;

        if ( fuzzyMatch(refhist[i], *period, 250) )
        {
            *period = refhist[i];
            return *period;
        }
    }
    return 0;
}

void adjustIRStream(tIRStream *stream, tFingerprint * UNUSED(fingerprint), tReferenceFingerprint *refprint, unsigned int *missed)
{
    unsigned long intracodeGap;
    unsigned long *period;
    tCount  count;
    
    count = stream->count;
    period = &stream->period[0];

    if ( (count & 1) == 1 )
    {
        logError("Internal error: IRStream count must be even");
        dumpStream(stream);
        --count;
    }

    if (refprint != NULL)
    {
        intracodeGap = refprint->duration;

        if (count >= 2)
        {
            if (refprint->leading.mark != 0)
            {
                *period = refprint->leading.mark;
            }
            else {
                if (normalizeFromReference( period, refprint->mark) == 0)
                {
                    logDebug(0, "leading mark period %lu didn't normalize", *period);
                    ++(*missed);
                }
            }
            intracodeGap -= *period++;
            --count;

            if (refprint->leading.space != 0)
            {
                *period = refprint->leading.space;
            }
            else {
                if (normalizeFromReference( period, refprint->space) == 0)
                {
                    logDebug(0, "leading space period %lu didn't normalize", *period);
                    ++(*missed);
                }
            }
            intracodeGap -= *period++;
            --count;
        }

        while (count > 0)
        {
            if (normalizeFromReference( period, refprint->mark) == 0)
            {
                logDebug(0, "mark period %lu didn't normalize", *period);
                ++(*missed);
            }
            intracodeGap -= *period++;
            --count;

            if (count == 1)
            {
                *period++ = intracodeGap;
            } 
            else {
                    
                if (normalizeFromReference( period, refprint->space) == 0)
                {
                    logDebug(0, "space period %lu didn't normalize", *period);
                    ++(*missed);
                }
                intracodeGap -= *period++;
            }
            --count;
        }
    }
}

void adjustIRCode(tIRCode *code)
{
    tFingerprint            *fingerprint = &code->fingerprint;
    tReferenceFingerprint   *refprint    = fingerprint->protocol;
    unsigned int            missed;
    
    missed = 0;
    adjustIRStream( code->first.a, fingerprint, refprint, &missed );
    if ( code->first.b != NULL )
        adjustIRStream( code->first.b, fingerprint, refprint, &missed );

    if (refprint != NULL)
    {
        switch (refprint->repeatStream)
        {
        case kUnknownRepeatStream:
            if ( code->repeat.a != NULL )
                adjustIRStream( code->repeat.a, fingerprint, refprint, &missed );

            if ( code->repeat.b != NULL )
                adjustIRStream( code->repeat.b, fingerprint, refprint, &missed );
            break;

        default:
            if (code->repeat.a != NULL)
                free( code->repeat.a );
                
            code->repeat.a = (tIRStream *)&gRepeatStream[refprint->repeatStream];

            if (code->repeat.b != NULL)
            {
                free( code->repeat.b );
                code->repeat.b = NULL;
            }
            break;
        }
    
        fingerprint->carrierFreq = refprint->carrierFreq;
    }

    if (missed != 0)
        logWarning("%u periods didn't normalize (on line %d)", missed, code->lineNumber );
}

void analyzeIRCode(tIRCode *code)
{
    tFingerprint    *fingerprint;
    tIRStream       *stream = NULL;

    if (code == NULL) return;
    
    fingerprint = &code->fingerprint;

    if (code->first.a != NULL)
    {
        stream = code->first.a;

        analyzeIRStream(code->first.a, fingerprint);
    }   

    fingerprint->protocol = identifyProtocol( fingerprint );

    if (fingerprint->protocol != NULL)
    {
        logDebug( 1, "Set %d (%s %s) - %s - protocol: %s",
                    code->parent->id,
                    gBrandName[code->parent->brand],
                    gDeviceTypeName[code->parent->deviceType],
                    code->button.label, fingerprint->protocol->name );
        if (logDebugEnabled(2))
        {
            dumpIRCode(code);
        }
        switch (fingerprint->protocol->confidence)
        {
        case kFromSpec:
        case kMeasured:
            adjustIRCode(code);
            if (logDebugEnabled(2))
            {
                logDebug(2, "######## After Adjustment ########");
                logDebug(2, "carrier %lu", fingerprint->carrierFreq );
                dumpIRStreams( code );
            }
            break;
        default:
            break;
        }
    }
    else {
        logError( "Set %d (%s %s) - %s - ### protocol not identified ###",
                    code->parent->id,
                    gBrandName[code->parent->brand],
                    gDeviceTypeName[code->parent->deviceType],
                    code->button.label );
        if (logDebugEnabled(0))
        {
            dumpIRCode(code);
        }
    }
}

void analyzeIRCodeSets(void)
{
    tIRCodeSet  *codeSet;
    tIRCode     *code;

    codeSet = gIRCodeSets;
    
    while (codeSet != NULL)
    {
        logDebug(1, "Set %d (%s %s)",
                    codeSet->id,
                    gBrandName[codeSet->brand],
                    gDeviceTypeName[codeSet->deviceType] );

        code = codeSet->irCodes;
        while (code != NULL)
        {
            analyzeIRCode(code);
            code = code->next;
        }
        codeSet = codeSet->next;
    }
    dumpFingerprintStats();
}
