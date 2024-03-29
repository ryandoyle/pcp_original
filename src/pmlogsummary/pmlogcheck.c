/*
 * Copyright (c) 1995-2003 Silicon Graphics, Inc.  All Rights Reserved.
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

#include <math.h>
#include <limits.h>
#include "pmapi.h"
#include "impl.h"

static void
usage(void)
{
    fprintf(stderr,
"Usage: %s [options] archive\n\n\
Options:\n\
  -l            print the archive label\n\
  -n pmnsfile   use an alternative PMNS\n\
  -S starttime  start of the time window\n\
  -T endtime    end of the time window\n\
  -Z timezone   set reporting timezone\n\
  -z            set reporting timezone to local time of metrics source\n",
	pmProgname);
}


typedef struct {
    int			inst;
    double		lastval;	/* value from previous sample */
    struct timeval	lasttime;	/* time of previous sample */
} instData;

typedef struct {
    pmDesc		desc;
    double		scale;
    instData		**instlist;
    unsigned int	listsize;
} checkData;

/*
 * Hash control for statistics about each metric
 */
static __pmHashCtl	hashlist;

/* time stuff */
static int		sflag = 0;
static int		tflag = 0;
static struct timeval	logstart = {0, 0};
static struct timeval   logend = {0, 0};
static struct timeval	windowstart = {0, 0};
static struct timeval   windowend = {0, 0};
static int		dayflag = 0;
static char		timebuf[32];		/* for pmCtime result + .xxx */

/* time manipulation */
static int
tsub(struct timeval *a, struct timeval *b)
{
    if ((a == NULL) || (b == NULL))
	return -1;
    a->tv_usec -= b->tv_usec;
    if (a->tv_usec < 0) {
	a->tv_usec += 1000000;
	a->tv_sec--;
    }
    a->tv_sec -= b->tv_sec;
    return 0;
}
static double
tosec(struct timeval t)
{
    return t.tv_sec + (t.tv_usec / 1000000.0);
}

static char *
typeStr(int pmtype)
{
    switch (pmtype) {
	case PM_TYPE_32:
	    return "signed 32-bit";
	case PM_TYPE_U32:
	    return "unsigned 32-bit";
	case PM_TYPE_64:
	    return "signed 64-bit";
	case PM_TYPE_U64:
	    return "unsigned 64-bit";
    }
    return "unknown type";
}

static void
print_metric(FILE *f, pmID pmid)
{
    char	*name = NULL;

    if (pmNameID(pmid, &name) < 0)
	fprintf(f, "%s", pmIDStr(pmid));
    else {
	fprintf(f, "%s", name);
	free(name);
    }
}

static void
print_stamp(FILE *f, struct timeval *stamp)
{
    if (dayflag) {
	char	*ddmm;
	char	*yr;

	ddmm = pmCtime(&stamp->tv_sec, timebuf);
	ddmm[10] = ' ';
	ddmm[11] = '\0';
	yr = &ddmm[20];
	fprintf(f, "%s", ddmm);
	__pmPrintStamp(f, stamp);
	fprintf(f, " %4.4s", yr);
    }
    else
	__pmPrintStamp(f, stamp);
}

static double
unwrap(double current, struct timeval *curtime, checkData *checkdata, int index)
{
    double	outval = current;
    int		wrapflag = 0;
    char	*str = NULL;

    if ((current - checkdata->instlist[index]->lastval) < 0.0) {
	switch (checkdata->desc.type) {
	    case PM_TYPE_32:
	    case PM_TYPE_U32:
		outval += (double)UINT_MAX+1;
		wrapflag = 1;
		break;
	    case PM_TYPE_64:
	    case PM_TYPE_U64:
		outval += (double)ULONGLONG_MAX+1;
		wrapflag = 1;
		break;
	    default:
		wrapflag = 0;
	}
    }

    if (wrapflag) {
	printf("[");
	print_stamp(stdout, curtime);
	printf("]: ");
	print_metric(stdout, checkdata->desc.pmid);
	if (pmNameInDom(checkdata->desc.indom, checkdata->instlist[index]->inst, &str) < 0)
	    printf(": %s wrap", typeStr(checkdata->desc.type));
	else {
	    printf("[%s]: %s wrap", str, typeStr(checkdata->desc.type));
	    free(str);
	}
	printf("\n\tvalue %.0f at ", checkdata->instlist[index]->lastval);
	print_stamp(stdout, &checkdata->instlist[index]->lasttime);
	printf("\n\tvalue %.0f at ", current);
	print_stamp(stdout, curtime);
	putchar('\n');
    }

    return outval;
}

