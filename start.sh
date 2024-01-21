source env/base.sh
make debug
cd bin || exit
./master --inhibitor
