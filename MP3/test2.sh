

num_it=$1;

echo "There will be $1"

for((i=0;i<${num_it};i++));do
    nice ./work 200 R 10000&
done

