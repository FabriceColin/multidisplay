/*
**
** MULTI DISPLAY
*/
#include "defs.h"
#include "mdstrings.h"

/* External functions */
#include "mdprincipal.h"
#include "mdfonctions.h"
#include "mdreqtools.h"

extern struct Library *SysBase,*DOSBase,*IconBase,*WorkbenchBase;

extern BOOL app,all,sync,show,loop;             /* Parameters */
extern struct LocaleInfo liLocale;
static struct DiskObject *g_pdoIcon=NULL;       /* AppIcon */

/* Read icon tooltypes and DiskObject
   icon.library */
BOOL LectureIcone(STRPTR strIconName,BPTR bpIconDir)
{
   BPTR bpOldDir;
   BOOL result=TRUE;

   if( (strIconName == NULL) || (bpIconDir == NULL) )
      return FALSE;

   /* Go to the current directory to read the icon */
   bpOldDir = CurrentDir(bpIconDir);

   /* GetDiskObjectNew() returns the default DiskObject if the icon can't be found */
   g_pdoIcon = GetDiskObjectNew(strIconName);

   if( g_pdoIcon )
   {
      /* Recherche des tooltypes */
      if( FindToolType((UBYTE **)g_pdoIcon ->do_ToolTypes,"APPICON") )
      {
#ifdef DEBUG_INFO
         printf("AppIcon is on...\n");
#endif
         app = TRUE;
      }
      if( FindToolType((UBYTE **)g_pdoIcon ->do_ToolTypes,"ALL") )
      {
#ifdef DEBUG_INFO
         printf("All is on...\n");
#endif
         all = TRUE;
      }
      if( FindToolType((UBYTE **)g_pdoIcon ->do_ToolTypes,"SYNC") )
      {
#ifdef DEBUG_INFO
         printf("Sync is on...\n");
#endif
         sync = TRUE;
      }
      if( FindToolType((UBYTE **)g_pdoIcon ->do_ToolTypes,"SHOWDT") )
      {
#ifdef DEBUG_INFO
         printf("ShowDT and Sync are on...\n");
#endif
         show = sync = TRUE;
      }
      if( FindToolType((UBYTE **)g_pdoIcon ->do_ToolTypes,"LOOP") )
      {
#ifdef DEBUG_INFO
         printf("Loop, ShowDT and Sync are on...\n");
#endif
         loop = show = sync = TRUE;
      }
   }
#ifdef DEBUG_INFO
   else
      printf("Cannot open %s icon\n",strIconName);
#endif

   CurrentDir(bpOldDir);
   return result;
}

/* Create an AppIcon
   icon.library, workbench.library */
BOOL CreerIcone(void)
{
   struct MsgPort *pmpPort;
   struct AppIcon *paiIcon;
   struct AppMessage *pamMsg;
   struct WBArg *pwbaList;
   LONG lArgsNbr;
   BOOL bResult=TRUE,bQuit=FALSE,bContinue=TRUE;

   if( !g_pdoIcon )
   {
      RTInfo(GetString(&liLocale,MSG_NOICON),MODE_ACK_INFO);
      return FALSE;
   }
   /* Make the AppIcon appear in the top left of the Workbench window */
   g_pdoIcon->do_CurrentX = 5;
   g_pdoIcon->do_CurrentY = 5;

   if( (pmpPort = CreateMsgPort()) )
   {
      /* Actual creation */
      paiIcon = (struct AppIcon *)AddAppIcon(1,0,GetString(&liLocale,MSG_ICON_TITLE),pmpPort,NULL,g_pdoIcon,NULL);
      if( paiIcon )
      {
         do
         {
            /* Wait for a message */
            WaitPort(pmpPort);
            /* One or more has arrived : get all of them */
            while( (pamMsg = (struct AppMessage *)GetMsg(pmpPort)) )
            {
               /* Each message has a list of arguments */
               pwbaList = pamMsg->am_ArgList;
               lArgsNbr = pamMsg->am_NumArgs;
#ifdef DEBUG_INFO
               printf("Message contains %ld arguments\n",lArgsNbr);
#endif
               /* If the arguments count is 0, the icon has been double clicked on : quit */
               if( lArgsNbr != 0 )
               {
                  if( bContinue == TRUE )
                     bContinue = TraitementWB(pwbaList,lArgsNbr);
               }
               else
                  bQuit = TRUE;
               /* Reply to this message to get rid of it */
               ReplyMsg((struct Message *)pamMsg);
            }
            bContinue = TRUE;
         }
         while( bQuit == FALSE );
         /* Remove the icon */
         RemoveAppIcon(paiIcon);
      }
      else
      {
#ifdef DEBUG_INFO
         printf("Cannot create AppIcon\n");
#endif
         bResult = FALSE;
      }
      DeleteMsgPort(pmpPort);
   }
   else
   {
#ifdef DEBUG_INFO
      printf("Cannot create message port\n");
#endif
      bResult = FALSE;
   }

   /* Free the DiskObject structure */
   FreeDiskObject(g_pdoIcon);
   g_pdoIcon = NULL;
   /* Confirm exit and thank user for having used this program */
   RTInfo(GetString(&liLocale,MSG_QUIT_APPICON),MODE_ACK_INFO);

   return bResult;
}
