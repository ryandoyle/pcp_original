/*
 * Copyright (c) 1995-2000 Silicon Graphics, Inc.  All Rights Reserved.
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

#include "pmapi.h"
#include "impl.h"
#include "pmda.h"
#include "libdefs.h"

/*
 * count the number of instances in an instance domain
 */

int
__pmdaCntInst(pmInDom indom, pmdaExt *pmda)
{
    int		i;
    int		sts = 0;

    if (indom == PM_INDOM_NULL)
	return 1;
    if (pmdaCacheOp(indom, PMDA_CACHE_CHECK)) {
	sts = pmdaCacheOp(indom, PMDA_CACHE_SIZE_ACTIVE);
    }
    else {
	for (i = 0; i < pmda->e_nindoms; i++) {
	    if (pmda->e_indoms[i].it_indom == indom) {
		sts = pmda->e_indoms[i].it_numinst;
		break;
	    }
	}
	if (i == pmda->e_nindoms) {
	    char	strbuf[20];
	    __pmNotifyErr(LOG_WARNING, "cntinst: unknown indom %s", pmInDomStr_r(indom, strbuf, sizeof(strbuf)));
	}
    }

#ifdef PCP_DEBUG
    if (pmDebug & DBG_TRACE_INDOM) {
	char	strbuf[20];
	fprintf(stderr, "__pmdaCntInst(indom=%s) -> %d\n", pmInDomStr_r(indom, strbuf, sizeof(strbuf)), sts);
    }
#endif

    return sts;
}

/*
 * commence a new round of instance selection
 */

static pmdaIndom	last;

/*
 * State between here and __pmdaNextInst is a little strange
 *
 * for the classical method,
 *    - pmda->e_idp is set here (points into indomtab[]) and
 *      pmda->e_idp->it_indom is used in __pmdaNextInst
 *
 * for the cache method
 *    - pmda->e_idp is set here (points into last) which is also set
 *      up with the it_indom field (other fields in last are not used),
 *      and pmda->e_idp->it_indom in __pmdaNextInst
 *
 * In both cases, pmda->e_ordinal and pmda->e_singular are set here
 * and updated in __pmdaNextInst.
 *
 * As in most other places, this is not thread-safe and we assume we
 * call __pmdaStartInst and then repeatedly call __pmdaNextInst all
 * for the same indom, before calling __pmdaStartInst again.
 *
 * If we could do this again, adding an indom argument to __pmdaNextInst
 * would be a better design, but the API to __pmdaNextInst has escaped.
 */
void
__pmdaStartInst(pmInDom indom, pmdaExt *pmda)
{
    int		i;

    pmda->e_ordinal = pmda->e_singular = -1;
    if (indom == PM_INDOM_NULL) {
	/* singular value */
	pmda->e_singular = 0;
    }
    else {
	if (pmdaCacheOp(indom, PMDA_CACHE_CHECK)) {
	    pmdaCacheOp(indom, PMDA_CACHE_WALK_REWIND);
	    last.it_indom = indom;
	    pmda->e_idp = &last;
	    pmda->e_ordinal = 0;
	}
	else {
	    for (i = 0; i < pmda->e_nindoms; i++) {
		if (pmda->e_indoms[i].it_indom == indom) {
		    /* multiple values are possible */
		    pmda->e_idp = &pmda->e_indoms[i];
		    pmda->e_ordinal = 0;
		    break;
		}
	    }
	}
    }

#ifdef PCP_DEBUG
    if (pmDebug & DBG_TRACE_INDOM) {
	char	strbuf[20];
	fprintf(stderr, "__pmdaStartInst(indom=%s) e_ordinal=%d\n",
	    pmInDomStr_r(indom, strbuf, sizeof(strbuf)), pmda->e_ordinal);
    }
#endif
    return;
}

/* 
 * select the next instance
 */

