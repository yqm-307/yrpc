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
            detail::SessionManager::GetInstance()->AsyncConnect(addr,nullptr);
            ret = -3;   // 连接未完成, 重试
        }
    }while(0);

    return ret;
}


void Rpc::AsyncConnect(const detail::Address& addr,const detail::CommCallback& cb)
{
    detail::SessionManager::GetInstance()->AsyncConnect(addr,[=](detail::SessionPtr){cb();});
}

void Rpc::StartServerListen(const detail::Address& addr)
{
    detail::SessionManager::GetInstance()->StartListen(addr);
}

int Rpc::YRpcInit()
{
    detail::SessionManager::GetInstance()->RegisterService();
    // register_service<C2S_HANDSHAKE_REQ, S2C_HANDSHAKE_RSP>("YRPC_HandShake", detail::SessionManager::);

}
