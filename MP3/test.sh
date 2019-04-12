

num_start=$1;
num_end=$2;

i=${num_start};

echo "There will be from $1 to $2"

#time
for ((;i<=${num_end};i++));do

  for((j=0;j<${i};j++));do
      nice ./work 200 R 10000&
  done 

  ./monitor >"profile2_${i}"

done
#exit
