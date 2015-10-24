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

/* Error classes */
VALUE pcp_pmapi_error = Qnil;
VALUE pcp_pmapi_pmns_error = Qnil;
VALUE pcp_pmapi_no_pmns_error = Qnil;
VALUE pcp_pmapi_dup_pmns_error = Qnil;
VALUE pcp_pmapi_text_error = Qnil;
VALUE pcp_pmapi_app_version_error = Qnil;
VALUE pcp_pmapi_value_error = Qnil;
VALUE pcp_pmapi_timeout_error = Qnil;
VALUE pcp_pmapi_no_data_error = Qnil;
VALUE pcp_pmapi_reset_error = Qnil;
VALUE pcp_pmapi_name_error = Qnil;
VALUE pcp_pmapi_pmid_error = Qnil;
VALUE pcp_pmapi_indom_error = Qnil;
VALUE pcp_pmapi_inst_error = Qnil;
VALUE pcp_pmapi_unit_error = Qnil;
VALUE pcp_pmapi_conv_error = Qnil;
VALUE pcp_pmapi_trunc_error = Qnil;
VALUE pcp_pmapi_sign_error = Qnil;
VALUE pcp_pmapi_profile_error = Qnil;
VALUE pcp_pmapi_ipc_error = Qnil;
VALUE pcp_pmapi_eof_error = Qnil;
VALUE pcp_pmapi_not_host_error = Qnil;
VALUE pcp_pmapi_eol_error = Qnil;
VALUE pcp_pmapi_mode_error = Qnil;
VALUE pcp_pmapi_label_error = Qnil;
VALUE pcp_pmapi_log_rec_error = Qnil;
VALUE pcp_pmapi_not_archive_error = Qnil;
VALUE pcp_pmapi_log_file_error = Qnil;
VALUE pcp_pmapi_no_context_error = Qnil;
VALUE pcp_pmapi_profile_spec_error = Qnil;
VALUE pcp_pmapi_pmid_log_error = Qnil;
VALUE pcp_pmapi_indom_log_error = Qnil;
VALUE pcp_pmapi_inst_log_error = Qnil;
VALUE pcp_pmapi_no_profile_error = Qnil;
VALUE pcp_pmapi_no_agent_error = Qnil;
VALUE pcp_pmapi_permission_error = Qnil;
VALUE pcp_pmapi_connlimit_error = Qnil;
VALUE pcp_pmapi_again_error = Qnil;
VALUE pcp_pmapi_is_conn_error = Qnil;
VALUE pcp_pmapi_not_conn_error = Qnil;
VALUE pcp_pmapi_need_port_error = Qnil;
VALUE pcp_pmapi_non_leaf_error = Qnil;
VALUE pcp_pmapi_type_error = Qnil;
VALUE pcp_pmapi_thread_error = Qnil;
VALUE pcp_pmapi_no_container_error = Qnil;
VALUE pcp_pmapi_bad_store_error = Qnil;
VALUE pcp_pmapi_too_small_error = Qnil;
VALUE pcp_pmapi_too_big_error = Qnil;
VALUE pcp_pmapi_fault_error = Qnil;
VALUE pcp_pmapi_pmda_ready_error = Qnil;
VALUE pcp_pmapi_pmda_not_ready_error = Qnil;
VALUE pcp_pmapi_nyi_error = Qnil;

