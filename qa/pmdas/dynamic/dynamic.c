/*
 * Dynamic PMDA for testing dynamic indom support
 *
 * Copyright (c) 1995-2002 Silicon Graphics, Inc.  All Rights Reserved.
 */

#include <stdio.h>
#include <syslog.h>
#include <errno.h>
#include <pcp/pmapi.h>
#include <pcp/impl.h>
#include <pcp/pmda.h>
#include "domain.h"

/*
 * Dynamic PMDA
 *
 * This PMDA has several metrics with a single dynamic indom.
 * This indom can be controlled by several storable metrics.
 *
 * Metrics
 *
 *	dynamic.numinst		- number of instances in the indom right now
 *	dynamic.discrete	- discrete metric
 *	dynamic.instant		- instantaneous metric
 *	dynamic.counter		- counter metric
 *	dynamic.control.add	- add an instance
 *	dynamic.control.del	- delete an instance
 *
 */

/* data and function prototypes for dynamic instance domain handling */
struct Dynamic {
    int		id;
    char	name[16];
    unsigned	counter;
};

/*
 * list of instances
 */

static struct Dynamic	*insts = NULL;
static pmdaInstid	*instids = NULL;
static unsigned		numInsts = 0;
static unsigned		sizeInsts = 0;

/*
 * list of instance domains
 */

static pmdaIndom indomtab[] = {
#define DYNAMIC_INDOM	0
    { DYNAMIC_INDOM, 0, NULL }
};

/*
 * all metrics supported in this PMDA - one table entry for each
 */

static pmdaMetric metrictab[] = {
/* numinsts */
    { NULL, 
      { PMDA_PMID(0,0), PM_TYPE_U32, PM_INDOM_NULL, PM_SEM_INSTANT, 
        PMDA_PMUNITS(0,0,0,0,0,0) }, },
/* discrete */
    { NULL, 
      { PMDA_PMID(0,1), PM_TYPE_STRING, DYNAMIC_INDOM, PM_SEM_DISCRETE, 
        PMDA_PMUNITS(0,0,0,0,0,0) }, },
/* instant */
    { NULL, 
      { PMDA_PMID(0,2), PM_TYPE_U32, DYNAMIC_INDOM, PM_SEM_INSTANT, 
        PMDA_PMUNITS(0,0,1,0,0,PM_COUNT_ONE) }, },
/* counter */
    { NULL, 
      { PMDA_PMID(0,3), PM_TYPE_U32, DYNAMIC_INDOM, PM_SEM_COUNTER, 
        PMDA_PMUNITS(0,0,1,0,0,PM_COUNT_ONE) }, },
/* control.add */
    { NULL, 
      { PMDA_PMID(1,4), PM_TYPE_32, PM_INDOM_NULL, PM_SEM_INSTANT, 
        PMDA_PMUNITS(0,0,0,0,0,0) }, },
/* control.del */
    { NULL, 
      { PMDA_PMID(1,5), PM_TYPE_32, PM_INDOM_NULL, PM_SEM_INSTANT, 
        PMDA_PMUNITS(0,0,0,0,0,0) }, },
};

/*
 * callback provided to pmdaFetch
 */
static int
dynamic_fetchCallBack(pmdaMetric *mdesc, unsigned int inst, pmAtomValue *atom)
{
    __pmID_int		*idp = (__pmID_int *)&(mdesc->m_desc.pmid);

    if (inst != PM_IN_NULL &&
	!(idp->cluster == 0 && idp->item >= 1 && idp->item <= 3))
	return PM_ERR_INST;

    __pmNotifyErr(LOG_DEBUG, "dynamic_fetch: %d.%d[%d]\n",
		  idp->cluster, idp->item, inst);

    if (idp->cluster == 0) {
	switch (idp->item) {
	case 0:					/* numinst */
	    atom->ul = numInsts;
	    break;

	case 1:					/* discrete */
	    if (inst < sizeInsts)
		if (insts[inst].id >= 0)
		    atom->cp = insts[inst].name;
	        else
		    return PM_ERR_INST;
	    else
		return PM_ERR_INST;
	    break;

	case 2:					/* instant */
	    if (inst < sizeInsts)
		if (insts[inst].id >= 0)
		    atom->ul = insts[inst].counter;
	        else
		    return PM_ERR_INST;
	    else
		return PM_ERR_INST;
	    break;

	case 3:					/* counter */
	    if (inst < sizeInsts)
		if (insts[inst].id >= 0)
		    atom->ul = insts[inst].counter;
	        else
		    return PM_ERR_INST;
	    else
		return PM_ERR_INST;
	    break;

	default:
	    return PM_ERR_PMID;
	}
    }
    else if (idp->cluster == 1) {		/* dynamic.control */
	switch(idp->item) {
	case 4:					/* add */
	    atom->ul = sizeInsts;
	    break;

	case 5:					/* del */
	    atom->ul = sizeInsts - numInsts;
	    break;

	default:
	    return PM_ERR_PMID;
	}
    }
    else
	return PM_ERR_PMID;

    return 1;
}

