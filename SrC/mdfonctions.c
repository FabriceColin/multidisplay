/*
**
** MULTI DISPLAY
** " Le vrai-faux viewer universel "
*/

#include <stdio.h>
#include <string.h>

#include <exec/types.h>
#include <exec/memory.h>            /* AllocVec() */
#include <exec/libraries.h>         /* struct Library */
#include <utility/tagitem.h>        /* struct TagItem */
#include <dos/dos.h>                /* BPTR */
#include <dos/dostags.h>            /* Tags */
#include <workbench/workbench.h>    /* struct DiskObject */

#include <libraries/locale.h>

#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/locale_pragmas.h>
#include <pragmas/icon_pragmas.h>
#include <pragmas/datatypes_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/reqtools_pragmas.h>

#include "md_strings.h"

/* Prototypes des fonctions */
extern STRPTR ExamenDT(STRPTR name);
extern BOOL RTInfo(STRPTR body);

extern struct Library *SysBase,*DOSBase;
struct Library *WorkbenchBase,*LocaleBase,*IconBase,*DataTypesBase,*IFFParseBase,*ReqToolsBase;

extern BPTR source;                             /* Fichier de config */
extern LONG position;                           /* Position initiale dans ce fichier */
extern STRPTR def;                              /* Instruction par defaut */
extern BOOL app,all,sync,show;                  /* Parametres */
extern struct DiskObject *md_appicon;           /* AppIcon pour MD */
struct LocaleInfo md_locale;                    /* Pour GetString() */

/* Ouvre toutes les librairies necessaires ainsi que le catalogue de MD */
BOOL OuvrirLibrairies(void)
{
   struct TagItem LocaleItems[2];
   BOOL result=TRUE;

   /* français est le langage par defaut , n'est ce pas ? */
   LocaleItems[0].ti_Tag = OC_BuiltInLanguage;
   LocaleItems[0].ti_Data = (ULONG)"français";
   LocaleItems[1].ti_Tag = TAG_DONE;

   if( !(WorkbenchBase = OpenLibrary("workbench.library",39)) )
      result = FALSE;
   if( !(IconBase = OpenLibrary("icon.library",39)) )
      result = FALSE;
   if( LocaleBase = OpenLibrary("locale.library",38) )
   {
      md_locale.li_LocaleBase = (APTR)LocaleBase;
      md_locale.li_Catalog = OpenCatalogA(NULL,"md.catalog",LocaleItems);
   }
   else result = FALSE;
   if( !(DataTypesBase = OpenLibrary ("datatypes.library", 39)) )
      result = FALSE;
   if( !(IFFParseBase = OpenLibrary ("iffparse.library", 39)) )
      result = FALSE;
   if( !(ReqToolsBase = OpenLibrary("reqtools.library",38)) )
      result = FALSE;
   return result;
}

/* Ferme toutes les librairies et le catalogue de MD */
void FermerLibrairies(void)
{
   CloseLibrary(ReqToolsBase);
   CloseLibrary(IFFParseBase);
   CloseCatalog(md_locale.li_Catalog);
   CloseLibrary(LocaleBase);
   CloseLibrary(DataTypesBase);
   CloseLibrary(IconBase);
   CloseLibrary(WorkbenchBase);
}

/* Fonction generee par 'CatComp md.cd CFILE md_strings.h'
   On la met ici pour eviter les erreurs de definitions multiples */
STRPTR GetString(struct LocaleInfo *li, LONG stringNum)
{
   LONG *l;
   UWORD *w;
   STRPTR builtIn;

   l = (LONG *)CatCompBlock;

   while( *l != stringNum )
   {
      w = (UWORD *)((ULONG)l + 4);
      l = (LONG *)((ULONG)l + (ULONG)*w + 6);
   }
   builtIn = (STRPTR)((ULONG)l + 6);

#undef LocaleBase
#define LocaleBase li->li_LocaleBase

    if( LocaleBase )
       return GetCatalogStr(li->li_Catalog,stringNum,builtIn);
#undef LocaleBase
    return builtIn;
}

/* Lit les tooltypes d'une icone
   Si keep_icon vaut TRUE alors on conserve la structure DiskObject pour l'utiliser en appicon a
   condition que le tooltype APPICON soit precise
   Necessite icon.library */
BOOL LectureIcone(STRPTR icon_name,BPTR icon_dir,BOOL keep_icon)
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

   /* Avec GetDiskObjectNew() si l'icone n'existe pas on a celle par defaut du type correspondant */
   if( keep_icon == FALSE )
       icon = GetDiskObjectNew(icon_name);
   else
   {
       /* On utilise l'icone courante pour l'appicon */
       md_appicon = GetDiskObjectNew(icon_name);
       icon = md_appicon;
   }

   if( icon )
   {
      /* Recherche des tooltypes : FindToolType() cause un warning ! */
      if( FindToolType(icon->do_ToolTypes,"APPICON") )
      {
         if( keep_icon == TRUE )
         {
#ifdef DEBUG
            printf("Tooltype AppIcon activé...\n");
#endif
            app = TRUE;
         }
         else result = FALSE;
      }
      if( FindToolType(icon->do_ToolTypes,"ALL") )
      {
#ifdef DEBUG
         printf("Tooltype All activé...\n");
#endif
         all = TRUE;
      }
      if( FindToolType(icon->do_ToolTypes,"SYNC") )
      {
#ifdef DEBUG
         printf("Tooltype Sync activé...\n");
#endif
         sync = TRUE;
      }
      if( FindToolType(icon->do_ToolTypes,"SHOWDT") )
      {
#ifdef DEBUG
         printf("Tooltype ShowDT (et donc Sync) activé...\n");
#endif
         show = TRUE;
         sync = TRUE;
      }
      /* Liberation possible seulement si icon alloue et keep_icon faux ! */
      if( keep_icon == FALSE )
          FreeDiskObject(icon);
   }
