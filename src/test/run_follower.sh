if [ $# -lt 1 ]; then
    echo "usage ./run_follower.sh {nodeid}"
    exit
fi
./server --nodeid $1 -j 9001
