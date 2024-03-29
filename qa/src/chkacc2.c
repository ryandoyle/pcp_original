/*
 * Copyright (c) 2012-2013 Red Hat.
 * Copyright (c) 1997-2001 Silicon Graphics, Inc.  All Rights Reserved.
 */

/* Check all ops except one turned off for all combinations.
 * Check connection limits
 */

#include <pcp/pmapi.h>
#include <pcp/impl.h>
#include "localconfig.h"

int
main(int argc, char **argv)
{
    int			s, sts, op, host;
    unsigned int	i;
    char		name[4*8 + 7 + 1]; /* handles full IPv6 address, if supported */
    int			ipv4 = -1;
    int			ipv6 = -1;
    int			errflag = 0;
    int			c;
#if PCP_VER >= 3611
    __pmSockAddr	*inaddr;
#else
    __pmInAddr		inaddr;
    __pmIPAddr		ipaddr;
#endif

    /* trim cmd name of leading directory components */
    __pmSetProgname(argv[0]);

    while ((c = getopt(argc, argv, "46D:?")) != EOF) {
	switch (c) {

	case '4':	/* ipv4 (default) */
	    ipv4 = 1;
	    break;

	case '6':	/* ipv6 */
	    ipv6 = 1;
	    break;

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

	case '?':
	default:
	    errflag++;
	    break;
	}
    }

    if (errflag) {
	fprintf(stderr,
"Usage: %s [options]\n\
\n\
Options:\n\
  -4             do IPv4 (default)\n\
  -6		 do IPv6\n",
                pmProgname);
        return 1;
    }

    /* defaults */
    if (ipv4 == -1) ipv4 = 1;
    if (ipv6 == -1) ipv6 = 0;

#if PCP_VER < 3611
    if (ipv6) {
	printf("No IPv6 support for this version of PCP\n");
	return 1;
    }
#endif

    sts = 0;
    for (op = 0; op < WORD_BIT; op++)
	if ((s = __pmAccAddOp(1 << op)) < 0) {
	    printf("Bad op %d: %s\n", op, strerror(errno));
	    sts = s;
	}

    if (sts < 0)
	return 1;


    for (host = 0; host < WORD_BIT; host++) {
	if (ipv4) {
	    sprintf(name, "155.%d.%d.%d", host * 3, 17+host, host);
	    if ((s = __pmAccAddHost(name, ~(1 << host), ~(1 << host), host)) < 0) {
		printf("cannot add inet host for op%d: %s\n", host, strerror(s));
		sts = s;
	    }
	}
#if PCP_VER >= 3611
	if (ipv6) {
	    sprintf(name, "fec0::%x:%x:%x:%x:%x:%x",
		    host * 3, 17+host, host,
		    host * 3, 17+host, host);
	    if ((s = __pmAccAddHost(name, ~(1 << host), ~(1 << host), host)) < 0) {
		printf("cannot add IPv6 host for op%d: %s\n", host, strerror(s));
		sts = s;
	    }
	}
#endif
    }
    if (sts < 0)
	return 1;

    putc('\n', stderr);
    __pmAccDumpHosts(stderr);

    if (ipv4) {
	for (host = 0; host < WORD_BIT; host++) {
	    int	j;

	    for (j = 0; j <= host; j++) {
		char	buf[20];
		sprintf(buf, "%d.%d.%d.%d", 155, host * 3, 17+host, host);
#if PCP_VER >= 3611
		if ((inaddr =__pmStringToSockAddr(buf)) == NULL) {
		  printf("insufficient memory\n");
		  continue;
		}
		sts = __pmAccAddClient(inaddr, &i);
		__pmSockAddrFree(inaddr);
#else
		inet_aton(buf, &inaddr);
		ipaddr = __pmInAddrToIPAddr(&inaddr);
		sts = __pmAccAddClient(ipaddr, &i);
#endif
		if (sts < 0) {
		    if (j == host && sts == PM_ERR_CONNLIMIT)
			continue;
		    printf("add inet client from host %d (j=%d): %s\n",
			   j, host, pmErrStr(sts));
		    continue;
		}
		else if (i != (~(1 << host)))
		    printf("inet host %d: __pmAccAddClient returns denyOpsResult 0x%x (expected 0x%x)\n",
			   host, i, ~(1 << host));
	    }
	}
    }
#if PCP_VER >= 3611
    if (ipv6) {
	for (host = 0; host < WORD_BIT; host++) {
	    int	j;

	    for (j = 0; j <= host; j++) {
		char	buf[4*8 + 7 + 1]; /* handles full IPv6 address */
		sprintf(buf, "fec0::%x:%x:%x:%x:%x:%x",
			host * 3, 17+host, host,
			host * 3, 17+host, host);
		if ((inaddr =__pmStringToSockAddr(buf)) == NULL) {
		  printf("insufficient memory\n");
		  continue;
		}
		sts = __pmAccAddClient(inaddr, &i);
		__pmSockAddrFree(inaddr);
		if (sts < 0) {
		    if (j == host && sts == PM_ERR_CONNLIMIT)
			continue;
		    printf("add IPv6 client from host %d (j=%d): %s\n",
			   j, host, pmErrStr(sts));
		    continue;
		}
		else if (i != (~(1 << host)))
		    printf("IPv6 host %d: __pmAccAddClient returns denyOpsResult 0x%x (expected 0x%x)\n",
			   host, i, ~(1 << host));
	    }
	}
    }
#endif

    putc('\n', stderr);
    __pmAccDumpHosts(stderr);

    return 0;
}
