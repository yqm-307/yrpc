#!/bin/bash
# shell 脚本，进行一个自动启动服务器、客户端
ARGS=$@

read -ra StrArr <<<$ARGS

# for ((i=1;i<${#StrArr};++i));
# do
#     echo ${StrArr:$i:i}
# done


for ((i=1;i<${#ARGS};++i));
do
    if [ ${ARGS:i:1}=="s" ];
    then
        ./bin/rpc/RpcServer_test
        break
    elif [ ${ARGS:i:1}=="c" ]; 
    then
        for ((j=0;j<${ARGS:i+1:1};++j));
        do
            ./bin/rpc/RpcClient_test -d
        done
    else
        echo "Arg bad!"
    fi


done
