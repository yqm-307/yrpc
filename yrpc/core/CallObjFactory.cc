#include "CallObjFactory.h"
using namespace yrpc::rpc;

int CallObjFactory::GetIds()
{
    return m_global_id.fetch_add(2);
}