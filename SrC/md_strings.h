/*
**
** MULTI DISPLAY
** " Le vrai-faux viewer universel "
*/

#ifndef MD_STRINGS_H
#define MD_STRINGS_H

#define MAX_BUFFER 512
#define SYSTEMFAIL (-1L) /* SystemTagList() */

#define CATCOMP_NUMBERS
#define CATCOMP_BLOCK

/****************************************************************************/


/* This file was created automatically by CatComp.
 * Do NOT edit by hand!
 * Oops ! I did it ... Sorry !
 */

#include <exec/types.h>

#ifdef CATCOMP_ARRAY
#undef CATCOMP_NUMBERS
#undef CATCOMP_STRINGS
#define CATCOMP_NUMBERS
#define CATCOMP_STRINGS
#endif

#ifdef CATCOMP_BLOCK
#undef CATCOMP_STRINGS
#define CATCOMP_STRINGS
#endif


/****************************************************************************/


#ifdef CATCOMP_NUMBERS

#define MSG_KSVERSION 1
#define MSG_NOLIBS 2
#define MSG_NOMATCH 3
#define MSG_UNKNOWNID 4
#define MSG_SHOWID 5
#define MSG_NOICON 6
#define MSG_REQFILE_TITLE 7
#define MSG_REQINFO_TITLE 8
#define MSG_ICON_TITLE 9
#define MSG_QUIT_APPICON 10

#endif /* CATCOMP_NUMBERS */


/****************************************************************************/


#ifdef CATCOMP_STRINGS

#define MSG_KSVERSION_STR "KS3.x ou supérieur nécessaire...\n"
#define MSG_NOLIBS_STR "Impossible d'ouvrir une des librairies nécessaires...\n"
#define MSG_NOMATCH_STR "Aucun fichier ne correspond à\n%s"
#define MSG_UNKNOWNID_STR "Impossible de déterminer le type de\n%s"
#define MSG_SHOWID_STR "Nom : %s\nType : %s"
#define MSG_NOICON_STR "Impossible de créer une AppIcon.\nL'icône de MD ou l'icône par défaut\nn'existe pas !"
#define MSG_REQFILE_TITLE_STR "Choisir un fichier à ouvrir"
#define MSG_REQINFO_TITLE_STR "MultiDisplay 1.8"
#define MSG_ICON_TITLE_STR "MultiDisplay"
#define MSG_QUIT_APPICON_STR "Merci d'avoir utilisé MultiDisplay.\nL'Amiga vaincra ...\ncolinf@chez.com (Fabrice Colin)"

#endif /* CATCOMP_STRINGS */


/****************************************************************************/


#ifdef CATCOMP_ARRAY

struct CatCompArrayType
{
    LONG   cca_ID;
    STRPTR cca_Str;
};

static const struct CatCompArrayType CatCompArray[] =
{
    {MSG_KSVERSION,(STRPTR)MSG_KSVERSION_STR},
    {MSG_NOLIBS,(STRPTR)MSG_NOLIBS_STR},
    {MSG_NOMATCH,(STRPTR)MSG_NOMATCH_STR},
    {MSG_UNKNOWNID,(STRPTR)MSG_UNKNOWNID_STR},
    {MSG_SHOWID,(STRPTR)MSG_SHOWID_STR},
    {MSG_NOICON,(STRPTR)MSG_NOICON_STR},
    {MSG_REQFILE_TITLE,(STRPTR)MSG_REQFILE_TITLE_STR},
    {MSG_REQINFO_TITLE,(STRPTR)MSG_REQINFO_TITLE_STR},
    {MSG_ICON_TITLE,(STRPTR)MSG_ICON_TITLE_STR},
    {MSG_QUIT_APPICON,(STRPTR)MSG_QUIT_APPICON_STR},
};

#endif /* CATCOMP_ARRAY */


/****************************************************************************/


#ifdef CATCOMP_BLOCK

static const char CatCompBlock[] =
{
    "\x00\x00\x00\x01\x00\x22"
    MSG_KSVERSION_STR "\x00"
    "\x00\x00\x00\x02\x00\x38"
    MSG_NOLIBS_STR "\x00\x00"
    "\x00\x00\x00\x03\x00\x22"
    MSG_NOMATCH_STR "\x00\x00"
    "\x00\x00\x00\x04\x00\x28"
    MSG_UNKNOWNID_STR "\x00\x00"
    "\x00\x00\x00\x05\x00\x14"
    MSG_SHOWID_STR "\x00\x00"
    "\x00\x00\x00\x06\x00\x54"
    MSG_NOICON_STR "\x00"
    "\x00\x00\x00\x07\x00\x1C"
    MSG_REQFILE_TITLE_STR "\x00"
    "\x00\x00\x00\x08\x00\x12"
    MSG_REQINFO_TITLE_STR "\x00\x00"
    "\x00\x00\x00\x09\x00\x0E"
    MSG_ICON_TITLE_STR "\x00\x00"
    "\x00\x00\x00\x0A\x00\x58"
    MSG_QUIT_APPICON_STR "\x00"
};

#endif /* CATCOMP_BLOCK */


/****************************************************************************/


struct LocaleInfo
{
    APTR li_LocaleBase;
    APTR li_Catalog;
};

/****************************************************************************/


#endif /* MD_STRINGS_H */
