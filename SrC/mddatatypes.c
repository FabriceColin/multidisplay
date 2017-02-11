/******************************************************************************
 *
 * COPYRIGHT: Unless otherwise noted, all files are Copyright (c) 1992-1996
 * ESCOM AG.  All rights reserved.
 *
 * DISCLAIMER: This software is provided "as is".  No representations or
 * warranties are made with respect to the accuracy, reliability, performance,
 * currentness, or operation of this software, and all use is at your own risk.
 * Neither ESCOM AG nor the authors assume any responsibility or liability
 * whatsoever with respect to your use of this software.
 *
 ******************************************************************************
 * examinedt.c
 * Shows how to examine a file using datatypes.library
 * Written by David N. Junod
 * Modified by Fabrice Colin
 *
 */

#include <stdio.h>
#include <string.h>

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <dos/dos.h>
#include <dos/rdargs.h>
#include <dos/dosextens.h>
#include <datatypes/datatypes.h>
#include <datatypes/datatypesclass.h>

#include <libraries/iffparse.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/datatypes_pragmas.h>
#include <pragmas/iffparse_pragmas.h>

#include "md_strings.h"

/* Prototypes des fonctions */
extern STRPTR GetString(struct LocaleInfo *li, LONG stringNum);

extern struct Library *SysBase,*DOSBase,*DataTypesBase,*IFFParseBase;
extern struct LocaleInfo md_locale;                     /* Pour GetString() */

/* Verify sytem version */
BOOL VerificationSys(void)
{
    if (SysBase->lib_Version < 39)
    {
        return FALSE;
    }
    return TRUE;
}

/* Say what the datatype of the file is */
STRPTR ExamenDT(STRPTR name)
{
    struct DataTypeHeader *dth;
    struct DataType *dtn;
    STRPTR buffer=NULL;
    BPTR lock;

    /* Lock the current name */
    if (lock = Lock (name, ACCESS_READ))
    {
        /* Determine the DataType of the file */
        if (dtn = ObtainDataTypeA (DTST_FILE, (APTR) lock, NULL))
        {
            buffer = (STRPTR)AllocVec(50,MEMF_PUBLIC|MEMF_CLEAR);
            if ( buffer )
            {
                dth = dtn->dtn_Header;
#ifdef DEBUG
                printf ("Nom du datatype : %s\n", dth->dth_BaseName);
#endif
                strcpy (buffer,dth->dth_BaseName);
            }
            /* Release the DataType */
            ReleaseDataType (dtn);
        }
#ifdef DEBUG
        else printf ("Impossible de déterminer le type de %s\n.", name);
#endif
        UnLock (lock);
    }
#ifdef DEBUG
    else printf ("Impossible d'examiner %s.\n", name);
#endif
    return buffer;
}
