uci set wireless.@wifi-iface[1].encryption=wpa2
uci set wireless.@wifi-iface[1].mode="sta"
uci set wireless.@wifi-iface[1].ssid="SET_AS_NEEDED"
uci set wireless.@wifi-iface[1].encryption=wpa2+ccmp
uci set wireless.@wifi-iface[1].eap_type=peap
uci set wireless.@wifi-iface[1].auth=gtc
uci set wireless.@wifi-iface[1].identity="SET_AS_NEEDED"
uci commit wireless
wifi