static const struct pmapi_to_ruby_exception {
    int pmapi_error;
    VALUE *ruby_exception;
} pmapi_to_ruby_exception_map[] = {
    {PM_ERR_GENERIC, &pcp_pmapi_error},
    {PM_ERR_PMNS, &pcp_pmapi_pmns_error},
    {PM_ERR_NOPMNS, &pcp_pmapi_no_pmns_error},
    {PM_ERR_DUPPMNS, &pcp_pmapi_dup_pmns_error},
    {PM_ERR_TEXT, &pcp_pmapi_text_error},
    {PM_ERR_APPVERSION, &pcp_pmapi_app_version_error},
    {PM_ERR_VALUE, &pcp_pmapi_value_error},
    {PM_ERR_TIMEOUT, &pcp_pmapi_timeout_error},
    {PM_ERR_NODATA, &pcp_pmapi_no_data_error},
    {PM_ERR_RESET, &pcp_pmapi_reset_error},
    {PM_ERR_NAME, &pcp_pmapi_name_error},
    {PM_ERR_PMID, &pcp_pmapi_pmid_error},
    {PM_ERR_INDOM, &pcp_pmapi_indom_error},
    {PM_ERR_INST, &pcp_pmapi_inst_error},
    {PM_ERR_UNIT, &pcp_pmapi_unit_error},
    {PM_ERR_CONV, &pcp_pmapi_conv_error},
    {PM_ERR_TRUNC, &pcp_pmapi_trunc_error},
    {PM_ERR_SIGN, &pcp_pmapi_sign_error},
    {PM_ERR_PROFILE, &pcp_pmapi_profile_error},
    {PM_ERR_IPC, &pcp_pmapi_ipc_error},
    {PM_ERR_EOF, &pcp_pmapi_eof_error},
    {PM_ERR_NOTHOST, &pcp_pmapi_not_host_error},
    {PM_ERR_EOL, &pcp_pmapi_eol_error},
    {PM_ERR_MODE, &pcp_pmapi_mode_error},
    {PM_ERR_LABEL, &pcp_pmapi_label_error},
    {PM_ERR_LOGREC, &pcp_pmapi_log_rec_error},
    {PM_ERR_NOTARCHIVE, &pcp_pmapi_not_archive_error},
    {PM_ERR_LOGFILE, &pcp_pmapi_log_file_error},
    {PM_ERR_NOCONTEXT, &pcp_pmapi_no_context_error},
    {PM_ERR_PROFILESPEC, &pcp_pmapi_profile_spec_error},
    {PM_ERR_PMID_LOG, &pcp_pmapi_pmid_log_error},
    {PM_ERR_INDOM_LOG, &pcp_pmapi_indom_log_error},
    {PM_ERR_INST_LOG, &pcp_pmapi_inst_log_error},
    {PM_ERR_NOPROFILE, &pcp_pmapi_no_profile_error},
    {PM_ERR_NOAGENT, &pcp_pmapi_no_agent_error},
    {PM_ERR_PERMISSION, &pcp_pmapi_permission_error},
    {PM_ERR_CONNLIMIT, &pcp_pmapi_connlimit_error},
    {PM_ERR_AGAIN, &pcp_pmapi_again_error},
    {PM_ERR_ISCONN, &pcp_pmapi_is_conn_error},
    {PM_ERR_NOTCONN, &pcp_pmapi_not_conn_error},
    {PM_ERR_NEEDPORT, &pcp_pmapi_need_port_error},
    {PM_ERR_NONLEAF, &pcp_pmapi_non_leaf_error},
    {PM_ERR_TYPE, &pcp_pmapi_type_error},
    {PM_ERR_THREAD, &pcp_pmapi_thread_error},
    {PM_ERR_NOCONTAINER, &pcp_pmapi_no_container_error},
    {PM_ERR_BADSTORE, &pcp_pmapi_bad_store_error},
    {PM_ERR_TOOSMALL, &pcp_pmapi_too_small_error},
    {PM_ERR_TOOBIG, &pcp_pmapi_too_big_error},
    {PM_ERR_FAULT, &pcp_pmapi_fault_error},
    {PM_ERR_PMDAREADY, &pcp_pmapi_pmda_ready_error},
    {PM_ERR_PMDANOTREADY, &pcp_pmapi_pmda_not_ready_error},
    {PM_ERR_NYI, &pcp_pmapi_nyi_error},
};

//#ifdef DEBUG
#include <pcp/impl.h>
//pmDebug = DBG_TRACE_CONTEXT && DBG_TRACE_PMNS;
//#endif

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

static VALUE get_exception_from_pmapi_error_code(int error_code) {
    int i, number_of_pmapi_to_ruby_errors;
    number_of_pmapi_to_ruby_errors = sizeof(pmapi_to_ruby_exception_map) / sizeof(struct pmapi_to_ruby_exception);

    for(i=0; i < number_of_pmapi_to_ruby_errors; i++) {
        if(pmapi_to_ruby_exception_map[i].pmapi_error == error_code) {
            return *pmapi_to_ruby_exception_map[i].ruby_exception;
        }
    }
    /* Default to a generic error */
    return pcp_pmapi_error;
}