static void
newHashInst(pmValue *vp,
	checkData *checkdata,		/* updated by this function */
	int valfmt,
	struct timeval *timestamp,	/* timestamp for this sample */
	int pos)			/* position of this inst in instlist */
{
    int		sts;
    size_t	size;
    pmAtomValue av;

    if ((sts = pmExtractValue(valfmt, vp, checkdata->desc.type, &av, PM_TYPE_DOUBLE)) < 0) {
	printf("[");
	print_stamp(stdout, timestamp);
	printf("] ");
	print_metric(stdout, checkdata->desc.pmid);
	printf(": pmExtractValue failed: %s\n", pmErrStr(sts));
	fprintf(stderr, "%s: possibly corrupt archive?\n", pmProgname);
	exit(1);
    }
    size = (pos+1)*sizeof(instData*);
    checkdata->instlist = (instData**) realloc(checkdata->instlist, size);
    if (!checkdata->instlist)
	__pmNoMem("newHashInst.instlist", size, PM_FATAL_ERR);
    size = sizeof(instData);
    checkdata->instlist[pos] = (instData*) malloc(size);
    if (!checkdata->instlist[pos])
	__pmNoMem("newHashInst.instlist[pos]", size, PM_FATAL_ERR);
    checkdata->instlist[pos]->inst = vp->inst;
    checkdata->instlist[pos]->lastval = av.d;
    checkdata->instlist[pos]->lasttime = *timestamp;
    checkdata->listsize++;
#ifdef PCP_DEBUG
    if (pmDebug & DBG_TRACE_APPL1) {
	char	*name;

	printf("[");
	print_stamp(stdout, timestamp);
	printf("] ");
	print_metric(stdout, checkdata->desc.pmid);
	if (vp->inst == PM_INDOM_NULL)
	    printf(": new singular metric\n");
	else {
	    printf(": new metric-instance pair ");
	    if (pmNameInDom(checkdata->desc.indom, vp->inst, &name) < 0)
		printf("%d\n", vp->inst);
	    else {
		printf("\"%s\"\n", name);
		free(name);
	    }
	}

    }
#endif
}

static void
newHashItem(pmValueSet *vsp,
	pmDesc *desc,
	checkData *checkdata,		/* output from this function */
	struct timeval *timestamp)	/* timestamp for this sample */
{
    int j;

    checkdata->desc = *desc;
    checkdata->scale = 0.0;

    /* convert counter metric units to rate units & get time scale */
    if (checkdata->desc.sem == PM_SEM_COUNTER) {
	if (checkdata->desc.units.dimTime == 0)
	    checkdata->scale = 1.0;
	else {
	    if (checkdata->desc.units.scaleTime > PM_TIME_SEC)
		checkdata->scale = pow(60.0, (double)(PM_TIME_SEC - checkdata->desc.units.scaleTime));
	    else
		checkdata->scale = pow(1000.0, (double)(PM_TIME_SEC - checkdata->desc.units.scaleTime));
	}
	if (checkdata->desc.units.dimTime == 0) checkdata->desc.units.scaleTime = PM_TIME_SEC;
	checkdata->desc.units.dimTime--;
    }

    checkdata->listsize = 0;
    checkdata->instlist = NULL;
    for (j = 0; j < vsp->numval; j++) {
	newHashInst(&vsp->vlist[j], checkdata, vsp->valfmt, timestamp, j);
    }
}

