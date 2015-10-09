/*
 * Copyright (C) 2015 Ryan Doyle.
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

#include <ruby.h>
#include <pcp/pmapi.h>

VALUE pcp_module = Qnil;
VALUE pcp_pmapi_class = Qnil;
VALUE pcp_metric_source_failed = Qnil;

#ifdef DEBUG
pmDebug = 16;
#endif

typedef struct {
    int pm_context;
} PmApi_Context;

static void deallocate(PmApi_Context *pmapi_context) {
    pmDestroyContext(pmapi_context->pm_context);
    free(pmapi_context);
}

static VALUE allocate(VALUE self) {
    /* TODO: Deal with malloc errors */
    PmApi_Context *pmapi_context = malloc(sizeof(PmApi_Context));
    return Data_Wrap_Struct(self, 0, deallocate, pmapi_context);
}

static int get_context(VALUE self) {
    PmApi_Context *pmapi_context;
    Data_Get_Struct(self, PmApi_Context, pmapi_context);
    return pmapi_context->pm_context;
}

static void use_context(VALUE self) {
    pmUseContext(get_context(self));
}

static VALUE rb_pmNewContext(VALUE self, VALUE metric_source, VALUE metric_source_argument) {
    int pm_context, pm_metric_source;
    char *pm_metric_source_argument = NULL;
    PmApi_Context *pmapi_context;
    Data_Get_Struct(self, PmApi_Context, pmapi_context);

    pm_metric_source = NUM2INT(metric_source);

    if(TYPE(metric_source_argument) == T_STRING) {
        pm_metric_source_argument = StringValuePtr(metric_source_argument);
    }

    if ((pm_context = pmNewContext(pm_metric_source, pm_metric_source_argument)) < 0) {
        if (pm_metric_source == PM_CONTEXT_HOST)
            rb_raise(pcp_metric_source_failed, "Cannot connect to PMCD on host %s", pm_metric_source_argument);
        else if (pm_metric_source == PM_CONTEXT_LOCAL)
            rb_raise(pcp_metric_source_failed, "Cannot make standalone connection on localhost");
        else
            rb_raise(pcp_metric_source_failed, "Cannot open archive %s", pm_metric_source_argument);
    }

    pmapi_context->pm_context = pm_context;

    return self;
}

static VALUE rb_pmGetContextHostName_r(VALUE self) {
    char result_buffer[MAXHOSTNAMELEN];

    pmGetContextHostName_r(get_context(self), (char *)&result_buffer, MAXHOSTNAMELEN);

    return rb_tainted_str_new_cstr(result_buffer);
}

static VALUE rb_pmGetPMNSLocation(VALUE self) {
    int pmns_location;

    use_context(self);

    pmns_location = pmGetPMNSLocation();

    return INT2NUM(pmns_location);
}