/*
 * wrapper for pmdaFetch which increments the counters for each instance
 */
static int
dynamic_fetch(int numpmid, pmID pmidlist[], pmResult **resp, pmdaExt *pmda)
{
    int i;

    for (i = 0; i < sizeInsts; i++)
	if (insts[i].id >= 0)
	    insts[i].counter += insts[i].id;

    return pmdaFetch(numpmid, pmidlist, resp, pmda);
}

/*
 * add or delete an instance
 */
/*ARGSUSED*/
static int
dynamic_store(pmResult *result, pmdaExt *pmda)
{
    int		i;
    int		j;
    int		changed = 0;
    int		sts = 0;
    int		val;
    pmValueSet	*vsp = NULL;
    __pmID_int	*pmidp = NULL;

    for (i = 0; i < result->numpmid; i++) {
	vsp = result->vset[i];
	pmidp = (__pmID_int *)&vsp->pmid;

	if (pmidp->cluster == 1) {	/* all storable metrics are cluster 1 */

	    switch (pmidp->item) {
	    	case 4:					/* add */
		    
		    val = vsp->vlist[0].value.lval;
		    if (val < 0) {
			sts = PM_ERR_SIGN;
		    }
		    else if (val < sizeInsts) {
			if (insts[val].id >= 0) {
			    sts = PM_ERR_INST;
			}
			else {
#ifdef PCP_DEBUG
			    if (pmDebug & DBG_TRACE_APPL0)
				__pmNotifyErr(LOG_DEBUG, 
					      "dynamic_store: Adding instance %d (size = %d)\n",
					      val, sizeInsts);
#endif

			    insts[val].id = val;
			    insts[val].counter = 0;
			    numInsts++;
			    changed = 1;
			}
		    }
		    else {
			insts = (struct Dynamic*)realloc(insts, (val + 1) * sizeof(struct Dynamic));
			if (insts == NULL) {
			    __pmNotifyErr(LOG_ERR, 
					  "dynamic_store: Unable to realloc %d bytes\n",
					  (int)(val * sizeof(struct Dynamic)));
			    sizeInsts = 0;
			    numInsts = 0;
			    changed = 1;
			    sts = PM_ERR_TOOBIG;
			}
			else {
			    for (i = sizeInsts; i <= val; i++) {
				insts[i].id = -1;
				sprintf(insts[i].name, "%d", i);
				insts[i].counter = 0;
			    }
			    insts[val].id = val;
			    changed = 1;
			    sizeInsts = val + 1;
			    numInsts++;

#ifdef PCP_DEBUG
			    if (pmDebug & DBG_TRACE_APPL0)
				__pmNotifyErr(LOG_DEBUG, 
					      "dynamic_store: Adding instance %d (size = %d)\n",
					      val, sizeInsts);
#endif
			}
		    }
		    break;

		case 5:					/* del */
		    val = vsp->vlist[0].value.lval;
		    if (val < 0) {			/* delete all */
#ifdef PCP_DEBUG
			    if (pmDebug & DBG_TRACE_APPL0)
				__pmNotifyErr(LOG_DEBUG, 
					      "dynamic_store: Removing all instances\n");
#endif

			for (i = 0; i < sizeInsts; i++) {
			    insts[i].id = -1;
			    insts[i].counter = 0;
			}
			numInsts = 0;
			changed = 1;
		    }
		    else if (val < sizeInsts) {
			if (insts[val].id < 0) {
			    sts = PM_ERR_INST;
			}
			else {
#ifdef PCP_DEBUG
			    if (pmDebug & DBG_TRACE_APPL0)
				__pmNotifyErr(LOG_DEBUG, 
					      "dynamic_store: Removing instance %d\n",
					      val);
#endif
			    insts[val].id = -1;
			    insts[val].counter = 0;
			    changed = 1;
			    numInsts--;
			}
		    }
		    else
			sts = PM_ERR_INST;
		    break;

		default:
		    sts = PM_ERR_PMID;
		    break;
	    }
	}
	else if (pmidp->cluster == 0 && pmidp->item <= 3) {
	    sts = -EACCES;
	    break;
	}
	else {
	    sts = PM_ERR_PMID;
	    break;
	}
    }

    if (changed) {

#ifdef PCP_DEBUG
	if (pmDebug & DBG_TRACE_APPL0)
	    __pmNotifyErr(LOG_DEBUG, 
			  "dynamic_store: Resizing to %d instances\n",
			  numInsts);
#endif

	if (numInsts > 0) {
	    instids = (pmdaInstid *)realloc(instids, 
					    numInsts * sizeof(pmdaInstid));
	    if (instids == NULL) {
		__pmNotifyErr(LOG_ERR, 
			      "dynamic_store: Could not realloc %d bytes\n",
			      (int)(numInsts * sizeof(pmdaInstid)));
		sts = PM_ERR_TOOBIG;
	    }
	    else {
		for (i = 0, j = 0; i < sizeInsts; i++) {
		    if (insts[i].id >= 0) {
			instids[j].i_inst = insts[i].id;
			instids[j].i_name = insts[i].name;

#ifdef PCP_DEBUG
			if (pmDebug & DBG_TRACE_APPL1)
			    __pmNotifyErr(LOG_DEBUG,
					  "dynamic_store: [%d] %d \"%s\"\n",
					  j, instids[j].i_inst, 
					  instids[j].i_name);
#endif

			j++;
		    }
		}
		indomtab[DYNAMIC_INDOM].it_numinst = numInsts;
		indomtab[DYNAMIC_INDOM].it_set = instids;
	    }
	}
	else {
	    indomtab[DYNAMIC_INDOM].it_numinst = 0;
	    indomtab[DYNAMIC_INDOM].it_set = NULL;
	}    
    }

    return sts;
}

