#!/bin/bash

# JCF, Oct-5-2017
# This script basically follows the instructions found in https://cdcvs.fnal.gov/redmine/projects/artdaq-utilities/wiki/Artdaq-daqinterface

if [ $# -lt 2 ];then
 echo "USAGE: $0 base_directory tools_directory"
 exit
fi
basedir=$1
toolsdir=$2

cd $basedir


daqintdir=$basedir/DAQInterface

git clone http://cdcvs.fnal.gov/projects/artdaq-utilities-daqinterface
cd artdaq-utilities-daqinterface
git checkout cc55ad50456f1cd2a6f1ce8e6c22b627a275ac24

mkdir $daqintdir
cd $daqintdir
cp ../artdaq-utilities-daqinterface/bin/mock_ups_setup.sh .
cp ../artdaq-utilities-daqinterface/docs/user_sourcefile_example .
cp ../artdaq-utilities-daqinterface/docs/settings_example .
cp ../artdaq-utilities-daqinterface/docs/known_boardreaders_list_example .
cp ../artdaq-utilities-daqinterface/docs/boot.txt .

sed -i -r 's!^\s*export DAQINTERFACE_DIR.*!export DAQINTERFACE_DIR='$basedir/artdaq-utilities-daqinterface'!' mock_ups_setup.sh
sed -i -r 's!^\s*export DAQINTERFACE_SETTINGS.*!export DAQINTERFACE_SETTINGS='$PWD/settings_example'!' user_sourcefile_example


# Figure out which products directory contains the xmlrpc package (for
# sending commands to DAQInterface) and set it in the settings file

productsdir=$( ups active | grep xmlrpc | awk '{print $NF}' )

if [[ -z $productsdir ]]; then
    echo "Unable to determine the products directory containing xmlrpc; will return..." >&2
    return 41
fi

sed -i -r 's!^\s*productsdir_for_bash_scripts:.*!productsdir_for_bash_scripts: '$productsdir'!' settings_example

mkdir -p $basedir/run_records

# Set the run records directory in the .settings file

sed -i -r 's!^\s*record_directory.*!record_directory: '$basedir/run_records'!' settings_example

# Set the artdaq-demo setup script whose creation was part of the artdaq-demo installation

sed -i -r 's!^\s*DAQ setup script:.*!DAQ setup script: '$basedir'/setupARTDAQDEMO!' boot.txt


# And now, actually run DAQInterface as described in
# https://cdcvs.fnal.gov/redmine/projects/artdaq-utilities/wiki/Artdaq-daqinterface

    $toolsdir/xt_cmd.sh $daqintdir --geom '132x33 -sl 2500' \
        -c 'source mock_ups_setup.sh' \
	-c 'export DAQINTERFACE_USER_SOURCEFILE=$PWD/user_sourcefile_example' \
	-c 'source $DAQINTERFACE_DIR/source_me' \
	-c 'DAQInterface'
    sleep 2

    $toolsdir/xt_cmd.sh $daqintdir --geom 132 \
        -c 'source mock_ups_setup.sh' \
	-c 'export DAQINTERFACE_USER_SOURCEFILE=$PWD/user_sourcefile_example' \
	-c 'source $DAQINTERFACE_DIR/source_me' \
	-c 'just_do_it.sh $PWD/boot.txt 20'

     sleep 14;

    xrdbproc=$( which xrdb )

    xloc=
    if [[ -e $xrdbproc ]]; then
    	xloc=$( xrdb -symbols | grep DWIDTH | awk 'BEGIN {FS="="} {pixels = $NF; print pixels/2}' )
    else
    	xloc=800
    fi

    $toolsdir/xt_cmd.sh $basedir --geom '100x33+'$xloc'+0 -sl 2500' \
        -c '. ./setupARTDAQDEMO' \
        -c 'art -c '$toolsdir'/fcl/TransferInputShmem.fcl'

    sleep 4;

    $toolsdir/xt_cmd.sh $basedir --geom '100x33+0+0 -sl 2500' \
        -c '. ./setupARTDAQDEMO' \
    	-c 'rm -f '$toolsdir'/fcl/TransferInputShmem2.fcl' \
        -c 'cp -p '$toolsdir'/fcl/TransferInputShmem.fcl '$toolsdir'/fcl/TransferInputShmem2.fcl' \
    	-c 'sed -r -i "s/.*modulus.*[0-9]+.*/modulus: 100/" '$toolsdir'/fcl/TransferInputShmem2.fcl' \
    	-c 'sed -r -i "/end_paths:/s/a3/a1/" '$toolsdir'/fcl/TransferInputShmem2.fcl' \
    	-c 'sed -r -i "/shm_key:/s/.*/shm_key: 0x40471453/" '$toolsdir'/fcl/TransferInputShmem2.fcl' \
    	-c 'sed -r -i "s/shmem1/shmem2/" '$toolsdir'/fcl/TransferInputShmem2.fcl' \
		-c 'sed -r -i "s/destination_rank: 6/destination_rank: 7/" '$toolsdir'/fcl/TransferInputShmem2.fcl' \
        -c 'art -c '$toolsdir'/fcl/TransferInputShmem2.fcl'

