#include "RpcClientSession.h"


namespace yrpc::rpc::detail
{

/**
 * @brief session 管理器，客户端最重要的管理模块
 * 
 */
class SessionManager : yrpc::util::noncopyable::noncopyable
{
    typedef int SessionID;
    // typedef std::map<SessionID,RpcClientSession*> SessionMap;       // session id <==> session
    typedef std::unordered_map<yrpc::detail::ynet::YAddress,RpcClientSession*> SessionMap;  // addr <==> session
public:
    SessionManager* GetInstance();

    int Make_Session(); // 创建一个session

    int Submit_Call(const yrpc::util::buffer::Buffer& buff,const yrpc::detail::ynet::YAddress& addr);  // 提交协议流 

private:
    SessionManager();  
    
private:
    SessionMap m_client_sessions;
    // yrpc::coroutine::poller::Epoller* scheduler_;   //协程调度器
    yrpc::detail::ynet::YAddress servaddr_;         //服务端地址
    yrpc::detail::ynet::Connector connector_;       //

};

}
