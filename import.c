/*
    @file import.c

    Copyright 2011, Paul Chambers. All Rights Reserved.
*/      

#include "common.h"

#include <sys/param.h>
#include <errno.h>
#include <dirent.h>

#include "analyse-ir-codes.h"
#include "import.h"

#include "stringHashes.h"

struct {
        unsigned int    id;
        tDeviceType deviceType;
        tBrand      brand;
} gCodesetMapping[] = {
#define defineCodesetMapping(id,deviceType,brand)  { id, deviceType, brand },
#include "codesetmapping.h"
#undef  defineCodesetMapping
    { 0, 0, 0 }
};


tIRStream *dupIRStream(tRawIRStream *raw)
{
    tIRStream *result;
    tCount i;

    if (raw == NULL || raw->count == 0)
        return NULL;
    
    result = calloc(1, sizeof(tIRStream) + raw->count * sizeof(int));
    if (result != NULL)
    {
        result->count = raw->count;

        for (i = 0; i < raw->count; ++i)
            result->period[i] = raw->period[i];
    }
    return result;
}


unsigned long parseNumber(const char **str, int lineNumber, int *error)
{
    const char      *p = *str;
    unsigned long   number = 0;

    while (*p != '\0' && isdigit(*p))
        number = (number * 10) + (*p++ - '0');
        
    switch (*p)
    {
    case '\0':
        --p;
    case '\r':
    case '\n':
        logError("truncated number field on line %d", lineNumber);
        *error = 1;
        break;

    case '|':
        break;

    default:
        logError("non-digits found in a number field on line %d", lineNumber);
        logError("%s", *str);
        *error = 1;
        break;
    }

    *str = p + 1;
    return number;
}

unsigned long hashString(const char **str, int lineNumber, int *error )
{
    const char      *p = *str;
    unsigned long   hash = 0;

    while (*p != '\0' && *p != '|')
    {
        hash = STRING_HASH_STEP(hash, *p++);
    }

    if (*p == '\0')
    {
        logError("truncated text field on line %d", lineNumber);
        --p;
        *error = 1;
    }
    *str = p + 1;
    return hash;
}

tRepeatType parseRepeatype( const char **str, int lineNumber, int *error )
{
    tRepeatType result;
    unsigned long hash;
    
    hash = hashString( str, lineNumber, error );

    switch ( hash )
    {
    case qHashFullRepeat:
        logDebug(2, "full repeat");
        result = kFullRepeat;
        break;

    case qHashPartialRepeat:
        logDebug(2, "partial repeat");
        result = kPartialRepeat;
        break;

    case qHashRepeat:
        logDebug(2, "repeat");
        result = kRepeat;
        break;

    case qHashToggle:
        logDebug(2, "toggle");
        result = kToggleRepeat;
        break;

    default:
        logError("unknown repeat behavior \'%s\' (hash 0x%08lx) on line %d", *str, hash, lineNumber);
        result = kUnknownRepeat;
        break;
    }
    return result;
}
/*
    Since the label field can contain |, compensate with an ugly hack
    scan forward to the irstream field boundry (| followed by a digit)
*/
char *parseLabel(const char **str, int lineNumber, int *error)
{
    const char  *p, *e;
    int         len;

    p = *str;
    e = *str;

    /* find the right | - the one immediately followed by a digit */
    while ( *e != '\0' && ( *e != '|' || !isdigit( e[1] ) ) )
        { ++e; }

    len = e - p;
    if (*e == '\0')
    {
        logError("truncated label field on line %d", lineNumber);
        --e;
        *error = 1;
    }
    *str = e + 1;

    return strndup( p, len );
}

void parseIRStream( const char **str, int lineNumber, int *error, tIRStream **streamA, tIRStream **streamB )
{
    tRawIRStream    *theCode, codeA, codeB;
    const char      *p = *str;
    int             done, seenDigits;
    unsigned long   number;

    done = 0;
    codeA.count = 0;
    codeB.count = 0;
    theCode = &codeA;
    number = 0;
    seenDigits = 0;
    do {
        switch (*p)
        {
        case '\0':  /* just to be safe */
            logError("truncated IR Stream field on line %d", lineNumber);
            *error = 1;
            --p;    /* precompensate for the (undesirable in this case) increment of p later */
            /* fall through */
        case '|':   /* field terminator */
            logDebug(2,"%lu/%lu periods parsed", codeA.count,codeB.count);
            done = 1;
            /* fall through */
        case ',':   /* number separator */
        case '^':   /* second part of a toggle */
            if (seenDigits)
            {
                if (number > 100000)
                    logWarning( "huge number (%lu) in irstream field on line %d", number, lineNumber );

                if (theCode->count < MAX_RAW_IR_COUNT)
                    theCode->period[theCode->count++] = number;
            }
            number = 0;
            seenDigits = 0;

            if (*p == '^')
            {
                theCode = &codeB;
            }
            break;

        default:
            if (isdigit(*p))
            {
                number = (number * 10) + (*p - '0');
                seenDigits = 1;
            }
            else
            {
                logError("invalid characters found in an irstream field: %s", *str);
                *error = 1;
                done = 1;
            }
            break;
        }
        ++p;
    } while (!done);
    
    if (streamA != NULL)
        *streamA = dupIRStream( &codeA );

    if (streamB != NULL)
        *streamB = dupIRStream( &codeB );
        
    *str = p;
}

