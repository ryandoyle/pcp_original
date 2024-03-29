'''
Python implementation of the "simple" Performance Metrics Domain Agent.
'''
#
# Copyright (c) 2013 Red Hat.
# 
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
# 

import os
import time
import cpmapi as c_api

from pcp import pmda
from pcp.pmda import PMDA, pmdaMetric, pmdaIndom, pmdaInstid
from pcp import pmapi
from pcp.pmapi import pmUnits, pmContext as PCP

class SimplePMDA(PMDA):
    '''
    A simple Performance Metrics Domain Agent with very simple metrics.
    Install it and make basic use of it, as follows:

    # $PCP_PMDAS_DIR/simple/Install
    [select python option]
    $ pminfo -fmdtT simple

    Then experiment with the simple.conf file (see simple.now metrics).
    '''

    # all indoms in this PMDA
    color_indom = 0
    now_indom = 1

    # simple.color instance domain
    red = 0
    green = 100
    blue = 200

    # simple.now instance domain
    configfile = ''
    timeslices = {}

    # simple.numfetch properties
    numfetch = 0
    oldfetch = -1


    def simple_instance(self):
        ''' Called once per "instance request" PDU '''
        self.simple_timenow_check()


    def simple_fetch(self):
        ''' Called once per "fetch" PDU, before callbacks '''
        self.numfetch += 1
        self.simple_timenow_check()

    def simple_fetch_color_callback(self, item, inst):
        '''
        Returns a list of value,status (single pair) for color cluster
        Helper for the fetch callback
        '''
        if (item == 0):
            return [self.numfetch, 1]
        elif (item == 1):
            if (inst == 0):
                self.red = (self.red + 1) % 255
                return [self.red, 1]
            elif (inst == 1):
                self.green = (self.green + 1) % 255
                return [self.green, 1]
            elif (inst == 2):
                self.blue = (self.blue + 1) % 255
                return [self.blue, 1]
            else:
                return [c_api.PM_ERR_INST, 0]
        else:
            return [c_api.PM_ERR_PMID, 0]

    def simple_fetch_times_callback(self, item):
        '''
        Returns a list of value,status (single pair) for times cluster
        Helper for the fetch callback
        '''
        times = ()
        if (self.oldfetch < self.numfetch):     # get current values, if needed
            times = os.times()
            self.oldfetch = self.numfetch
        if (item == 2):
            return [times[0], 1]
        elif (item == 3):
            return [times[1], 1]
        return [c_api.PM_ERR_PMID, 0]

    def simple_fetch_now_callback(self, item, inst):
        '''
        Returns a list of value,status (single pair) for "now" cluster
        Helper for the fetch callback
        '''
        if (item == 4):
            value = self.pmda_inst_lookup(self.now_indom, inst)
            if (value == None):
                return [c_api.PM_ERR_INST, 0]
            return [value, 1]
        return [c_api.PM_ERR_PMID, 0]

    def simple_fetch_callback(self, cluster, item, inst):
        '''
        Main fetch callback, defers to helpers for each cluster.
        Returns a list of value,status (single pair) for requested pmid/inst
        '''
        if (not (inst == c_api.PM_IN_NULL or
            (cluster == 0 and item == 1) or (cluster == 2 and item == 4))):
            return [c_api.PM_ERR_INST, 0]
        if (cluster == 0):
            return self.simple_fetch_color_callback(item, inst)
        elif (cluster == 1):
            return self.simple_fetch_times_callback(item)
        elif (cluster == 2):
            return self.simple_fetch_now_callback(item, inst)
        return [c_api.PM_ERR_PMID, 0]


    def simple_store_count_callback(self, val):
        ''' Helper for the store callback, handles simple.numfetch '''
        sts = 0
        if (val < 0):
            sts = c_api.PM_ERR_SIGN
            val = 0
        self.numfetch = val
        return sts

    def simple_store_color_callback(self, val, inst):
        ''' Helper for the store callback, handles simple.color '''
        sts = 0
        if (val < 0):
            sts = c_api.PM_ERR_SIGN
            val = 0
        elif (val > 255):
            sts = c_api.PM_ERR_CONV
            val = 255

        if (inst == 0):
            self.red = val
        elif (inst == 1):
            self.green = val
        elif (inst == 2):
            self.blue = val
        else:
            sts = c_api.PM_ERR_INST
        return sts

    def simple_store_callback(self, cluster, item, inst, val):
        '''
        Store callback, executed when a request to write to a metric happens
        Defers to helpers for each storable metric.  Returns a single value.
        '''
        if (cluster == 0):
            if (item == 0):
                return self.simple_store_count_callback(val)
            elif (item == 1):
                return self.simple_store_color_callback(val, inst)
            else:
                return c_api.PM_ERR_PMID
        elif ((cluster == 1 and (item == 2 or item == 3))
            or (cluster == 2 and item == 4)):
            return c_api.PM_ERR_PERMISSION
        return c_api.PM_ERR_PMID


    def simple_timenow_check(self):
        '''
        Read our configuration file and update instance domain
        '''
        self.timeslices.clear()
        with open(self.configfile) as cfg:
            fields = time.localtime()
            values = {'sec': fields[5], 'min': fields[4], 'hour': fields[3]}
            config = cfg.readline().strip()
            for key in config.split(','):
                if (values[key] != None):
                    self.timeslices[key] = values[key]
        self.replace_indom(self.now_indom, self.timeslices)


    def __init__(self, name, domain):
        PMDA.__init__(self, name, domain)

        self.configfile = PCP.pmGetConfig('PCP_PMDAS_DIR')
        self.configfile += '/' + name + '/' + name + '.conf'

        self.add_metric(name + '.numfetch', pmdaMetric(self.pmid(0, 0),
                c_api.PM_TYPE_U32, c_api.PM_INDOM_NULL, c_api.PM_SEM_INSTANT,
                pmUnits(0, 0, 0, 0, 0, 0)))
        self.add_metric(name + '.color', pmdaMetric(self.pmid(0, 1),
                c_api.PM_TYPE_32, self.color_indom, c_api.PM_SEM_INSTANT,
                pmUnits(0, 0, 0, 0, 0, 0)))
        self.add_metric(name + '.time.user', pmdaMetric(self.pmid(1, 2),
                c_api.PM_TYPE_DOUBLE, c_api.PM_INDOM_NULL, c_api.PM_SEM_COUNTER,
                pmUnits(0, 1, 0, 0, c_api.PM_TIME_SEC, 0)))
        self.add_metric(name + '.time.sys', pmdaMetric(self.pmid(1, 3),
                c_api.PM_TYPE_DOUBLE, c_api.PM_INDOM_NULL, c_api.PM_SEM_COUNTER,
                pmUnits(0, 1, 0, 0, c_api.PM_TIME_SEC, 0)))
        self.add_metric(name + '.now', pmdaMetric(self.pmid(2, 4),
                c_api.PM_TYPE_U32, self.now_indom, c_api.PM_SEM_INSTANT,
                pmUnits(0, 0, 0, 0, 0, 0)))

        colors = [pmdaInstid(0, 'red'),
                  pmdaInstid(1, 'green'),
                  pmdaInstid(2, 'blue')]
        self.add_indom(pmdaIndom(self.color_indom, colors))
        self.add_indom(pmdaIndom(self.now_indom, None))

        self.set_fetch(self.simple_fetch)
        self.set_instance(self.simple_instance)
        self.set_fetch_callback(self.simple_fetch_callback)
        self.set_store_callback(self.simple_store_callback)
        self.set_user(PCP.pmGetConfig('PCP_USER'))
        self.simple_timenow_check()


if __name__ == '__main__':

    SimplePMDA('simple', 253).run()

