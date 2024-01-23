source env/base.sh
make $1
cd bin || exit
./master --inhibitor
