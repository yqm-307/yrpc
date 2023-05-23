#include "SessionManager.h"
#include "CallObjFactory.h"
#include "../msg/servicemap.h"
#include "Define.h"

namespace yrpc::rpc
{

class Rpc
{
public:
    Rpc(){}
    ~Rpc(){}

    /** 发起一次远程调用,
     * 返回值含义:
     * 小于等于0失败，大于0成功.
     * -1,连接已关闭 | -2,发送失败 | -3,连接中 | -4,其他错误
     */
    int RemoteOnce(const detail::Address& addr,
        const std::string& funcname,
        detail::CallObj::Ptr obj);

    /* 设置服务器监听端口、IP */
    void SetServerListen(const detail::Address& addr);


    /* 注册服务 */
    template<class ParamPackType,class ReturnPackType>
    void register_service(std::string name,yrpc::detail::ServiceFunc func);
    /* 注册服务 */
    template<class ParamPackType,class ReturnPackType>
    void register_service(std::string name,int id,yrpc::detail::ServiceFunc func);

private:
    void AsyncConnect(detail::OnConnCallBack func);
private:

};





    /**
     * @brief 注册Service到ServiceManager
     * 
     * @param string name 服务名，即该服务的名字，不允许重名服务存在，服务名hash生成id
     * @param function<google::protobuf::Message*(const_std::any)> func 服务处理函数,当连接建立并接收到protomsg时,将解析服务名并调用该服务; google::protobuf::Message*(ArgsPtr&)为服务原型函数,该服务需要
     * 返回一个message的指针（要求用户new创建，会由rpc框架回收内存）。传入ArgsPtr原型是 std::vector<std::any> 的智能指针。数组元素为msg对象顺序结构。
     * 如果元素中包含数组,则any需要被解释为vector,否则为单一元素;比如 int 就 转化为 int，int[] 转化为 vector<int>;
     * @param codec 编码解码处理函数，函数原型为 void(bool,std::any&,std::any&) 其中根据bool值的情况函数分为两种解释，且行为不同。
     * 如果bool 为 true,则执行为Parse行为;如果bool 为false,则执行为Serilize行为。
     */
template<class ParamPackType,class ReturnPackType>
void Rpc::register_service(std::string name,yrpc::detail::ServiceFunc func)
{
    uint32_t ret = yrpc::detail::ServiceMap::GetInstance()->insert(name,func,
    [](bool is_parse,MessagePtr& packet,std::string& bytes){
        if(is_parse)
            packet = yrpc::detail::Codec::ParseToMessage<ParamPackType>(bytes);
        else //序列化
        {
            yrpc::detail::Codec::Serialize(packet,bytes);
        }
    });
    assert(ret >= 0);   //服务注册失败，大概率注册时导致的服务名冲突
}

template<class ParamPackType,class ReturnPackType>
void Rpc::register_service(std::string name,int id,yrpc::detail::ServiceFunc func)
{
    uint32_t ret = yrpc::detail::ServiceMap::GetInstance()->insert(name,id,func,
    [](bool is_parse,std::any& arg1,std::any& arg2){
        if(is_parse)
            arg2 = yrpc::detail::Codec::ParseToMessage<ParamPackType>(std::any_cast<std::string_view&>(arg1));
        else //序列化
        {
            yrpc::detail::Codec::Serialize<ReturnPackType>(std::any_cast<google::protobuf::Message*>(arg1),std::any_cast<std::string&>(arg2));
        }
    });
    assert(ret >= 0);   //服务注册失败，大概率注册时导致的服务名冲突
}
}// namespace yrpc