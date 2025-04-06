#!/bin/bash
# 将 $1 目录中所有源文件删除，并拷贝至 $2 路径下，并命名为 $3

# copy的目标路径
dstdir=$1
# 工程根目录名
rootdir=$2
# 相对工程根目录名
subdir=$3


# 拷贝名字
cp -rf ${rootdir}/${subdir} tmp_header_only
find tmp_header_only -name '*.c' -o -name '*cc' -print | xargs rm


# 创建空的目标目录
if [ ! -d "$dstdir/${rootdir}/${subdir}" ];then
    sudo mkdir -p $dstdir/${rootdir}
fi

# 删除旧的目录
if [ -d "$dstdir/${rootdir}/${subdir}" ];then
    sudo rm -rf $dstdir/${rootdir}/${subdir}
fi

sudo mv tmp_header_only $dstdir/${rootdir}/${subdir}
rm -rf tmp_header_only