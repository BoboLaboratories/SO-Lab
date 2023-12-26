make clean
make
cd bin || exit
touch ftok
./master "$@"