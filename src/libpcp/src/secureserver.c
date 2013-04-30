/*
 * Copyright (c) 2012-2013 Red Hat.
 *
 * Server side security features - via Network Security Services (NSS) and
 * the Simple Authentication and Security Layer (SASL).
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

#include "pmapi.h"
#include "impl.h"
#define SOCKET_INTERNAL
#include "internal.h"
#include <keyhi.h>
#include <secder.h>
#include <pk11pub.h>
#include <sys/stat.h>

#define MAX_NSSDB_PASSWORD_LENGTH	256

static struct {
    /* NSS certificate management */
    CERTCertificate	*certificate;
    SECKEYPrivateKey	*private_key;
    const char		*password_file;
    SSLKEAType		certificate_KEA;
    unsigned int	certificate_verified : 1;
    unsigned int	ssl_session_cache_setup : 1;
    char		database_path[MAXPATHLEN];

    /* SASL authentication fields */
} secure_server;

int
__pmServerHasFeature(__pmServerFeature query)
{
    int sts = 0;

    switch (query) {
    case PM_SERVER_FEATURE_SECURE:
	PM_INIT_LOCKS();
	PM_LOCK(__pmLock_libpcp);
	sts = secure_server.certificate_verified;
	PM_UNLOCK(__pmLock_libpcp);
	break;
    case PM_SERVER_FEATURE_COMPRESS:
    case PM_SERVER_FEATURE_USER_AUTH:
	sts = 1;
	break;
    case PM_SERVER_FEATURE_IPV6:
	sts = (strcmp(__pmGetAPIConfig("ipv6"), "true") == 0);
	break;
    default:
	break;
    }
    return sts;
}

static int
secure_file_contents(const char *filename, char **passwd, size_t *length)
{
    struct stat	stat;
    size_t	size = *length;
    char	*pass = NULL;
    FILE	*file = NULL;
    int		sts;

    if ((file = fopen(filename, "r")) == NULL)
	goto fail;
    if (fstat(fileno(file), &stat) < 0)
	goto fail;
    if (stat.st_size > size) {
	setoserror(E2BIG);
	goto fail;
    }
    if ((pass = (char *)PORT_Alloc(stat.st_size)) == NULL) {
	setoserror(ENOMEM);
	goto fail;
    }
    sts = fread(pass, 1, stat.st_size, file);
    if (sts < 1) {
	setoserror(EINVAL);
	goto fail;
    }
    while (sts > 0 && (pass[sts-1] == '\r' || pass[sts-1] == '\n'))
	pass[--sts] = '\0';
    *passwd = pass;
    *length = sts;
    fclose(file);
    return 0;

fail:
    sts = -oserror();
    if (file)
	fclose(file);
    if (pass)
	PORT_Free(pass);
    return sts;
}

static char *
certificate_database_password(PK11SlotInfo *info, PRBool retry, void *arg)
{
    size_t length = MAX_NSSDB_PASSWORD_LENGTH;
    char *password = NULL;
    char passfile[MAXPATHLEN];
    int sts;

    (void)arg;
    (void)info;

    PM_INIT_LOCKS();
    PM_LOCK(__pmLock_libpcp);
    passfile[0] = '\0';
    if (secure_server.password_file)
	strncpy(passfile, secure_server.password_file, MAXPATHLEN-1);
    passfile[MAXPATHLEN-1] = '\0';
    PM_UNLOCK(__pmLock_libpcp);

    if (passfile[0] == '\0') {
	__pmNotifyErr(LOG_ERR, "Password sought but no password file given");
	return NULL;
    }
    if (retry) {
	__pmNotifyErr(LOG_ERR, "Retry attempted during password extraction");
	return NULL;	/* no soup^Wretries for you */
    }

    sts = secure_file_contents(passfile, &password, &length);
    if (sts < 0) {
	__pmNotifyErr(LOG_ERR, "Cannot read password file \"%s\": %s",
			passfile, pmErrStr(sts));
	return NULL;
    }
    return password;
}

static int
__pmCertificateTimestamp(SECItem *vtime, char *buffer, size_t size)
{
    PRExplodedTime exploded;
    SECStatus secsts;
    int64 itime;

    switch (vtime->type) {
    case siUTCTime:
	secsts = DER_UTCTimeToTime(&itime, vtime);
	break;
    case siGeneralizedTime:
	secsts = DER_GeneralizedTimeToTime(&itime, vtime);
	break;
    default:
	return -EINVAL;
    }
    if (secsts != SECSuccess)
	return __pmSecureSocketsError();

    /* Convert to local time */
    PR_ExplodeTime(itime, PR_GMTParameters, &exploded);
    if (!PR_FormatTime(buffer, size, "%a %b %d %H:%M:%S %Y", &exploded))
	return __pmSecureSocketsError();
    return 0;
}

