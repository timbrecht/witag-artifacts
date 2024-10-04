if [ "$#" -ne 1 ]; then 
  echo "Provide the MAC address of the card that receives the Block ACKs"
  exit;
fi
mac=$1
#tshark -l -i mon -T fields -e radiotap.mactime -e wlan.fixed.ssc.sequence -e wlan.ba.bm -e wlan.ta -e wlan.ra  "(wlan[0] == 0x94) and (wlan[1] == 0x00)"
tshark -l -i mon -T fields -e radiotap.mactime -e wlan.fixed.ssc.sequence -e wlan.ba.bm -e wlan.ta -e wlan.ra  "(wlan[0] == 0x94) and (wlan addr1 $mac)"
