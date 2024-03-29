/*
 * Copyright (c) International Business Machines Corp., 2002
 * This code contributed by Mike Mason <mmlnx@us.ibm.com>
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
 */

#include <fcntl.h>
#include "pmapi.h"
#include "proc_uptime.h"

int
refresh_proc_uptime(proc_uptime_t *proc_uptime)
{
    char buf[80];
    int fd, n;
    float uptime, idletime;

    memset(proc_uptime, 0, sizeof(proc_uptime_t));

    if ((fd = open("/proc/uptime", O_RDONLY)) < 0)
	return -oserror();

    n = read(fd, buf, sizeof(buf));
    close(fd);
    if (n < 0)
	return -oserror();
    buf[n] = '\0';

    sscanf((const char *)buf, "%f %f", &uptime, &idletime);
    proc_uptime->uptime = (unsigned long) uptime;
    proc_uptime->idletime = (unsigned long) idletime;
    return 0;
}
