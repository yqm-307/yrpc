/*
    jump中 返回值形式返回上下文，且第一个参数是跳转目标上下文，第二个参数是返回值的一部分

    rax 用作储存返回值
    rdi 用作函数第一个参数
    rsi 用作函数第二个参数
*/
    .globl YContextjump
    .type YContextjump, @function
    /* 准备备份栈 38字节*/
    leaq  -0x38(%rsp), %rsp
    
    /* 保存当前寄存器上下文 */
    /*返回值地址 8字节 ，总计38字节*/
    movq  %r12, 0x8(%rsp)   /* 保存R12 8字节 */
    movq  %r13, 0x10(%rsp)  /* 保存R13 2字节 */
    movq  %r14, 0x18(%rsp)  /* 保存R14 8字节 */
    movq  %r15, 0x20(%rsp)  /* 保存R15 2字节 */
    movq  %rbx, 0x28(%rsp)  /* 保存RBX 8字节 */
    movq  %rbp, 0x30(%rsp)  /* 保存RBP 2字节*/

    movq  %rsp, %rax    /*让rax指向当前上下文地址*/

    movq  %rdi, %rsp    /* 拿到第一个目标协程栈地址，存到rdi中 */

    movq  0x38(%rsp), %r8   /* 返回值地址 8字节*/

    movq  0x8(%rsp), %r12   /* 切换到目标协程栈的R12 */
    movq  0x10(%rsp), %r13  /* 切换到目标协程栈的R13 */
    movq  0x18(%rsp), %r14  /* 切换到目标协程栈的R14 */
    movq  0x20(%rsp), %r15  /* 切换到目标协程栈的R15 */
    movq  0x28(%rsp), %rbx  /* 切换到目标协程栈的RBX */
    movq  0x30(%rsp), %rbp  /* 切换到目标协程栈的RBP */

    /*此时返回值，和当前上下文切换完毕*/
    /*封装第二个参数*/
    leaq  0x40(%rsp), %rsp /* prepare stack */
    
    salq  $32, %rsi     /*rsi中值左移32位*/
    orq   %rsi, %rax    
    movq  %rax, %rdi

    jmp  *%r8

    //.size jump_fcontext,.-jump_fcontext
    //.section .note.GNU-stack,"",%progbits
    .end