/*
**
** MULTI DISPLAY
** " Le vrai-faux viewer universel "
*/
#ifndef MD_DEFS_H
#define MD_DEFS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <exec/libraries.h>
#include <dos/dos.h>
#include <dos/dostags.h>
#include <dos/dosasl.h>
#include <dos/rdargs.h>
#include <dos/dosextens.h>
#include <utility/tagitem.h>
#include <datatypes/datatypes.h>
#include <datatypes/datatypesclass.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>

#include <libraries/locale.h>
#include <libraries/iffparse.h>
#include <libraries/reqtools.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/locale_pragmas.h>
#include <pragmas/icon_pragmas.h>
#include <pragmas/datatypes_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/reqtools_pragmas.h>
#include <pragmas/wb_pragmas.h>

#define MAX_BUFFER 512
#define SYSTEMFAIL (-1L)

#define CATCOMP_NUMBERS
#define CATCOMP_BLOCK

/* RTInfo () modes */
#define MODE_ACK_INFO           0L
#define MODE_BROWSE_INFO        1L

/* RTInfo() return values */
#define REPLY_OK_INFO           0L
#define REPLY_NEXT_INFO         1L
#define REPLY_STOP_INFO         2L

struct ConfigElem
{
 STRPTR m_strId;                /* This element ID (eg gif or #.TXT) */
 BOOL   m_bDatatype;            /* TRUE if m_strId is a datatype ID */
 STRPTR m_strExe;               /* Program to be launched */
 struct ConfigElem      *m_pceNext;
};

#endif  /* MD_DEFS_H */
