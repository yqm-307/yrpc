##
## 生成protobuf 文件，并构建项目
##

SRC_DIR=".."
DST_DIR="../rpc/proto"
SRC_DIR="../message"




# 获取所有proto文件
function main()
{
    ./generate.sh

    # 创建目录
    if [ !  -d "../build"  ]
    then
        mkdir ../build
    fi
    
    cd ../build 
    cmake ..
    make -j4
}



main

