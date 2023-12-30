#!/bin/bash

echo "connect \"CacheStore: tcp -h 10.15.10.74 -p 10700\"" > test.txt
echo "open volume $" >> test.txt

for ((i=20; i<50; i++))
do
echo "open CDNTEST12345678910${i}xor.com true" >> test.txt
echo "set startTime=+0" >> test.txt
echo "set endTime=+600" >> test.txt
echo "set bitrate=3750000" >> test.txt
echo "provision ftp://aa:aa@192.168.81.97/testrest" >> test.txt
echo "up" >> test.txt
echo "up" >> test.txt
echo "up" >> test.txt
echo "sleep 10000" >> test.txt
done

#for ((i=10; i<19; i++))
#do
#echo "open cdntest12345678910${i}xor.com true" >> test.txt
#echo "set startTime=+0" >> test.txt
#echo "set endTime=+600" >> test.txt
#echo "set bitrate=3750000" >> test.txt
#echo "provision ftp://wm:itv@192.168.81.106/Good" >> test.txt
#echo "up" >> test.txt
#done

echo "quit" >> test.txt


