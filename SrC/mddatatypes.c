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

/* External functions */
#include "mdfonctions.h"

extern struct Library *SysBase,*DOSBase,*DataTypesBase,*IFFParseBase;

/* Say what the datatype of the file is */
BOOL ExamenDT(STRPTR strName,STRPTR *strGroup,STRPTR *strBaseName)
{
   struct DataTypeHeader *pdthHeader;
   struct DataType *pdtDT;
   BOOL bReturn=TRUE;
   BPTR bpLock;

  if( (strName == NULL) || (*strGroup == NULL) || (*strBaseName == NULL) )
      return FALSE;

   /* Lock the current strName */
   bpLock = Lock(strName,ACCESS_READ);
   if( bpLock )
   {
      /* Determine the DataType of the file */
      pdtDT = ObtainDataTypeA(DTST_FILE,(APTR)bpLock,NULL);
      if( pdtDT )
      {
         pdthHeader = pdtDT->dtn_Header;

         strcpy(*strGroup,GetDTString(pdthHeader->dth_GroupID));
         strcpy(*strBaseName,pdthHeader->dth_BaseName);

#ifdef DEBUG_INFO
         printf("Group : %s",*strGroup);
         printf("Base strName : %s\n",*strBaseName);
#endif
         /* Release the DataType */
         ReleaseDataType(pdtDT);
      }
      else
      {
#ifdef DEBUG_INFO
         printf("Cannot obtain datatype information about %s\n",strName);
#endif
         bReturn = FALSE;
      }
      UnLock(bpLock);
   }
   else
   {
#ifdef DEBUG_INFO
      printf("Cannot examine %s\n",strName);
#endif
      bReturn = FALSE;
   }

   return bReturn;
}
