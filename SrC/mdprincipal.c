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
#include <dos/dos.h>            /* BPTR */
#include <dos/dosasl.h>         /* struct AnchorPath */
#include <dos/rdargs.h>         /* Pour ReadArgs() */
#include <workbench/workbench.h>
#include <workbench/startup.h>  /* struct WBStartup */

#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>

#include "md_strings.h"

#define TEMPLATE          "All/S,Sync/S,ShowDT/S,Files/A/M"
#define OPT_ALL           0
#define OPT_SYNC          1
#define OPT_SHOW          2
#define OPT_FILES         3
#define OPT_COUNT         4

/* Prototypes des fonctions */
extern BOOL OuvrirLibrairies(void);
extern void FermerLibrairies(void);
extern BOOL LectureIcone(STRPTR icon_name,BPTR icon_dir,BOOL keep_icon);
extern BOOL Recherche(STRPTR arg);
extern BOOL VerificationSys(void);
extern BOOL RTRequete(void);
extern BOOL RTInfo(STRPTR body);
extern BOOL CreerIcone(void);
extern STRPTR GetString(struct LocaleInfo *li, LONG stringNum);

struct AnchorPath ap;                           /* Alignement en LONGWORD */
extern struct Library *SysBase,*DOSBase;        /* Deja ouvertes */

extern struct LocaleInfo md_locale;             /* Pour GetString() */
extern BPTR in_dir;
STRPTR VERSION = "$VER: Multidisplay 1.8 (6.9.97)\n\0";
LONG opts[OPT_COUNT];                           /* Parametres */
BPTR source=NULL;                               /* Fichier de config */
LONG position;                                  /* Position initiale dans ce fichier */
STRPTR def;                                     /* Instruction par defaut */
BOOL app=FALSE,all=FALSE,sync=FALSE,show=FALSE; /* Parametres */

/* Determine les fichiers qui sont à traiter dans le cas de joker
   Sinon ne fait qu'appeller Recherche() */
BOOL TraitementCli(STRPTR arg)
{
   BPTR old_dir;
   BOOL changer_rep=FALSE;
   ULONG result;
   STRPTR pbuffer,buffer;

   pbuffer = (STRPTR)AllocVec(3*strlen(arg),MEMF_PUBLIC|MEMF_CLEAR);
   if( !pbuffer )
      return FALSE;

#ifdef DEBUG
   printf("Traitement de %s\n",arg);
#endif

   /* Teste si l'argument contient un joker */
   result = ParsePattern(arg,pbuffer, 3*strlen(arg) );
   if( result == 1)
   {
#ifdef DEBUG
      printf("Joker utilisé dans %s\n",arg);
#endif
      /* Initialisation de la structure AnchorPath */
      ap.ap_BreakBits = SIGBREAKF_CTRL_C;
      ap.ap_Strlen = 0;
      /* Recherche le s'il existe au moins un objet DOS correspondant */
      result = MatchFirst(arg,&ap);
      if( result == 0 )
      {
         /* Oui */
         changer_rep = TRUE;
      }
      else
      {
         /* Non */
         FreeVec(pbuffer);
         buffer = (STRPTR)AllocVec(MAX_BUFFER,MEMF_PUBLIC|MEMF_CLEAR);
         if( !pbuffer )
            return FALSE;
         sprintf(buffer,GetString(&md_locale,MSG_NOMATCH),arg);
         RTInfo(buffer);
         MatchEnd(&ap);
         FreeVec(buffer);
         return FALSE;
      }
   }

   if( changer_rep == TRUE )
   {
      do
      {
         old_dir = CurrentDir(ap.ap_Current->an_Lock);
         strcpy(pbuffer,ap.ap_Info.fib_FileName);
         if( ap.ap_Info.fib_DirEntryType > 0 )
         {
#ifdef DEBUG
            printf("Répértoire : %s\n",pbuffer);
#endif
            if( all == TRUE )
            {
               /* On autorise l'entree dans ce repertoire si on n'en vient pas ! */
               if( ! (ap.ap_Flags & APF_DIDDIR) )
                  ap.ap_Flags = ap.ap_Flags | APF_DODIR;
               /* On nettoie pour pouvoir entrer dans d'autres repertoires */
               ap.ap_Flags = ap.ap_Flags & ~APF_DIDDIR;
            }
         }
         else
         {
#ifdef DEBUG
            printf("Occurence : %s\n",pbuffer);
#endif
            Recherche(pbuffer);
         }
         CurrentDir(old_dir);
      }while( MatchNext(&ap) == 0 );
   }
   else Recherche(arg);

   MatchEnd(&ap);
   FreeVec(pbuffer);

   return TRUE;
}

/* Decortique la liste des arguments et appelle Recherche() s'il y a des arguments
   Si l'argument/tooltype AppIcon existe on cree une AppIcon qu'il y ait des arguments ou non
   Si une erreur se produit, on renvoie FALSE pour ouvrir la requete de fichiers */
