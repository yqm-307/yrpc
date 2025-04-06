
#!/bin/bash

work_dir=$(pwd)
echo "Current working directory: $work_dir"

ps -aux | grep echo_server | awk '{print $2}' | xargs kill -9
ps -aux | grep echo_client | awk '{print $2}' | xargs kill -9

if [ ! -d "${work_dir}/build/bin/benchmark/fatiguetest" ]; then
    echo "Directory ${work_dir}/build/bin/benchmark/fatiguetest does not exist. Please build the project first."
    exit 1
fi

# Start the echo_server in the background using nohup
nohup ${work_dir}/build/bin/benchmark/fatiguetest/echo_server > echo_server.log 2>&1 &
echo "echo_server started with nohup. Logs are being written to echo_server.log"

for i in {1..10}
do
    nohup ${work_dir}/build/bin/benchmark/fatiguetest/echo_client > echo_client_$i.log 2>&1 &
    echo "echo_client $i started with nohup. Logs are being written to echo_client_$i.log"
    # sleep 1
done