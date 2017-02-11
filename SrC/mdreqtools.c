/*
**
** MULTI DISPLAY
** " Le vrai-faux viewer universel "
*/
#include "defs.h"
#include "mdstrings.h"
/* Fonctions externes */
#include "mdprincipal.h"        /* TraitementCli */
#include "mdfonctions.h"        /* GetString */

extern struct Library *SysBase,*DOSBase,*ReqToolsBase;  /* Deja ouvertes */

extern struct LocaleInfo md_locale;                     /* Pour GetString() */
STRPTR wb_name="workbench";     /* Ecran */

/* Affiche une requete d'information pour les erreurs ou l'option ShowDT */
ULONG RTInfo(STRPTR strBody,ULONG ulMode)
{
   struct TagItem taglist[4];
   struct rtReqInfo *rt_info;
   ULONG choice;

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
      switch( ulMode )
      {
         case MODE_BROWSE_INFO:
            switch( rtEZRequestA(strBody,GetString(&md_locale,MSG_REQINFO_BROWSE),rt_info,NULL,taglist) )
            {
               case 1:
                  choice = REPLY_OK_INFO;
                  break;
               case 2:
                  choice = REPLY_NEXT_INFO;
                  break;
               case 0:
               default:
                  choice = REPLY_STOP_INFO;
                  break;
            }
            break;
         case MODE_ACK_INFO:
         default:
            rtEZRequestA(strBody,GetString(&md_locale,MSG_REQINFO_ACK),rt_info,NULL,taglist);
            choice = REPLY_OK_INFO;
            break;
      }
   }
   else choice = -1;

   rtFreeRequest(rt_info);

   return choice;
}

/* Affiche une requete de fichiers
   Appelle TraitementCli() pour les fichiers selectionnes */
BOOL RTRequete(void)
{
   static UBYTE filename[MAX_BUFFER];
   static STRPTR dirname = NULL;
   struct TagItem taglist[4];
   struct rtFileRequester *rt_req;
   struct rtFileList *res_list;
   BPTR old_dir,in_dir=NULL;
   BOOL result=TRUE,continuer=TRUE;
   ULONG ulMode;

   /* Allocation */
   rt_req = (struct rtFileRequester *)rtAllocRequestA(RT_FILEREQ,NULL);
   if( rt_req )
   {
      taglist[0].ti_Tag = RT_PubScrName;
      taglist[0].ti_Data = (ULONG)wb_name;
      taglist[1].ti_Tag = RTFI_Flags;
      taglist[1].ti_Data = FREQF_MULTISELECT|FREQF_PATGAD;
      taglist[2].ti_Tag = TAG_END;

      /* On retourne dans le repertoire choisi precedemment */
      if( dirname )
         rtChangeReqAttr((APTR )rt_req,RTFI_Dir,dirname,TAG_END);
      res_list = (struct rtFileList *)rtFileRequestA(rt_req,filename,GetString(&md_locale,MSG_REQFILE_TITLE),taglist);

      /* Traitement de tous les arguments */
      if( res_list )
      {
         while( (res_list) && (continuer == TRUE) )
         {
            /* Changement de mode pour le dernier argument */
            if( res_list->Next )
               ulMode = MODE_BROWSE_INFO;
            else ulMode = MODE_ACK_INFO;

            /* On change de repertoire pour lancer la commande */
            in_dir = Lock(rt_req->Dir,ACCESS_READ);
            old_dir = CurrentDir(in_dir);
#ifdef DEBUG
            printf("Choix de %s\n",res_list->Name);
#endif
            /* On utilise TraitementCli() car l'utilisateur peut entrer un joker */
            continuer = TraitementCli(res_list->Name,ulMode);

            /* On revient dans l'ancien repertoire */
            CurrentDir(old_dir);
            UnLock(in_dir);
            in_dir = NULL;
            res_list = res_list->Next;
         }
         rtFreeFileList(res_list);
      }
      else
         result = FALSE;
   }
   else
      result = FALSE;
   /* Sauvegarde du repertoire choisi pour le prochain appel */
   if( dirname == NULL )
      dirname = (STRPTR)MemoryAlloc(strlen(rt_req->Dir)+1,TRUE);
   strcpy(dirname,rt_req->Dir);
   /* Liberation */
   rtFreeRequest(rt_req);

   return result;
}
