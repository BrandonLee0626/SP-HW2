git pull

make clean
make

./server&

./client& -ip "10.8.131.119" -port "5000" -username "Alice"
./client& -ip "10.8.131.119" -port "5000" -username "Bob"
