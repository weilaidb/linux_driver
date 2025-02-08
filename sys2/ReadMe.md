# 打印所有线程
echo "showallthread" > /sys/kernel/mymodule/my_data && dmesg -c
cat /sys/kernel/mymodule/my_data

# 设置线程优先级
echo setthreadpriority 31065 10 > /sys/kernel/mymodule/my_data && dmesg -c
cat /sys/kernel/mymodule/my_data

# 打印网口统计
echo showinterface lo > /sys/kernel/mymodule/my_data && dmesg -c
echo showinterface ens33 > /sys/kernel/mymodule/my_data && dmesg -c
echo showinterface ens38 > /sys/kernel/mymodule/my_data && dmesg -c
cat /sys/kernel/mymodule/my_data

# 更新网口mtu
echo setmtu ens33 1500 > /sys/kernel/mymodule/my_data && dmesg -c
echo setmtu ens33 100 > /sys/kernel/mymodule/my_data && dmesg -c
echo setmtu ens33 500 > /sys/kernel/mymodule/my_data && dmesg -c
echo setmtu ens33 9000 > /sys/kernel/mymodule/my_data && dmesg -c
cat /sys/kernel/mymodule/my_data

