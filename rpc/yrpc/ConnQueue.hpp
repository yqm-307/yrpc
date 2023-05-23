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
    OnSessionCallback PopUpById(SessionID id);
    /* 查找并插入，id存在成功插入 ret >= 0 ，找不到或插入失败 ret < 0 */
    int   FindAndPush(SessionID id, const OnSessionCallback& func);
    OnSessionCallback Find(SessionID id);
private:
    Map m_map;
};

}// yrpc::rpc::detail