/*
**
** MULTI DISPLAY
*/
#include "defs.h"
#include "mdstrings.h"

/* External functions */
#include "mdfonctions.h"
#include "mdreqtools.h"
#include "mddatatypes.h"
#include "mdappicon.h"

#define TEMPLATE          "ALL/S,SYNC/S,SHOWDT/S,LOOP/S,FILES/M"
#define OPT_ALL           0
#define OPT_SYNC          1
#define OPT_SHOW          2
#define OPT_LOOP          3
#define OPT_FILES         4
#define OPT_COUNT         5

struct AnchorPath g_apPath;                     /* LONGWORD aligned */
extern struct Library *SysBase,*DOSBase;
extern struct LocaleInfo liLocale;

STRPTR VERSION = "$VER: Multidisplay 1.10a (18.09.99)\n\0";
LONG plOpts[OPT_COUNT];
STRPTR g_strDef=NULL;                           /* Default instruction */
BOOL app=FALSE,all=FALSE,sync=FALSE,show=FALSE,loop=FALSE;      /* Parameters */
static struct ConfigElem *g_pceHeader=NULL;     /* Configuration */

/* Free the memory space allocated to the configuration list */
void LibererConfig(void)
{
   struct ConfigElem *pceElem=g_pceHeader, *pceNextElem=NULL;

   while( pceElem != NULL )
   {
      if( pceElem->m_strId != NULL )
         FreeVec(pceElem->m_strId);
      if( pceElem->m_strExe != NULL )
         FreeVec(pceElem->m_strExe);
      pceNextElem = pceElem->m_pceNext;
      FreeVec(pceElem);
      pceElem = pceNextElem;
   }

   g_pceHeader = NULL;
}

/* Read the configuration file */
BOOL LectureConfig(STRPTR *pstrDefault)
{
   struct ConfigElem *pcePrevElem=NULL, *pceElem=NULL;
   BOOL bCompare=TRUE;
   STRPTR strBuff;
   BPTR bpSource=NULL;

   strBuff = (STRPTR)MemoryAlloc(MAX_BUFFER,TRUE);

   /* Try to open it in the current directory or in S: if it fails */
   bpSource = Open("PROGDIR:MD.config",MODE_OLDFILE);
   if( !bpSource )
   {
      bpSource = Open("S:MD.config",MODE_OLDFILE);
      if( !bpSource )
      {
         Fault(IoErr(),"MD.config",strBuff,MAX_BUFFER);
         RTInfo(strBuff,MODE_ACK_INFO);
         FreeVec(strBuff);
         return FALSE;
      }
   }

   /* Read default instruction */
   if( FGets(bpSource,*pstrDefault,MAX_BUFFER) == NULL )
      return FALSE;
   /* without the trailing backslash */
   (*pstrDefault)[ strlen(*pstrDefault)-1 ] = '\0';
#ifdef DEBUG_INFO
   printf("Default instruction : %s\n",*pstrDefault);
#endif

   while( FGets(bpSource,strBuff,MAX_BUFFER) )
   {
      /* Comments are allowed from the second line on */
      if( strBuff[0] == ';' )
         continue;

      strBuff[ strlen(strBuff)-1 ] = '\0';
      /* When the line is a data type description, bCompare is TRUE */
      if( bCompare == TRUE )
      {
         /* Allocation */
         pceElem = (struct ConfigElem *)MemoryAlloc(sizeof(struct ConfigElem),TRUE);
         pceElem->m_strId = (STRPTR)MemoryAlloc(strlen(strBuff)+1,TRUE);
         strcpy(pceElem->m_strId,strBuff);
         pceElem->m_pceNext = NULL;

         /* Is a DataType or a file name pattern ? */
         if( strBuff[0] != '#' )
            pceElem->m_bDatatype = TRUE;
         else
            pceElem->m_bDatatype = FALSE;

         /* First read element ? */
         if( pcePrevElem == NULL )
            g_pceHeader = pceElem;
         else
            pcePrevElem->m_pceNext = pceElem;
         pcePrevElem = pceElem;

         bCompare = FALSE;
      }
      else
      {
         pceElem->m_strExe = (STRPTR)MemoryAlloc(strlen(strBuff)+1,TRUE);
         strcpy(pceElem->m_strExe,strBuff);
#ifdef DEBUG_INFO
         printf("Read %s,%s\n",pceElem->m_strId,pceElem->m_strExe);
#endif
         bCompare = TRUE;
      }
   }

   FreeVec(strBuff);
   if( bpSource )
      Close(bpSource);

   return TRUE;
}

