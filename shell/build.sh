#!/bin/bash
##
## 生成protobuf 文件，并构建项目
##

SRC_DIR=".."
DST_DIR="../rpc/proto"
SRC_DIR="../message"


function info_build()
{
    info=$1
    echo "[yrpc auto build] ${info}"
}

# 生成头文件目录，并copy到 /usr/local/include
function generate_head_dir_and_copy_to_dstdir()
{
    dstdir=$1
    cp -rf yrpc rpc_h
    find rpc_h -name '*.c' -o -name '*cc' -print | xargs rm
    if [ -d "$dstdir/yrpc" ]
    then
        sudo rm -rf $dstdir/yrpc
    fi
    sudo mv rpc_h $dstdir/yrpc
    info_build "copy over! cpp head file copy to ${dstdir}/yrpc"
    rm -rf rpc_h
}

# 获取所有proto文件
function main()
{
    info_build "building: rpc protocol & example protocol."
    ./generate.sh
    info_build "building: protobuf generated successfully!"
    # 创建目录
    if [ !  -d "../build"  ]
    then
        mkdir ../build
        info_build "create build dir"
    fi
    
    # 编译
    cd ../build 
    info_build "start compiled!"
    cmake ..
    make -j4

    # 回到project root目录
    cd ..

    # 拷贝头文件
    info_build "copy cpp head file to /usr/local/include"
    generate_head_dir_and_copy_to_dstdir /usr/local/include
}



main