static void raise_error_from_pmapi_error_code(int error_number) {
    char errmsg[PM_MAXERRMSGLEN];
    VALUE exception_to_raise;

    exception_to_raise = get_exception_from_pmapi_error_code(error_number);

    rb_raise(exception_to_raise, (const char *)pmErrStr_r(error_number, (char *)&errmsg, sizeof(errmsg)));
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
        raise_error_from_pmapi_error_code(pm_context);
    }

    pmapi_context->pm_context = pm_context;

    return self;
}
/* call-seq: pmapi.pmGetContextHostName
 *
 * Returns the hostname that the context is connected to
 *
 */
static VALUE rb_pmGetContextHostName(VALUE self) {
    char result_buffer[MAXHOSTNAMELEN];

    pmGetContextHostName_r(get_context(self), (char *)&result_buffer, MAXHOSTNAMELEN);

    return rb_tainted_str_new_cstr(result_buffer);
}

/* call-seq: pmapi.pmGetPMNSLocation => location
 *
 * Returns the location of the PMNS. Either +PCP::PMAPI::PMNS_LOCAL+,
 * +PCP::PMAPI::PMNS_REMOTE+ or +PCP::PMAPI::PMNS_ARCHIVE+ depending
 * how the context was created.
 *
 *     pmapi.pmGetPMNSLocation #=> PMNS_LOCAL
 *
 */
static VALUE rb_pmGetPMNSLocation(VALUE self) {
    int pmns_location;

    use_context(self);

    pmns_location = pmGetPMNSLocation();

    return INT2NUM(pmns_location);
}

/* call-seq: pmapi.pmLoadNameSpace(filename)
 *
 * Load the namespace from a file on disk.
 *
 *     pmapi.pmLoadNameSpace('/path/to/namespace.pmns')
 */
static VALUE rb_pmLoadNameSpace(VALUE self, VALUE filename) {
    int error;

    use_context(self);

    error = pmLoadNameSpace(StringValuePtr(filename));
    if(error < 0 ) {
        raise_error_from_pmapi_error_code(error);
    }

    return Qnil;
}

static VALUE rb_pmUnloadNameSpace(VALUE self) {
    use_context(self);

    pmUnloadNameSpace();

    return Qnil;
}

static VALUE rb_pmTrimNameSpace(VALUE self) {
    use_context(self);

    pmTrimNameSpace();

    return Qnil;
}

static VALUE rb_pmLookupName(VALUE self, VALUE names) {

    int number_of_names, error;
    char ** namelist;
    pmID * pmidlist;
    long i;
    VALUE result;

    use_context(self);

    number_of_names = RARRAY_LENINT(names);
    namelist = malloc(sizeof(char *) * number_of_names);
    pmidlist = malloc(sizeof(pmID) * number_of_names);
    result = rb_ary_new2(number_of_names);

    /* Populate name list */
    for(i = 0; i < number_of_names; i++) {
        namelist[i] = RSTRING_PTR(rb_ary_entry(names, i));
    }

    error = pmLookupName(number_of_names, namelist, pmidlist);
    if(error < 0 ) {
        raise_error_from_pmapi_error_code(error);
    } else {
        /* Build up the array of hashes to return */
        for(i = 0; i < number_of_names; i++) {
            VALUE hash = rb_hash_new();
            VALUE hash_key = rb_ary_entry(names, i);
            VALUE pmid = UINT2NUM(pmidlist[i]);
            rb_hash_aset(hash, hash_key, pmid);
            rb_ary_push(result, hash);
        }
    }

    free(namelist);
    free(pmidlist);

    return result;
}

static VALUE rb_pmGetChildren(VALUE self, VALUE root_name) {

    char ** offspring;
    int number_of_children, error, i;
    VALUE children_names;

    use_context(self);

    children_names = rb_ary_new();

    if((error = pmGetChildren(StringValueCStr(root_name), &offspring)) < 0 ) {
        raise_error_from_pmapi_error_code(error);
    } else {
        number_of_children = error;
        for(i = 0; i < number_of_children; i++) {
            rb_ary_push(children_names, rb_tainted_str_new_cstr(offspring[i]));
        }
    }

    if(number_of_children > 0) {
        free(offspring);
    }

    return children_names;
}

