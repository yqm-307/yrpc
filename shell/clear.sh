#!/bin/bash




#清理cmake和makefile文件
clearfile()
{
    find . -name 'Makefile' -o -name '*.camke' -o -name 'CMakeCache.txt' -o -name 'cmake_install.cmake'  -print | xargs rm
    find . -name '*.log' -print | xargs rm
    find . -name *.pb.cc -print | xargs rm
    find . -name *.pb.h -print | xargs rm
    find . -name 'Makefile' -print |xargs rm
    find . -name 'CMakeFiles' -type d -print | xargs rm -rf


    rm -rf build
    
    echo "清理成功"
}

#清理可执行文件和库文件
clearexec()
{
    rm -rf bin  #可执行文件
    rm -rf lib  #库文件
}



main()
{
cd ..

clearexec
clearfile

rm  CMakeCache.txt
}





main



