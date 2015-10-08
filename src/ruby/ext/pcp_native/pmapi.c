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

static VALUE native_pm_new_context(VALUE self, VALUE metric_source, VALUE metric_source_argument) {
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

static VALUE native_pm_get_context_hostname_r(VALUE self) {
    char result_buffer[MAXHOSTNAMELEN];

    pmGetContextHostName_r(get_context(self), (char *)&result_buffer, MAXHOSTNAMELEN);

    return rb_tainted_str_new_cstr(result_buffer);
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

    rb_define_alloc_func(pcp_pmapi_class, allocate);
    rb_define_method(pcp_pmapi_class, "native_pm_new_context", native_pm_new_context, 2);
    rb_define_method(pcp_pmapi_class, "native_pm_get_context_hostname_r", native_pm_get_context_hostname_r, 0);

}






