/*
**
** MULTI DISPLAY
*/
#ifndef MD_FONCTIONS_H
#define MD_FONCTIONS_H

BOOL SysVersionCheck(void);
BOOL OuvrirLibrairies(void);
void FermerLibrairies(void);
STRPTR GetString(struct LocaleInfo *li, LONG stringNum);
void *MemoryAlloc(ULONG ulSize,BOOL bClear);
BOOL ComparaisonID(STRPTR buffer,STRPTR id);
BOOL ComparaisonNom(STRPTR buffer,STRPTR arg);
BOOL Recherche(STRPTR arg,ULONG ulMode);

#endif  /* MD_FONCTIONS_H */
