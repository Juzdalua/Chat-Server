#include "pch.h"
#include "IocpCore.h"

void IocpWorker(shared_ptr<IocpCore> iocpCore);

int main()
{
    shared_ptr<IocpCore> iocpCore = make_shared<IocpCore>();
    iocpCore->StartServer();

    shared_ptr<Session> session = make_shared<Session>();
    iocpCore->StartAccept(session);

    thread t1(IocpWorker, iocpCore);
    t1.join();

    cout << "exit" << '\n';
    return 0;
}

void IocpWorker(shared_ptr<IocpCore> iocpCore)
{
    while (true)
    {
        iocpCore->Dispatch(10);
    }
}