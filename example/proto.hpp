#pragma once

#include <bbt/rpc/detail/Define.hpp>


typedef bbt::rpc::RemoteCallTemplateRequest<int32_t, int64_t, int32_t, std::string> Test1Request;
typedef bbt::rpc::RemoteCallTemplateReply<std::string> Test1Reply;