BOOL TraitementWB(char **args)
{
   struct WBStartup *wbs;
   struct WBArg *wbarg;
   BPTR old_dir;
   LONG nbr,i;

   wbs = (struct WBStartup *)args;
   wbarg = wbs->sm_ArgList;
   nbr = wbs->sm_NumArgs;

   /* On en profite pour recuperer la structure DiskObject qui correspond au tooltype APPICON */
   LectureIcone(wbarg->wa_Name,wbarg->wa_Lock,TRUE);

   /* Pas d'arguments, juste le nom de MD (0 impossible, non ?) */
   if( nbr <= 1 )
      return FALSE;
   /* Sinon on a des arguments : on les traite */
   wbarg++;

   for(i=1;i<nbr;i++,wbarg++)
   {
      /* Quel genre d'objet ne peut pas avoir de lock */
      if( wbarg->wa_Lock )
      {
#ifdef DEBUG
         printf("Traitement de %ld : %s (lock)\n",i,wbarg->wa_Name);
#endif
         old_dir = CurrentDir(wbarg->wa_Lock);
      }
#ifdef DEBUG
      else
         printf("Traitement de %ld : %s\n",i,wbarg->wa_Name);
#endif
      /* Ici on ne peut pas avoir de joker */
      Recherche(wbarg->wa_Name);
      if( old_dir )
         CurrentDir(old_dir);
   }

   return TRUE;
}

/* Ferme "tout" en cas de break inopine */
void Break(void)
{
   if( source ) Close(source);
   if( in_dir ) UnLock(in_dir);
}

/* Ouvre les librairies et le fichier de configuration
   Determine le mode de lancement puis les options eventuelles
   Appelle TraitementCli(), TraitementWB() ou RTRequete() selon le cas */
int main(int argn,char **argv)
{
   struct RDArgs *argsptr;
   BOOL ask_user=FALSE;
   STRPTR buffer,*sptr;

   if( VerificationSys() == FALSE )
   {
      printf(GetString(&md_locale,MSG_KSVERSION));
      return RETURN_ERROR;
   }
   atexit(Break);

   def = (STRPTR)AllocVec(MAX_BUFFER,MEMF_PUBLIC|MEMF_CLEAR);
   buffer = (STRPTR)AllocVec(MAX_BUFFER,MEMF_PUBLIC|MEMF_CLEAR);
   if( (!buffer) || (!def) )
      return RETURN_ERROR;

   /* Ouverture des librairies necessaires */
   if( OuvrirLibrairies() == FALSE )
   {
      printf(GetString(&md_locale,MSG_NOLIBS));
      FermerLibrairies();
      FreeVec(buffer);
      FreeVec(def);
      return RETURN_ERROR;
   }

   /* Ouverture du fichier de configuration */
   source = Open("S:MD.config",MODE_OLDFILE);
   if( !source )
   {
      Fault(IoErr(),"S:MD.config",buffer,MAX_BUFFER);
      RTInfo(buffer);
      FermerLibrairies();
      FreeVec(buffer);
      FreeVec(def);
      return RETURN_ERROR;
   }

   /* Lecture de l'instruction par defaut */
   FGets(source,def,MAX_BUFFER);
#ifdef DEBUG
   printf("Instruction par défaut : %s",def);
#endif
   /* Sauvegarde de l'emplacement actuel dans le fichier */
   position = Seek(source,0,OFFSET_CURRENT);

   /* Teste si MD est lance depuis un Shell car ReadArgs() a besoin d'une entree
      depuis laquelle il lit les parametres
      Dans le cas d'un lancement depuis le WB on aurait alors ouverture d'une fenetre
      Un equivalent serait de tester le retour de Cli() */
   if( argn != 0 )
   {
      /* "Tache CLI" : lecture parametres */
      argsptr = (struct RDArgs *)ReadArgs(TEMPLATE,opts,NULL);
      if( argsptr != NULL )
      {
         if( opts[OPT_ALL] )
         {
#ifdef DEBUG
            printf("Option All activée...\n");
#endif
            all = TRUE;
         }
         if( opts[OPT_SYNC] )
         {
#ifdef DEBUG
            printf("Option Sync activée...\n");
#endif
            sync = TRUE;
         }
         if( opts[OPT_SHOW] )
         {
#ifdef DEBUG
            printf("Option ShowDT (et donc Sync) activée...\n");
#endif
            show = TRUE;
            sync = TRUE;
         }
         sptr = (STRPTR *)(opts[OPT_FILES]);
         if( sptr )
         {
            /* Traitement de tous les arguments */
            while( *sptr )
            {
               TraitementCli(*sptr);
               sptr++;
            }
         }
      }
      else ask_user = TRUE;     /* Pas de parametres */
      /* Liberation seulement si argsptr utilise ! */
      FreeArgs(argsptr);
   }
   else
   {
      /* "Tache WB" : argn vaut 0 mais argv peut pointer sur une structure WBStartup
         s'il y a des arguments. Normalement il y en a au moins un, qui est le
         nom de MD, donc la condition devrait etre toujours verifiee !?
         Avec DICE, le point d'entrée serait wbmain() et non main() */
      if( argv )
      {
         if( TraitementWB(argv) == FALSE )
            ask_user = TRUE;
      }
      else ask_user = TRUE;
   }

   /* On regarde s'il faut iconifier */
   if( app == TRUE )
       CreerIcone();
   /* Afficher la requete de fichiers ? */
   else if( ask_user == TRUE )
      while( RTRequete() == TRUE );

   FreeVec(def);
   FreeVec(buffer);
   FermerLibrairies();
   Close(source);
   source = NULL;
   return RETURN_OK;
}
