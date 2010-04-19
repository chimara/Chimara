export DISPLAY=:0
export LD_LIBRARY_PATH=.
export scriptdir=`/usr/bin/dirname $0`
cd $scriptdir
cd chimara
export HOME=`pwd`
#./xepdmgr :0 ./iliad ../games/anchor.z8
./iliad ../games/anchor.z8
