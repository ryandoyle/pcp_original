/*
 * Copyright (c) 2011 Ken McDonell.  All Rights Reserved.
 *
 * exercise multi-threaded multiple host contexts with PMNS functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <pcp/pmapi.h>
#include <pcp/impl.h>
#include <pthread.h>

#ifndef HAVE_PTHREAD_BARRIER_T
#include "pthread_barrier.h"
#endif

static char	*namelist[] = {
    "sample.secret",		// non-leaf
    "sampledso.bin",
    "sample.longlong.one",
    "pmcd.pmlogger"		// non-leaf
};

#define NMETRIC (sizeof(namelist)/sizeof(namelist[0]))
static pmID	pmidlist[NMETRIC];
static pmDesc	desclist[NMETRIC];
static char	**chn[NMETRIC];
static int	leaf_chn[NMETRIC];
static int	nonleaf_chn[NMETRIC];
/*
 * values here come from
 * pminfo sample.secret | wc -lc => 9     273 => 264
 * pminfo pmcd.pmlogger | wc -lc => 4      84 => 80
 */
static int	sum_traverse[NMETRIC] = { 264, 0, 0, 80 };

static pthread_barrier_t barrier;

static int	ctx1;
static int	ctx2;
static int	ctx3;

static int	count1;
static int	count2;
static int	count3;

static void
dometric(const char *name, void *closure)
{
    *((int *)closure) += strlen(name);
}

static void
foo(FILE *f, char *fn, int i, void *closure)
{
    int		sts;
    int		j;
    int		leaf;
    pmID	pmids[NMETRIC];
    char	*tmp;
    char	**tmpset;
    int		*stsset;
    char	strbuf[20];

    if ((sts = pmLookupName(NMETRIC-i, &namelist[i], pmids)) < 0) {
	if (sts != PM_ERR_NONLEAF) {
	    fprintf(f, "%s: %s ...: pmLookupName Error: %s\n", fn, namelist[i], pmErrStr(sts));
	    pthread_exit("botch");
	}
    }
    for (j = i; j < NMETRIC; j++) {
	if (pmids[j-i] != pmidlist[j]) {
	    fprintf(f, "%s: %s: Botch: expecting %s", fn, namelist[j], pmIDStr_r(pmidlist[j], strbuf, sizeof(strbuf)));
	    fprintf(f, ", got %s\n", pmIDStr_r(pmids[j-i], strbuf, sizeof(strbuf)));
	    pthread_exit("botch");
	}
    }
    fprintf(f, "%s: %s ... pmLookupName OK\n", fn, namelist[i]);

    fprintf(f, "%s: %s", fn, namelist[i]);
    if (pmidlist[i] != PM_ID_NULL) {
	/* leaf node in PMNS */
	if ((sts = pmNameID(pmidlist[i], &tmp)) < 0) {
	    fprintf(f, "\n%s: %s ...: pmNameID Error: %s\n", fn, namelist[i], pmErrStr(sts));
	    pthread_exit("botch");
	}
	if (strcmp(tmp, namelist[i]) != 0) {
	    fprintf(f, "\n%s: %s: Botch: expecting %s, got %s\n", fn, pmIDStr_r(pmidlist[i], strbuf, sizeof(strbuf)), namelist[i], tmp);
	    pthread_exit("botch");
	}
	free(tmp);
	fprintf(f, " pmNameID OK");
	if ((sts = pmNameAll(pmidlist[i], &tmpset)) < 0) {
	    fprintf(f, "\n%s: %s ...: pmNameAll Error: %s\n", fn, namelist[i], pmErrStr(sts));
	    pthread_exit("botch");
	}
	if (sts != 1) {
	    fprintf(f, "\n%s: %s ...: pmNameAll Botch: expecting %d, got %d\n", fn, namelist[i], 1, sts);
	    pthread_exit("botch");
	}
	if (strcmp(tmpset[0], namelist[i]) != 0) {
	    fprintf(f, "\n%s: %s: Botch: expecting %s, got %s\n", fn, pmIDStr_r(pmidlist[i], strbuf, sizeof(strbuf)), namelist[i], tmpset[0]);
	    pthread_exit("botch");
	}
	free(tmpset);
	fprintf(f, " pmNameAll OK");
    }
    else {
	/* non-leaf node in PMNS */
	int	keep = 0;
	if ((sts = pmGetChildrenStatus(namelist[i], &tmpset, &stsset)) < 0) {
	    fprintf(f, "\n%s: %s ...: pmGetChildrenStatus Error: %s\n", fn, namelist[i], pmErrStr(sts));
	    pthread_exit("botch");
	}
	leaf = 0;
	for (j = 0; j < sts; j++) {
	    if (stsset[j] == PMNS_LEAF_STATUS) leaf++;
	}
	if (leaf_chn[i] == -1) {
	    leaf_chn[i] = leaf;
	    nonleaf_chn[i] = sts - leaf;
	    chn[i] = tmpset;
	    keep = 1;
	}
	else {
	    if (leaf != leaf_chn[i] || sts - leaf != nonleaf_chn[i]) {
		fprintf(f, "\n%s: %s: Botch: expecting %d leaf & %d non-leaf, got %d leaf & %d non-leaf\n", fn, namelist[i], leaf_chn[i], nonleaf_chn[i], leaf, sts - leaf);
		pthread_exit("botch");
	    }
	    for (j = 0; j < sts; j++) {
		if (strcmp(chn[i][j], tmpset[j]) != 0) {
		    fprintf(f, "\n%s: %s: Botch: child[%d] expecting %s, got %s\n", fn, namelist[i], j, chn[i][j], tmpset[j]);
		    pthread_exit("botch");
		}
	    }
	}
	if (keep == 0)
	    free(tmpset);
	free(stsset);
	fprintf(f, " pmGetChildrenStatus OK");
	if ((sts = pmGetChildren(namelist[i], &tmpset)) < 0) {
	    fprintf(f, "\n%s: %s ...: pmGetChildren Error: %s\n", fn, namelist[i], pmErrStr(sts));
	    pthread_exit("botch");
	}
	if (sts != leaf_chn[i] + nonleaf_chn[i]) {
	    fprintf(f, "\n%s: %s: Botch: expecting %d children, got %d\n", fn, namelist[i], leaf_chn[i] + nonleaf_chn[i], sts);
	    pthread_exit("botch");
	}
	for (j = 0; j < sts; j++) {
	    if (strcmp(chn[i][j], tmpset[j]) != 0) {
		fprintf(f, "\n%s: %s: Botch: child[%d] expecting %s, got %s\n", fn, namelist[i], j, chn[i][j], tmpset[j]);
		pthread_exit("botch");
	    }
	}
	free(tmpset);
	fprintf(f, " pmGetChildren OK");
	*((int *)closure) = 0;
	if ((sts == pmTraversePMNS_r(namelist[i], dometric, closure)) < 0) {
	    fprintf(f, "\n%s: %s ...: pmTraversePMNS_r Error: %s\n", fn, namelist[i], pmErrStr(sts));
	    pthread_exit("botch");
	}
	if (sum_traverse[i] != *((int *)closure)) {
	    fprintf(f, "\n%s: %s: Botch: sum strlen(descendent names) expecting %d, got %d\n", fn, namelist[i], sum_traverse[i], *((int *)closure));
	    pthread_exit("botch");
	}
	fprintf(f, " pmTraversePMNS_r OK");
    }
    fputc('\n', f);

}