int
__pmdaNextInst(int *inst, pmdaExt *pmda)
{
    int		j;
    int		myinst;

    if (pmda->e_singular == 0) {
	/* PM_INDOM_NULL ... just the one value */
	*inst = 0;
	pmda->e_singular = -1;
	return 1;
    }
    if (pmda->e_ordinal >= 0) {
	/* scan for next value in the profile */
	if (pmda->e_idp == &last) {
	    /* cache-driven */
	    while ((myinst = pmdaCacheOp(pmda->e_idp->it_indom, PMDA_CACHE_WALK_NEXT)) != -1) {
		pmda->e_ordinal++;
		if (__pmInProfile(pmda->e_idp->it_indom, pmda->e_prof, myinst)) {
		    *inst = myinst;
#ifdef PCP_DEBUG
		    if (pmDebug & DBG_TRACE_INDOM) {
			char	strbuf[20];
			fprintf(stderr, "__pmdaNextInst(indom=%s) -> %d e_ordinal=%d (cache)\n",
			    pmInDomStr_r(pmda->e_idp->it_indom, strbuf, sizeof(strbuf)), myinst, pmda->e_ordinal);
		    }
#endif
		    return 1;
		}
	    }
	}
	else {
	    /* indomtab[]-driven */
	    for (j = pmda->e_ordinal; j < pmda->e_idp->it_numinst; j++) {
		if (__pmInProfile(pmda->e_idp->it_indom, 
				 pmda->e_prof, 
				 pmda->e_idp->it_set[j].i_inst)) {
		    *inst = pmda->e_idp->it_set[j].i_inst;
		    pmda->e_ordinal = j+1;
#ifdef PCP_DEBUG
		    if (pmDebug & DBG_TRACE_INDOM) {
			char	strbuf[20];
			fprintf(stderr, "__pmdaNextInst(indom=%s) -> %d e_ordinal=%d\n",
			    pmInDomStr_r(pmda->e_idp->it_indom, strbuf, sizeof(strbuf)), *inst, pmda->e_ordinal);
		    }
#endif
		    return 1;
		}
	    }
	}
	pmda->e_ordinal = -1;
    }
    return 0;
}

/*
 * Save the profile away for use in __pmdaNextInst() during subsequent
 * fetches ... it is the _caller_ of pmdaProfile()'s responsibility to
 * ensure that the profile is not freed while it is being used here.
 *
 * For DSO pmdas, the profiles are managed per client in DoProfile() and
 * DeleteClient() but sent to the pmda in SendFetch()
 *
 * For daemon pmdas, the profile is received from pmcd in __pmdaMainPDU
 * and the last received profile is held there
 */

int
pmdaProfile(__pmProfile *prof, pmdaExt *pmda)
{
    pmda->e_prof = prof;
    return 0;
}

/*
 * return desciption of an instance or instance domain
 */

