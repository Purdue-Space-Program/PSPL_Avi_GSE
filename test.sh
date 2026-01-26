#!/bin/bash

# Trap interrupts and kill the whole process group
# "$$" is the script's PID. 
# "-" before the PID tells kill to target the Process Group.
trap "kill 0" EXIT

synnax start -im --no-driver &
nc -lk 2703 &
python3 mock/main.py &

wait
