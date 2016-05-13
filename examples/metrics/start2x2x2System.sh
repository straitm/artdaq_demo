#!/bin/bash

echo `which setupDemoEnvironment.sh`
source `which setupDemoEnvironment.sh`

# create the configuration file for PMT
tempFile="/tmp/pmtConfig.$$"

echo "BoardReaderMain `hostname` ${ARTDAQDEMO_BR_PORT[0]}" >> $tempFile
echo "BoardReaderMain `hostname` ${ARTDAQDEMO_BR_PORT[1]}" >> $tempFile
echo "EventBuilderMain `hostname` ${ARTDAQDEMO_EB_PORT[0]}" >> $tempFile
echo "EventBuilderMain `hostname` ${ARTDAQDEMO_EB_PORT[1]}" >> $tempFile
echo "AggregatorMain `hostname` ${ARTDAQDEMO_AG_PORT[0]}" >> $tempFile
echo "AggregatorMain `hostname` ${ARTDAQDEMO_AG_PORT[1]}" >> $tempFile

# create the logfile directories, if needed
logroot="/tmp"
mkdir -p -m 0777 ${logroot}/pmt
mkdir -p -m 0777 ${logroot}/masterControl
mkdir -p -m 0777 ${logroot}/boardreader
mkdir -p -m 0777 ${logroot}/eventbuilder
mkdir -p -m 0777 ${logroot}/aggregator

if [[ -n $PRODUCTS ]]; then
    . $PRODUCTS/setup
else
    echo "Unable to find $PRODUCTS/setup, exiting..." >&2
    exit 1
fi

cmd="setup artdaq_mfextensions v1_0_6 -q e9:prof:s21"

$cmd

if [[ "$?" != "0" ]]; then
    echo "Problem executing \"$cmd\", exiting..." >&2
    exit 2
fi

# If present, start the msgviewer dialog
msgviewer -c $ARTDAQ_MFEXTENSIONS_FQ_DIR/bin/msgviewer.fcl 2>&1 >/dev/null &

# start PMT
pmt.rb -p ${ARTDAQDEMO_PMT_PORT} -d $tempFile --logpath ${logroot} --logfhicl "$ARTDAQDEMO_REPO/examples/metrics/fcl/MessageFacility.fcl" --display ${DISPLAY}
rm $tempFile
