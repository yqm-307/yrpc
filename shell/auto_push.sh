#!/bin/bash
git_project_path=$1
git_origin=$2       # git origin
git_branch=$3       # git 分支名
git_commit_info=$4  # git commit 信息


function help()
{
    echo "======================================================"
    echo "Git自动提交工具: 将 path 目录下的 origin 的 branch 分支，提交信息为 commit info"
    echo ""
    echo "用法: auto_push [path] [origin] [branch] [commit msg]"
    echo "======================================================"
}

function check_args()
{
    
    if [ -z "${git_project_path}" ]
    then
        echo "失败: path 不能为空"
        help
        exit -1
    fi
    if [ -z "${git_origin}" ]
    then
        echo "失败: origin 不能为空"
        help
        exit -1
    fi

    if [ -z "${git_branch}" ]
    then
        echo "失败: branch 不能为空"
        help
        exit -1
    fi
    if [ -z "${git_commit_info}" ]
    then
        echo "失败: commit msg 不能为空"
        help
        exit -1
    fi

}

function git_auto_push() 
{
    # 1、目录跳转
    cd ${git_project_path}

    # 2、git switch
    `git switch ${git_branch}` 
    
    # 2、git add
    git add *

    # 3、git commit
    git commit -m "${git_commit_info}"

    # 4、git push
    `git push ${git_origin} ${git_branch}` 
}

function main()
{
    check_args
    git_auto_push
}

main