static void *
func1(void *arg)
{
    char	*fn = "func1";
    int		i;
    int		j;
    FILE	*f;

    f = fopen("/tmp/func1.out", "w");

    j = pmUseContext(ctx1);
    if ( j < 0) {
	fprintf(f, "Error: %s: pmUseContext(%d) -> %s\n", fn, ctx1, pmErrStr(j));
	pthread_exit("botch");
    }

    pthread_barrier_wait(&barrier);

    for (j = 0; j < 100; j++) {
	for (i = 0; i < NMETRIC; i++) {
	    foo(f, fn, i, &count1);
	}
    }

    pthread_exit(NULL);
}

static void *
func2(void *arg)
{
    char	*fn = "func2";
    int		i;
    int		j;
    FILE	*f;

    f = fopen("/tmp/func2.out", "w");

    j = pmUseContext(ctx2);
    if ( j < 0) {
	fprintf(f, "Error: %s: pmUseContext(%d) -> %s\n", fn, ctx2, pmErrStr(j));
	pthread_exit("botch");
    }

    pthread_barrier_wait(&barrier);

    for (j = 0; j < 100; j++) {
	for (i = NMETRIC-1; i >= 0; i--) {
	    foo(f, fn, i, &count2);
	}
    }

    pthread_exit(NULL);
}

