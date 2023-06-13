#pragma once
#include "RpcSession.h"
#include "../network/all.h"
#include "ServiceModule.h"
#include <vector>
#include <map>

namespace yrpc::rpc::detail
{

class ConnQueue : bbt::noncopyable
{
    typedef uint64_t SessionID;
    typedef std::shared_ptr<RpcSession>         SessionPtr;
    typedef std::function<void(SessionPtr)>     OnSessionCallback;
    typedef yrpc::detail::net::YAddress         YAddress;
    typedef std::map<SessionID,OnSessionCallback>       Map;
public:
    typedef std::unique_ptr<ConnQueue>  Ptr;
    /* 弹出一个元素 , 失败返回 nullptr*/
    std::pair<OnSessionCallback,bool> PopUpById(SessionID id);
    /**
     * @brief 查找并插入一个半连接SessionID(连接中的Session)
     * 
     * @param id    会话ID
     * @param func  会话建立时的回调
     * @return int  # -2(插入失败), 1(已经存在), 2(插入成功)
     */
    int FindAndPush(SessionID id, const OnSessionCallback& func);
    OnSessionCallback Find(SessionID id);
private:
    Map m_map;
};

}// yrpc::rpc::detail