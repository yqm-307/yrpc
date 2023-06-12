/**
 * @file PointServer.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2023-06-12
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include <sys/signal.h>
#include <yrpc/core/YRpc.h>
#include "yrpc/network/all.h"
#include "../all_example.pb.h"
#include <bbt/random/random.hpp>

using namespace yrpc;
typedef yrpc::detail::ProtoMsgPtr MsgPtr;



class PointServer
{
public:
    /**
     * @brief Construct a new Point Server object
     * 
     * @param ip 对端ip
     * @param port 对端port
     */
    PointServer(
        const std::string& ip_peer, int port_peer,
        int port_local,
        int min = 10000, int max = 30000)
        :peer_addr(ip_peer, port_peer),
        local_addr(port_local),
        m_max_call_wait_num(min),
        m_min_call_wait_num(max),
        m_stop(false)
    {
        rpc::Rpc::register_service<AddReq, AddRsp>("Remote_Add", [this](MsgPtr m){ return Remote_Add(m); });
    }
    ~PointServer(){}


    void StartService()
    {
        rpc::Rpc::StartServerListen(local_addr); // 启动 Recv 线程 和 Service Handle
    }

    rpc::detail::CallObj::Ptr Generate()
    {
        int result = rand();
        int b = rand() % result;
        auto it = m_result.find(result);
        if (it == m_result.end())
        {
            m_result.insert(std::make_pair(result,0));
        }
        else
        {
            (it->second)++;
        }
        AddReq req;
        req.set_a(result - b);
        req.set_b(b);
        auto call_obj = rpc::CallObjFactory::GetInstance()->Create<AddReq, AddRsp>(std::move(req), "Remote_Add", 
        [this](yrpc::detail::ProtoMsgPtr rsp){
            Destroy(rsp);
        });
    }

    void Destroy(MsgPtr packet)
    {
        auto rsp = std::static_pointer_cast<AddRsp>(packet);
        int result = rsp->result();
        auto it = m_result.find(result);
        assert(it != m_result.end());
        (it->second)--;
        m_complete_req++;
    }

    void Start(){
        _co_scheduler->AddTimer([this](){PrintOnce();}, 5000);
        while(!m_stop)
        {   
            auto call_obj = Generate();
            rpc::Rpc::RemoteOnce(peer_addr, "Remote_Add", call_obj);
            m_total_req++;
        }
        return;
    }
private:
    MsgPtr Remote_Add(MsgPtr ReqPtr)
    {
        auto req = std::static_pointer_cast<AddReq>(ReqPtr);
        auto rval = req->a();
        auto lval = req->b();
        auto rsp = std::make_shared<AddRsp>();
        rsp->set_result(rval + lval);
        return rsp;
    }

    void PrintOnce()
    {
        static int begin = 5; 
        printf("当前经过 %d 秒, 以发送总请求数量 %d 个, 以完成总请求数量 %d 个.\n", begin, m_total_req.load(), m_complete_req.load());
        begin += 5;
    }
private:
    yrpc::detail::net::YAddress peer_addr;   // 对端地址
    yrpc::detail::net::YAddress local_addr; // 本地监听地址
    // 启动PointServer之后会持续发送请求，并将未处理完毕的请求数量保持在 n ( max < n < min)
    int m_max_call_wait_num;    // 等待中的包最大数量
    int m_min_call_wait_num;    // 等待中的包最小数量
    std::atomic_bool m_stop;

    std::map<int, int> m_result;        // result map
    std::atomic_int m_total_req;        // 总请求数
    std::atomic_int m_complete_req;     // 已完成请求数

    bbt::random::mt_random<int,1,1000> rand;
};