static void
docheck(pmResult *result)
{
    int			i, j, k;
    int			sts;
    pmDesc		desc;
    pmAtomValue 	av;
    pmValue		*vp;
    pmValueSet		*vsp;
    __pmHashNode	*hptr = NULL;
    checkData		*checkdata = NULL;
    double		diff;
    struct timeval	timediff;

    for (i = 0; i < result->numpmid; i++) {
	vsp = result->vset[i];

#ifdef PCP_DEBUG
	if (pmDebug & DBG_TRACE_APPL1) {
	    if (vsp->numval == 0) {
		printf("[");
		print_stamp(stdout, &result->timestamp);
		printf("] ");
		print_metric(stdout, vsp->pmid);
		printf(": No values returned\n");
		continue;
	    }
	    else if (vsp->numval < 0) {
		printf("[");
		print_stamp(stdout, &result->timestamp);
		printf("] ");
		print_metric(stdout, vsp->pmid);
		printf(": Error from numval: %s\n", pmErrStr(vsp->numval));
		continue;
	    }
	}
#endif

	/* check if pmid already in hash list */
	if ((hptr = __pmHashSearch(vsp->pmid, &hashlist)) == NULL) {
	    if ((sts = pmLookupDesc(vsp->pmid, &desc)) < 0) {
		printf("[");
		print_stamp(stdout, &result->timestamp);
		printf("] ");
		print_metric(stdout, vsp->pmid);
		printf(": pmLookupDesc failed: %s\n", pmErrStr(sts));
		continue;
	    }

	    if (desc.type != PM_TYPE_32 && desc.type != PM_TYPE_U32 &&
		desc.type != PM_TYPE_64 && desc.type != PM_TYPE_U64 &&
		desc.type != PM_TYPE_FLOAT && desc.type != PM_TYPE_DOUBLE) {
		continue;	/* no checks for non-numeric metrics */
	    }

	    /* create a new one & add to list */
	    checkdata = (checkData*) malloc(sizeof(checkData));
	    newHashItem(vsp, &desc, checkdata, &result->timestamp);
	    if (__pmHashAdd(checkdata->desc.pmid, (void*)checkdata, &hashlist) < 0) {
		printf("[");
		print_stamp(stdout, &result->timestamp);
		printf("] ");
		print_metric(stdout, vsp->pmid);
		printf(": __pmHashAdd failed (internal pmlogcheck error)\n");
		/* free memory allocated above on insert failure */
		for (j = 0; j < vsp->numval; j++) {
		    if (checkdata->instlist[j] != NULL)
			free(checkdata->instlist[j]);
		}
		if (checkdata->instlist != NULL)
		    free(checkdata->instlist);
		continue;
	    }
	}
	else {	/* pmid exists - update statistics */
	    checkdata = (checkData *)hptr->data;
	    for (j = 0; j < vsp->numval; j++) {	/* iterate thro result values */
		vp = &vsp->vlist[j];
		k = j;	/* index into stored inst list, result may differ */
		if ((vsp->numval > 1) || (checkdata->desc.indom != PM_INDOM_NULL)) {
		    /* must store values using correct inst - probably in correct order already */
		    if ((k < checkdata->listsize) && (checkdata->instlist[k]->inst != vp->inst)) {
			for (k = 0; k < checkdata->listsize; k++) {
			    if (vp->inst == checkdata->instlist[k]->inst) {
				break;	/* k now correct */
			    }
			}
			if (k == checkdata->listsize) {	/* no matching inst was found */
			    newHashInst(vp, checkdata, vsp->valfmt, &result->timestamp, k);
			    continue;
			}
		    }
		    else if (k >= checkdata->listsize) {
			k = checkdata->listsize;
			newHashInst(vp, checkdata, vsp->valfmt, &result->timestamp, k);
			continue;
		    }
		}

		timediff = result->timestamp;
		tsub(&timediff, &(checkdata->instlist[k]->lasttime));
		if (timediff.tv_sec < 0) {
		    /* clip negative values at zero */
		    timediff.tv_sec = 0;
		    timediff.tv_usec = 0;
		}
		diff = tosec(timediff);
		if ((sts = pmExtractValue(vsp->valfmt, vp, checkdata->desc.type, &av, PM_TYPE_DOUBLE)) < 0) {
		    printf("[");
		    print_stamp(stdout, &result->timestamp);
		    printf("] ");
		    print_metric(stdout, vsp->pmid);
		    printf(": pmExtractValue failed: %s\n", pmErrStr(sts));
		    continue;
		}
		if (checkdata->desc.sem == PM_SEM_COUNTER) {
		    if (diff == 0.0) continue;
		    diff *= checkdata->scale;
#ifdef PCP_DEBUG
		    if (pmDebug & DBG_TRACE_APPL2) {
			printf("[");
			print_stamp(stdout, &result->timestamp);
			printf("] ");
			print_metric(stdout, checkdata->desc.pmid);
			printf(": current counter value is %.0f\n", av.d);
		    }
#endif
		    unwrap(av.d, &(result->timestamp), checkdata, k);
		}
		checkdata->instlist[k]->lastval = av.d;
		checkdata->instlist[k]->lasttime = result->timestamp;
	    }
	}
    }
}


