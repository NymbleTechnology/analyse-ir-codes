/*
    @file export.c

    Copyright 2011, Paul Chambers. All Rights Reserved.
*/      

#include "common.h"

#include <sys/param.h>
#include <errno.h>
#include <dirent.h>

#include "analyse-ir-codes.h"
#include "export.h"

int exportIRStream( FILE *file, const char prefix, tIRStream *stream)
{
    tCount  i, count;
    char sep = prefix;
    
    if (stream != NULL)
    {
        count = stream->count;
        i = 0;
        while ( i < count )
        {
            fprintf(file, "%c%lu", sep, stream->period[i++]);
            if (ferror(file)) return 0;
            sep = ',';
        }
    }
    return 1;
}

int exportDB( FILE * file )
{
    const char  *repeatStr;

    tIRCodeSet  *codeSet;
    tIRCode     *code;

    codeSet = gIRCodeSets;
    if (codeSet != NULL)
        code = codeSet->irCodes;

    while ( codeSet != NULL )
    {
        /* dump this IR code */
        switch (code->fingerprint.repeatType)
        {
        case kFullRepeat:     repeatStr = "Full_Repeat";    break;
        case kPartialRepeat:  repeatStr = "Partial_Repeat"; break;
        case kRepeat:         repeatStr = "Repeat";         break;
        case kToggleRepeat:   repeatStr = "Toggle";         break;
        default:              repeatStr = "Unknown";        break;
        }

        fprintf( file, "%u|%lu|%s|%s",
                codeSet->id,
                code->fingerprint.carrierFreq,
                repeatStr,
                code->button.label);
        if (ferror(file)) break;

        if (code->first.a != NULL)
        {
            if ( !exportIRStream( file, '|', code->first.a ) ) break;
        }
        else putc('|', file);

        if (code->first.b != NULL)
        {
            if ( !exportIRStream( file, '^', code->first.b ) ) break;
        }
        
        if (code->repeat.a != NULL)
        {
            if ( !exportIRStream( file, '|', code->repeat.a ) ) break;
        }
        else putc('|', file);

        if (code->repeat.b != NULL)
        {
            if ( !exportIRStream( file, '^', code->repeat.b ) ) break;
        }

        fprintf(file, "|\r\n");

        /* advance to the next IR code */
        code = code->next;
        if (code == NULL)
        {
            codeSet = codeSet->next;
            if (codeSet != NULL)
                code = codeSet->irCodes;
        }
    } 

    if (ferror(file))
    {
        logDebugErrno(0, "error writing to output");
        return (-3);
    }
    return (0);
}

