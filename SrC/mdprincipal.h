/*
**
** MULTI DISPLAY
*/
#ifndef MD_PRINCIPAL_H
#define MD_PRINCIPAL_H

BOOL TraitementCli(STRPTR arg,ULONG ulMode);
BOOL TraitementWB(struct WBArg *wbarg, LONG lArgsNbr);
const STRPTR TrouverConfig(STRPTR arg,STRPTR id);

#endif  /* MD_PRINCIAPL_H */
