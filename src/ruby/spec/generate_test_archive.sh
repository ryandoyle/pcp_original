#!/usr/bin/env bash
set -e

SPEC_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PMLOGGER_CONFIG=$SPEC_ROOT/logger.config
SECONDS_TO_LOG=30
PMLOGGER_ARCHIVE=$SPEC_ROOT/test_archive

echo "Starting pmlogger for $SECONDS_TO_LOG seconds writing to $PMLOGGER_ARCHIVE"
rm -f ${PMLOGGER_ARCHIVE}.*
pmlogger --host=localhost --interval=1 --config=$PMLOGGER_CONFIG -T ${SECONDS_TO_LOG}secs $PMLOGGER_ARCHIVE