int importDB( FILE *  file )
{
    int     lineNumber;
    char    line[1024];

    const char *p;
    int         i;
    unsigned long number;
    
    int         fieldNumber;
    int         finished;
    tIRCodeSet  *codeSet = NULL;
    tIRCode     *code    = NULL;

    lineNumber = 1;
    do {
        if (fgets(line, sizeof(line), file) == NULL)
        {
            if (ferror(file))
            {
                logDebugErrno(0, "error reading file");
                return (-3);
            }
        }
        else 
        {
            p = line;
            fieldNumber = 0;
            finished = 0;
            
            do {
                switch (fieldNumber)
                {
                case 0:
                    while (*p != '\0' && isspace(*p))
                        { ++p; }

                    if ( *p == '\0' || *p == '#' || *p == ';')
                        finished = 1;

                    logDebug(2, "line start: %s", p);
                    break;

                case 1: /* code set ID */
                    number = parseNumber(&p, lineNumber, &finished);
                    logDebug(2, "code set ID: %ld", number);

                    /* take care of the code set */
                    if (codeSet == NULL || codeSet->id != number)
                    {
                        if (codeSet == NULL)
                        {
                            gIRCodeSets = calloc(1, sizeof(tIRCodeSet));
                            codeSet = gIRCodeSets;
                        }
                        else
                        {
                            codeSet->next = calloc(1, sizeof(tIRCodeSet));
                            codeSet = codeSet->next;
                        }
                        codeSet->id = number;
                        /* look up the brand and device type */
                        /* linear lookup, rather inefficient... */
                        i = 0;
                        while ( gCodesetMapping[i].id != 0 )
                        {
                            if (gCodesetMapping[i].id == number)
                            {
                                codeSet->deviceType = gCodesetMapping[i].deviceType;
                                codeSet->brand      = gCodesetMapping[i].brand;
                                break;
                            }
                            ++i;
                        }
                        if (gCodesetMapping[i].id == 0)
                            logWarning("Codeset %lu has no mapping information on line %d", number, lineNumber);
                    }

                    /* allocate a new IRCode */
                    if (gIRCodes == NULL)
                    {
                        gIRCodes = calloc(1, sizeof(tIRCode));
                        code = gIRCodes;
                    }
                    else
                    {
                        code->nextA = calloc(1, sizeof(tIRCode));
                        code = code->nextA;
                    }

                    if (code != NULL)
                    {
                        /* point the new IRCode to its parent IRCodeSet */
                        code->parent = codeSet;
                        code->lineNumber = lineNumber;
                        
                        /* now add it to the chain for this set */
                        if (codeSet->irCodes == NULL)
                        { /* first one to be added */
                            codeSet->irCodes = code;
                            codeSet->lastIrCode = code;
                        }
                        else
                        { /* append subsequent ones */
                            codeSet->lastIrCode->next = code;
                            codeSet->lastIrCode = code;
                        }
                    }
                    break;

                case 2: /* carrier freq */
                    code->fingerprint.carrierFreq = parseNumber( &p, lineNumber, &finished );
                    logDebug(2, "carrier freq: %ld", code->fingerprint.carrierFreq);
                    break;

                case 3: /* repeat behavior */
                    code->fingerprint.repeatType = parseRepeatype( &p, lineNumber, &finished );
                    break;

                case 4: /* button label */
                    code->button.label = parseLabel( &p, lineNumber, &finished );
                    logDebug(3, "button label: \'%s\'", code->button.label);
                    break;

                case 5: /* first code */
                    logDebug(2, "first stream: %s", p);
                    parseIRStream( &p, lineNumber, &finished, &code->first.a, &code->first.b );
                    break;

                case 6: /* repeat code */
                    logDebug(2, "repeat stream: %s", p);
                    parseIRStream( &p, lineNumber, &finished, &code->repeat.a, &code->repeat.b );
                    break;

                default: /* line end, should be no more data */
                    while ( *p != '\0' && isspace(*p) )
                        { ++p; }

                    if ( *p != '\0' )
                        logError("spurious characters \"%s\" at end of line %d", p, lineNumber);

                    finished = 1;
                    break;
                }
                ++fieldNumber;

            } while (!finished);

            ++lineNumber;
        }

    } while (!feof(file));

    return (0);
}
