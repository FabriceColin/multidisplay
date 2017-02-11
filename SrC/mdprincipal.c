/*
**
** MULTI DISPLAY
** " Le vrai-faux viewer universel "
*/
#include "defs.h"
#include "mdstrings.h"
/* Fonctions externes */
#include "mdfonctions.h"        /* OuvrirLibrairies , FermerLibrairies , LectureIcone , Recherche , GetString */
#include "mdreqtools.h"         /* RTRequete , RTInfo */
#include "mddatatypes.h"        /* VerificationSys */
#include "mdappicon.h"          /* CreerIcone */

#define TEMPLATE          "ALL/S,SYNC/S,SHOWDT/S,FILES/M"
#define OPT_ALL           0
#define OPT_SYNC          1
#define OPT_SHOW          2
#define OPT_FILES         3
#define OPT_COUNT         4

struct AnchorPath g_apPath;                     /* Alignement en LONGWORD */
extern struct Library *SysBase,*DOSBase;        /* Deja ouvertes */
extern struct LocaleInfo md_locale;             /* Pour GetString() */

STRPTR VERSION = "$VER: Multidisplay 1.10 (01.01.99)\n\0";
LONG opts[OPT_COUNT];                           /* Parametres */
STRPTR g_strDef=NULL;                           /* Instruction par defaut */
BOOL app=FALSE,all=FALSE,sync=FALSE,show=FALSE; /* Parametres */
static struct ConfigElem *g_pceHeader=NULL;     /* Configuration */

