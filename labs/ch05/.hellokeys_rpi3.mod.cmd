savedcmd_/root/rpi5/labs/ch05/hellokeys_rpi3.mod := printf '%s\n'   hellokeys_rpi3.o | awk '!x[$$0]++ { print("/root/rpi5/labs/ch05/"$$0) }' > /root/rpi5/labs/ch05/hellokeys_rpi3.mod