static VALUE rb_pmGetChildrenStatus(VALUE self, VALUE root_name) {

    char ** offspring;
    int * offspring_leaf_status;
    int number_of_children, error, i;
    VALUE children_names_and_status;

    use_context(self);

    children_names_and_status = rb_ary_new();

    if((error = pmGetChildrenStatus(StringValueCStr(root_name), &offspring, &offspring_leaf_status)) < 0 ) {
        raise_error_from_pmapi_error_code(error);
    } else {
        number_of_children = error;
        for(i = 0; i < number_of_children; i++) {
            VALUE child_hash = rb_hash_new();
            VALUE child_name = rb_tainted_str_new_cstr(offspring[i]);
            VALUE child_status = INT2NUM(offspring_leaf_status[i]);
            rb_hash_aset(child_hash, child_name, child_status);
            rb_ary_push(children_names_and_status, child_hash);
        }
    }

    if(number_of_children > 0) {
        free(offspring);
        free(offspring_leaf_status);
    }

    return children_names_and_status;
}

static VALUE rb_pmNameID(VALUE self, VALUE pmid) {
    int error;
    char *name_result;
    VALUE result;

    use_context(self);

    if((error = pmNameID(NUM2UINT(pmid), &name_result)) < 0) {
        raise_error_from_pmapi_error_code(error);
        return Qnil;
    } else {
        result = rb_tainted_str_new_cstr(name_result);
        /* TODO: Do we need to free tainted strings? */
        free(name_result);
        return result;
    }
}

static VALUE rb_pmNameAll(VALUE self, VALUE pmid) {
    int error, number_of_names, i;
    char ** name_result;
    VALUE result;

    use_context(self);

    if((error = pmNameAll(NUM2UINT(pmid), &name_result)) < 0) {
        raise_error_from_pmapi_error_code(error);
        return Qnil;
    } else {
        number_of_names = error;
        result = rb_ary_new2(number_of_names);
        for(i = 0; i < number_of_names; i++) {
            rb_ary_push(result, rb_tainted_str_new_cstr(name_result[i]));
        }
    }

    free(name_result);
    return result;
}

static void dometric(const char * name) {
    rb_yield(rb_tainted_str_new_cstr(name));
}

static VALUE rb_pmTraversePMNS(VALUE self, VALUE name) {
    int error;

    use_context(self);

    if((error = pmTraversePMNS(StringValueCStr(name), dometric)) < 0) {
        raise_error_from_pmapi_error_code(error);
    }

    return Qnil;
}

static VALUE rb_create_symbol_from_str(const char *name) {
    return ID2SYM(rb_intern(name));
}

static VALUE create_pmUnits(pmUnits units) {
    VALUE pmUnitsHash = rb_hash_new();

    rb_hash_aset(pmUnitsHash, rb_create_symbol_from_str("dimSpace"), INT2NUM(units.dimSpace));
    rb_hash_aset(pmUnitsHash, rb_create_symbol_from_str("dimTime"), INT2NUM(units.dimTime));
    rb_hash_aset(pmUnitsHash, rb_create_symbol_from_str("dimCount"), INT2NUM(units.dimCount));
    rb_hash_aset(pmUnitsHash, rb_create_symbol_from_str("scaleSpace"), UINT2NUM(units.scaleSpace));
    rb_hash_aset(pmUnitsHash, rb_create_symbol_from_str("scaleTime"), UINT2NUM(units.scaleTime));
    rb_hash_aset(pmUnitsHash, rb_create_symbol_from_str("scaleCount"), INT2NUM(units.scaleCount));

    return pmUnitsHash;
}

static VALUE create_pmDesc(pmDesc desc) {
    VALUE pmDescHash = rb_hash_new();

    rb_hash_aset(pmDescHash, rb_create_symbol_from_str("pmid"), UINT2NUM(desc.pmid));
    rb_hash_aset(pmDescHash, rb_create_symbol_from_str("type"), INT2NUM(desc.type));
    rb_hash_aset(pmDescHash, rb_create_symbol_from_str("indom"), UINT2NUM(desc.indom));
    rb_hash_aset(pmDescHash, rb_create_symbol_from_str("sem"), INT2NUM(desc.sem));
    rb_hash_aset(pmDescHash, rb_create_symbol_from_str("units"), create_pmUnits(desc.units));

    return pmDescHash;
}