/* Libere la memoire allouee pour la configuration */
void LibererConfig(void)
{
   struct ConfigElem *pceElem=g_pceHeader, *pceNextElem=NULL;

   while( pceElem != NULL )
   {
#ifdef DEBUG
      printf("Libération de %s,%s\n",pceElem->m_strId,pceElem->m_strExe);
#endif
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

/* Lit le fichier de configuration */
BOOL LectureConfig(STRPTR *pstrDefault)
{
   struct ConfigElem *pcePrevElem=NULL, *pceElem=NULL;
   BOOL comp=TRUE;
   STRPTR buffer;
   BPTR bpSource=NULL;

   buffer = (STRPTR)MemoryAlloc(MAX_BUFFER,TRUE);
   if( buffer == NULL )
      return FALSE;

   /* Ouverture du fichier de configuration dans le repertoire courant ou S: */
   bpSource = Open("PROGDIR:MD.config",MODE_OLDFILE);
   if( !bpSource )
   {
      bpSource = Open("S:MD.config",MODE_OLDFILE);
      if( !bpSource )
      {
         Fault(IoErr(),"MD.config",buffer,MAX_BUFFER);
         RTInfo(buffer,MODE_ACK_INFO);
         FreeVec(buffer);
         return FALSE;
      }
   }

   /* Lecture de l'instruction par defaut */
   if( FGets(bpSource,*pstrDefault,MAX_BUFFER) == NULL )
      return FALSE;

#ifdef DEBUG
   printf("Instruction par défaut : %s",*pstrDefault);
#endif

   while( FGets(bpSource,buffer,MAX_BUFFER) )
   {
      /* Les commentaires sont autorisés à partir de ce point */
      if( buffer[0] == ';' )
         continue;

      /* On se debarasse du \n obtenu a la lecture ... */
      buffer[ strlen(buffer)-1 ] = '\0';

      /* comp est vrai si la ligne du fichier est un type de donnees */
      if( comp == TRUE )
      {
         /* Allocation */
         pceElem = (struct ConfigElem *)MemoryAlloc(sizeof(struct ConfigElem),TRUE);
         pceElem->m_strId = (STRPTR)MemoryAlloc(strlen(buffer)+1,TRUE);
         strcpy(pceElem->m_strId,buffer);
         pceElem->m_pceNext = NULL;

         /* Teste s'il s'agit d'un datatype ou d'une extension de nom de fichier */
         if( buffer[0] != '#' )
            pceElem->m_bDatatype = TRUE;
         else
            pceElem->m_bDatatype = FALSE;

         /* Premier element ? */
         if( pcePrevElem == NULL )
            g_pceHeader = pceElem;
         else
            pcePrevElem->m_pceNext = pceElem;
         pcePrevElem = pceElem;

         comp = FALSE;
      }
      else
      {
         pceElem->m_strExe = (STRPTR)MemoryAlloc(strlen(buffer)+1,TRUE);
         strcpy(pceElem->m_strExe,buffer);
#ifdef DEBUG
         printf("Lecture de %s,%s\n",pceElem->m_strId,pceElem->m_strExe);
#endif
         comp = TRUE;
      }
   }

   FreeVec(buffer);
   if( bpSource )
      Close(bpSource);

   return TRUE;
}

/* Determine quelle commande correspond au fichier donne selon son type */
const STRPTR TrouverConfig(STRPTR arg,STRPTR id)
{
   struct ConfigElem *pcePrevElem,*pceElem;
   BOOL stop=FALSE;

   pcePrevElem = pceElem = g_pceHeader;

   while( (pceElem != NULL) && (stop == FALSE) )
   {
#ifdef DEBUG
      printf("\tde %s, type %s, avec %s\n",arg,id,pceElem->m_strId);
#endif
      /* Teste s'il s'agit d'un datatype ou d'une extension de nom de fichier */
      if( pceElem->m_bDatatype == TRUE )
         stop = ComparaisonID(pceElem->m_strId,id);
      else
         stop = ComparaisonNom(pceElem->m_strId,arg);

      /* Element suivant */
      pcePrevElem = pceElem;
      pceElem = pceElem->m_pceNext;
   }
   if( stop == TRUE )
      return pcePrevElem->m_strExe;

   return NULL;
}

/* Determine les fichiers qui sont à traiter dans le cas de joker
   Sinon ne fait qu'appeller Recherche()
   Retourne FALSE si une erreur s'est produite */
BOOL TraitementCli(STRPTR arg,ULONG ulMode)
{
   BPTR old_dir;
   BOOL changer_rep=FALSE,continuer=TRUE;
   ULONG result;
   STRPTR pbuffer,buffer;

   pbuffer = (STRPTR)MemoryAlloc(3*strlen(arg),TRUE);
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
      g_apPath.ap_BreakBits = SIGBREAKF_CTRL_C;
      g_apPath.ap_Strlen = 0;
      /* Recherche le s'il existe au moins un objet DOS correspondant */
      result = MatchFirst(arg,&g_apPath);
      if( result == 0 )
      {
         /* Oui */
         changer_rep = TRUE;
      }
      else
      {
         /* Non */
         FreeVec(pbuffer);
         buffer = (STRPTR)MemoryAlloc(MAX_BUFFER,TRUE);
         if( !pbuffer )
            return FALSE;
         sprintf(buffer,GetString(&md_locale,MSG_NOMATCH),arg);
         RTInfo(buffer,MODE_ACK_INFO);
         MatchEnd(&g_apPath);
         FreeVec(buffer);
         return FALSE;
      }
   }

   /* Un ou plusieurs objets correspondent au joker */
   if( changer_rep == TRUE )
   {
      do
      {
         old_dir = CurrentDir(g_apPath.ap_Current->an_Lock);
         strcpy(pbuffer,g_apPath.ap_Info.fib_FileName);
         if( g_apPath.ap_Info.fib_DirEntryType > 0 )
         {
            /* Cette entree est un repertoire */
#ifdef DEBUG
            printf("Répértoire : %s\n",pbuffer);
#endif
            if( all == TRUE )
            {
               /* On autorise l'entree dans ce repertoire si on n'en vient pas ! */
               if( ! (g_apPath.ap_Flags & APF_DIDDIR) )
                  g_apPath.ap_Flags = g_apPath.ap_Flags | APF_DODIR;
               /* On nettoie pour pouvoir entrer dans d'autres repertoires */
               g_apPath.ap_Flags = g_apPath.ap_Flags & ~APF_DIDDIR;
            }
         }
         else
         {
#ifdef DEBUG
            printf("Occurence : %s\n",pbuffer);
#endif
            /* Ici on ne peut pas savoir s'il s'agit du dernier de la liste donc on reste
               tout le temps dans le meme mode */
            continuer = Recherche(pbuffer,MODE_BROWSE_INFO);
         }
         CurrentDir(old_dir);
      }while( (MatchNext(&g_apPath) == 0) && (continuer == TRUE) );
   }
   else continuer = Recherche(arg,ulMode);

   MatchEnd(&g_apPath);
   FreeVec(pbuffer);

   return continuer;
}

/* Decortique la liste des arguments et appelle Recherche() s'il y a des arguments
   Si l'argument/tooltype AppIcon existe on cree une AppIcon qu'il y ait des arguments ou non
   Si une erreur se produit, on renvoie FALSE pour ouvrir la requete de fichiers
   Renvoie TRUE si une requete de fichiers doit etre ouverte */
BOOL TraitementWB(char **args)
{
   struct WBStartup *wbs;
   struct WBArg *wbarg;
   BPTR old_dir;
   BOOL bContinue=TRUE;
   ULONG ulMode;
   LONG i;

   wbs = (struct WBStartup *)args;
   wbarg = wbs->sm_ArgList;

   /* Lecture des types d'outils
      On en profite pour recuperer la structure DiskObject qui correspond au tooltype APPICON */
   LectureIcone(wbarg->wa_Name,wbarg->wa_Lock);

   /* Pas d'arguments, juste le nom de MD (0 impossible, non ?) */
   if( wbs->sm_NumArgs <= 1 )
      return TRUE;

   /* Sinon on a des arguments : on les traite
      Passer un argument en mode WB met sm_NumArgs a 1
      Avec ToolManager , on a 2 !? */
#ifdef DEBUG
   printf("Nombre d'arguments WB: %ld\n",wbs->sm_NumArgs);
#endif
   /* On passe le nom de MD */
   wbarg++;

   if( wbs->sm_NumArgs > 1 )
      ulMode = MODE_BROWSE_INFO;
   else
      ulMode = MODE_ACK_INFO;

   for(i=1;i<wbs->sm_NumArgs;i++,wbarg++)
   {
      if( wbarg == NULL )
         break;

      /* Quel genre d'objet ne peut pas avoir de lock ? */
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
      /* Changement de mode pour le dernier argument */
      if( i == wbs->sm_NumArgs-1 )
         ulMode = MODE_ACK_INFO;

      /* bContinue a pu etre mis a FALSE par l'appel precedent */
      if( bContinue == TRUE )
         /* Ici on ne peut pas avoir de joker */
         bContinue = Recherche(wbarg->wa_Name,ulMode);

      if( wbarg->wa_Lock && old_dir )
         CurrentDir(old_dir);
   }
#ifdef DEBUG
   printf("Traitement terminé avec l'argument %ld\n",i);
#endif

   /* On desactive le mode AppIcon puisqu on a eu des arguments a traiter */
   app = FALSE;

   return FALSE;
}

/* Ferme "tout" en cas de break inopine */
void Break(void)
{
   LibererConfig();
   FermerLibrairies();
}

/* Ouvre les librairies et le fichier de configuration
   Determine le mode de lancement puis les options eventuelles
   Appelle TraitementCli(), TraitementWB() ou RTRequete() selon le cas */
int main(int argn,char **argv)
{
   struct RDArgs *argsptr;
   BOOL ask_user=FALSE,continuer=TRUE;
   ULONG ulMode;
   STRPTR buffer,*sptr,*nextsptr,strDefault;
   int iNbrArg;

   if( SysVersionCheck() == FALSE )
     return RETURN_ERROR;

   atexit(Break);

   buffer = (STRPTR)MemoryAlloc(MAX_BUFFER,TRUE);
   strDefault = (STRPTR)MemoryAlloc(MAX_BUFFER,TRUE);
   if( (buffer == NULL) || (strDefault == NULL) )
      return RETURN_ERROR;

   /* Ouverture des librairies necessaires */
   if( OuvrirLibrairies() == FALSE )
   {
      printf(GetString(&md_locale,MSG_NOLIBS));
      FermerLibrairies();
      FreeVec(strDefault);
      FreeVec(buffer);
      return RETURN_ERROR;
   }

   if( LectureConfig(&strDefault) == FALSE )
   {
      FermerLibrairies();
      FreeVec(strDefault);
      FreeVec(buffer);
      return RETURN_ERROR;
   }
   /* L'instruction par defaut doit etre disponible a d'autres */
   g_strDef = strDefault;

   /* Teste si MD est lance depuis un Shell car ReadArgs() a besoin d'une entree
      depuis laquelle il lit les parametres
      Dans le cas d'un lancement depuis le WB on aurait alors ouverture d'une fenetre
      Un equivalent serait de tester le retour de Cli() */
   if( argn > 0 )
   {
      /* "Tache CLI" : lecture parametres */
      argsptr = (struct RDArgs *)ReadArgs(TEMPLATE,opts,NULL);
      iNbrArg = argn-1;
      if( argsptr != NULL )
      {
         if( opts[OPT_ALL] )
         {
#ifdef DEBUG
            printf("Option All activée...\n");
#endif
            all = TRUE;
            iNbrArg--;
         }
         if( opts[OPT_SYNC] )
         {
#ifdef DEBUG
            printf("Option Sync activée...\n");
#endif
            sync = TRUE;
            iNbrArg--;
         }
         if( opts[OPT_SHOW] )
         {
#ifdef DEBUG
            printf("Option ShowDT (et donc Sync) activée...\n");
#endif
            show = TRUE;
            sync = TRUE;
            iNbrArg--;
         }
#ifdef DEBUG
         printf("Lancement avec %d/%d arguments\n",iNbrArg,argn);
#endif

         sptr = (STRPTR *)(opts[OPT_FILES]);
         if( sptr )
         {
            /* Le mode depend du nombre d'arguments */
            if( argn > 1 )
               ulMode = MODE_BROWSE_INFO;
            else ulMode = MODE_ACK_INFO;

            /* Traitement de tous les arguments */
            nextsptr = sptr;
            while( (*sptr) && (continuer == TRUE) )
            {
               nextsptr++;
               /* Changement de mode pour le dernier argument */
               if( iNbrArg == 1 )
                  ulMode = MODE_ACK_INFO;

               continuer = TraitementCli(*sptr,ulMode);
               sptr++;
               iNbrArg--;
            }
         }
         /* Puisque FILES n'est pas obligatoire, on doit demander un choix a l'utilisateur
            tout en gardant les options qu'il a pu active sur la ligne de commande */
         else ask_user = TRUE;
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
         ask_user = TraitementWB(argv);
      else ask_user = TRUE;
   }

   /* On regarde s'il faut iconifier
      Cette option n'est activee qu'en mode WB */
   if( app == TRUE )
       CreerIcone();
   /* Afficher la requete de fichiers ? */
   else if( ask_user == TRUE )
      while( RTRequete() == TRUE );

   FreeVec(strDefault);
   FreeVec(buffer);
   LibererConfig();
   FermerLibrairies();

   return RETURN_OK;
}
