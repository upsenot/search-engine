#include "Configuration.h"
#include "../../../Include/EchoServer.h"

void test() {
    EchoServer SE(stoul(GetPath("WORKERNUM")),
                          stoul(GetPath("TASKQUENUM")),
                          GetPath("IP"),
                          stoul(GetPath("PORT")));
    SE.start();
}

int main() {
    test();
    return 0;
}