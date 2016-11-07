#! /bin/bash
# quick-mrb-start.sh - Eric Flumerfelt, May 20, 2016
# Downloads, installs, and runs the artdaq_demo as an MRB-controlled repository

git_status=`git status 2>/dev/null`
git_sts=$?
if [ $git_sts -eq 0 ];then
    echo "This script is designed to be run in a fresh install directory!"
    exit 1
fi


starttime=`date`
Base=$PWD
test -d products || mkdir products
test -d download || mkdir download
test -d log || mkdir log

env_opts_var=`basename $0 | sed 's/\.sh$//' | tr 'a-z-' 'A-Z_'`_OPTS
USAGE="\
   usage: `basename $0` [options] [demo_root]
examples: `basename $0` .
          `basename $0` --run-demo
          `basename $0` --debug
          `basename $0` --tag v2_08_01
If the \"demo_root\" optional parameter is not supplied, the user will be
prompted for this location.
--run-demo    runs the demo
--debug       perform a debug build
--viewer      install and run the artdaq Message Viewer
--tag         Install a specific tag of artdaq_demo
-e, -s        Use specific qualifiers when building ARTDAQ (only e9:s31, e9:s21, e7:s15 supported)
-v            Be more verbose
-x            set -x this script
-w            Check out repositories read/write
"

# Process script arguments and options
eval env_opts=\${$env_opts_var-} # can be args too
eval "set -- $env_opts \"\$@\""
op1chr='rest=`expr "$op" : "[^-]\(.*\)"`   && set -- "-$rest" "$@"'
op1arg='rest=`expr "$op" : "[^-]\(.*\)"`   && set --  "$rest" "$@"'
reqarg="$op1arg;"'test -z "${1+1}" &&echo opt -$op requires arg. &&echo "$USAGE" &&exit'
args= do_help= opt_v=0; opt_w=0
while [ -n "${1-}" ];do
    if expr "x${1-}" : 'x-' >/dev/null;then
        op=`expr "x$1" : 'x-\(.*\)'`; shift   # done with $1
        leq=`expr "x$op" : 'x-[^=]*\(=\)'` lev=`expr "x$op" : 'x-[^=]*=\(.*\)'`
        test -n "$leq"&&eval "set -- \"\$lev\" \"\$@\""&&op=`expr "x$op" : 'x\([^=]*\)'`
        case "$op" in
            \?*|h*)     eval $op1chr; do_help=1;;
            v*)         eval $op1chr; opt_v=`expr $opt_v + 1`;;
            x*)         eval $op1chr; set -x;;
            s*)         eval $op1arg; squalifier=$1; shift;;
            e*)         eval $op1arg; equalifier=$1; shift;;
			w*)         eval $op1chr; opt_w=`expr $opt_w + 1`;;
            -run-demo)  opt_run_demo=--run-demo;;
	    -debug)     opt_debug=--debug;;
			-tag)       eval $reqarg; tag=$1; shift;;
	    -viewer)    opt_viewer=--viewer;;
            *)          echo "Unknown option -$op"; do_help=1;;
        esac
    else
        aa=`echo "$1" | sed -e"s/'/'\"'\"'/g"` args="$args '$aa'"; shift
    fi
done
eval "set -- $args \"\$@\""; unset args aa
set -u   # complain about uninitialed shell variables - helps development

test -n "${do_help-}" -o $# -ge 2 && echo "$USAGE" && exit

# JCF, 1/16/15
# Save all output from this script (stdout + stderr) in a file with a
# name that looks like "quick-start.sh_Fri_Jan_16_13:58:27.script" as
# well as all stderr in a file with a name that looks like
# "quick-start.sh_Fri_Jan_16_13:58:27_stderr.script"
alloutput_file=$( date | awk -v "SCRIPTNAME=$(basename $0)" '{print SCRIPTNAME"_"$1"_"$2"_"$3"_"$4".script"}' )
stderr_file=$( date | awk -v "SCRIPTNAME=$(basename $0)" '{print SCRIPTNAME"_"$1"_"$2"_"$3"_"$4"_stderr.script"}' )
exec  > >(tee "$Base/log/$alloutput_file")
exec 2> >(tee "$Base/log/$stderr_file")

