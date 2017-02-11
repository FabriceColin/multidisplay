/*
**
** MULTI DISPLAY
** " Le vrai-faux viewer universel "
*/
#include "defs.h"
#include "mdstrings.h"
/* Fonctions externes */
#include "mdfonctions.h"        /* Recherche , GetString */
#include "mdreqtools.h"         /* RTInfo */

extern struct Library *SysBase,*DOSBase,*IconBase,*WorkbenchBase;

extern BOOL app,all,sync,show;                  /* Parametres */
extern struct LocaleInfo md_locale;             /* Pour GetString() */
static struct DiskObject *g_pdoIcon=NULL;       /* AppIcon pour MD */

/* Lit les tooltypes d'une icone
   Necessite icon.library */
BOOL LectureIcone(STRPTR icon_name,BPTR icon_dir)
{
   struct DiskObject *icon;
   BPTR old_dir;
   BOOL result=TRUE;

   if( !icon_name )
      return FALSE;
   if( !icon_dir )
   {
#ifdef DEBUG
      printf("Impossible de trouver le répértoire de MD (résident ?).\n");
#endif
      return FALSE;
   }

   /* Si le repertoire n'est pas connu on ne pourra pas lire l'icone */
   old_dir = CurrentDir(icon_dir);

   /* Avec GetDiskObjectNew() si l'icone n'existe pas on a celle par defaut du type correspondant
      On utilise l'icone courante pour l'appicon */
   g_pdoIcon = GetDiskObjectNew(icon_name);
   icon = g_pdoIcon;

   if( icon )
   {
      /* Recherche des tooltypes */
      if( FindToolType((UBYTE **)icon->do_ToolTypes,"APPICON") )
      {
#ifdef DEBUG
         printf("Tooltype AppIcon activé...\n");
#endif
         app = TRUE;
      }
      if( FindToolType((UBYTE **)icon->do_ToolTypes,"ALL") )
      {
#ifdef DEBUG
         printf("Tooltype All activé...\n");
#endif
         all = TRUE;
      }
      if( FindToolType((UBYTE **)icon->do_ToolTypes,"SYNC") )
      {
#ifdef DEBUG
         printf("Tooltype Sync activé...\n");
#endif
         sync = TRUE;
      }
      if( FindToolType((UBYTE **)icon->do_ToolTypes,"SHOWDT") )
      {
#ifdef DEBUG
         printf("Tooltype ShowDT (et donc Sync) activé...\n");
#endif
         show = TRUE;
         sync = TRUE;
      }
   }
#ifdef DEBUG
   else
      printf("Impossible d'ouvrir l'icône de %s.\n",icon_name);
#endif

   CurrentDir(old_dir);
   return result;
}

/* Cree l'appicon sur le Workbench
   Necessite icon.library et workbench.library */
BOOL CreerIcone(void)
{
   struct MsgPort *msgport;
   struct AppIcon *appicon;
   struct AppMessage *appmsg;
   struct WBArg *argptr;
   ULONG i,ulMode;
   BPTR old_dir;
   BOOL bResult=TRUE,bQuit=FALSE,bContinue=TRUE;

   if( !g_pdoIcon )
   {
      RTInfo(GetString(&md_locale,MSG_NOICON),MODE_ACK_INFO);
      return FALSE;
   }
   /* On s'assure que l'appicon va apparaitre en haut a gauche */
   g_pdoIcon->do_CurrentX = 5;
   g_pdoIcon->do_CurrentY = 5;

   if( (msgport = CreateMsgPort()) )
   {
      /* Creation de l'appicon : id et userdata sont uniquement a l'usage de l'utilisateur */
      appicon = (struct AppIcon *)AddAppIcon(1,0,GetString(&md_locale,MSG_ICON_TITLE),msgport,NULL,g_pdoIcon,NULL);
      if( appicon )
      {
         do
         {
            /* Attente du premier message */
            WaitPort(msgport);
            /* Recuperation de tous les eventuels messages */
            while( (appmsg = (struct AppMessage *)GetMsg(msgport)) )
            {
#ifdef DEBUG
               printf("Message de %ld arguments\n",appmsg->am_NumArgs);
#endif
               /* On procede comme dans TraitementWB() */
               argptr = appmsg->am_ArgList;
               /* Si NumArgs vaut 0 , l'utilisateur a double-clique sur l'icone
                  C'est donc qu'il veut arreter */
               if( appmsg->am_NumArgs != 0 )
               {
                  /* S'il y a plusieurs arguments on peut examiner,sauter ou stopper */
                  if( appmsg->am_NumArgs > 1 )
                     ulMode = MODE_BROWSE_INFO;
                  else
                     ulMode = MODE_ACK_INFO;

                  i = 0;
                  while( i < appmsg->am_NumArgs )
                  {
#ifdef DEBUG
                     printf("Argument %ld: %s, (Lock=%lx)\n",i,argptr->wa_Name,argptr->wa_Lock);
#endif
                     if( i == appmsg->am_NumArgs-1 )
                        ulMode = MODE_ACK_INFO;

                     /* La aussi il faut changer de repertoire */
                     old_dir = CurrentDir(argptr->wa_Lock);
                     /* BContinue a pu etre mis a FALSE au tour precedent */
                     if( bContinue == TRUE )
                        /* On ne peut pas avoir de joker donc on appelle directement Recherche() */
                        bContinue = Recherche(argptr->wa_Name,ulMode);
                     CurrentDir(old_dir);

                     /* Argument suivant */
                     argptr++;
                     i++;
                  }
                  /* Plusieurs messages peuvent etre en attente */
                  bContinue = TRUE;
               }
               else
                  bQuit = TRUE;
               /* On repond au message pour s'en debarasser */
               ReplyMsg((struct Message *)appmsg);
            }
         }
         while( bQuit == FALSE );
         /* On enleve l'appicon */
         RemoveAppIcon(appicon);
      }
      else
      {
#ifdef DEBUG
         printf("Impossible de créer l'AppIcon\n");
#endif
         bResult = FALSE;
      }
      DeleteMsgPort(msgport);
   }
   else
   {
#ifdef DEBUG
      printf("Impossible de créer le port message\n");
#endif
      bResult = FALSE;
   }

   /* On n'oublie pas de liberer la structure DiskObject (icon.library !) */
   FreeDiskObject(g_pdoIcon);
   g_pdoIcon = NULL;
   /* On confirme a l'utilisateur en le remerciant */
   RTInfo(GetString(&md_locale,MSG_QUIT_APPICON),MODE_ACK_INFO);

   return bResult;
}