int
pmdaInstance(pmInDom indom, int inst, char *name, __pmInResult **result, pmdaExt *pmda)
{
    int			i;
    int			namelen;
    int			err = 0;
    __pmInResult  	*res;
    pmdaIndom		*idp = NULL;	/* initialize to pander to gcc */
    int			have_cache = 0;
    int			myinst;
    char		*np;

    if (pmdaCacheOp(indom, PMDA_CACHE_CHECK)) {
	have_cache = 1;
    }
    else {
	/*
	 * check this is an instance domain we know about -- code below
	 * assumes this test is complete
	 */
	for (i = 0; i < pmda->e_nindoms; i++) {
	    if (pmda->e_indoms[i].it_indom == indom)
		break;
	}
	if (i >= pmda->e_nindoms)
	    return PM_ERR_INDOM;
	idp = &pmda->e_indoms[i];
    }

    if ((res = (__pmInResult *)malloc(sizeof(*res))) == NULL)
        return -oserror();
    res->indom = indom;

    if (name == NULL && inst == PM_IN_NULL)
	res->numinst = __pmdaCntInst(indom, pmda);
    else
	res->numinst = 1;

    if (inst == PM_IN_NULL) {
	if ((res->instlist = (int *)malloc(res->numinst * sizeof(res->instlist[0]))) == NULL) {
	    free(res);
	    return -oserror();
	}
    }
    else
	res->instlist = NULL;

    if (name == NULL) {
	if ((res->namelist = (char **)malloc(res->numinst * sizeof(res->namelist[0]))) == NULL) {
	    __pmFreeInResult(res);
	    return -oserror();
	}
	for (i = 0; i < res->numinst; i++)
	    res->namelist[0] = NULL;
    }
    else
	res->namelist = NULL;

    if (name == NULL && inst == PM_IN_NULL) {
	/* return inst and name for everything */
	if (have_cache) {
	    pmdaCacheOp(indom, PMDA_CACHE_WALK_REWIND);
	    i = 0;
	    while (i < res->numinst && (myinst = pmdaCacheOp(indom, PMDA_CACHE_WALK_NEXT)) != -1) {
		if (pmdaCacheLookup(indom, myinst, &np, NULL) != PMDA_CACHE_ACTIVE)
		    continue;

		res->instlist[i] = myinst;
		if ((res->namelist[i++] = strdup(np)) == NULL) {
		    __pmFreeInResult(res);
		    return -oserror();
		}
	    }
	}
	else {
	    for (i = 0; i < res->numinst; i++) {
		res->instlist[i] = idp->it_set[i].i_inst;
		if ((res->namelist[i] = strdup(idp->it_set[i].i_name)) == NULL) {
		    __pmFreeInResult(res);
		    return -oserror();
		}
	    }
	}
    }
    else if (name == NULL) {
	/* given an inst, return the name */
	if (have_cache) {
	    if (pmdaCacheLookup(indom, inst, &np, NULL) == PMDA_CACHE_ACTIVE) {
		if ((res->namelist[0] = strdup(np)) == NULL) {
		    __pmFreeInResult(res);
		    return -oserror();
		}
	    }
	    else
		err = 1;
	}
	else {
	    for (i = 0; i < idp->it_numinst; i++) {
		if (inst == idp->it_set[i].i_inst) {
		    if ((res->namelist[0] = strdup(idp->it_set[i].i_name)) == NULL) {
			__pmFreeInResult(res);
			return -oserror();
		    }
		    break;
		}
	    }
	    if (i == idp->it_numinst)
		err = 1;
	}
    }
    else if (inst == PM_IN_NULL && (namelen = (int)strlen(name)) > 0) {
	if (have_cache) {
	    if (pmdaCacheLookupName(indom, name, &myinst, NULL) == PMDA_CACHE_ACTIVE)
		res->instlist[0] = myinst;
	    else
		err = 1;
	}
	else {
	    /* given a name, return an inst. If the name contains spaces,
	     * only exact matches are good enough for us, otherwise, we're
	     * prepared to accept a match upto the first space in the
	     * instance name on the assumption that pmdas will play by the
	     * rules and guarantee the first "word" in the instance name
	     * is unique. That allows for the things like "1 5 15" to match
	     * instances for kernel.all.load["1 minute","5 minute","15 minutes"]
	     */
	    char * nspace = strchr (name, ' ');

	    for (i = 0; i < idp->it_numinst; i++) {
		char *instname = idp->it_set[i].i_name;
		if (strcmp(name, instname) == 0) {
		    /* accept an exact match */
#ifdef PCP_DEBUG
		    if (pmDebug & DBG_TRACE_LIBPMDA) {
			fprintf(stderr, 
				"pmdaInstance: exact match name=%s id=%d\n",
				name, idp->it_set[i].i_inst);
		    }
#endif
		    res->instlist[0] = idp->it_set[i].i_inst;
		    break;
		}
		else if (nspace == NULL) {
		    /* all of name must match instname up to the the first
		     * space in instname.  */
		    char *p = strchr(instname, ' ');
		    if (p != NULL) {
			int len = (int)(p - instname);
			if (namelen == len && strncmp(name, instname, len) == 0) {
#ifdef PCP_DEBUG
			    if (pmDebug & DBG_TRACE_LIBPMDA) {
				fprintf(stderr, "pmdaInstance: matched argument name=\"%s\" with indom id=%d name=\"%s\" len=%d\n",
				    name, idp->it_set[i].i_inst, instname, len);
			    }
#endif
			    res->instlist[0] = idp->it_set[i].i_inst;
			    break;
			}
		    }
		}
	    }
	    if (i == idp->it_numinst)
		err = 1;
	}
    }
    else
	err = 1;
    if (err == 1) {
	/* bogus arguments or instance id/name */
	__pmFreeInResult(res);
	return PM_ERR_INST;
    }

    *result = res;
    return 0;
}