int
main(int argc, char *argv[])
{
    int			c, sts;
    char		*pmnsfile = PM_NS_DEFAULT;
    char		*startstr = NULL;
    char		*endstr = NULL;
    char		*msg = NULL;
    int			errflag = 0;
    int			lflag = 0;		/* no label by default */
    pmResult		*result;
    pmLogLabel		label;
    struct timeval 	unused = {0, 0};
    struct timeval	timespan = {0, 0};
    struct timeval	last_stamp;
    struct timeval	delta_stamp;
    int			zflag = 0;		/* for -z */
    char 		*tz = NULL;		/* for -Z timezone */

    __pmSetProgname(argv[0]);

    while ((c = getopt(argc, argv, "D:ln:S:T:zZ:?")) != EOF) {
	switch (c) {

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

	case 'l':	/* display label */
	    lflag = 1;
	    break;

	case 'n':	/* alternative name space file */
	    pmnsfile = optarg;
	    break;

	case 'S':
	    startstr = optarg;
	    sflag = 1;
	    break;

	case 'T':
	    endstr = optarg;
	    tflag = 1;
	    break;

	case 'z':	/* timezone from host */
	    if (tz != NULL) {
		fprintf(stderr, "%s: at most one of -Z and/or -z allowed\n", pmProgname);
		errflag++;
	    }
	    zflag++;
	    break;

	case 'Z':	/* $TZ timezone */
	    if (zflag) {
		fprintf(stderr, "%s: at most one of -Z and/or -z allowed\n", pmProgname);
		errflag++;
	    }
	    tz = optarg;
	    break;

	case '?':
	default:
	    errflag++;
	    break;
	}
    }

    if (errflag || optind >= argc) {
	usage();
	exit(1);
    }

    if ((sts = pmNewContext(PM_CONTEXT_ARCHIVE, argv[optind])) < 0) {
	fprintf(stderr, "%s: Cannot open archive \"%s\": %s\n", pmProgname, argv[optind], pmErrStr(sts));
	exit(1);
    }
    optind++;

    if (pmnsfile != PM_NS_DEFAULT) {
	if ((sts = pmLoadNameSpace(pmnsfile)) < 0) {
	    fprintf(stderr, "%s: Cannot load namespace from \"%s\": %s\n", pmProgname, pmnsfile, pmErrStr(sts));
	    exit(1);
	}
    }

    if ((sts = pmTrimNameSpace()) < 0) {
	fprintf(stderr, "%s: pmTrimNamespace failed: %s\n", pmProgname, pmErrStr(sts));
	exit(1);
    }

    if ((sts = pmGetArchiveLabel(&label)) < 0) {
	fprintf(stderr, "%s: Cannot get archive label record: %s\n",
	    pmProgname, pmErrStr(sts));
	exit(1);
    }
    else {
	logstart = label.ll_start;
	last_stamp = logstart;
    }

    if (zflag) {
	if ((sts = pmNewContextZone()) < 0) {
	    fprintf(stderr, "%s: Cannot set context timezone: %s\n",
		pmProgname, pmErrStr(sts));
	    exit(1);
	}
	printf("Note: timezone set to local timezone of host \"%s\" from archive\n\n",
	    label.ll_hostname);
    }
    else if (tz != NULL) {
	if ((sts = pmNewZone(tz)) < 0) {
	    fprintf(stderr, "%s: Cannot set timezone to \"%s\": %s\n",
		pmProgname, tz, pmErrStr(sts));
	    exit(1);
	}
	printf("Note: timezone set to \"TZ=%s\"\n\n", tz);
    }

    if ((sts = pmGetArchiveEnd(&logend)) < 0) {
	fprintf(stderr, "%s: Cannot locate end of archive: %s\n",
	    pmProgname, pmErrStr(sts));
	exit(1);
    }

    if (tflag || sflag) {
	if (pmParseTimeWindow(startstr, endstr, NULL, NULL, &logstart, &logend,
		&windowstart, &windowend, &unused, &msg) < 0) {
	    fprintf(stderr, "%s: Invalid time window specified: %s\n", pmProgname, msg);
	    exit(1);
	}
#ifdef PCP_DEBUG
	else if (pmDebug & DBG_TRACE_APPL0) {
	    printf("parseTimeWindow: \n "
		"Window start=%.0f end=%.0f sec\nArchive start=%.0f end=%.0f sec\n",
		tosec(windowstart), tosec(windowend), tosec(logstart), tosec(logend));
	}
#endif
    }
    else {	/* time window covers whole log */
	windowstart = logstart;
	windowend = logend;
    }

    if ((sts = pmSetMode(PM_MODE_FORW, &windowstart, 0)) < 0) {
	fprintf(stderr, "%s: pmSetMode failed: %s\n", pmProgname, pmErrStr(sts));
	exit(1);
    }

    if (lflag == 1) {
	char	       *ddmm;
	char	       *yr;

	printf("Log Label (Log Format Version %d)\n", label.ll_magic & 0xff);
	printf("Performance metrics from host %s\n", label.ll_hostname);

	ddmm = pmCtime(&logstart.tv_sec, timebuf);
	ddmm[10] = '\0';
	yr = &ddmm[20];
	printf("  commencing %s ", ddmm);
	__pmPrintStamp(stdout, &logstart);
	printf(" %4.4s\n", yr);

	ddmm = pmCtime(&logend.tv_sec, timebuf);
	ddmm[10] = '\0';
	yr = &ddmm[20];
	printf("  ending     %s ", ddmm);
	__pmPrintStamp(stdout, &logend);
	printf(" %4.4s\n", yr);
    }

    timespan = windowend;
    tsub(&timespan, &windowstart);
    if (timespan.tv_sec > 86400) /* seconds per day: 60*60*24 */
	dayflag = 1;

    sts = 0;
    for ( ; ; ) {
	if ((sts = pmFetchArchive(&result)) < 0)
	    break;
	delta_stamp = result->timestamp;
	tsub(&delta_stamp, &last_stamp);
#ifdef PCP_DEBUG
	if (pmDebug & DBG_TRACE_APPL0) {
	    int		i;
	    int		sum_val = 0;
	    int		cnt_noval = 0;
	    int		cnt_err = 0;
	    pmValueSet	*vsp;

	    printf("[");
	    print_stamp(stdout, &result->timestamp);
	    for (i = 0; i < result->numpmid; i++) {
		vsp = result->vset[i];
		if (vsp->numval > 0)
		    sum_val += vsp->numval;
		else if (vsp->numval == 0)
		    cnt_noval++;
		else
		    cnt_err++;
	    }
	    printf("] delta(stamp)=%d.%03fsec",
		(int)delta_stamp.tv_sec, (double)(delta_stamp.tv_usec)/1000000);
	    printf(" numpmid=%d sum(numval)=%d", result->numpmid, sum_val);
	    if (cnt_noval > 0)
		printf(" count(numval=0)=%d", cnt_noval);
	    if (cnt_err > 0)
		printf(" count(numval<0)=%d", cnt_err);
	    fputc('\n', stdout);
	}
#endif
	if (delta_stamp.tv_sec < 0) {
	    /* time went backwards! */
	    printf("[");
	    print_stamp(stdout, &result->timestamp);
	    printf("]: timestamp went backwards, prior timestamp: ");
	    print_stamp(stdout, &last_stamp);
	    printf("\n");
	}

	last_stamp = result->timestamp;
	if ((windowend.tv_sec > result->timestamp.tv_sec) ||
	    ((windowend.tv_sec == result->timestamp.tv_sec) &&
	     (windowend.tv_usec >= result->timestamp.tv_usec))) {
	    docheck(result);
	    pmFreeResult(result);
	}
	else {
	    pmFreeResult(result);
	    sts = PM_ERR_EOL;
	    break;
	}
    }
    if (sts != PM_ERR_EOL) {
	printf("[after ");
	print_stamp(stdout, &last_stamp);
	printf("]: pmFetch: Error: %s\n", pmErrStr(sts));
	exit(1);
    }

    return 0;
}
