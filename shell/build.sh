SRC_DIR=".."
DST_DIR="../rpc/proto"
SRC_DIR="../message"


function generate()
{
    protofilepath=$1;
    echo "生成 $protofilepath"
    protoc -I=${SRC_DIR} --cpp_out=${DST_DIR} $protofilepath
}

## 递归获取 SRC_DIR 下所有protocol文件
function getallprotofile()
{
    filelist=()
    
    files=`find ${SRC_DIR} -type f -name '*.proto' -print`      # 获取当前列表下的file
    for file in $files
    do
        generate $file
    done
}



# 获取所有proto文件
function main()
{
    # 创建目录
    if [ !  -d "../rpc/proto"  ]
    then
        mkdir ../proto
    fi

    getallprotofile
    cd .. && cmake .
    make -j4
}



main

