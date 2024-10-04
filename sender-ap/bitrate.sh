echo "1 64" > /sys/kernel/debug/ieee80211/phy0/ath10k/htt_max_amsdu_ampdu
iw wlan-2400mhz set bitrates legacy-2.4  ht-mcs-2.4 15 vht-mcs-2.4 sgi-2.4
