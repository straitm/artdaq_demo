
basedir=$1
toolsdir=$2

    $toolsdir/xt_cmd.sh $basedir --geom '132x33 -sl 2500' \
        -c '. ./setupARTDAQDEMO' \
        -c 'cp -p '$toolsdir'/fcl/TransferInputShmem.fcl '$toolsdir'/fcl/TransferInputShmem2.fcl' \
        -c start2x2x2System.sh
    sleep 2

    $toolsdir/xt_cmd.sh $basedir --geom 132 \
        -c '. ./setupARTDAQDEMO' \
        -c ':,sleep 10' \
        -c 'manage2x2x2System.sh init' \
        -c ':,sleep 5' \
        -c 'manage2x2x2System.sh -N 101 start' \
        -c ':,sleep 60' \
        -c 'manage2x2x2System.sh stop' \
        -c ':,sleep 5' \
        -c 'manage2x2x2System.sh shutdown' \
        -c ': For additional commands, see output from: manage2x2x2System.sh --help' \
        -c ':: manage2x2x2System.sh --help' \
        -c ':: manage2x2x2System.sh exit'

    sleep 14;

    $toolsdir/xt_cmd.sh $basedir --geom '132x33 -sl 2500' \
        -c '. ./setupARTDAQDEMO' \
        -c 'art -c '$toolsdir'/fcl/TransferInputShmem.fcl'

    sleep 4;

    $toolsdir/xt_cmd.sh $basedir --geom '132x33 -sl 2500' \
        -c '. ./setupARTDAQDEMO' \
	-c 'sed -r -i "s/.*modulus.*[0-9]+.*/modulus: 10/" '$toolsdir'/fcl/TransferInputShmem2.fcl' \
	-c 'sed -r -i "/end_paths:/s/a1/a3/" '$toolsdir'/fcl/TransferInputShmem2.fcl' \
	-c 'sed -r -i "/shm_key:/s/.*/shm_key: 0x40471453/" '$toolsdir'/fcl/TransferInputShmem2.fcl' \
	-c 'sed -r -i "s/shmem1/shmem2/" '$toolsdir'/fcl/TransferInputShmem2.fcl' \
        -c 'art -c '$toolsdir'/fcl/TransferInputShmem2.fcl'

