#!/bin/bash
##
## Spawning VM1 on VROUTER1
##

. 00.config.sh

#################################################################
## Spawn a VM
${QEMU_DIR}/x86_64-softmmu/qemu-system-x86_64 -cpu host -smp 4 -enable-kvm \
    -drive if=virtio,file=vm1.qcow2,cache=none \
    -object memory-backend-file,id=mem,size=1024M,mem-path=${TLBFS_DIR},share=on \
    -numa node,memdev=mem \
    -m 1024 -vnc :1 \
    -chardev socket,id=charnet0,path=${UVH_PREFIX}_vm1 \
    -netdev type=vhost-user,id=hostnet0,chardev=charnet0 \
    -device virtio-net-pci,netdev=hostnet0,mac=${VM1_MAC} \