static void *
func3(void *arg)
{
    char	*fn = "func3";
    int		i;
    int		j;
    FILE	*f;

    f = fopen("/tmp/func3.out", "w");

    j = pmUseContext(ctx3);
    if ( j < 0) {
	fprintf(f, "Error: %s: pmUseContext(%d) -> %s\n", fn, ctx3, pmErrStr(j));
	pthread_exit("botch");
    }

    pthread_barrier_wait(&barrier);

    for (j = 0; j < 100; j++) {
	for (i = 0; i < NMETRIC; i += 2)
	    foo(f, fn, i, &count3);
	for (i = 1; i < NMETRIC; i += 2)
	    foo(f, fn, i, &count3);
    }

    pthread_exit(NULL);
}

int
main(int argc, char **argv)
{
    pthread_t	tid1;
    pthread_t	tid2;
    pthread_t	tid3;
    int		sts;
    char	*msg;
    int		errflag = 0;
    int		c;
    int		i;

    __pmSetProgname(argv[0]);

    while ((c = getopt(argc, argv, "D:")) != EOF) {
	switch (c) {

#ifdef PCP_DEBUG
	case 'D':	/* debug flag */
	    sts = __pmParseDebug(optarg);
	    if (sts < 0) {
		fprintf(stderr, "%s: unrecognized debug flag specification (%s)\n",
		    pmProgname, optarg);
		errflag++;
	    }
	    else
		pmDebug |= sts;
	    break;
#endif

	case '?':
	default:
	    errflag++;
	    break;
	}
    }

    if (errflag || optind == argc || argc-optind > 3) {
	fprintf(stderr, "Usage: %s [-D...] host1 [host2 [host3]]\n", pmProgname);
	exit(1);
    }

    ctx1 = pmNewContext(PM_CONTEXT_HOST, argv[optind]);
    if (ctx1 < 0) {
	printf("Error: pmNewContext(%s) -> %s\n", argv[optind], pmErrStr(ctx1));
	exit(1);
    }
    optind++;

    if (optind < argc) {
	ctx2 = pmNewContext(PM_CONTEXT_HOST, argv[optind]);
	if (ctx2 < 0) {
	    printf("Error: pmNewContext(%s) -> %s\n", argv[optind], pmErrStr(ctx2));
	    exit(1);
	}
	optind++;
    }
    else
	ctx2 = ctx1;

    if (optind < argc) {
	ctx3 = pmNewContext(PM_CONTEXT_HOST, argv[optind]);
	if (ctx3 < 0) {
	    printf("Error: pmNewContext(%s) -> %s\n", argv[optind], pmErrStr(ctx2));
	    exit(1);
	}
	optind++;
    }
    else
	ctx3 = ctx2;

    sts = pmLookupName(NMETRIC, namelist, pmidlist);
    if (sts != NMETRIC) {
	printf("Warning: pmLookupName -> %s\n", pmErrStr(sts));
	for (i = 0; i < NMETRIC; i++) {
	    printf("    %s -> %s\n", namelist[i], pmIDStr(pmidlist[i]));
	}
    }

    for (i = 0; i < NMETRIC; i++) {
	if (pmidlist[i] != PM_ID_NULL) {
	    if ((sts = pmLookupDesc(pmidlist[i], &desclist[i])) < 0) {
		printf("Error: pmLookupDesc(%s) -> %s\n", namelist[i], pmErrStr(sts));
		exit(1);
	    }
	}
	chn[i] = NULL;
	leaf_chn[i] = nonleaf_chn[i] = -1;
    }

    sts = pthread_barrier_init(&barrier, NULL, 3);
    if (sts != 0) {
	printf("pthread_barrier_init: sts=%d\n", sts);
	exit(1);
    }

    sts = pthread_create(&tid1, NULL, func1, NULL);
    if (sts != 0) {
	printf("thread_create: tid1: sts=%d\n", sts);
	exit(1);
    }
    sts = pthread_create(&tid2, NULL, func2, NULL);
    if (sts != 0) {
	printf("thread_create: tid2: sts=%d\n", sts);
	exit(1);
    }
    sts = pthread_create(&tid3, NULL, func3, NULL);
    if (sts != 0) {
	printf("thread_create: tid3: sts=%d\n", sts);
	exit(1);
    }

    pthread_join(tid1, (void *)&msg);
    if (msg != NULL) printf("tid1: %s\n", msg);
    pthread_join(tid2, (void *)&msg); 
    if (msg != NULL) printf("tid2: %s\n", msg);
    pthread_join(tid3, (void *)&msg); 
    if (msg != NULL) printf("tid3: %s\n", msg);

    exit(0);
}
