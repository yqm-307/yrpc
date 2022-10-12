#include "RpcClientSession.h"
#include <vector>

namespace yrpc::rpc::detail
{

/**
 * @brief session 管理器，客户端最重要的管理模块
 * 1、负载均衡加在这里
 * 2、服务探测也加在这里
 * 3、心跳检测也加在这里
 * 4、承上启下，上对应RpcClient的使用者rpc call，下对应 network io
 */
class SessionManager : yrpc::util::noncopyable::noncopyable
{
public:
    typedef uint32_t SessionID;
private:
    // typedef std::map<SessionID,RpcClientSession*> SessionMap;       // session id <==> session
    typedef std::unordered_map<SessionID,RpcClientSession*> SessionMap;  // servid <==> session
    typedef std::vector<yrpc::detail::net::YAddress> ServAddrList;     // 服务器列表

public:
    static SessionManager* GetInstance();

    int CreateNewSession(); // 创建一个session

    int Submit(const yrpc::util::buffer::Buffer& buff,SessionID session_id);  // 提交协议流 

    // session is alive
    bool SessionIsAlive(SessionID session);

    bool SessionIsAlive(yrpc::detail::net::YAddress addr);

    // rpc client 注册到session manager
    int RpcClientRegister();

    
    
private:
    SessionManager();  

    SessionID GetSessionID();

private:
    SessionMap m_client_sessions;   // session map
    ServAddrList m_serv_list;       // 服务器列表，通过框架RpcClient主动连接，注册在这里
    yrpc::coroutine::poller::Epoller* scheduler_;    //协程调度器
    // yrpc::detail::net::YAddress servaddr_;             //服务端地址
    const int port;
    
    // yrpc::detail::net::Connector connector_;        //

};

}