/*
 * resize the pmResult and call the e_callback for each metric instance
 * required in the profile.
 */

int
pmdaFetch(int numpmid, pmID pmidlist[], pmResult **resp, pmdaExt *pmda)
{
    int			i;		/* over pmidlist[] */
    int			j;		/* over metatab and vset->vlist[] */
    int			sts;
    int			need;
    int			inst;
    int			numval;
    pmValueSet		*vset;
    pmDesc		*dp;
    __pmID_int		*pmidp;
    pmdaMetric		*metap;
    pmAtomValue		atom;
    int			type;
    e_ext_t		*extp = (e_ext_t *)pmda->e_ext;

    if (extp->pmda_interface >= PMDA_INTERFACE_5)
	__pmdaSetContext(pmda->e_context);

    if (numpmid > extp->maxnpmids) {
	if (extp->res != NULL)
	    free(extp->res);
	/* (numpmid - 1) because there's room for one valueSet in a pmResult */
	need = (int)sizeof(pmResult) + (numpmid - 1) * (int)sizeof(pmValueSet *);
	if ((extp->res = (pmResult *) malloc(need)) == NULL)
	    return -oserror();
	extp->maxnpmids = numpmid;
    }

    extp->res->timestamp.tv_sec = 0;
    extp->res->timestamp.tv_usec = 0;
    extp->res->numpmid = numpmid;

    for (i = 0; i < numpmid; i++) {

    	dp = NULL;
	metap = NULL;
	pmidp = (__pmID_int *)&pmidlist[i];

	if (pmda->e_direct) {
/*
 * pmidp->domain is correct ... PMCD gurantees this, but
 * next to check the cluster
 */
	    if (pmidp->item < pmda->e_nmetrics &&
		pmidlist[i] == pmda->e_metrics[pmidp->item].m_desc.pmid) {
/* 
 * pmidp->item is unsigned, so must be >= 0 
 */
		metap = &pmda->e_metrics[pmidp->item];
		dp = &(metap->m_desc);
	    }
	}
	else {
	    for (j = 0; j < pmda->e_nmetrics; j++) {
		if (pmidlist[i] == pmda->e_metrics[j].m_desc.pmid) {
/*
 * found the hard way 
 */
		    metap = &pmda->e_metrics[j];
		    dp = &(metap->m_desc);
		    break;
		}
	    }
	}
	
	if (dp == NULL) {
	    /* dynamic name metrics may often vanish, avoid log spam */
	    if (extp->pmda_interface < PMDA_INTERFACE_4) {
		char	strbuf[20];
		__pmNotifyErr(LOG_ERR,
			"pmdaFetch: Requested metric %s is not defined",
			 pmIDStr_r(pmidlist[i], strbuf, sizeof(strbuf)));
	    }
	    numval = PM_ERR_PMID;
	}
	else {

	    if (dp->indom != PM_INDOM_NULL) {
		/* count instances in the profile */
		numval = 0;
		/* count instances in indom */
		__pmdaStartInst(dp->indom, pmda);
		while (__pmdaNextInst(&inst, pmda)) {
		    numval++;
		}
	    }
	    else {
		/* singular instance domains */
		numval = 1;
	    }
	}

	/* Must use individual malloc()s because of pmFreeResult() */
	if (numval >= 1)
	    extp->res->vset[i] = vset = (pmValueSet *)malloc(sizeof(pmValueSet) +
					    (numval - 1)*sizeof(pmValue));
	else
	    extp->res->vset[i] = vset = (pmValueSet *)malloc(sizeof(pmValueSet) -
					    sizeof(pmValue));
	if (vset == NULL) {
	    sts = -oserror();
	    goto error;
	}
	vset->pmid = pmidlist[i];
	vset->numval = numval;
	vset->valfmt = PM_VAL_INSITU;
	if (vset->numval <= 0)
	    continue;

	if (dp->indom == PM_INDOM_NULL)
	    inst = PM_IN_NULL;
	else {
	    __pmdaStartInst(dp->indom, pmda);
	    __pmdaNextInst(&inst, pmda);
	}
	type = dp->type;
	j = 0;
	do {
	    if (j == numval) {
		/* more instances than expected! */
		numval++;
		extp->res->vset[i] = vset = (pmValueSet *)realloc(vset,
			    sizeof(pmValueSet) + (numval - 1)*sizeof(pmValue));
		if (vset == NULL) {
		    sts = -oserror();
		    goto error;
		}
	    }
	    vset->vlist[j].inst = inst;

	    if ((sts = (*(pmda->e_fetchCallBack))(metap, inst, &atom)) < 0) {

		if (sts == PM_ERR_PMID) {
		    char	strbuf[20];
		    __pmNotifyErr(LOG_ERR, 
		        "pmdaFetch: PMID %s not handled by fetch callback\n",
				 pmIDStr_r(dp->pmid, strbuf, sizeof(strbuf)));
		}
		else if (sts == PM_ERR_INST) {
#ifdef PCP_DEBUG
		    if (pmDebug & DBG_TRACE_LIBPMDA) {
			char	strbuf[20];
			__pmNotifyErr(LOG_ERR,
			    "pmdaFetch: Instance %d of PMID %s not handled by fetch callback\n",
				     inst, pmIDStr_r(dp->pmid, strbuf, sizeof(strbuf)));
		    }
#endif
		}
		else if (sts == PM_ERR_APPVERSION ||
			(sts == PM_ERR_AGAIN) ||
			(sts == PM_ERR_PERMISSION)) {
#ifdef PCP_DEBUG
		    if (pmDebug & DBG_TRACE_LIBPMDA) {
			char	strbuf[20];
			__pmNotifyErr(LOG_ERR,
				 "pmdaFetch: Unavailable metric PMID %s\n",
				 pmIDStr_r(dp->pmid, strbuf, sizeof(strbuf)));
		    }
#endif
		}
		else
		    __pmNotifyErr(LOG_ERR,
				 "pmdaFetch: Fetch callback error: %s\n",
				 pmErrStr(sts));
	    }
	    else {
		/*
		 * PMDA_INTERFACE_2
		 *	>= 0 => OK
		 * PMDA_INTERFACE_3 or PMDA_INTERFACE_4
		 *	== 0 => no values
		 *	> 0  => OK
		 * PMDA_INTERFACE_5 or later
		 *	== 0 (PMDA_FETCH_NOVALUES) => no values
		 *	== 1 (PMDA_FETCH_STATIC) or > 2 => OK
		 *	== 2 (PMDA_FETCH_DYNAMIC) => OK and free(atom.vp)
		 *	     after __pmStuffValue() called
		 */
		if (extp->pmda_interface == PMDA_INTERFACE_2 ||
		    (extp->pmda_interface >= PMDA_INTERFACE_3 && sts > 0)) {
		    int		lsts;

		    if ((lsts = __pmStuffValue(&atom, &vset->vlist[j], type)) == PM_ERR_TYPE) {
			char	strbuf[20];
			char	st2buf[20];
			__pmNotifyErr(LOG_ERR, 
				     "pmdaFetch: Descriptor type (%s) for metric %s is bad",
				     pmTypeStr_r(type, strbuf, sizeof(strbuf)),
				     pmIDStr_r(dp->pmid, st2buf, sizeof(st2buf)));
		    }
		    else if (lsts >= 0) {
			vset->valfmt = lsts;
			j++;
		    }
		    if (extp->pmda_interface >= PMDA_INTERFACE_5 && sts == PMDA_FETCH_DYNAMIC) {
			if (type == PM_TYPE_STRING)
			    free(atom.cp);
			else if (type == PM_TYPE_AGGREGATE)
			    free(atom.vbp);
			else {
			    char	strbuf[20];
			    char	st2buf[20];
			    __pmNotifyErr(LOG_WARNING,
					  "pmdaFetch: Attempt to free value for metric %s of wrong type %s\n",
					  pmIDStr_r(dp->pmid, strbuf, sizeof(strbuf)),
					  pmTypeStr_r(type, st2buf, sizeof(st2buf)));
			}
		    }
		    if (lsts < 0)
			sts = lsts;
		}
	    }
	} while (dp->indom != PM_INDOM_NULL && __pmdaNextInst(&inst, pmda));

	if (j == 0)
	    vset->numval = sts;
	else
	    vset->numval = j;

    }
    *resp = extp->res;
    return 0;

 error:

    if (i) {
	extp->res->numpmid = i;
	__pmFreeResultValues(extp->res);
    }
    return sts;
}