static VALUE rb_pmLookupDesc(VALUE self, VALUE pmid) {
    int error;
    pmDesc pmDesc;

    use_context(self);

    if((error = pmLookupDesc(NUM2UINT(pmid), &pmDesc)) < 0) {
        raise_error_from_pmapi_error_code(error);
        return Qnil;
    }

    return create_pmDesc(pmDesc);

}


static VALUE pmLookupInDom_with_lookup_function(VALUE self, VALUE indom, VALUE name, int(*indom_lookup_function)(pmInDom, const char *)) {
    int error_or_result;

    use_context(self);

    if((error_or_result = indom_lookup_function(NUM2UINT(indom), StringValueCStr(name))) < 0) {
        raise_error_from_pmapi_error_code(error_or_result);
        return Qnil;
    }

    return INT2NUM(error_or_result);
}

static VALUE rb_pmLookupInDom(VALUE self, VALUE indom, VALUE name) {
    return pmLookupInDom_with_lookup_function(self, indom, name, pmLookupInDom);
}

static VALUE rb_pmLookupInDomArchive(VALUE self, VALUE indom, VALUE name) {
    return pmLookupInDom_with_lookup_function(self, indom, name, pmLookupInDomArchive);
}

static VALUE pmNameInDom_with_lookup_function(VALUE self, VALUE indom, VALUE instance_id, int(*indom_lookup_function)(pmInDom, int, char **)) {
    int error;
    char *name;
    VALUE result;

    use_context(self);

    if((error = indom_lookup_function(NUM2UINT(indom), NUM2INT(instance_id), &name)) < 0) {
        raise_error_from_pmapi_error_code(error);
        return Qnil;
    }

    result = rb_tainted_str_new_cstr(name);
    free(name);

    return result;
}

static VALUE rb_pmNameInDom(VALUE self, VALUE indom, VALUE instance_id) {
    return pmNameInDom_with_lookup_function(self, indom, instance_id, pmNameInDom);
}

static VALUE rb_pmNameInDomArchive(VALUE self, VALUE indom, VALUE instance_id) {
    return pmNameInDom_with_lookup_function(self, indom, instance_id, pmNameInDomArchive);
}

static VALUE pmGetInDom_with_lookup_function(VALUE self, VALUE indom, int(*indom_lookup_function)(pmInDom, int **, char ***)) {
    int error_or_number_of_instances, i;
    int *instance_ids;
    char **instance_names;
    VALUE result;

    use_context(self);

    if((error_or_number_of_instances = indom_lookup_function(NUM2UINT(indom), &instance_ids, &instance_names)) < 0) {
        raise_error_from_pmapi_error_code(error_or_number_of_instances);
        return Qnil;
    }

    result = rb_ary_new2(error_or_number_of_instances);

    for(i=0; i<error_or_number_of_instances; i++) {
        VALUE instance_hash = rb_hash_new();
        VALUE instance_name = rb_tainted_str_new_cstr(instance_names[i]);
        VALUE instance_id = INT2NUM(instance_ids[i]);
        rb_hash_aset(instance_hash, instance_id, instance_name);
        rb_ary_push(result, instance_hash);

    }

    free(instance_ids);
    free(instance_names);

    return result;
}

static VALUE rb_pmGetInDom(VALUE self, VALUE indom) {
    return pmGetInDom_with_lookup_function(self, indom, pmGetInDom);
}

static VALUE rb_pmGetInDomArchive(VALUE self, VALUE indom) {
    return pmGetInDom_with_lookup_function(self, indom, pmGetInDomArchive);
}

static VALUE rb_pmWhichContext(VALUE self) {
    int error_or_context_number;

    use_context(self);

    if((error_or_context_number = pmWhichContext()) < 0) {
        raise_error_from_pmapi_error_code(error_or_context_number);
        return Qnil;
    }

    return INT2NUM(error_or_context_number);
}

static VALUE rb_pmDestroyContext() {
    rb_raise(rb_eNotImpError, "pmDestroyContext is controlled by object lifecycle");
    return Qnil;
}

static VALUE rb_pmDupContext() {
    rb_raise(rb_eNotImpError, "pmDupContext not supported");
    return Qnil;
}

static VALUE rb_pmUseContext() {
    rb_raise(rb_eNotImpError, "pmUseContext not supported");
    return Qnil;
}

static VALUE rb_pmReconnectContext(VALUE self) {
    int error;

    if((error = pmReconnectContext(get_context(self))) < 0) {
        raise_error_from_pmapi_error_code(error);
    }

    return Qnil;
}

