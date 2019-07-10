build_flags=( "BUILD_UNSAFE" "BUILD_MUTEX" "BUILD_ATOMIC" "BUILD_FLAGGED" )
loop_counts=( 100000 500000 1000000 5000000 10000000 50000000 100000000 500000000 )

TIMEFORMAT="%E,%U,%S"

echo 'build,count,actual,user,sys' > testout.csv

for macro in "${build_flags[@]}"
do
    for loops in "${loop_counts[@]}"
    do
        gcc -D${macro} sync_wo.c -lpthread -o tester
        test_time=$( { time ./tester ${loops} > /dev/null; } 2>&1 )
        echo $macro,$loops,$test_time
        echo $macro,$loops,$test_time >> testout.csv
    done
done


