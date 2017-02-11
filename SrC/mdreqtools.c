/*
**
** MULTI DISPLAY
*/
#include "defs.h"
#include "mdstrings.h"

/* External functions */
#include "mdprincipal.h"
#include "mdfonctions.h"

extern struct Library *SysBase,*DOSBase,*ReqToolsBase;

extern BOOL loop;                                       /* Parameters */
extern struct LocaleInfo liLocale;
STRPTR strWBName="workbench";                             /* Public screen */

/* Display an information request
   ulMode can take different values depending on the number of buttons to display */
ULONG RTInfo(STRPTR strBody,ULONG ulMode)
{
   struct TagItem ptiTags[4];
   struct rtReqInfo *prtriInfo;
   ULONG ulChoice;

   /* Allocation */
   prtriInfo = (struct rtReqInfo *)rtAllocRequestA(RT_REQINFO,NULL);
   if( prtriInfo )
   {
      ptiTags[0].ti_Tag = RT_PubScrName;
      ptiTags[0].ti_Data = (ULONG)strWBName;
      ptiTags[1].ti_Tag = RTEZ_ReqTitle;
      ptiTags[1].ti_Data = (ULONG)GetString(&liLocale,MSG_REQINFO_TITLE);
      ptiTags[2].ti_Tag = RTEZ_Flags;
      ptiTags[2].ti_Data = EZREQF_CENTERTEXT;
      ptiTags[3].ti_Tag = TAG_END;
      switch( ulMode )
      {
         case MODE_BROWSE_INFO:
            switch( rtEZRequestA(strBody,GetString(&liLocale,MSG_REQINFO_BROWSE),prtriInfo,NULL,ptiTags) )
            {
               case 0:  /* Stop button */
                  ulChoice = REPLY_STOP_INFO;
                  break;
               case 2:  /* Next button */
                  ulChoice = REPLY_NEXT_INFO;
                  break;
               default:
               case 1:  /* OK button */
                  ulChoice = REPLY_OK_INFO;
                  break;
            }
            break;
         case MODE_ACK_INFO:
         default:
            rtEZRequestA(strBody,GetString(&liLocale,MSG_REQINFO_ACK),prtriInfo,NULL,ptiTags);
            ulChoice = REPLY_OK_INFO;
            break;
      }
   }
   else ulChoice = -1;

   rtFreeRequest(prtriInfo);

   return ulChoice;
}

/* Display a file requester */
BOOL RTRequete(void)
{
   static UBYTE pubFileName[MAX_BUFFER];
   static STRPTR strDirName = NULL;
   struct TagItem ptiTags[4];
   struct rtFileRequester *prtfrReq;
   struct rtFileList *prtflRes, *prtflList;
   BPTR bpOldDir,bpInDir=NULL;
   BOOL bResult=TRUE,bContinue=TRUE;
   ULONG ulMode;

   /* Allocation */
   prtfrReq = (struct rtFileRequester *)rtAllocRequestA(RT_FILEREQ,NULL);
   if( prtfrReq )
   {
      ptiTags[0].ti_Tag = RT_PubScrName;
      ptiTags[0].ti_Data = (ULONG)strWBName;
      ptiTags[1].ti_Tag = RTFI_Flags;
      ptiTags[1].ti_Data = FREQF_MULTISELECT|FREQF_PATGAD;
      ptiTags[2].ti_Tag = TAG_END;

      /* Go back to the directory which has been previously selected */
      if( strDirName )
         rtChangeReqAttr((APTR )prtfrReq,RTFI_Dir,strDirName,TAG_END);
      prtflRes = (struct rtFileList *)rtFileRequestA(prtfrReq,pubFileName,GetString(&liLocale,MSG_REQFILE_TITLE),ptiTags);

      if( prtflRes )
      {
         do
         {
            prtflList = prtflRes;
            ulMode = MODE_BROWSE_INFO;
            do
            {
               /* Change information mode for the last argument if Loop is not set */
               if( (prtflRes->Next == NULL) && (loop == FALSE) )
                  ulMode = MODE_ACK_INFO;

               /* Change current directory first */
               bpInDir = Lock(prtfrReq->Dir,ACCESS_READ);
               bpOldDir = CurrentDir(bpInDir);
#ifdef DEBUG_INFO
               printf("Choice is %s\n",prtflRes->Name);
#endif
               /* A joker may be used */
               bContinue = TraitementCli(prtflRes->Name,ulMode);

               /* CD again... */
               CurrentDir(bpOldDir);
               UnLock(bpInDir);
               bpInDir = NULL;
               prtflRes = prtflRes->Next;
            }while( (prtflRes) && (bContinue == TRUE) );
            /* Reinitialisation */
            prtflRes = prtflList;
         }while( (loop == TRUE) && (bContinue == TRUE) );
         rtFreeFileList(prtflRes);
      }
      else
         bResult = FALSE;
   }
   else
      bResult = FALSE;
   /* Save current directory name for the next time */
   if( strDirName == NULL )
      strDirName = (STRPTR)MemoryAlloc(strlen(prtfrReq->Dir)+1,TRUE);
   strcpy(strDirName,prtfrReq->Dir);

   rtFreeRequest(prtfrReq);

   return bResult;
}
