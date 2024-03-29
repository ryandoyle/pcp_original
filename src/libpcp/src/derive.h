/*
 * Copyright (c) 2009 Ken McDonell.  All Rights Reserved.
 * 
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 */

#include <ctype.h>
#include "pmapi.h"
#include "impl.h"

/*
 * Derived Metrics support
 */

typedef struct {		/* one value in the expression tree */
    int		inst;
    pmAtomValue	value;
    int		vlen;		/* from vlen of pmValueBlock for string and aggregates */
} val_t;

typedef struct {		/* dynamic information for an expression node */
    pmID	pmid;
    int		numval;		/* length of ivlist[] */
    int		mul_scale;	/* scale multiplier */
    int		div_scale;	/* scale divisor */
    val_t	*ivlist;	/* instance-value pairs */
    int		last_numval;	/* length of last_ivlist[] */
    val_t	*last_ivlist;	/* values from previous fetch for delta() */

} info_t;

typedef struct node {		/* expression tree node */
    int		type;
    pmDesc	desc;
    int		save_last;
    struct node	*left;
    struct node	*right;
    char	*value;
    info_t	*info;
} node_t;

typedef struct {		/* one derived metric */
    char	*name;
    pmID	pmid;
    node_t	*expr;
} dm_t;

/*
 * Control structure for a set of derived metrics.
 * This is used for the static definitions (registered) and the dynamic
 * tree of expressions maintained per context.
 */
typedef struct {
#ifdef PM_MULTI_THREAD
    pthread_mutex_t	mutex;
#endif
    int			nmetric;	/* derived metrics */
    dm_t		*mlist;
    int			fetch_has_dm;	/* ==1 if pmResult rewrite needed */
    int			numpmid;	/* from pmFetch before rewrite */
} ctl_t;

/* lexical types */
#define L_ERROR		-2
#define	L_EOF		-1
#define L_UNDEF		0
#define L_NUMBER	1
#define L_NAME		2
#define L_PLUS		3
#define L_MINUS		4
#define L_STAR		5
#define L_SLASH		6
#define L_LPAREN	7
#define L_RPAREN	8
#define L_AVG		9
#define L_COUNT		10
#define L_DELTA		11
#define L_MAX		12
#define L_MIN		13
#define L_SUM		14
#define L_ANON		15

void __dmdumpexpr(node_t *, int);
