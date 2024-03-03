#!/bin/bash

#                     +------------------+      +------------------+
#                     |        r0        |      |        r1        |
#                     |                  |      |                  |
#                     |                  |      |                  |
#                     |            veth0 +------+ veth1            |
#                     |                  |      |                  |
#                     |          10.0.0.1/24   10.0.0.2/24         |
#                     |                  |      |                  |
#                     +------------------+      +------------------+
#

set -x

TMUX=sfw
IPP=ip

# Kill tmux previous session
tmux kill-session -t ${TMUX} 2>/dev/null

# Clean up previous network namespaces
$IPP -all netns delete

$IPP netns add r0
$IPP netns add r1

$IPP link add name veth0 netns r0 type veth peer name veth1 netns r1

###################
#### Node: r0 #####
###################
echo -e "\nNode: r0"

# not really required for this test
$IPP netns exec r0 sysctl -w net.ipv4.ip_forward=1
$IPP netns exec r0 sysctl -w net.ipv6.conf.all.forwarding=1

$IPP netns exec r0 $IPP link set dev lo up
$IPP netns exec r0 $IPP link set dev veth0 up
$IPP netns exec r0 $IPP addr add 10.0.0.1/24 dev veth0


###################
#### Node: r1 #####
###################
echo -e "\nNode: r1"

# not really required for this test
$IPP netns exec r1 sysctl -w net.ipv4.ip_forward=1
$IPP netns exec r1 sysctl -w net.ipv6.conf.all.forwarding=1

$IPP netns exec r1 $IPP link set dev lo up
$IPP netns exec r1 $IPP link set dev veth1 up
$IPP netns exec r1 $IPP addr add 10.0.0.2/24 dev veth1

sleep 2

## Create a new tmux session
tmux new-session -d -s $TMUX -n r0 $IPP netns exec r0 bash
tmux new-window -t $TMUX -n r1 $IPP netns exec r1 bash
tmux select-window -t :0
tmux set-option -g mouse on
tmux attach -t $TMUX
