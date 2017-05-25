#!/bin/bash

# JCF, May-25-2017

# This is an initial attempt at getting run_demo to use DAQInterface
# rather than the traditional start / manage scripts. Note that since
# there's a certain amount of decoupling between artdaq v2_02_03 and
# DAQInterface v1_00_02, some tweaks to the DAQInterface code via sed
# are performed below. Some of these tweaks can and should be
# eliminated in future artdaq / DAQInterface releases.

if [ $# -lt 2 ];then
 echo "USAGE: $0 base_directory tools_directory"
 exit
fi
basedir=$1
toolsdir=$2

cd $basedir

# If we don't already have the DAQInterface git repo cloned into the
# base directory and checked out to the version number corresponding
# to the artdaq-demo instllation, do so...

if [[ -d $basedir/artdaq-utilities-daqinterface ]]; then
    cd $basedir/artdaq-utilities-daqinterface
else
    
    if [[ -e $basedir/products/artdaq_daqinterface ]]; then
	daqinterface_version=$( ls -1 $basedir/products/artdaq_daqinterface | grep "^v[0-9]_[0-9][0-9]_[0-9][0-9]$"  )

	if [[ -z $daqinterface_version ]]; then
	    echo "Unable to determine the DAQInterface version from looking in $basedir/products/artdaq_daqinterface; will exit..." >&2
	    return 10
	fi

	git clone http://cdcvs.fnal.gov/projects/artdaq-utilities-daqinterface
	
	if [[ "$?" != "0" ]]; then
	    echo "Problem attempting to clone DAQInterface; will return..." >&2
	    return 20
	fi

	cd ./artdaq-utilities-daqinterface
	git checkout ${daqinterface_version}
	
	if [[ "$?" != "0" ]]; then
	    echo "Problem trying to check out DAQInterface version ${daqinterface_version} in directory $PWD ; will return..." >&2
	    return 30
	fi

    else
	echo "Unable to determine version of DAQInterface as there appears to be no $basedir/products/artdaq_daqinterface directory; will return..." >&2
	return 40
    fi

fi

# JCF, May-25-2017

# Now automate the edits described to new DAQInterface users in
# https://cdcvs.fnal.gov/redmine/projects/artdaq-utilities/wiki/Artdaq-daqinterface

mkdir -p $basedir/run_records

# Set the run records directory in the .settings file

sed -i -r 's!^\s*record_directory.*!record_directory: '$basedir/run_records'!' .settings


# Figure out which products directory contains the xmlrpc package (for
# sending commands to DAQInterface) and set it in the .settings
# file. Notice the setup of artdaq-demo is done in a subshell so as
# not to affect this environment

returndir=$PWD
cd $basedir
productsdir=$( ( . setupARTDAQDEMO >&/dev/null; ups active | grep xmlrpc | awk '{print $NF}' ) )
cd $returndir

sed -i -r 's!^\s*productsdir_for_bash_scripts.*!productsdir_for_bash_scripts: '$productsdir'!' .settings

if [[ ! -e ./docs/config.txt ]]; then
    echo "Unable to find the DAQInterface configuration file ./docs/config.txt; will return..." >&2
    return 50
fi

# Set the artdaq-demo installation location described in the
# DAQInterface configuration file to the one this script is running
# out of

sed -i -r 's!^\s*DAQ\s*directory\s*:.*!DAQ directory: '$basedir'!' ./docs/config.txt

# Set the configuration used in run_demo to "multiple_dispatchers"

sed -i -r 's/^(\s*)config=.*/config="multiple_dispatchers"/' ./bin/just_do_it.sh


# Hacky: comment out a crosscheck performed in just_do_it.sh since it
# may fail due to overly stringent assumptions (the performing of the
# crosscheck, that is, not the crosscheck itself) as of DAQInterface
# v1_00_02

sed -i -r 's/^(\s*check_event_count\s*)$/#\1/' ./bin/just_do_it.sh  

# Hacky-er: update the bookkeeping function used by DAQInterface
# v1_00_02 so it works with artdaq v2_02_03 and not prior artdaq
# versions

sed -i -r 's/bookkeeping_for_fhicl_documents_artdaq_v2/bookkeeping_for_fhicl_documents_artdaq_v3/' ./rc/control/daqinterface.py

# Finally, in the DAQInterface configuration file, adjust the port #'s
# so that they agree between DAQInterface v1_00_02 and the online
# monitoring FHiCL documents in artdaq v2_02_03

sed -i -r 's/54([0-9][0-9])/52\1/' ./docs/config.txt

# And now, actually run DAQInterface as described in
# https://cdcvs.fnal.gov/redmine/projects/artdaq-utilities/wiki/Artdaq-daqinterface

    $toolsdir/xt_cmd.sh $basedir --geom '132x33 -sl 2500' \
        -c 'cd ./artdaq-utilities-daqinterface' \
        -c 'source source_me' \
	-c 'DAQInterface'
    sleep 2

    $toolsdir/xt_cmd.sh $basedir --geom 132 \
	-c 'cd ./artdaq-utilities-daqinterface' \
        -c 'source source_me' \
	-c 'just_do_it.sh 20'

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
        -c 'art -c '$toolsdir'/fcl/TransferInputShmem2.fcl'

