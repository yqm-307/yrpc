#include "YRpc.h"

using namespace yrpc::rpc;

int Rpc::RemoteOnce(const detail::Address& addr,const std::string& funcname,detail::CallObj::Ptr obj)
{
    int ret = -1;
    do{
        auto session = detail::SessionManager::GetInstance()->TryGetSession(addr);
        if (session != nullptr)
        {
            if (session->IsClosed())
            {
                ERROR("[YRPC][Rpc::RemoteOnce] session is closed!");
                ret = -1;
                break;
            }
            if (session->SendACallObj(obj) < 0)
            {
                ERROR("[YRPC][Rpc::RemoteOnce] send request fatal!");
                ret = -2;
                break;
            }
            ret = 1;
        }
        else
        {
            if (detail::SessionManager::GetInstance()->IsConnecting(addr))
            {
                ret = -3;
                break;
            }
            else
            {
                ret = -4;
                detail::SessionManager::GetInstance()->AsyncConnect(addr,nullptr);
                break;
            }
        }
    }while(0);

    return ret;
}


int Rpc::AsyncConnect(const detail::Address& addr,const detail::CommCallback& cb)
{
    detail::SessionManager::GetInstance()->AsyncConnect(addr,[=](detail::SessionPtr){cb();});
}

void Rpc::StartServerListen(const detail::Address& addr)
{
    yrpc::rpc::detail::__YRPC_SessionManager::GetInstance()->AsyncAccept(addr);
}
