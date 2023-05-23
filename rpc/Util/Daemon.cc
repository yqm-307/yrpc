#include "Daemon.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/fcntl.h>
#include <iostream>
#include <filesystem>
#include <fstream>

void ign_singal()
{
    signal(SIGCHLD,SIG_IGN);    // 这个是一定要无视的，否则会被干掉
    /* 暂时没有特殊需求，如有需要修改代码力 */
}

void create_pidfile(pid_t pid)
{
    int pidfilefd{-1};
    std::string buf(std::to_string(pid));
    
    /* 如果pid文件不存在，创建 */
    if( !std::filesystem::exists("pid.txt") )
    {
        pidfilefd = open("pid.txt",O_CREAT | O_WRONLY | O_TRUNC,0x0640);
    }    
    else
    {   
        pidfilefd = open("pid.txt",O_WRONLY | O_TRUNC);
    }
    assert(write(pidfilefd,buf.c_str(),buf.size()) > 0);
}

bool yrpc::util::Daemon::damon()
{
    pid_t pid{0};

    int fd{-1};

    /* fork 第一次的话，是需要一个新的进程，作为后台进程 */
    if ((pid = fork()) < 0 )
    {
        perror("fork() error!\n");
        exit(-1);
    }

    /* 杀死父进程 */ 
    if (pid > 0)    
    {
        exit(0);
    }

    /* 让子进程成为会话组的leader,切割和之前终端的联系 */
    if (setsid() < 0)
    {
        perror("setsid() error!\n");
        exit(-1);
    }


    ign_singal();

    /**
     *  切断和之前终端的联系，之前的话没有成为session leader时，
     *  fork出的子进程，并没有脱离之前的终端。所以在成为leader
     *  之后，需要再次fork脱离终端。
    */
    if ( (pid = fork()) < 0 )
    {
        exit(-1);
    }

    if ( pid > 0 )
    {
        exit(0);
    }


    umask(0);   // 文件权限屏蔽字,新文件都是777

    /* 关闭所有文件描述符 */
    for ( fd =sysconf(_SC_OPEN_MAX); fd > 0;fd--)
    {
        close(fd);
    }


    /* 通常不希望意外的从 0|1|2 读取或输出信息，所以都指向空洞 */
    /* todo release 版本需要将框架内的无用代码删除*/
    stdin   = fopen("/dev/null","r");
    stdout  = fopen("/dev/null","w+");
    stderr  = fopen("/dev/null","w+");

    // pid文件
    create_pidfile(pid);
    return true;
}