function detectAndPull() {
    local startDir=$PWD
    cd $Base/download
    local packageName=$1
    local packageOs=$2

    if [ $# -gt 2 ];then
	local qualifiers=$3
    fi
    if [ $# -gt 3 ];then
	local packageVersion=$4
    else
	local packageVersion=`curl http://scisoft.fnal.gov/scisoft/packages/${packageName}/ 2>/dev/null|grep ${packageName}|grep "id=\"v"|tail -1|sed 's/.* id="\(v.*\)".*/\1/'`
    fi
    local packageDotVersion=`echo $packageVersion|sed 's/_/\./g'|sed 's/v//'`

    if [[ "$packageOs" != "noarch" ]]; then
        local upsflavor=`ups flavor`
	local packageQualifiers="-`echo $qualifiers|sed 's/:/-/g'`"
	local packageUPSString="-f $upsflavor -q$qualifiers"
    fi
    local packageInstalled=`ups list -aK+ $packageName $packageVersion ${packageUPSString-}|grep -c "$packageName"`
    if [ $packageInstalled -eq 0 ]; then
	local packagePath="$packageName/$packageVersion/$packageName-$packageDotVersion-${packageOs}${packageQualifiers-}.tar.bz2"
	wget http://scisoft.fnal.gov/scisoft/packages/$packagePath >/dev/null 2>&1
	local packageFile=$( echo $packagePath | awk 'BEGIN { FS="/" } { print $NF }' )

	if [[ ! -e $packageFile ]]; then
	    echo "Unable to download $packageName"
	    exit 1
	fi

	local returndir=$PWD
	cd $Base/products
	tar -xjf $Base/download/$packageFile
	cd $returndir
    fi
    cd $startDir
}

cd $Base/download

echo "Cloning cetpkgsupport to determine current OS"
git clone http://cdcvs.fnal.gov/projects/cetpkgsupport
os=`./cetpkgsupport/bin/get-directory-name os`

if [[ "$os" == "u14" ]]; then
	echo "-H Linux64bit+3.19-2.19" >../products/ups_OVERRIDE.`hostname`
fi

# Get all the information we'll need to decide which exact flavor of the software to install
if [ -z "${tag:-}" ]; then tag=develop;fi
wget https://cdcvs.fnal.gov/redmine/projects/artdaq-demo/repository/revisions/$tag/raw/ups/product_deps
demo_version=`grep "parent artdaq_demo" $Base/download/product_deps|awk '{print $3}'`
artdaq_version=`grep "^artdaq " $Base/download/product_deps | awk '{print $2}'`
coredemo_version=`grep "^artdaq_core_demo " $Base/download/product_deps | awk '{print $2}'`
defaultQuals=`grep "defaultqual" $Base/download/product_deps|awk '{print $2}'`
defaultE=`echo $defaultQuals|cut -f1 -d:`
defaultS=`echo $defaultQuals|cut -f2 -d:`
if [ -n "${equalifier-}" ]; then 
    equalifier="e${equalifier}";
else
    equalifier=$defaultE
fi
if [ -n "${squalifier-}" ]; then
    squalifier="s${squalifier}"
else
    squalifier=$defaultS
fi
if [[ -n "${opt_debug:-}" ]] ; then
    build_type="debug"
else
    build_type="prof"
fi

wget http://scisoft.fnal.gov/scisoft/bundles/tools/pullProducts
chmod +x pullProducts
./pullProducts $Base/products ${os} artdaq-${artdaq_version} ${squalifier}-${equalifier} ${build_type}
    if [ $? -ne 0 ]; then
	echo "Error in pullProducts. Please go to http://scisoft.fnal.gov/scisoft/bundles/artdaq/${artdaq_version}/manifest and make sure that a manifest for the specified qualifiers ($defaultqualWithS) exists."
	exit 1
    fi
detectAndPull mrb noarch
source $Base/products/setup
setup mrb
setup git
setup gitflow

export MRB_PROJECT=artdaq_demo
cd $Base
mrb newDev -f -v $demo_version -q ${equalifier}:${squalifier}:${build_type}
set +u
source $Base/localProducts_artdaq_demo_${demo_version}_${equalifier}_${squalifier}_${build_type}/setup
set -u

cd $MRB_SOURCE
if [[ "$tag" == "develop" ]]; then
if [ $opt_w -gt 0 ];then
mrb gitCheckout -d artdaq_core ssh://p-artdaq@cdcvs.fnal.gov/cvs/projects/artdaq-core
mrb gitCheckout -d artdaq_utilities ssh://p-artdaq-utilities@cdcvs.fnal.gov/cvs/projects/artdaq-utilities
mrb gitCheckout ssh://p-artdaq@cdcvs.fnal.gov/cvs/projects/artdaq
mrb gitCheckout -d artdaq_core_demo ssh://p-artdaq-core-demo@cdcvs.fnal.gov/cvs/projects/artdaq-core-demo
mrb gitCheckout -d artdaq_demo ssh://p-artdaq-demo@cdcvs.fnal.gov/cvs/projects/artdaq-demo
else
mrb gitCheckout -d artdaq_core http://cdcvs.fnal.gov/projects/artdaq-core
mrb gitCheckout -d artdaq_utilities http://cdcvs.fnal.gov/projects/artdaq-utilities
mrb gitCheckout http://cdcvs.fnal.gov/projects/artdaq
mrb gitCheckout -d artdaq_core_demo http://cdcvs.fnal.gov/projects/artdaq-core-demo
mrb gitCheckout -d artdaq_demo http://cdcvs.fnal.gov/projects/artdaq-demo
fi
else
if [ $opt_w -gt 0 ];then
mrb gitCheckout -t ${artdaq_version} ssh://p-artdaq@cdcvs.fnal.gov/cvs/projects/artdaq
mrb gitCheckout -t ${coredemo_version} -d artdaq_core_demo ssh://p-artdaq-core-demo@cdcvs.fnal.gov/cvs/projects/artdaq-core-demo
mrb gitCheckout -t ${demo_version} -d artdaq_demo ssh://p-artdaq-demo@cdcvs.fnal.gov/cvs/projects/artdaq-demo
else
mrb gitCheckout -t ${artdaq_version} http://cdcvs.fnal.gov/projects/artdaq
mrb gitCheckout -t ${coredemo_version} -d artdaq_core_demo http://cdcvs.fnal.gov/projects/artdaq-core-demo
mrb gitCheckout -t ${demo_version} -d artdaq_demo http://cdcvs.fnal.gov/projects/artdaq-demo
fi
fi

if [[ "x${opt_viewer-}" != "x" ]]; then
    os=`$Base/download/cetpkgsupport/bin/get-directory-name os`
    detectAndPull qt ${os}-x86-64 ${equalifier} v5_4_2a
    cd $MRB_SOURCE
    mrb gitCheckout -d artdaq_mfextensions http://cdcvs.fnal.gov/projects/mf-extensions-git
fi

ARTDAQ_DEMO_DIR=$Base/srcs/artdaq_demo
cd $Base
    cat >setupARTDAQDEMO <<-EOF
	echo # This script is intended to be sourced.

	sh -c "[ \`ps \$\$ | grep bash | wc -l\` -gt 0 ] || { echo 'Please switch to the bash shell before running the artdaq-demo.'; exit; }" || exit

	source $Base/products/setup
        setup mrb
        source $Base/localProducts_artdaq_demo_${demo_version}_${equalifier}_${squalifier}_${build_type}/setup
        source mrbSetEnv

    export ARTDAQDEMO_REPO=$ARTDAQ_DEMO_DIR
    export ARTDAQDEMO_BUILD=$MRB_BUILDDIR/artdaq_demo
	#export ARTDAQDEMO_BASE_PORT=52200
	export DAQ_INDATA_PATH=$ARTDAQ_DEMO_DIR/test/Generators:$ARTDAQ_DEMO_DIR/inputData

	export FHICL_FILE_PATH=.:\$ARTDAQ_DEMO_DIR/tools/snippets:\$ARTDAQ_DEMO_DIR/tools/fcl:\$FHICL_FILE_PATH

	alias toy1toy2EventDump="art -c $ARTDAQ_DEMO_DIR/artdaq-demo/ArtModules/fcl/toy1toy2Dump.fcl"
	alias rawEventDump="art -c $ARTDAQ_DEMO_DIR/artdaq-demo/ArtModules/fcl/rawEventDump.fcl"
	alias compressedEventDump="art -c $ARTDAQ_DEMO_DIR/artdaq-demo/ArtModules/fcl/compressedEventDump.fcl"
	alias compressedEventComparison="art -c $ARTDAQ_DEMO_DIR/artdaq-demo/ArtModules/fcl/compressedEventComparison.fcl"
	EOF
    #

# Build artdaq_demo
cd $MRB_BUILDDIR
set +u
source mrbSetEnv
set -u
export CETPKG_J=$((`cat /proc/cpuinfo|grep processor|tail -1|awk '{print $3}'` + 1))
mrb build    # VERBOSE=1
installStatus=$?

if [ $installStatus -eq 0 ] && [ "x${opt_run_demo-}" != "x" ]; then
    echo doing the demo

    toolsdir=${ARTDAQ_DEMO_DIR}/tools

    . $toolsdir/run_demo.sh $Base $toolsdir

elif [ $installStatus -eq 0 ]; then
    echo "artdaq-demo has been installed correctly. Please see: "
    echo "https://cdcvs.fnal.gov/redmine/projects/artdaq-demo/wiki/Running_a_sample_artdaq-demo_system"
    echo "for instructions on how to run, or re-run this script with the --run-demo option"
    echo
else
    echo "BUILD ERROR!!! SOMETHING IS VERY WRONG!!!"
    echo
fi

endtime=`date`

echo "Build start time: $starttime"
echo "Build end time:   $endtime"

