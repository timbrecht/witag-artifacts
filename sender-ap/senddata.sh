ifconfig wlan0 mtu 256 up
sleep 3
iperf  -c 192.168.2.1  -u  -b 150M -P 3 -l 20700 -w 4M -t $1
ifconfig wlan0 mtu 1400 up
