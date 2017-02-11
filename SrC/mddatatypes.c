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
#include "defs.h"
#include "mdstrings.h"
 /* Fonctions externes */
#include "mdfonctions.h"        /* GetString */

extern struct Library *SysBase,*DOSBase,*DataTypesBase,*IFFParseBase;
extern struct LocaleInfo md_locale;                     /* Pour GetString() */

/* Say what the datatype of the file is */
BOOL ExamenDT(STRPTR name,STRPTR *group,STRPTR *basename)
{
   struct DataTypeHeader *dth;
   struct DataType *dtn;
   BOOL bReturn=TRUE;
   BPTR lock;

  if( (name == NULL) || (*group == NULL) || (*basename == NULL) )
      return FALSE;

   /* Lock the current name */
   lock = Lock(name,ACCESS_READ);
   if( lock )
   {
      /* Determine the DataType of the file */
      dtn = ObtainDataTypeA(DTST_FILE,(APTR)lock,NULL);
      if( dtn )
      {
         dth = dtn->dtn_Header;

         *group = (STRPTR)MemoryAlloc(strlen(GetDTString(dth->dth_GroupID))+1,TRUE);
         *basename = (STRPTR)MemoryAlloc(strlen(dth->dth_BaseName)+1,TRUE);
         strcpy(*group,GetDTString(dth->dth_GroupID));
         strcpy(*basename,dth->dth_BaseName);

#ifdef DEBUG
         printf("Group : %s",*group);
         printf("Base name : %s\n",*basename);
#endif
         /* Release the DataType */
         ReleaseDataType (dtn);
      }
      else
      {
#ifdef DEBUG
         printf("Cannot obtain datatype information about %s\n",name);
#endif
         *group = *basename = NULL;
         bReturn = FALSE;
      }
      UnLock(lock);
   }
   else
   {
#ifdef DEBUG
      printf("Cannot examine %s\n",name);
#endif
      bReturn = FALSE;
   }

   return bReturn;
}
