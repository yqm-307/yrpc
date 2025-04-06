#!/bin/bash

installpath="/usr/local/include"
libpath="/usr/local/lib"

sudo rm -rf $installpath/bbt/rpc
sudo rm -rf $libpath/libbbt_rpc.so

echo "删除完毕"