static void
__pmDumpCertificate(FILE *fp, const char *nickname, CERTCertificate *cert)
{
    CERTValidity *valid = &cert->validity;
    char tbuf[256];

    fprintf(fp, "Certificate: %s", nickname);
    if (__pmCertificateTimestamp(&valid->notBefore, tbuf, sizeof(tbuf)) == 0)
	fprintf(fp, "  Not Valid Before: %s UTC", tbuf);
    if (__pmCertificateTimestamp(&valid->notAfter, tbuf, sizeof(tbuf)) == 0)
	fprintf(fp, "  Not Valid After: %s UTC", tbuf);
}

static int
__pmValidCertificate(CERTCertDBHandle *db, CERTCertificate *cert, PRTime stamp)
{
    SECCertificateUsage usage = certificateUsageSSLServer;
    SECStatus secsts = CERT_VerifyCertificate(db, cert, PR_TRUE, usage,
						stamp, NULL, NULL, &usage);
    return (secsts == SECSuccess);
}

static char *
serverdb(char *path, size_t size)
{
    int sep = __pmPathSeparator();
    char *nss_method = getenv("PCP_SECURE_DB_METHOD");

    if (nss_method == NULL)
	nss_method = "sql:";

    /*
     * Fill in a buffer with the server NSS database specification.
     * Return a pointer to the filesystem path component - without
     * the <method>:-prefix - for other routines to work with.
     */
    snprintf(path, size, "%s" "%c" "etc" "%c" "pki" "%c" "nssdb",
		nss_method, sep, sep, sep);
    return path + strlen(nss_method);
}

int
__pmSecureServerSetup(const char *db, const char *passwd)
{
    const char *nickname = SECURE_SERVER_CERTIFICATE;
    SECStatus secsts;
    int sts = -EINVAL;

    PR_Init(PR_SYSTEM_THREAD, PR_PRIORITY_NORMAL, 1);

    PM_INIT_LOCKS();
    PM_LOCK(__pmLock_libpcp);

    /* Configure optional (cmdline) password file in case DB locked */
    secure_server.password_file = passwd;
    PK11_SetPasswordFunc(certificate_database_password);

    /*
     * Configure location of the NSS database with a sane default.
     * For servers, we default to the shared (sql) system-wide database.
     * If command line db specified, pass it directly through - allowing
     * any old database format, at the users discretion.
     */

    if (!db) {
	char *path = serverdb(secure_server.database_path, MAXPATHLEN);

	/* this is the default case on some platforms, so no log spam */
	if (access(path, R_OK|X_OK) < 0) {
	    if (pmDebug & DBG_TRACE_CONTEXT)
		__pmNotifyErr(LOG_INFO,
			"Cannot access system security database: %s",
			secure_server.database_path);
	    sts = 0;	/* not fatal - just no secure connections */
	    goto done;
	}
    } else {
	/* shortened-buffer-size (-2) guarantees null-termination */
	strncpy(secure_server.database_path, db, MAXPATHLEN-2);
    }

    secsts = NSS_Init(secure_server.database_path);
    if (secsts != SECSuccess) {
	__pmNotifyErr(LOG_ERR, "Cannot setup certificate DB (%s): %s",
			secure_server.database_path,
			pmErrStr(__pmSecureSocketsError()));
	goto done;
    }

    secsts = NSS_SetExportPolicy();
    if (secsts != SECSuccess) {
	__pmNotifyErr(LOG_ERR, "Unable to set NSS export policy: %s",
		pmErrStr(__pmSecureSocketsError()));
	goto done;
    }

    /* Configure SSL session cache for multi-process server, using defaults */
    secsts = SSL_ConfigMPServerSIDCache(0, 0, 0, NULL);
    if (secsts != SECSuccess) {
	__pmNotifyErr(LOG_ERR, "Unable to configure SSL session ID cache: %s",
		pmErrStr(__pmSecureSocketsError()));
	goto done;
    } else {
	secure_server.ssl_session_cache_setup = 1;
    }

    /*
     * Iterate over any/all PCP Collector nickname certificates,
     * seeking one valid certificate.  No-such-nickname is not an
     * error (not configured by admin at all) but anything else is.
     */
    CERTCertList *certlist;
    CERTCertDBHandle *nssdb = CERT_GetDefaultCertDB();
    CERTCertificate *dbcert = PK11_FindCertFromNickname(nickname, NULL);

    if (dbcert) {
	PRTime now = PR_Now();
	SECItem *name = &dbcert->derSubject;
	CERTCertListNode *node;

	certlist = CERT_CreateSubjectCertList(NULL, nssdb, name, now, PR_FALSE);
	if (certlist) {
	    for (node = CERT_LIST_HEAD(certlist);
		 !CERT_LIST_END(node, certlist);
		 node = CERT_LIST_NEXT (node)) {
		if (pmDebug & DBG_TRACE_CONTEXT)
		    __pmDumpCertificate(stderr, nickname, node->cert);
		if (!__pmValidCertificate(nssdb, node->cert, now))
		    continue;
		secure_server.certificate_verified = 1;
		break;
	    }
	    CERT_DestroyCertList(certlist);
	}

	if (secure_server.certificate_verified) {
	    secure_server.certificate_KEA = NSS_FindCertKEAType(dbcert);
	    secure_server.private_key = PK11_FindKeyByAnyCert(dbcert, NULL);
	    if (!secure_server.private_key) {
		__pmNotifyErr(LOG_ERR, "Unable to extract %s private key",
				nickname);
		CERT_DestroyCertificate(dbcert);
		secure_server.certificate_verified = 0;
		goto done;
	    }
	} else {
	    __pmNotifyErr(LOG_ERR, "Unable to find a valid %s", nickname);
	    CERT_DestroyCertificate(dbcert);
	    goto done;
	}
    }

    if (secure_server.certificate_verified) {
	secure_server.certificate = dbcert;
    } else if (pmDebug & DBG_TRACE_CONTEXT) {
	__pmNotifyErr(LOG_INFO, "No valid %s in security database: %s",
		nickname, secure_server.database_path);
    }
    sts = 0;

done:
    PM_UNLOCK(__pmLock_libpcp);
    return sts;
}

