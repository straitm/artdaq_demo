
if [ -z "${ARTDAQDEMO_REPO:-}" ]; then
  echo "Please set ARTDAQDEMO_REPO to the artdaq-demo directory (containing tools/ and examples/)"
  exit 1
fi

basedir=$MRB_SOURCE/..
toolsdir=$ARTDAQDEMO_REPO/tools

    $toolsdir/xt_cmd.sh $basedir --geom '132x33 -sl 2500' \
        -c '. ./setupARTDAQDEMO' \
        -c $ARTDAQDEMO_REPO/examples/asciiSimulator/startAsciiSimulatorExample.sh
    sleep 2

    $toolsdir/xt_cmd.sh $basedir --geom 132 \
        -c '. ./setupARTDAQDEMO' \
        -c ':,sleep 10' \
        -c ' $ARTDAQDEMO_REPO/examples/asciiSimulator/manageAsciiSimulatorExample.sh init' \
        -c ':,sleep 5' \
        -c ' $ARTDAQDEMO_REPO/examples/asciiSimulator/manageAsciiSimulatorExample.sh -N 101 start' \
        -c ':,sleep 60' \
        -c ' $ARTDAQDEMO_REPO/examples/asciiSimulator/manageAsciiSimulatorExample.sh stop' \
        -c ':,sleep 5' \
        -c 'm $ARTDAQDEMO_REPO/examples/asciiSimulator/anage2x2x2System.sh shutdown' \
        -c ': For additional commands, see output from: manageAsciiSimulatorExample.sh --help' \
        -c ':: manageAsciiSimulatorExample.sh --help' \
        -c ':: manageAsciiSimulatorExample.sh exit'
