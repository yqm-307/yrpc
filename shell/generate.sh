##
## 自动生成 protobuf 文件
##


SRC_DIR=".."
DST_DIR="../rpc/proto"
SRC_DIR="../message"


function generate()
{
    protofilepath=$1;
    echo "生成 $protofilepath"
    protoc -I=${SRC_DIR} --cpp_out=${DST_DIR} $protofilepath
}

function generate_ex()
{
    output_path=$1
    input_path=$2
    filename=$3

    echo "将 $filename 从 $input_path 生成 proto文件到 $output_path"
    protoc -I=${input_path} --cpp_out=${output_path} $filename
}

## 递归获取 SRC_DIR 下所有protocol文件
function getallprotofile()
{
    filelist=()
    SRC_DIR=$1
    DST_DIR=$2
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
    if [ !  -d "../rpc/protocol/protoc"  ]
    then
        mkdir ../rpc/protocol/protoc
    fi

    ##########
    # rpc 内部协议
    ##########
    generate_ex ../rpc/protocol/protoc ../rpc/protocol/protobuf c2s.proto
    generate_ex ../rpc/protocol/protoc ../rpc/protocol/protobuf s2c.proto

    ##########
    # example
    ##########
    generate_ex ../example/ ../example all_example.proto

}



main