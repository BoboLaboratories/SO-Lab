source env/base.sh
make
cd bin || exit
./master --inhibitor
