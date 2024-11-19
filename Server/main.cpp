#include "pch.h"
#include "IocpCore.h"

int main()
{
    shared_ptr<IocpCore> iocpCore = make_shared<IocpCore>();
    iocpCore->StartServer();

    shared_ptr<Session> session = make_shared<Session>();
    iocpCore->StartAccept(session);

    return 0;
}

