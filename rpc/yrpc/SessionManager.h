#include "RpcClientSession.h"
#include "RpcSession.h"
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
    typedef yrpc::coroutine::poller::Epoller    Epoller;
    typedef yrpc::util::lock::CountDownLatch    CountDownLatch;
    typedef yrpc::detail::net::Acceptor         Acceptor;
    typedef yrpc::detail::net::ConnectionPtr    ConnPtr;

    typedef std::unordered_map<SessionID,RpcSession*>   SessionMap;

public:
    static SessionManager* GetInstance(int n=0);   

    int CreateNewSession(int NThread); // 创建一个session

    int Submit(const yrpc::util::buffer::Buffer& buff,SessionID session_id);  // 提交协议流 

    // session is alive
    bool SessionIsAlive(SessionID session);

    bool SessionIsAlive(yrpc::detail::net::YAddress addr);

    // rpc client 注册到session manager
    int RpcClientRegister();

    
    
private:
    SessionManager(int Nthread);  

    SessionID GetSessionID();

    // 运行在 main loop 中的，只做新连接的分发
    void RunInMainLoop();
    // 运行在 sub loop 中的，只做io、协议解析
    void RunInSubLoop(Epoller*);

    // 连接建立
    void OnAccept(ConnPtr,void*);

private:
    Epoller*            m_main_loop;        // 只负责 listen 的 epoll
    Acceptor            m_main_acceptor;    // listen 
    Epoller**           m_sub_loop;         // 协程调度器
    CountDownLatch      m_loop_latch;       // 
    
    std::thread*        m_main_thread;
    std::thread**       m_sub_threads;   

    SessionMap          m_sessions;         // 会话

    // main loop 控制
    std::atomic_bool    m_run;

    const int port;
    
};

}