#ifdef DEBUG
   else
   {
         printf("Impossible d'ouvrir l'icône de %s.\n",icon_name);
   }
#endif

   CurrentDir(old_dir);
   return result;
}

/* Passe la commande adéquate au systéme */
BOOL Commande(STRPTR commande,STRPTR arg)
{
   struct TagItem taglist[5];
   BOOL result=TRUE;
   int i=2;

#ifdef DEBUG
   printf("Appel : %s\n",commande);
#else
   /* Teste si l argument existe */
   if( arg != NULL )
   {
      /* Suppression de \0 a la fin de commande et " " necessaires */
      commande[ strlen(commande)-1 ] = ' ';
      sprintf(commande,"%s \"%s\"",commande,arg);
   }

   /* Tags de System() */
   taglist[0].ti_Tag = SYS_Input;
   taglist[0].ti_Data = NULL;
   taglist[1].ti_Tag = SYS_Output;
   taglist[1].ti_Data = NULL;
   if( sync == FALSE )
   {
      taglist[2].ti_Tag = SYS_Asynch;
      taglist[2].ti_Data = TRUE;
      i = 3;
   }
   taglist[i].ti_Tag = SYS_UserShell;
   taglist[i].ti_Data = TRUE;
   taglist[i+1].ti_Tag = TAG_DONE;
   /* Passage de la commande */
   if( SystemTagList(commande, taglist) == SYSTEMFAIL )
      result = FALSE;
#endif
   return result;
}

/* Compare le type decrit dans le fichier de config avec le nom
   du datatype correspondant */
BOOL ComparaisonID(STRPTR buffer,STRPTR id)
{
   BOOL arret=FALSE;
   int i;

   buffer[ strlen(buffer)-1 ] = '\0';
   if( stricmp( buffer,id ) == 0 )
   {
      arret = TRUE;
#ifdef DEBUG
      printf("ID : %s\n",buffer);
#endif
   }
   return arret;
}

/* Compare le type decrit dans le fichier de config avec le nom du fichier */
BOOL ComparaisonNom(STRPTR buffer,STRPTR arg)
{
   BOOL arret=FALSE;
   STRPTR arg2,buffer2;
   int i;

   arg2 = (STRPTR)AllocVec(MAX_BUFFER,MEMF_PUBLIC|MEMF_CLEAR);
   buffer2 = (STRPTR)AllocVec(MAX_BUFFER,MEMF_PUBLIC|MEMF_CLEAR);
   if( (!arg2) || (!buffer2) )
      exit(1);

   for(i=0;i<strlen( arg );i++)
      arg2[i] = tolower(arg[i]);
   arg2[i] = '\0';
   for(i=0;i<strlen( buffer );i++)
      buffer2[i] = tolower(buffer[i+1]);
   /* Suppression du caractere '\n' obtenu a la lecture */
   buffer2[ strlen(buffer2)-1 ] = '\0';

   if( strstr( arg2,buffer2 ) )
   {
      arret = TRUE;
#ifdef DEBUG
      printf("Nom : %s\n",buffer2);
#endif
   }
   FreeVec(arg2);
   FreeVec(buffer2);
   return arret;
}

/* Recherche quel est le type du fichier passe en argument en comparant le fichier
   de config et le retour de ExamenDT() */
BOOL Recherche(STRPTR arg)
{
   BOOL stop=FALSE,comp=TRUE,result=TRUE;
   STRPTR buffer,id;

   buffer = (STRPTR)AllocVec(MAX_BUFFER,MEMF_PUBLIC|MEMF_CLEAR);
   if( !buffer )
      exit(1);

   /* Identification du datatype du fichier */
   id = ExamenDT(arg);
   if( !id )
   {
      sprintf(buffer,GetString(&md_locale,MSG_UNKNOWNID),arg);
      RTInfo(buffer);
      result = Commande(def,arg);
      FreeVec(buffer);
      return result;
   }
   if( show == TRUE )
   {
      sprintf(buffer,GetString(&md_locale,MSG_SHOWID),arg,id);
      RTInfo(buffer);
   }

   /* Stop est VRAI si la boucle doit être arrêtée et donc si Commande()
      doit etre appellée. Sinon on utilise l'instruction par défaut */
   while( (FGets(source,buffer,MAX_BUFFER)) && (stop == FALSE) )
   {
      /* comp est vrai si la ligne du fichier est un type de donnees */
      if( comp == TRUE )
      {
         /* Teste s'il s'agit d'un datatype ou d'une extension de nom de fichier */
         if( buffer[0] != '#' )
            stop = ComparaisonID(buffer,id);
         else
            stop = ComparaisonNom(buffer,arg);
         comp = FALSE;
      }
      else comp = TRUE;
   }

   /* Stop est VRAI or la ligne suivant le type de donnée valide a été lue dans le while()
      precédent : on se sert donc de buffer pour appeller Commande() */
   if( stop == TRUE )
      result = Commande(buffer,arg);
   else result = Commande(def,arg);

#ifdef DEBUG
   if( result == FALSE )
      printf("Echec sur %s ...\n",arg);
#endif
   /* Retour au début du fichier */
   Seek(source,position,OFFSET_BEGINNING);
   FreeVec(buffer);
   return result;
}