/*
 * Return the metric description
 */

int
pmdaDesc(pmID pmid, pmDesc *desc, pmdaExt *pmda)
{
    int			j;
    int			sts = 0;
    e_ext_t		*extp = (e_ext_t *)pmda->e_ext;

    if (extp->pmda_interface >= PMDA_INTERFACE_5)
	__pmdaSetContext(pmda->e_context);

    if (pmda->e_direct) {
	__pmID_int	*pmidp = (__pmID_int *)&pmid;
	/*
	 * pmidp->domain is correct ... PMCD gurantees this, but
	 * pmda->e_direct only works for a single cluster
	 */
	if (pmidp->item < pmda->e_nmetrics && 
	    pmidp->cluster == 
	     	((__pmID_int *)&pmda->e_metrics[pmidp->item].m_desc.pmid)->cluster) {
	    /* pmidp->item is unsigned, so must be >= 0 */
	    *desc = pmda->e_metrics[pmidp->item].m_desc;
	}
	else {
	    char	strbuf[20];
	    __pmNotifyErr(LOG_ERR, "Requested metric %s is not defined",
			 pmIDStr_r(pmid, strbuf, sizeof(strbuf)));
	    sts = PM_ERR_PMID;
	}
    }
    else {
	for (j = 0; j < pmda->e_nmetrics; j++) {
	    if (pmda->e_metrics[j].m_desc.pmid == pmid) {
		/* found the hard way */
		*desc = pmda->e_metrics[j].m_desc;
		break;
	    }
	}
	if (j == pmda->e_nmetrics) {
	    char	strbuf[20];
	    __pmNotifyErr(LOG_ERR, "Requested metric %s is not defined",
			 pmIDStr_r(pmid, strbuf, sizeof(strbuf)));
	    sts = PM_ERR_PMID;
	}
    }
    return sts;
}

