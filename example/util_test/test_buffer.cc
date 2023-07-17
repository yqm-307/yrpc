#include "yrpc/Util/Buffers.h"
#include "yrpc/protocol/proto.h"
typedef bbt::buffer::Buffer Buffer;
#define ToInt(n) yrpc::util::protoutil::BytesToType<int>(n)

char *Malloc(char c, size_t len)
{
    char *p = (char *)malloc(len);
    memset(p, c, len);
    return p;
}

// 写入测试
void test1()
{
    Buffer buf;
    srand((unsigned)time(NULL));
    for (int i = 0; i < 100; ++i)
    {
        int n = 500 + rand() % 500;
        char *tmp = (char *)malloc(n);
        memset(tmp, 'a', n);
        buf.WriteString(tmp, n);
        free(tmp);
    }
}

void test2()
{
    Buffer buf;

    char *a_arr = Malloc('a', 1024);
    char *b_arr = Malloc('b', 1024);
    char *c_arr = Malloc('c', 1024);
    char *d_arr = Malloc('d', 1024);
    char *e_arr = Malloc('e', 1);
    buf.WriteString(a_arr, 1024);
    buf.WriteString(b_arr, 1024);
    buf.WriteString(c_arr, 1024);
    buf.WriteString(d_arr, 1024);
    char read[1024 * 4];
    buf.ReadString(read, 1024 * 4);
    buf.WriteString(e_arr, 1);
}

void test3()
{
    Buffer buf;

    char *a_arr = Malloc('a', 1024);
    char *b_arr = Malloc('b', 1024);
    char *c_arr = Malloc('c', 1024);
    char *d_arr = Malloc('d', 1024);
    char *e_arr = Malloc('e', 4096);
    buf.WriteString(a_arr, 1024);
    buf.WriteString(b_arr, 1024);
    buf.WriteString(c_arr, 1024);
    buf.WriteString(d_arr, 1024);
    char read[1024 * 4];
    buf.ReadString(read, 1024 * 4 - 1);
    buf.WriteString(e_arr, 4096);
}

void test4()
{
    Buffer buf;
    srand((unsigned)time(NULL));
    char str[1024];
    int a = 1;
    memset(str, '0', 1024);
    memcpy(str, (char *)&a, sizeof(int));

    while (1)
    {

        if (rand() % 2 == 0)
        { // read
            if (buf.ReadableBytes() > 0)
            {
                char tmp[1024];
                buf.ReadString(tmp, 1024);
                assert(((ToInt(tmp)) == 1));
            }
            else
                buf.WriteString(str, 1024);
        }
        else
        { // write
            buf.WriteString(str, 1024);
        }
    }
}

int main()
{
    // test1();
    // test2();
    // test3();
    test4();
}