static VALUE rb_pmDelProfile(VALUE self, VALUE indom, VALUE instance_identifiers) {
    int error, number_of_instance_identifiers, i;
    int *instlist = NULL;

    use_context(self);

    number_of_instance_identifiers = RARRAY_LENINT(instance_identifiers);

    if(number_of_instance_identifiers > 0) {
        instlist = malloc(sizeof(int *) * number_of_instance_identifiers);
        for(i = 0; i < number_of_instance_identifiers; i++) {
            instlist[i] = NUM2INT(rb_ary_entry(instance_identifiers, i));
        }
    }

    if((error = pmDelProfile(NUM2UINT(indom), number_of_instance_identifiers, instlist)) < 0) {
        raise_error_from_pmapi_error_code(error);
    }

    if(instlist) {
        free(instlist);
    }

    return Qnil;
}

static VALUE rb_pmAddProfile(VALUE self, VALUE indom, VALUE instance_identifiers) {
    int error, number_of_instance_identifiers, i;
    int *instlist = NULL;

    use_context(self);

    number_of_instance_identifiers = RARRAY_LENINT(instance_identifiers);

    if(number_of_instance_identifiers > 0) {
        instlist = malloc(sizeof(int *) * number_of_instance_identifiers);
        for(i = 0; i < number_of_instance_identifiers; i++) {
            instlist[i] = NUM2INT(rb_ary_entry(instance_identifiers, i));
        }
    }

    if((error = pmAddProfile(NUM2UINT(indom), number_of_instance_identifiers, instlist)) < 0) {
        raise_error_from_pmapi_error_code(error);
    }

    if(instlist) {
        free(instlist);
    }

    return Qnil;
}