/*
 * Initialise the agent (both daemon and DSO).
 */
void 
dynamic_init(pmdaInterface *dp)
{
    if (dp->status != 0)
	return;

    dp->version.two.fetch = dynamic_fetch;
    dp->version.two.store = dynamic_store;

    pmdaSetFetchCallBack(dp, dynamic_fetchCallBack);

    pmdaInit(dp, indomtab, sizeof(indomtab)/sizeof(indomtab[0]), metrictab,
	     sizeof(metrictab)/sizeof(metrictab[0]));
}

static void
usage(void)
{
    fprintf(stderr, "Usage: %s [options]\n\n", pmProgname);
    fputs("Options:\n"
	  "  -d domain    use domain (numeric) for metrics domain of PMDA\n"
	  "  -l logfile   write log into logfile rather than using default log name\n"
	  "\nExactly one of the following options may appear:\n"
	  "  -i port      expect PMCD to connect on given inet port (number or name)\n"
	  "  -p           expect PMCD to supply stdin/stdout (pipe)\n"
	  "  -u socket    expect PMCD to connect on given unix domain socket\n",
	  stderr);		
    exit(1);
}

/*
 * Set up the agent if running as a daemon.
 */
int
main(int argc, char **argv)
{
    int			sep = __pmPathSeparator();
    int			err = 0;
    pmdaInterface	dispatch;
    char		helppath[MAXPATHLEN];

    __pmSetProgname(argv[0]);
    snprintf(helppath, sizeof(helppath),
		"%s%c" "testsuite" "%c" "pmdas" "%c" "dynamic" "%c" "help",
		pmGetConfig("PCP_VAR_DIR"), sep, sep, sep, sep);
    pmdaDaemon(&dispatch, PMDA_INTERFACE_4, pmProgname, DYNAMIC,
		"dynamic.log", helppath);

    if (pmdaGetOpt(argc, argv, "D:d:h:i:l:pu:?", &dispatch, &err) != EOF)
    	err++;

    if (err)
    	usage();

    pmdaOpenLog(&dispatch);
    dynamic_init(&dispatch);
    pmdaConnect(&dispatch);
    pmdaMain(&dispatch);

    exit(0);
}
