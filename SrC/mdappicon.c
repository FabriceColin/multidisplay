/*
**
** MULTI DISPLAY
** " Le vrai-faux viewer universel "
*/
#include <stdio.h>

#include <exec/memory.h>            /* AllocMem() */
#include <exec/ports.h>             /* struct MsgPort */
#include <workbench/startup.h>      /* struct WBArg */
#include <workbench/workbench.h>    /* struct AppMessage */

#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/icon_pragmas.h>

#include "md_strings.h"

/* Prototypes des fonctions */
extern BOOL Recherche(STRPTR arg);
extern BOOL RTInfo(STRPTR body);
extern STRPTR GetString(struct LocaleInfo *li, LONG stringNum);

extern struct Library *SysBase,*DOSBase,*IconBase;
extern struct LocaleInfo md_locale; /* Pour GetString() */
struct DiskObject *md_appicon=NULL; /* AppIcon pour MD */

/* Cree l'appicon sur le Workbench
   Necessite icon.library et workbench.library */
BOOL CreerIcone(void)
{
    struct MsgPort *msgport;
    struct AppIcon *appicon;
    struct AppMessage *appmsg;
    struct WBArg *argptr;
    ULONG i;
    BPTR old_dir;
    BOOL result=TRUE,stop=FALSE;

    if( !md_appicon )
    {
        RTInfo(GetString(&md_locale,MSG_NOICON));
        return FALSE;
    }
    /* On s'assure que l'appicon va apparaitre en haut a gauche */
    md_appicon->do_CurrentX = 5;
    md_appicon->do_CurrentY = 5;

    if( msgport = CreateMsgPort() )
    {
        /* Creation de l'appicon : id et userdata sont uniquement a l'usage de l'utilisateur */
        appicon = (struct AppIcon *)AddAppIcon(1, 0, GetString(&md_locale,MSG_ICON_TITLE), msgport, NULL, md_appicon, NULL);
        if( appicon )
        {
            do
            {
                /* Attente du premier message */
                WaitPort(msgport);
                /* Recuperation de tous les eventuels messages */
                while( appmsg = (struct AppMessage *)GetMsg(msgport) )
                {
#ifdef DEBUG
                    printf("Message de %ld arguments\n",appmsg->am_NumArgs);
#endif
                    /* On procede comme dans TraitementWB() */
                    argptr = appmsg->am_ArgList;
                    /* Si NumArgs vaut 0 , l'utilisateur a double-clique sur l'icone
                       C'est donc qu'il veut arreter */
                    if( appmsg->am_NumArgs != 0 )
                        for(i=0;i<appmsg->am_NumArgs;i++,argptr++)
                        {
#ifdef DEBUG
                            printf("Argument %ld: %s, (Lock=%lx)\n",i,argptr->wa_Name,argptr->wa_Lock);
#endif
                            /* La aussi il faut changer de repertoire */
                            old_dir = CurrentDir(argptr->wa_Lock);
                            /* On ne peut pas avoir de joker */
                            Recherche(argptr->wa_Name);
                            CurrentDir(old_dir);
                        }
                    else stop = TRUE;
                    /* On repond au message pour s'en debarasser */
                    ReplyMsg((struct Message *)appmsg);
                }
            }
            while( stop == FALSE );
            /* On enleve l'appicon */
            RemoveAppIcon(appicon);
        }
        else
        {
#ifdef DEBUG
            printf("Impossible de créer l'AppIcon\n");
#endif
            result = FALSE;
        }
        DeleteMsgPort(msgport);
    }
    else
    {
#ifdef DEBUG
        printf("Impossible de créer le port message\n");
#endif
        result = FALSE;
    }

    /* On n'oublie pas de liberer la structure DiskObject (icon.library !) */
    FreeDiskObject(md_appicon);
    md_appicon = NULL;
    /* On confirme a l'utilisateur en le remerciant */
    RTInfo(GetString(&md_locale,MSG_QUIT_APPICON));
    return result;
}