/* Determine which call is to be made for a file depending on its type */
const STRPTR TrouverConfig(STRPTR strFileName,STRPTR strId)
{
   struct ConfigElem *pcePrevElem,*pceElem;
   BOOL bStop=FALSE;

   pcePrevElem = pceElem = g_pceHeader;

   while( (pceElem != NULL) && (bStop == FALSE) )
   {
#ifdef DEBUG_INFO
      printf("\t%s, type %s, with %s\n",strFileName,strId,pceElem->m_strId);
#endif
      /* Is it a DataType or a name pattern ? */
      if( (pceElem->m_bDatatype == TRUE) && (strId != NULL) )
         bStop = ComparaisonID(pceElem->m_strId,strId);
      else
         bStop = ComparaisonNom(pceElem->m_strId,strFileName);

      /* Next element */
      pcePrevElem = pceElem;
      pceElem = pceElem->m_pceNext;
   }
   if( bStop == TRUE )
      return pcePrevElem->m_strExe;

   return NULL;
}

/* Determine what files have to be examined in case of a joker
   Return FALSE if MultiDisplay must stop */
BOOL TraitementCli(STRPTR strFileName,ULONG ulMode)
{
   BPTR bpOldDir = 0;
   BOOL bContinue=TRUE;
   STRPTR strBuff;

   strBuff = (STRPTR)MemoryAlloc(MAX_BUFFER,TRUE);

   /* Is there a joker in this ? */
   if( ParsePattern(strFileName,strBuff,MAX_BUFFER) == 1)
   {
#ifdef DEBUG_INFO
      printf("Joker used in %s\n",strFileName);
#endif
      /* Initialisation of the AnchorPath structure */
      g_apPath.ap_BreakBits = SIGBREAKF_CTRL_C;
      g_apPath.ap_Strlen = 0;
      /* Search if there is at least a corresponding DOS object */
      if( MatchFirst(strFileName,&g_apPath) != 0 )
      {
         /* No */
         sprintf(strBuff,GetString(&liLocale,MSG_NOMATCH),strFileName);
         RTInfo(strBuff,MODE_ACK_INFO);
         MatchEnd(&g_apPath);
         FreeVec(strBuff);
         return FALSE;
      }
      do
      {
         /* Go back to the previous directory */
         if( bpOldDir )
            CurrentDir(bpOldDir);
         bpOldDir = CurrentDir(g_apPath.ap_Current->an_Lock);

         strcpy(strBuff,g_apPath.ap_Info.fib_FileName);
         if( g_apPath.ap_Info.fib_DirEntryType > 0 )
         {
            /* This entry is a directory */
#ifdef DEBUG_INFO
            printf("Directory : %s\n",strBuff);
#endif
            if( all == TRUE )
            {
               /* Allow to enter this directory if we don't come from it */
               if( ! (g_apPath.ap_Flags & APF_DIDDIR) )
                  g_apPath.ap_Flags = g_apPath.ap_Flags | APF_DODIR;
               /* Clear to get in other directories */
               g_apPath.ap_Flags = g_apPath.ap_Flags & ~APF_DIDDIR;
            }
         }
         else
         {
#ifdef DEBUG_INFO
            printf("Pattern matching object : %s\n",strBuff);
#endif
            /* Since we can't know if it is the last of the list, stay in the same mode */
            bContinue = Recherche(strBuff,ulMode);
         }
      }while( (MatchNext(&g_apPath) == 0) && (bContinue == TRUE) );

      MatchEnd(&g_apPath);
   }
   else bContinue = Recherche(strFileName,ulMode);

   FreeVec(strBuff);

   return bContinue;
}

/* Return FALSE if MultiDisplay must stop */
BOOL TraitementWB(struct WBArg *pwbaArgs, LONG lArgsNbr)
{
   struct WBArg *pwbaList = pwbaArgs;
   BPTR bpOldDir;
   BOOL bContinue=TRUE;
   ULONG ulMode;
   LONG i;

#ifdef DEBUG_INFO
   printf("Number of WB arguments: %ld\n",lArgsNbr);
#endif

   do
   {
      ulMode = MODE_BROWSE_INFO;

      for(i=0;i<lArgsNbr;i++)
      {
         if( pwbaArgs == NULL )
            break;

         /* What DOS object has no lock ? */
         if( pwbaArgs->wa_Lock )
         {
#ifdef DEBUG_INFO
            printf("%ld : %s (lock)\n",i,pwbaArgs->wa_Name);
#endif
            bpOldDir = CurrentDir(pwbaArgs->wa_Lock);
         }
#ifdef DEBUG_INFO
         else
            printf("%ld : %s\n",i,pwbaArgs->wa_Name);
#endif
         /* Change information mode for the last argument if Loop is not set */
         if( (i == lArgsNbr-1) && (loop == FALSE) )
            ulMode = MODE_ACK_INFO;

         /* bContinue could have been set by the previous call */
         if( bContinue == TRUE )
            bContinue = Recherche(pwbaArgs->wa_Name,ulMode);

         if( pwbaArgs->wa_Lock && bpOldDir )
            CurrentDir(bpOldDir);
         /* Next argument */
         pwbaArgs++;
      }
      /* Reinitialisation */
      pwbaArgs = pwbaList;
   }while( (loop == TRUE) && (bContinue == TRUE) );

   return bContinue;
}