/*
 * Return the help text for a metric
 */

int
pmdaText(int ident, int type, char **buffer, pmdaExt *pmda)
{
    e_ext_t		*extp = (e_ext_t *)pmda->e_ext;

    if (extp->pmda_interface >= PMDA_INTERFACE_5)
	__pmdaSetContext(pmda->e_context);

    if (pmda->e_help >= 0) {
	if ((type & PM_TEXT_PMID) == PM_TEXT_PMID)
	    *buffer = pmdaGetHelp(pmda->e_help, (pmID)ident, type);
	else
	    *buffer = pmdaGetInDomHelp(pmda->e_help, (pmInDom)ident, type);
    }
    else
	*buffer = NULL;

    return (*buffer == NULL) ? PM_ERR_TEXT : 0;
}

/*
 * Tell PMCD there is nothing to store
 */

int
pmdaStore(pmResult *result, pmdaExt *pmda)
{
    return PM_ERR_PERMISSION;
}

/*
 * Expect these ones ...
 *	pmdaPMID()
 *	pmdaName()
 *	pmdaChildren()
 * to be overridden with real routines for any PMDA that is
 * using PMDA_INTERFACE_4 or later and supporting dynamic metrics.
 *
 * These implementations are stubs that return appropriate errors
 * if they are ever called.
 */

int
pmdaPMID(const char *name, pmID *pmid, pmdaExt *pmda)
{
    return PM_ERR_NAME;
}

int
pmdaName(pmID pmid, char ***nameset, pmdaExt *pmda)
{
    return PM_ERR_PMID;
}

int
pmdaChildren(const char *name, int traverse, char ***offspring, int **status, pmdaExt *pmda)
{
    return PM_ERR_NAME;
}
