# tap_ip_mac_tap
tap实现ip列表修改默认mac地址
./ser  

把虚拟网络国外vpn0 ,虚拟网络国内eth0  加入br0 同一网段
tap1分享   tap0加入br0 
iptables.c   可访问国外的虚拟网络mac  ip国外地址 ip国外地址 ...  #google 
删除所有规则
echo >/dev/iptables
