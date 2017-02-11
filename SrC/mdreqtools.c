/*
**
** MULTI DISPLAY
** " Le vrai-faux viewer universel "
*/

#include <stdio.h>
#include <string.h>

#include <exec/types.h>
#include <exec/memory.h>        /* AllocVec() */
#include <exec/libraries.h>     /* struct Library */
#include <utility/tagitem.h>    /* struct TagItem */
#include <dos/dos.h>            /* BPTR */
#include <dos/dostags.h>        /* Tags */

#include <libraries/reqtools.h>

#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/reqtools_pragmas.h>

#include "md_strings.h"

/* Prototypes des fonctions */
extern void TraitementCli(STRPTR arg);
extern STRPTR GetString(struct LocaleInfo *li, LONG stringNum);

extern struct Library *SysBase,*DOSBase,*ReqToolsBase;  /* Deja ouvertes */

extern struct LocaleInfo md_locale;                     /* Pour GetString() */
BPTR in_dir=NULL;               /* Repertoire courant */

/* Affiche une requete de fichiers
   Appelle TraitementCli() pour les fichiers selectionnes */
BOOL RTRequete(void)
{
   struct TagItem taglist[4];
   struct rtFileRequester *rt_req;
   struct rtFileList *res_list;
   BPTR old_dir;
   STRPTR filename,wb_name="workbench";
   BOOL result=TRUE;

   filename = (STRPTR)AllocVec(MAX_BUFFER,MEMF_PUBLIC|MEMF_CLEAR);
   if( !filename )
      return FALSE;

   /* Allocation */
   rt_req = (struct rtFileRequester *)rtAllocRequestA(RT_FILEREQ,NULL);
   if( rt_req )
   {
      taglist[0].ti_Tag = RT_PubScrName;
      taglist[0].ti_Data = (ULONG)wb_name;
      taglist[1].ti_Tag = RTFI_Flags;
      taglist[1].ti_Data = FREQF_MULTISELECT;
      taglist[2].ti_Tag = FREQF_PATGAD;
      taglist[2].ti_Data = TRUE;
      taglist[3].ti_Tag = TAG_END;

      res_list = (struct rtFileList *)rtFileRequestA(rt_req,filename,GetString(&md_locale,MSG_REQFILE_TITLE),taglist);
      if( res_list )
      {
         /* Traitement de tous les arguments */
         while( res_list )
         {
            /* On change de repertoire pour lancer la commande */
            in_dir = Lock(rt_req->Dir,ACCESS_READ);
            old_dir = CurrentDir(in_dir);
#ifdef DEBUG
            printf("Choix de %s\n",res_list->Name);
#endif
            /* On utilise TraitementCli() car l'utilisateur peut entrer un joker */
            TraitementCli(res_list->Name);
            /* On revient dans l'ancien repertoire */
            CurrentDir(old_dir);
            UnLock(in_dir);
            in_dir = NULL;
            res_list = res_list->Next;
         }
         rtFreeFileList(res_list);
      }
      else result = FALSE;
   }
   else result = FALSE;
   /* Liberation */
   rtFreeRequest(rt_req);

   FreeVec(filename);
   return result;
}

/* Affiche une requete d'information pour les erreurs ou l'option ShowDT */
BOOL RTInfo(STRPTR body)
{
   struct TagItem taglist[4];
   struct rtReqInfo *rt_info;
   STRPTR wb_name="workbench";
   BOOL result=TRUE;

   /* Allocation */
   rt_info = (struct rtReqInfo *)rtAllocRequestA(RT_REQINFO,NULL);
   if( rt_info )
   {
      taglist[0].ti_Tag = RT_PubScrName;
      taglist[0].ti_Data = (ULONG)wb_name;
      taglist[1].ti_Tag = RTEZ_ReqTitle;
      taglist[1].ti_Data = (ULONG)GetString(&md_locale,MSG_REQINFO_TITLE);
      taglist[2].ti_Tag = RTEZ_Flags;
      taglist[2].ti_Data = EZREQF_CENTERTEXT;
      taglist[3].ti_Tag = TAG_END;
      /* Le retour n'est pas important car il n'y a qu'un seul gadget ! */
      rtEZRequestA(body,"OK",rt_info,NULL,taglist);
   }
   else result = FALSE;

   rtFreeRequest(rt_info);
   return result;
}