void Init_pcp_native() {
    pcp_module = rb_define_module("PCP");
    pcp_pmapi_class = rb_define_class_under(pcp_module, "PMAPI", rb_cObject);
    /* Exceptions */
    pcp_pmapi_error = rb_define_class_under(pcp_pmapi_class, "Error", rb_eStandardError);
    pcp_pmapi_pmns_error = rb_define_class_under(pcp_pmapi_class, "PMNSError", pcp_pmapi_error);
    pcp_pmapi_no_pmns_error = rb_define_class_under(pcp_pmapi_class, "NoPMNSError", pcp_pmapi_error);
    pcp_pmapi_dup_pmns_error = rb_define_class_under(pcp_pmapi_class, "DupPMNSError", pcp_pmapi_error);
    pcp_pmapi_text_error = rb_define_class_under(pcp_pmapi_class, "TextError", pcp_pmapi_error);
    pcp_pmapi_app_version_error = rb_define_class_under(pcp_pmapi_class, "AppVersionError", pcp_pmapi_error);
    pcp_pmapi_value_error = rb_define_class_under(pcp_pmapi_class, "ValueError", pcp_pmapi_error);
    pcp_pmapi_timeout_error = rb_define_class_under(pcp_pmapi_class, "TimeoutError", pcp_pmapi_error);
    pcp_pmapi_no_data_error = rb_define_class_under(pcp_pmapi_class, "NoDataError", pcp_pmapi_error);
    pcp_pmapi_reset_error = rb_define_class_under(pcp_pmapi_class, "ResetError", pcp_pmapi_error);
    pcp_pmapi_name_error = rb_define_class_under(pcp_pmapi_class, "NameError", pcp_pmapi_error);
    pcp_pmapi_pmid_error = rb_define_class_under(pcp_pmapi_class, "PMIDError", pcp_pmapi_error);
    pcp_pmapi_indom_error = rb_define_class_under(pcp_pmapi_class, "InDomError", pcp_pmapi_error);
    pcp_pmapi_inst_error = rb_define_class_under(pcp_pmapi_class, "InstError", pcp_pmapi_error);
    pcp_pmapi_unit_error = rb_define_class_under(pcp_pmapi_class, "UnitError", pcp_pmapi_error);
    pcp_pmapi_conv_error = rb_define_class_under(pcp_pmapi_class, "ConvError", pcp_pmapi_error);
    pcp_pmapi_trunc_error = rb_define_class_under(pcp_pmapi_class, "TruncError", pcp_pmapi_error);
    pcp_pmapi_sign_error = rb_define_class_under(pcp_pmapi_class, "SignError", pcp_pmapi_error);
    pcp_pmapi_profile_error = rb_define_class_under(pcp_pmapi_class, "ProfileError", pcp_pmapi_error);
    pcp_pmapi_ipc_error = rb_define_class_under(pcp_pmapi_class, "IPCError", pcp_pmapi_error);
    pcp_pmapi_eof_error = rb_define_class_under(pcp_pmapi_class, "EOFError", pcp_pmapi_error);
    pcp_pmapi_not_host_error = rb_define_class_under(pcp_pmapi_class, "NotHostError", pcp_pmapi_error);
    pcp_pmapi_eol_error = rb_define_class_under(pcp_pmapi_class, "EOLError", pcp_pmapi_error);
    pcp_pmapi_mode_error = rb_define_class_under(pcp_pmapi_class, "ModeError", pcp_pmapi_error);
    pcp_pmapi_label_error = rb_define_class_under(pcp_pmapi_class, "LabelError", pcp_pmapi_error);
    pcp_pmapi_log_rec_error = rb_define_class_under(pcp_pmapi_class, "LogRecError", pcp_pmapi_error);
    pcp_pmapi_not_archive_error = rb_define_class_under(pcp_pmapi_class, "NotArchiveError", pcp_pmapi_error);
    pcp_pmapi_log_file_error = rb_define_class_under(pcp_pmapi_class, "LogFileError", pcp_pmapi_error);
    pcp_pmapi_no_context_error = rb_define_class_under(pcp_pmapi_class, "NoContextError", pcp_pmapi_error);
    pcp_pmapi_profile_spec_error = rb_define_class_under(pcp_pmapi_class, "ProfileSpecError", pcp_pmapi_error);
    pcp_pmapi_pmid_log_error = rb_define_class_under(pcp_pmapi_class, "PMIDLogError", pcp_pmapi_error);
    pcp_pmapi_indom_log_error = rb_define_class_under(pcp_pmapi_class, "InDomLogError", pcp_pmapi_error);
    pcp_pmapi_inst_log_error = rb_define_class_under(pcp_pmapi_class, "InstLogError", pcp_pmapi_error);
    pcp_pmapi_no_profile_error = rb_define_class_under(pcp_pmapi_class, "NoProfileError", pcp_pmapi_error);
    pcp_pmapi_no_agent_error = rb_define_class_under(pcp_pmapi_class, "NoAgentError", pcp_pmapi_error);
    pcp_pmapi_permission_error = rb_define_class_under(pcp_pmapi_class, "PermissionError", pcp_pmapi_error);
    pcp_pmapi_connlimit_error = rb_define_class_under(pcp_pmapi_class, "ConnLimitError", pcp_pmapi_error);
    pcp_pmapi_again_error = rb_define_class_under(pcp_pmapi_class, "AgainError", pcp_pmapi_error);
    pcp_pmapi_is_conn_error = rb_define_class_under(pcp_pmapi_class, "IsConnError", pcp_pmapi_error);
    pcp_pmapi_not_conn_error = rb_define_class_under(pcp_pmapi_class, "NotConnError", pcp_pmapi_error);
    pcp_pmapi_need_port_error = rb_define_class_under(pcp_pmapi_class, "NeedPortError", pcp_pmapi_error);
    pcp_pmapi_non_leaf_error = rb_define_class_under(pcp_pmapi_class, "NonLeafError", pcp_pmapi_error);
    pcp_pmapi_type_error = rb_define_class_under(pcp_pmapi_class, "TypeError", pcp_pmapi_error);
    pcp_pmapi_thread_error = rb_define_class_under(pcp_pmapi_class, "ThreadError", pcp_pmapi_error);
    pcp_pmapi_no_container_error = rb_define_class_under(pcp_pmapi_class, "NoContainerError", pcp_pmapi_error);
    pcp_pmapi_bad_store_error = rb_define_class_under(pcp_pmapi_class, "BadStoreError", pcp_pmapi_error);
    pcp_pmapi_too_small_error = rb_define_class_under(pcp_pmapi_class, "TooSmallError", pcp_pmapi_error);
    pcp_pmapi_too_big_error = rb_define_class_under(pcp_pmapi_class, "TooBigError", pcp_pmapi_error);
    pcp_pmapi_fault_error = rb_define_class_under(pcp_pmapi_class, "FaultError", pcp_pmapi_error);
    pcp_pmapi_pmda_ready_error = rb_define_class_under(pcp_pmapi_class, "PMDAReadyError", pcp_pmapi_error);
    pcp_pmapi_pmda_not_ready_error = rb_define_class_under(pcp_pmapi_class, "PMDANotReadyError", pcp_pmapi_error);
    pcp_pmapi_nyi_error = rb_define_class_under(pcp_pmapi_class, "NYIError", pcp_pmapi_error);

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

    rb_define_const(pcp_pmapi_class, "PMNS_LEAF_STATUS", INT2NUM(PMNS_LEAF_STATUS));
    rb_define_const(pcp_pmapi_class, "PMNS_NONLEAF_STATUS", INT2NUM(PMNS_NONLEAF_STATUS));

    rb_define_const(pcp_pmapi_class, "PM_ID_NULL", INT2NUM(PM_ID_NULL));

    rb_define_const(pcp_pmapi_class, "PM_INDOM_NULL", INT2NUM(PM_INDOM_NULL));
    rb_define_const(pcp_pmapi_class, "PM_IN_NULL", INT2NUM(PM_IN_NULL));

    rb_define_alloc_func(pcp_pmapi_class, allocate);
    rb_define_private_method(pcp_pmapi_class, "pmNewContext", rb_pmNewContext, 2);
    rb_define_method(pcp_pmapi_class, "pmGetContextHostName", rb_pmGetContextHostName, 0);
    rb_define_method(pcp_pmapi_class, "pmGetPMNSLocation", rb_pmGetPMNSLocation, 0);
    rb_define_method(pcp_pmapi_class, "pmLoadNameSpace", rb_pmLoadNameSpace, 1);
    rb_define_method(pcp_pmapi_class, "pmUnloadNameSpace", rb_pmUnloadNameSpace, 0);
    rb_define_method(pcp_pmapi_class, "pmTrimNameSpace", rb_pmTrimNameSpace, 0);
    rb_define_method(pcp_pmapi_class, "pmLookupName", rb_pmLookupName, 1);
    rb_define_method(pcp_pmapi_class, "pmGetChildren", rb_pmGetChildren, 1);
    rb_define_method(pcp_pmapi_class, "pmGetChildrenStatus", rb_pmGetChildrenStatus, 1);
    rb_define_method(pcp_pmapi_class, "pmNameID", rb_pmNameID, 1);
    rb_define_method(pcp_pmapi_class, "pmNameAll", rb_pmNameAll, 1);
    rb_define_method(pcp_pmapi_class, "pmTraversePMNS", rb_pmTraversePMNS, 1);
    rb_define_method(pcp_pmapi_class, "pmLookupDesc", rb_pmLookupDesc, 1);
    rb_define_method(pcp_pmapi_class, "pmLookupInDom", rb_pmLookupInDom, 2);
    rb_define_method(pcp_pmapi_class, "pmLookupInDomArchive", rb_pmLookupInDomArchive, 2);
    rb_define_method(pcp_pmapi_class, "pmNameInDom", rb_pmNameInDom, 2);
    rb_define_method(pcp_pmapi_class, "pmNameInDomArchive", rb_pmNameInDomArchive, 2);
    rb_define_method(pcp_pmapi_class, "pmGetInDom", rb_pmGetInDom, 1);
    rb_define_method(pcp_pmapi_class, "pmGetInDomArchive", rb_pmGetInDomArchive, 1);
    rb_define_method(pcp_pmapi_class, "pmWhichContext", rb_pmWhichContext, 0);
    rb_define_method(pcp_pmapi_class, "pmDestroyContext", rb_pmDestroyContext, 0);
    rb_define_method(pcp_pmapi_class, "pmDupContext", rb_pmDupContext, 0);
    rb_define_method(pcp_pmapi_class, "pmUseContext", rb_pmUseContext, 0);
    rb_define_method(pcp_pmapi_class, "pmReconnectContext", rb_pmReconnectContext, 0);
    rb_define_method(pcp_pmapi_class, "pmDelProfile", rb_pmDelProfile, 2);
    rb_define_method(pcp_pmapi_class, "pmAddProfile", rb_pmAddProfile, 2);

}






