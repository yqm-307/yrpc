#include "../YRoutineContext.h"
using namespace yrpc::coroutine::context;


void f1(void *);
void f2(void *);
YRoutineContext c1(64 * 1024, &f1, nullptr, nullptr, true);
YRoutineContext c2(64 * 1024, &f2, nullptr, nullptr, true);

int test_count = 0;

void f1(void *) {
    for (int i = 0; i < test_count; i++) {
        printf("f1 resume\n");
        c1.Yield();
    }
    printf("f1 end\n");
}

void f2(void *) {
    for (int i = 0; i < test_count; i++) {
        printf("f2 resume\n");
        c2.Yield();
    }
    printf("f2 end\n");
}




int main(int argc,char* argv[]) {

    test_count = 10;
    int test_count_2 = (test_count + 1) * 2;
    int test_count_3 = (test_count + 1);
    for (int i = 0; i < test_count_2; i++) {
        if (i < test_count_3) {
            printf("start resume f1\n");
            c1.Resume();
        } else {
            printf("start resume f2\n");
            c2.Resume();
        }
    }

    return 0;
}
