/*
 * Copyright (c) 1997 Silicon Graphics, Inc.  All Rights Reserved.
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "pmapi.h"
#include "impl.h"
#include "pmda.h"

/* yacc/lex routines */
extern char lastinput(void);
extern void yyerror(const char *);
extern void yywarn(char *);
extern int yywrap(void);
extern int yylex(void);
extern int markpos(void);
extern void locateError(void);

/* utiltity routines */
extern void setup_context(void);
extern void reset_profile(void);
extern char *strcons(char *, char *);
extern char *strnum(int);
extern void initmetriclist(void);
extern void addmetriclist(pmID);
extern void initarglist(void);
extern void addarglist(char *);
extern void doargs(void);
extern void printindom(FILE *, __pmInResult *);
extern void dohelp(int, int);
extern void dostatus(void);
extern int fillResult(pmResult *, int);
extern void _dbDumpResult(FILE *, pmResult *, pmDesc *);

/* pmda exerciser routines */
extern void opendso(char *, char *, int);
extern void closedso(void);
extern void dodso(int);
extern void openpmda(char *);
extern void closepmda(void);
extern void dopmda(int);
extern void watch(char *);
extern void opensocket(char *);

/*
 * connection states
 */
#define NO_CONN		-1
#define CONN_DSO	0
#define CONN_DAEMON	1
extern int	connmode;

/* parameters for action routines ... */
typedef struct {
    int		number;
    char	*name;
    pmID	pmid;
    pmInDom	indom;
    int		numpmid;
    pmID	*pmidlist;
    int		argc;
    char	**argv;
} param_t;

extern param_t	param;

/* the single profile */
extern __pmProfile	*profile;
extern int		profile_changed;

/* status info */
extern char		*myPmdaName;

/* help text formats */
#define HELP_USAGE	0
#define HELP_FULL	1

/* timing information */
extern int timer;

/* get descriptor for fetch or not */
extern int get_desc;

/* namespace pathnames */
extern char *pmnsfile;
extern char *cmd_namespace;