/* Set parameters according to CLI arguments */
STRPTR *ReadParameters(struct RDArgs *prdaArgs,LONG lArgsNbr)
{
   if( (prdaArgs == NULL) || (lArgsNbr == 0) )
      return NULL;

   if( plOpts[OPT_ALL] )
   {
#ifdef DEBUG_INFO
      printf("Option All on...\n");
#endif
      all = TRUE;
      lArgsNbr--;
   }
   if( plOpts[OPT_SYNC] )
   {
#ifdef DEBUG_INFO
      printf("Option Sync on...\n");
#endif
      sync = TRUE;
      lArgsNbr--;
   }
   if( plOpts[OPT_SHOW] )
   {
#ifdef DEBUG_INFO
      printf("Option ShowDT and Sync on...\n");
#endif
      show = sync = TRUE;
      lArgsNbr--;
   }
   if( plOpts[OPT_LOOP] )
   {
#ifdef DEBUG_INFO
      printf("Option Loop and ShowDT/Sync on...\n");
#endif
      loop = show = sync = TRUE;
      lArgsNbr--;
   }
#ifdef DEBUG_INFO
   printf("%ld arguments to examine\n",lArgsNbr);
#endif

   return (STRPTR *)(plOpts[OPT_FILES]);
}

/* User break */
void Break(void)
{
   LibererConfig();
   FermerLibrairies();
}

int main(int argn,char **argv)
{
   struct WBStartup *pwbsStartup;
   struct WBArg *pwbaArgs;
   struct RDArgs *prdaArgs;
   STRPTR strBuff,strDefault,*pstrArgsList,*pstrCliArgs;
   LONG lArgsNbr, lIndex;
   ULONG ulMode;
   BOOL ask_user=TRUE, bContinue=TRUE;

   if( SysVersionCheck() == FALSE )
     return RETURN_ERROR;

   atexit(Break);

   strBuff = (STRPTR)MemoryAlloc(MAX_BUFFER,TRUE);
   strDefault = (STRPTR)MemoryAlloc(MAX_BUFFER,TRUE);

   if( OuvrirLibrairies() == FALSE )
   {
      printf(GetString(&liLocale,MSG_NOLIBS));
      FermerLibrairies();
      FreeVec(strDefault);
      FreeVec(strBuff);
      return RETURN_ERROR;
   }

  /* Read configuration */
   if( LectureConfig(&strDefault) == FALSE )
   {
       FermerLibrairies();
       FreeVec(strDefault);
       FreeVec(strBuff);
       return RETURN_ERROR;
   }
   /* Make the default instruction available globally */
   g_strDef = strDefault;

   /* Is MultiDisplay launched from CLI or WB
      Testing Cli() would be equivalent */
   if( argn == 0 )
   {
      /* "WB task" : argn equals 0 and argv points to a WBStartup structure */
      if( argv != NULL )
      {
         pwbsStartup = (struct WBStartup *)argv;
         pwbaArgs = pwbsStartup->sm_ArgList;
         lArgsNbr = pwbsStartup->sm_NumArgs;

         /* Read tool types and icon DiskObject */
         LectureIcone(pwbaArgs->wa_Name,pwbaArgs->wa_Lock);

         /* Any argument ? */
         if( lArgsNbr > 1 )
         {
            /* Skip MultiDiplay name */
            pwbaArgs++;
            lArgsNbr--;
            TraitementWB(pwbaArgs,lArgsNbr);
            ask_user = FALSE;
         }
         /* Iconify or not ?
            This option is activated in WB mode only */
         else if( app == TRUE )
         {
            CreerIcone();
            ask_user = FALSE;
         }
      }
   }
   else
   {
      /* "CLI task" */
      prdaArgs = (struct RDArgs *)ReadArgs(TEMPLATE,plOpts,NULL);
      if( prdaArgs != NULL )
      {
         /* Skip MultiDisplay name */
         lArgsNbr = (LONG )(argn-1);

         pstrCliArgs = ReadParameters(prdaArgs,lArgsNbr);
         if( pstrCliArgs )
         {
            do
            {
               /* Initialisation */
               pstrArgsList = pstrCliArgs;
               lIndex = 0;
               ulMode = MODE_BROWSE_INFO;
               /* Parse all arguments */
               do
               {
                  /* Change information mode for the last argument if Loop has not been set */
                  if( (lIndex == lArgsNbr-1) && (loop == FALSE) )
                     ulMode = MODE_ACK_INFO;

                  bContinue = TraitementCli(*pstrArgsList,ulMode);
                  pstrArgsList++;
                  lIndex++;
               }while( (bContinue == TRUE) && (*pstrArgsList) );
            }while( (loop == TRUE) && (bContinue == TRUE) );
            ask_user = FALSE;
         }
      }
      FreeArgs(prdaArgs);
   }

   /* Display a file requester ? */
   if( ask_user == TRUE )
      while( RTRequete() == TRUE );

   FreeVec(strDefault);
   FreeVec(strBuff);
   LibererConfig();
   FermerLibrairies();

   return RETURN_OK;
}
