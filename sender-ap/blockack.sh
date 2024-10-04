#tcpdump -xx -i mon-2400mhz wlan[0] == 0x94 | grep 0x
#tcpdump -XXnl -i mon "(wlan[0] == 0x94) and (wlan[1] == 0x00)"  | grep 0x
tcpdump -XXnl -i mon-2400mhz "(wlan[0] == 0x94) and (wlan[1] == 0x00)" 