void
__pmSecureServerShutdown(void)
{
    PM_INIT_LOCKS();
    PM_LOCK(__pmLock_libpcp);
    if (secure_server.certificate) {
	CERT_DestroyCertificate(secure_server.certificate);
	secure_server.certificate = NULL;
    }
    if (secure_server.private_key) {
	SECKEY_DestroyPrivateKey(secure_server.private_key);
	secure_server.private_key = NULL;
    }
    if (secure_server.ssl_session_cache_setup) {
	SSL_ShutdownServerSessionIDCache();
	secure_server.ssl_session_cache_setup = 0;
    }    
    PM_UNLOCK(__pmLock_libpcp);
    NSS_Shutdown();
}

int
__pmSecureServerHandshake(int fd, int flags)
{
    PRIntervalTime timer;
    PRFileDesc	*sslsocket;
    SECStatus	secsts;
    int		msec;
    int		sts;

    /* protect from unsupported requests from future/oddball clients */
    if ((flags & ~(PDU_FLAG_SECURE|PDU_FLAG_COMPRESS)) != 0)
	return PM_ERR_IPC;

    if ((sts = __pmSecureServerIPCFlags(fd, flags)) < 0)
	return sts;

    /* if no request for TLS/SSL has been made, our work here is done */
    if (!(flags & PDU_FLAG_SECURE))
	return 0;

    sslsocket = (PRFileDesc *)__pmGetSecureSocket(fd);
    if (!sslsocket)
	return PM_ERR_IPC;

    PM_INIT_LOCKS();
    PM_LOCK(__pmLock_libpcp);
    secsts = SSL_ConfigSecureServer(sslsocket,
			secure_server.certificate,
			secure_server.private_key,
			secure_server.certificate_KEA);
    PM_UNLOCK(__pmLock_libpcp);

    if (secsts != SECSuccess) {
	__pmNotifyErr(LOG_ERR, "Unable to configure secure server: %s",
			    pmErrStr(__pmSecureSocketsError()));
	return PM_ERR_IPC;
    }

    secsts = SSL_ResetHandshake(sslsocket, PR_TRUE /*server*/);
    if (secsts != SECSuccess) {
	__pmNotifyErr(LOG_ERR, "Unable to reset secure handshake: %s",
			    pmErrStr(__pmSecureSocketsError()));
	return PM_ERR_IPC;
    }

    /* Server initiates handshake now to get early visibility of errors */
    msec = __pmConvertTimeout(TIMEOUT_DEFAULT);
    timer = PR_MillisecondsToInterval(msec);
    secsts = SSL_ForceHandshakeWithTimeout(sslsocket, timer);
    if (secsts != SECSuccess) {
	__pmNotifyErr(LOG_ERR, "Unable to force secure handshake: %s",
			    pmErrStr(__pmSecureSocketsError()));
	return PM_ERR_IPC;
    }

    return 0;
}
