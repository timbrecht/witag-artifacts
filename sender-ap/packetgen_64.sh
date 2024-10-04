#!/bin/bash
# pktgen.conf -- Sample configuration for send on two devices on a UP system

/sbin/modprobe -r pktgen

/sbin/modprobe pktgen

if [[ $1 == "" ]];then
   pktsize=256
else
   pktsize=$1
fi

if [[ $2 == "" ]];then
   pktCount=0
else
   pktCount=$2
fi

function pgset() {
    local result

    echo $1 > $PGDEV

    result=`cat $PGDEV | fgrep "Result: OK:"`
    if [ "$result" = "" ]; then
         cat $PGDEV | fgrep Result:
    fi
}

function pg() {
    echo inject > $PGDEV
    cat $PGDEV
}

# On UP systems only one thread exists -- so just add devices
# We use eth1, eth1
	#echo "Adding devices to run".

PGDEV=/proc/net/pktgen/kpktgend_0
pgset "rem_device_all"
pgset "add_device wlan0@0"
#pgset "max_before_softirq 1"

# Configure the individual devices
# echo "Configuring devices"

PGDEV=/proc/net/pktgen/wlan0@0

pgset "clone_skb 0"
pgset "pkt_size $pktsize"
pgset "src_mac 00:22:33:44:AA:A0"
pgset "flag IPSRC_RND"
pgset "src_min 192.168.3.127"
pgset "src_max 192.168.3.127"
pgset "dst 192.168.3.1"
pgset "dst_mac  00:22:33:44:AA:90"
pgset "count $pktCount"
pgset "flag NO_TIMESTAMP"


PGDEV=/proc/net/pktgen/kpktgend_1
pgset "rem_device_all"
pgset "add_device wlan0@1"
#pgset "max_before_softirq 1"

# Configure the individual devices
# echo "Configuring devices"

PGDEV=/proc/net/pktgen/wlan0@1

pgset "clone_skb 0"
pgset "pkt_size $pktsize"
pgset "src_mac 00:22:33:44:AA:A0"
pgset "flag IPSRC_RND"
pgset "src_min 192.168.3.127"
pgset "src_max 192.168.3.127"
pgset "dst 192.168.3.1"
pgset "dst_mac  00:22:33:44:AA:90"
pgset "count $pktCount"
pgset "flag NO_TIMESTAMP"

PGDEV=/proc/net/pktgen/kpktgend_2
pgset "rem_device_all"
pgset "add_device wlan0@2"
#pgset "max_before_softirq 1"

# Configure the individual devices
# echo "Configuring devices"

PGDEV=/proc/net/pktgen/wlan0@2

pgset "clone_skb 0"
pgset "pkt_size $pktsize"
pgset "src_mac 00:22:33:44:AA:A0"
pgset "flag IPSRC_RND"
pgset "src_min 192.168.3.127"
pgset "src_max 192.168.3.127"
pgset "dst 192.168.3.1"
pgset "dst_mac  00:22:33:44:AA:90"
pgset "count $pktCount"
pgset "flag NO_TIMESTAMP"

PGDEV=/proc/net/pktgen/kpktgend_3
pgset "rem_device_all"
pgset "add_device wlan0@3"
#pgset "max_before_softirq 1"

# Configure the individual devices
# echo "Configuring devices"

PGDEV=/proc/net/pktgen/wlan0@3

pgset "clone_skb 0"
pgset "pkt_size $pktsize"
pgset "src_mac 00:22:33:44:AA:A0"
pgset "flag IPSRC_RND"
pgset "src_min 192.168.3.127"
pgset "src_max 192.168.3.127"
pgset "dst 192.168.3.1"
pgset "dst_mac  00:22:33:44:AA:90"
pgset "count $pktCount"
pgset "flag NO_TIMESTAMP"

echo "Pktgen configuration done!"