void Init_pcp_native() {
    pcp_module = rb_define_module("PCP");
    pcp_metric_source_failed = rb_define_class_under(pcp_module, "MetricSourceFailed", rb_eStandardError);
    pcp_pmapi_class = rb_define_class_under(pcp_module, "PMAPI", rb_cObject);

    rb_define_const(pcp_pmapi_class, "PM_SPACE_BYTE", INT2NUM(PM_SPACE_BYTE));
    rb_define_const(pcp_pmapi_class, "PM_SPACE_KBYTE", INT2NUM(PM_SPACE_KBYTE));
    rb_define_const(pcp_pmapi_class, "PM_SPACE_MBYTE", INT2NUM(PM_SPACE_MBYTE));
    rb_define_const(pcp_pmapi_class, "PM_SPACE_GBYTE", INT2NUM(PM_SPACE_GBYTE));
    rb_define_const(pcp_pmapi_class, "PM_SPACE_TBYTE", INT2NUM(PM_SPACE_TBYTE));
    rb_define_const(pcp_pmapi_class, "PM_SPACE_PBYTE", INT2NUM(PM_SPACE_PBYTE));
    rb_define_const(pcp_pmapi_class, "PM_SPACE_EBYTE", INT2NUM(PM_SPACE_EBYTE));

    rb_define_const(pcp_pmapi_class, "PM_TIME_NSEC", INT2NUM(PM_TIME_NSEC));
    rb_define_const(pcp_pmapi_class, "PM_TIME_USEC", INT2NUM(PM_TIME_USEC));
    rb_define_const(pcp_pmapi_class, "PM_TIME_MSEC", INT2NUM(PM_TIME_MSEC));
    rb_define_const(pcp_pmapi_class, "PM_TIME_SEC", INT2NUM(PM_TIME_SEC));
    rb_define_const(pcp_pmapi_class, "PM_TIME_MIN", INT2NUM(PM_TIME_MIN));
    rb_define_const(pcp_pmapi_class, "PM_TIME_HOUR", INT2NUM(PM_TIME_HOUR));

    rb_define_const(pcp_pmapi_class, "PM_CONTEXT_UNDEF", INT2NUM(PM_CONTEXT_UNDEF));
    rb_define_const(pcp_pmapi_class, "PM_CONTEXT_HOST", INT2NUM(PM_CONTEXT_HOST));
    rb_define_const(pcp_pmapi_class, "PM_CONTEXT_ARCHIVE", INT2NUM(PM_CONTEXT_ARCHIVE));
    rb_define_const(pcp_pmapi_class, "PM_CONTEXT_LOCAL", INT2NUM(PM_CONTEXT_LOCAL));
    rb_define_const(pcp_pmapi_class, "PM_CONTEXT_TYPEMASK", INT2NUM(PM_CONTEXT_TYPEMASK));
    rb_define_const(pcp_pmapi_class, "PM_CTXFLAG_SECURE", INT2NUM(PM_CTXFLAG_SECURE));
    rb_define_const(pcp_pmapi_class, "PM_CTXFLAG_COMPRESS", INT2NUM(PM_CTXFLAG_COMPRESS));
    rb_define_const(pcp_pmapi_class, "PM_CTXFLAG_RELAXED", INT2NUM(PM_CTXFLAG_RELAXED));
    rb_define_const(pcp_pmapi_class, "PM_CTXFLAG_AUTH", INT2NUM(PM_CTXFLAG_AUTH));
    rb_define_const(pcp_pmapi_class, "PM_CTXFLAG_CONTAINER", INT2NUM(PM_CTXFLAG_CONTAINER));

    rb_define_const(pcp_pmapi_class, "PMNS_LOCAL", INT2NUM(PMNS_LOCAL));
    rb_define_const(pcp_pmapi_class, "PMNS_REMOTE", INT2NUM(PMNS_REMOTE));
    rb_define_const(pcp_pmapi_class, "PMNS_ARCHIVE", INT2NUM(PMNS_ARCHIVE));

    rb_define_const(pcp_pmapi_class, "PM_ERR_GENERIC", INT2NUM(PM_ERR_GENERIC));
    rb_define_const(pcp_pmapi_class, "PM_ERR_PMNS", INT2NUM(PM_ERR_PMNS));
    rb_define_const(pcp_pmapi_class, "PM_ERR_NOPMNS", INT2NUM(PM_ERR_NOPMNS));
    rb_define_const(pcp_pmapi_class, "PM_ERR_DUPPMNS", INT2NUM(PM_ERR_DUPPMNS));
    rb_define_const(pcp_pmapi_class, "PM_ERR_TEXT", INT2NUM(PM_ERR_TEXT));
    rb_define_const(pcp_pmapi_class, "PM_ERR_APPVERSION", INT2NUM(PM_ERR_APPVERSION));
    rb_define_const(pcp_pmapi_class, "PM_ERR_VALUE", INT2NUM(PM_ERR_VALUE));
    rb_define_const(pcp_pmapi_class, "PM_ERR_TIMEOUT", INT2NUM(PM_ERR_TIMEOUT));
    rb_define_const(pcp_pmapi_class, "PM_ERR_NODATA", INT2NUM(PM_ERR_NODATA));
    rb_define_const(pcp_pmapi_class, "PM_ERR_RESET", INT2NUM(PM_ERR_RESET));
    rb_define_const(pcp_pmapi_class, "PM_ERR_NAME", INT2NUM(PM_ERR_NAME));
    rb_define_const(pcp_pmapi_class, "PM_ERR_PMID", INT2NUM(PM_ERR_PMID));
    rb_define_const(pcp_pmapi_class, "PM_ERR_INDOM", INT2NUM(PM_ERR_INDOM));
    rb_define_const(pcp_pmapi_class, "PM_ERR_INST", INT2NUM(PM_ERR_INST));
    rb_define_const(pcp_pmapi_class, "PM_ERR_UNIT", INT2NUM(PM_ERR_UNIT));
    rb_define_const(pcp_pmapi_class, "PM_ERR_CONV", INT2NUM(PM_ERR_CONV));
    rb_define_const(pcp_pmapi_class, "PM_ERR_TRUNC", INT2NUM(PM_ERR_TRUNC));
    rb_define_const(pcp_pmapi_class, "PM_ERR_SIGN", INT2NUM(PM_ERR_SIGN));
    rb_define_const(pcp_pmapi_class, "PM_ERR_PROFILE", INT2NUM(PM_ERR_PROFILE));
    rb_define_const(pcp_pmapi_class, "PM_ERR_IPC", INT2NUM(PM_ERR_IPC));
    rb_define_const(pcp_pmapi_class, "PM_ERR_EOF", INT2NUM(PM_ERR_EOF));
    rb_define_const(pcp_pmapi_class, "PM_ERR_NOTHOST", INT2NUM(PM_ERR_NOTHOST));
    rb_define_const(pcp_pmapi_class, "PM_ERR_EOL", INT2NUM(PM_ERR_EOL));
    rb_define_const(pcp_pmapi_class, "PM_ERR_MODE", INT2NUM(PM_ERR_MODE));
    rb_define_const(pcp_pmapi_class, "PM_ERR_LABEL", INT2NUM(PM_ERR_LABEL));
    rb_define_const(pcp_pmapi_class, "PM_ERR_LOGREC", INT2NUM(PM_ERR_LOGREC));
    rb_define_const(pcp_pmapi_class, "PM_ERR_NOTARCHIVE", INT2NUM(PM_ERR_NOTARCHIVE));
    rb_define_const(pcp_pmapi_class, "PM_ERR_LOGFILE", INT2NUM(PM_ERR_LOGFILE));
    rb_define_const(pcp_pmapi_class, "PM_ERR_NOCONTEXT", INT2NUM(PM_ERR_NOCONTEXT));
    rb_define_const(pcp_pmapi_class, "PM_ERR_PROFILESPEC", INT2NUM(PM_ERR_PROFILESPEC));
    rb_define_const(pcp_pmapi_class, "PM_ERR_PMID_LOG", INT2NUM(PM_ERR_PMID_LOG));
    rb_define_const(pcp_pmapi_class, "PM_ERR_INDOM_LOG", INT2NUM(PM_ERR_INDOM_LOG));
    rb_define_const(pcp_pmapi_class, "PM_ERR_INST_LOG", INT2NUM(PM_ERR_INST_LOG));
    rb_define_const(pcp_pmapi_class, "PM_ERR_NOPROFILE", INT2NUM(PM_ERR_NOPROFILE));
    rb_define_const(pcp_pmapi_class, "PM_ERR_NOAGENT", INT2NUM(PM_ERR_NOAGENT));
    rb_define_const(pcp_pmapi_class, "PM_ERR_PERMISSION", INT2NUM(PM_ERR_PERMISSION));
    rb_define_const(pcp_pmapi_class, "PM_ERR_CONNLIMIT", INT2NUM(PM_ERR_CONNLIMIT));
    rb_define_const(pcp_pmapi_class, "PM_ERR_AGAIN", INT2NUM(PM_ERR_AGAIN));
    rb_define_const(pcp_pmapi_class, "PM_ERR_ISCONN", INT2NUM(PM_ERR_ISCONN));
    rb_define_const(pcp_pmapi_class, "PM_ERR_NOTCONN", INT2NUM(PM_ERR_NOTCONN));
    rb_define_const(pcp_pmapi_class, "PM_ERR_NEEDPORT", INT2NUM(PM_ERR_NEEDPORT));
    rb_define_const(pcp_pmapi_class, "PM_ERR_NONLEAF", INT2NUM(PM_ERR_NONLEAF));
    rb_define_const(pcp_pmapi_class, "PM_ERR_TYPE", INT2NUM(PM_ERR_TYPE));
    rb_define_const(pcp_pmapi_class, "PM_ERR_THREAD", INT2NUM(PM_ERR_THREAD));
    rb_define_const(pcp_pmapi_class, "PM_ERR_NOCONTAINER", INT2NUM(PM_ERR_NOCONTAINER));
    rb_define_const(pcp_pmapi_class, "PM_ERR_BADSTORE", INT2NUM(PM_ERR_BADSTORE));
    rb_define_const(pcp_pmapi_class, "PM_ERR_TOOSMALL", INT2NUM(PM_ERR_TOOSMALL));
    rb_define_const(pcp_pmapi_class, "PM_ERR_TOOBIG", INT2NUM(PM_ERR_TOOBIG));
    rb_define_const(pcp_pmapi_class, "PM_ERR_FAULT", INT2NUM(PM_ERR_FAULT));
    rb_define_const(pcp_pmapi_class, "PM_ERR_PMDAREADY", INT2NUM(PM_ERR_PMDAREADY));
    rb_define_const(pcp_pmapi_class, "PM_ERR_PMDANOTREADY", INT2NUM(PM_ERR_PMDANOTREADY));
    rb_define_const(pcp_pmapi_class, "PM_ERR_NYI", INT2NUM(PM_ERR_NYI));

    rb_define_alloc_func(pcp_pmapi_class, allocate);
    rb_define_private_method(pcp_pmapi_class, "pmNewContext", rb_pmNewContext, 2);
    rb_define_method(pcp_pmapi_class, "pmGetContextHostName_r", rb_pmGetContextHostName_r, 0);
    rb_define_method(pcp_pmapi_class, "pmGetPMNSLocation", rb_pmGetPMNSLocation, 0);

}






