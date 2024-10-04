tcpdump -XXnl -i mon type ctl | grep -A 6 "BA RA:00:22:33:44:aa:a0"  > $1
