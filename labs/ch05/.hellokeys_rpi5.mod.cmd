savedcmd_/root/rpi5/LinuxDriverDevelopmenWithRaspberryPi5/labs/ch05/hellokeys_rpi5.mod := printf '%s\n'   hellokeys_rpi5.o | awk '!x[$$0]++ { print("/root/rpi5/LinuxDriverDevelopmenWithRaspberryPi5/labs/ch05/"$$0) }' > /root/rpi5/LinuxDriverDevelopmenWithRaspberryPi5/labs/ch05/hellokeys_rpi5.mod
