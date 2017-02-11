/*
**
** MULTI DISPLAY
** " Le vrai-faux viewer universel "
*/
#include "defs.h"
#include "mdstrings.h"
/* Fonctions externes */
#include "mdprincipal.h"        /* TrouverConfig */
#include "mddatatypes.h"        /* ExamenDT */
#include "mdreqtools.h"         /* RTInfo */

extern struct Library *SysBase,*DOSBase;
struct Library *WorkbenchBase,*LocaleBase,*IconBase,*DataTypesBase,*IFFParseBase,*ReqToolsBase;

extern BOOL sync,show;                          /* Parametres */
extern STRPTR g_strDef;                         /* Instruction par defaut */
struct LocaleInfo md_locale;                    /* Pour GetString() */

/* Verify sytem version */
BOOL SysVersionCheck(void)
{
    if (SysBase->lib_Version < 39)
    {
        return FALSE;
    }
    return TRUE;
}

/* Ouvre toutes les librairies necessaires ainsi que le catalogue de MD */
BOOL OuvrirLibrairies(void)
{
   struct TagItem LocaleItems[2];
   BOOL result=TRUE;

   /* Français est le langage par defaut , n'est ce pas ? */
   LocaleItems[0].ti_Tag = OC_BuiltInLanguage;
   LocaleItems[0].ti_Data = (ULONG)"français";
   LocaleItems[1].ti_Tag = TAG_DONE;

   if( !(WorkbenchBase = OpenLibrary("workbench.library",39)) )
      result = FALSE;
   if( !(IconBase = OpenLibrary("icon.library",39)) )
      result = FALSE;
   if( (LocaleBase = OpenLibrary("locale.library",38)) )
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
   if( ReqToolsBase )
   {
      CloseLibrary(ReqToolsBase);
      ReqToolsBase = NULL;
   }
   if( ReqToolsBase )
   {
      CloseLibrary(IFFParseBase);
      ReqToolsBase = NULL;
   }
   if( md_locale.li_Catalog )
   {
      CloseCatalog(md_locale.li_Catalog);
      md_locale.li_Catalog = NULL;
   }
   if( LocaleBase )
   {
      CloseLibrary(LocaleBase);
      LocaleBase = NULL;
   }
   if( ReqToolsBase )
   {
      CloseLibrary(DataTypesBase);
      DataTypesBase = NULL;
   }
   if( IconBase )
   {
      CloseLibrary(IconBase);
      IconBase = NULL;
   }
   if( WorkbenchBase )
   {
      CloseLibrary(WorkbenchBase);
      WorkbenchBase = NULL;
   }
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

void *MemoryAlloc(ULONG ulSize,BOOL bClear)
{
   ULONG ulFlags = MEMF_PUBLIC|MEMF_REVERSE;
   void *pMem;

   if( bClear == TRUE )
      ulFlags = ulFlags | MEMF_CLEAR;
   pMem = AllocVec(ulSize,ulFlags);
   if( pMem == NULL )
   {
#ifdef DEBUG
      printf("Couldn't allocate %ld bytes\n",ulSize);
#endif
      exit(RETURN_ERROR);
   }
#ifdef DEBUG
   printf("%ld bytes allocated at %p\n",ulSize,pMem);
#endif
   return pMem;
}

/* Passe la commande adéquate au systéme */
BOOL Commande(STRPTR commande,STRPTR arg)
{
   struct TagItem taglist[5];
   STRPTR buffer;
   BOOL result=TRUE;
   int i=2;

   buffer = (STRPTR)MemoryAlloc(MAX_BUFFER,TRUE);
   if( buffer == NULL )
      return FALSE;

   /* Teste si l argument existe */
   if( arg != NULL )
   {
      sprintf(buffer,"%s \"%s\"\0",commande,arg);
   }
#ifdef DEBUG
   printf("Appel : %s\n",buffer);
#endif

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
   if( SystemTagList(buffer, taglist) == SYSTEMFAIL )
      result = FALSE;

   FreeVec(buffer);

   return result;
}

/* Compare le type decrit dans le fichier de config avec le nom
   du datatype correspondant */
BOOL ComparaisonID(STRPTR buffer,STRPTR id)
{
   BOOL arret=FALSE;

   if( stricmp(buffer,id) == 0 )
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

   arg2 = (STRPTR)MemoryAlloc(MAX_BUFFER,TRUE);
   buffer2 = (STRPTR)MemoryAlloc(MAX_BUFFER,TRUE);
   if( (!arg2) || (!buffer2) )
      exit(1);

   for(i=0;i<strlen( arg );i++)
      arg2[i] = tolower(arg[i]);
   arg2[i] = '\0';
   for(i=0;i<strlen( buffer );i++)
      buffer2[i] = tolower(buffer[i+1]);

   if( strstr(arg2,buffer2) )
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
   de config et le retour de ExamenDT()
   complex indique si RTInfo() doit avoir un ou trois boutons
   Retourne FALSE si MD doit tout stopper */
BOOL Recherche(STRPTR arg,ULONG ulMode)
{
   BOOL bResult=TRUE, bReturn = FALSE;
   STRPTR buffer,strGroup,strBaseName;

   buffer = (STRPTR)MemoryAlloc(MAX_BUFFER,TRUE);
   if( buffer == NULL )
      exit(1);

   /* Le datatype n'a pas pu être trouvé */
   if( ExamenDT(arg,&strGroup,&strBaseName) == FALSE )
   {
#ifdef DEBUG
      printf("Type de données inconnu ...\n");
#endif
      sprintf(buffer,GetString(&md_locale,MSG_UNKNOWNID),arg);
      switch( RTInfo(buffer,ulMode) )
      {
         case REPLY_OK_INFO:
            bResult = Commande(g_strDef,arg);   /* OK : passer la commande */
            break;
         case REPLY_NEXT_INFO:                  /* Next: ne pas examiner ce fichier */
            bResult = TRUE;
            break;
         case REPLY_STOP_INFO:                  /* Stop : stopper tout */
         default:
            bResult = FALSE;
            break;
      }
      /* On arrete la pour ce fichier */
      bReturn = TRUE;
   }
   else if( show == TRUE )
   {
      sprintf(buffer,GetString(&md_locale,MSG_SHOWID),arg,strGroup,strBaseName);
      switch( RTInfo(buffer,ulMode) )
      {
         case REPLY_OK_INFO:                    /* OK : continuer */
            break;
         case REPLY_NEXT_INFO:                  /* Ne pas examiner ce fichier */
            bResult = TRUE;
            bReturn = TRUE;
            break;
         case REPLY_STOP_INFO:                  /* Stopper tout */
         default:
            bResult = FALSE;
            bReturn = TRUE;
            break;
      }
   }

   /* On peut liberer ces chaines maintenant */
   FreeVec(buffer);
   if( strGroup != NULL )
      FreeVec(strGroup);
   buffer = NULL;
   /* Est ce qu'il faut quitter ? */
   if( bReturn == TRUE )
   {
      if( strBaseName != NULL )
         FreeVec(strBaseName);
      return bResult;
   }

#ifdef DEBUG
    printf("Comparaisons ...\n");
#endif
   buffer = TrouverConfig(arg,strBaseName);
   if( buffer == NULL )
      bResult = Commande(g_strDef,arg);
   else
      bResult = Commande(buffer,arg);

#ifdef DEBUG
   if( bResult == FALSE )
      printf("Echec sur %s ...\n",arg);
#endif

   if( strBaseName != NULL )
      FreeVec(strBaseName);

   return bResult;
}
