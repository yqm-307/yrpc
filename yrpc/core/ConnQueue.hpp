#pragma once
#include "RpcSession.h"
#include "../network/all.h"
#include "ServiceModule.h"
#include <vector>
#include <map>

namespace yrpc::rpc::detail
{

struct HandShakeData
{
    typedef std::function<void(const yrpc::detail::shared::errorcode&, SessionPtr)>     
                            OnHandShakeSuccCallback;
    OnHandShakeSuccCallback m_succ;
};

/**
 * @brief 半连接队列
 *  为了实现连接复用，做了握手机制，那么两个节点连接的时候会进行握手。
 * 正在握手中的连接都会被管理在连接队列中;
 *  等到握手完成后，一个 RpcSession 才算是完成，才会到 SessionManager
 * 中做最后处理（丢弃或应用）
 * 
 * 简化理解：
 *      可以思考在YRPC中一个RpcSession真正可以被用户使用（加入到Session-
 * Manager的管理中），就只有一个渠道，就是完成半连接队列。根据时序不同，
 * 我们考虑几种可能出现的极限情况：
 *  (1)节点a连接到节点b，连接完全建立。此时节点b再尝试connect到节点a，会
 * 复用连接，反之亦然。
 *  (2)在ConnQueue中，存在节点a到节点b的临时RpcSession，同时存在节点b到
 * 节点a的临时RpcSession。那么这个时候是个临界点，对于tcp连接不是问题（
 * 可以支持两个进程之间多连接）。但是在ConnQueue中就需要判断了，ConnQueue
 * 一定要让握手最后阶段完成时，先完成的连接保存在SessionManager中，后完
 * 成的释放自己的连接。
 */
class ConnQueue : bbt::noncopyable
{
    typedef uint64_t SessionID;
    typedef std::shared_ptr<RpcSession>         SessionPtr;
    typedef yrpc::detail::net::YAddress         Address;
    typedef std::unordered_map<Address,HandShakeData>   RpcSessionMap;
    typedef std::unordered_set<Address>         TcpConnMap;
public:
    typedef std::unique_ptr<ConnQueue>  Ptr;
    /* 弹出一个元素 , 失败返回 nullptr*/
    std::pair<HandShakeData,bool> PopUpById(const Address& addr);
    int FindAndPush(const Address& addr, const HandShakeData& func);
    const HandShakeData* Find(const Address& id);

    /* 是否有等待中的session或tcp连接 */
    bool HasWaitting(const Address& addr);
    /* 添加一个tcp连接 */
    bool AddTcpConn(const Address& addr);
    /* 是否有某个tcp连接 */
    bool HasTcpConn(const Address& addr);
    /* 删除一个TcpConn */
    bool DelTcpConn(const Address& addr);

    // void GetTcpConn(std::vector<const Address&>& addrlist);

private:
    RpcSessionMap   m_undone_rpc_session_map;   // session 半连接队列
    TcpConnMap      m_undone_tcp_conn_map;      // tcp 发起连接队列
};

}// yrpc::rpc::detail