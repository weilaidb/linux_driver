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

# 清除网口统计
echo clearinterface ens33 > /sys/kernel/mymodule/my_data && dmesg -c
cat /sys/kernel/mymodule/my_data
ifconfig ens33

- lo无法清除统计信息，why?
echo clearinterface lo > /sys/kernel/mymodule/my_data && dmesg -c
cat /sys/kernel/mymodule/my_data
ifconfig lo


# 显示系统文件描述符
echo showprocessfd > /sys/kernel/mymodule/my_data && dmesg -c
cat /sys/kernel/mymodule/my_data
