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
    typedef uint32_t SessionID;
    // typedef std::map<SessionID,RpcClientSession*> SessionMap;       // session id <==> session
    typedef std::unordered_map<SessionID,RpcClientSession*> SessionMap;  // servid <==> session
    typedef std::vector<yrpc::detail::ynet::YAddress> ServAddrList;     // 服务器列表
public:
    SessionManager* GetInstance();

    int Make_Session(); // 创建一个session

    int Submit_Call(const yrpc::util::buffer::Buffer& buff,SessionID session_id);  // 提交协议流 

private:
    SessionManager();  

    SessionID GetSessionID();

private:
    SessionMap m_client_sessions;   // 客户端session
    ServAddrList m_serv_list;       // 服务器列表
    // yrpc::coroutine::poller::Epoller* scheduler_;    //协程调度器
    yrpc::detail::ynet::YAddress servaddr_;             //服务端地址
    yrpc::detail::ynet::Connector connector_;           //

};

}
