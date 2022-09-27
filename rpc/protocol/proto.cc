/**
 * @file proto.cc
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-06-14
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "./proto.h"

namespace yrpc::util::protoutil
{


Argspkg* InitArgs()
{
    Argspkg* args = (Argspkg*)calloc(1,sizeof(Argspkg));
}


void DestoryArgs(Argspkg* args)
{
    free(args);
}





ArgsPtr ParseProtoMsg(const google::protobuf::Message& message)
{
    ArgsPtr Args = std::make_shared<Argspkg>();

    const google::protobuf::Descriptor *des = message.GetDescriptor();
    const google::protobuf::Reflection *ref = message.GetReflection();
    int fieldCount = des->field_count();

    //遍历所有字段
    for (int i = 0; i < fieldCount; i++)
    {
        ///< 当前字段下标
        const google::protobuf::FieldDescriptor *field = des->field(i);

        //switch匹配字段类型，不同字段不同解析方式
        switch (field->type())
        {
            case google::protobuf::FieldDescriptor::Type::TYPE_INT32:
            case google::protobuf::FieldDescriptor::Type::TYPE_SINT32:
            case google::protobuf::FieldDescriptor::Type::TYPE_SFIXED32:
                Args->push_back(ParseInt32(message,ref,field));
                break;

            case google::protobuf::FieldDescriptor::Type::TYPE_INT64:
            case google::protobuf::FieldDescriptor::Type::TYPE_SINT64:
            case google::protobuf::FieldDescriptor::Type::TYPE_SFIXED64:
            {
                Args->push_back(ParseInt64(message,ref,field));
                break;
            }
            case google::protobuf::FieldDescriptor::Type::TYPE_UINT32:
            case google::protobuf::FieldDescriptor::Type::TYPE_FIXED32:
            {
                Args->push_back(ParseUInt32(message,ref,field));
                break;
            }
            case google::protobuf::FieldDescriptor::Type::TYPE_UINT64:
            case google::protobuf::FieldDescriptor::Type::TYPE_FIXED64:
            {
                Args->push_back(ParseUInt64(message,ref,field));
                break;
            }
            case google::protobuf::FieldDescriptor::Type::TYPE_DOUBLE:
            {
                Args->push_back(ParseDouble(message,ref,field));
                break;  
            }
            case google::protobuf::FieldDescriptor::Type::TYPE_FLOAT:
            {
                Args->push_back(ParseFloat(message,ref,field));
                break;
            }
            case google::protobuf::FieldDescriptor::Type::TYPE_BOOL:
            {
                Args->push_back(ParseBool(message,ref,field));
                break;
            }
            case google::protobuf::FieldDescriptor::Type::TYPE_ENUM:
            {
                Args->push_back(ParseEnum(message,ref,field));
                break;
            }
            case google::protobuf::FieldDescriptor::Type::TYPE_STRING:
            case google::protobuf::FieldDescriptor::Type::TYPE_BYTES:
            {
                Args->push_back(ParseString(message,ref,field));
                break;
            }
            case google::protobuf::FieldDescriptor::Type::TYPE_MESSAGE:
            {//需要递归展开
                Args->push_back(ParseMessage(message,ref,field));
                break;
            }
        default:
            break;
        }
    }

    return Args;
}




std::any ParseInt32(const Message& message,const Reflection* ref,const FieldDescriptor* field)
{
    if (field->is_repeated())
    {//数组形式
        std::vector<int32_t> ret;
        int count = ref->FieldSize(message, field);
        for (int j = 0; j < count; j++)
        {
            int32_t data = ref->GetRepeatedInt32(message, field, j);
            ret.push_back(data);
        }
        return ret;
    }
    else
    {//单个参数

        int32_t data = ref->GetInt32(message, field);
        return data;
    }
}

std::any ParseInt64(const Message& message,const Reflection* ref,const FieldDescriptor* field)
{
    if (field->is_repeated())
    {
        std::vector<int64_t> ret;
        int count = ref->FieldSize(message, field);
        for (int j = 0; j < count; j++)
        {
            int64_t data = ref->GetRepeatedInt64(message, field, j);
            ret.push_back(data);
        }
        return ret;
    }
    else
    {
        int64_t data = ref->GetInt64(message, field);
        return data;        
    }
}

std::any ParseUInt32(const Message &message, const Reflection *ref, const FieldDescriptor *field)
{
    if (field->is_repeated())
    {
        std::vector<uint32_t> ret;
        int count = ref->FieldSize(message, field);
        for (int j = 0; j < count; j++)
        {
            uint32_t data = ref->GetRepeatedUInt32(message, field, j);
            ret.push_back(data);
        }
        return ret;
    }
    else
    {
        uint32_t data = ref->GetUInt32(message, field);
        return data;
    }
}

std::any ParseUInt64(const Message &message, const Reflection *ref, const FieldDescriptor *field)
{
    if (field->is_repeated())
    {   
        std::vector<uint64_t> ret;
        int count = ref->FieldSize(message, field);
        for (int j = 0; j < count; j++)
        {
            uint64_t data = ref->GetRepeatedUInt64(message, field, j);
            ret.push_back(data);
        }
    }
    else
    {
        uint64_t data = ref->GetUInt64(message, field);
        return data;
    }
}

std::any ParseDouble(const Message &message, const Reflection *ref, const FieldDescriptor *field)
{
    if (field->is_repeated())
    {   
        std::vector<double> ret;
        int count = ref->FieldSize(message, field);
        for (int j = 0; j < count; j++)
        {
            double data = ref->GetRepeatedDouble(message, field, j);
            ret.push_back(data);
        }
    }
    else
    {
        double data = ref->GetDouble(message, field);
        return data;
    }
}

std::any ParseFloat(const Message &message, const Reflection *ref, const FieldDescriptor *field)
{
    if (field->is_repeated())
    {
        std::vector<float> ret;
        int count = ref->FieldSize(message, field);
        for (int j = 0; j < count; j++)
        {
            float data = ref->GetRepeatedFloat(message, field, j);
            ret.push_back(data);
        }
    }
    else
    {
        float data = ref->GetFloat(message, field);
        return data;
    }
}

std::any ParseBool(const Message &message, const Reflection *ref, const FieldDescriptor *field)
{
    if (field->is_repeated())
    {
        std::vector<bool> ret;
        int count = ref->FieldSize(message, field);
        for (int j = 0; j < count; j++)
        {
            bool data = ref->GetRepeatedBool(message, field, j);
            ret.push_back(data);
        }
    }
    else
    {
        bool data = ref->GetBool(message, field);
        return data;
    }
}

std::any ParseEnum(const Message &message, const Reflection *ref, const FieldDescriptor *field)
{
    if (field->is_repeated())
    {
        std::vector<const google::protobuf::EnumValueDescriptor*> ret;
        int count = ref->FieldSize(message, field);
        for (int j = 0; j < count; j++)
        {
            const google::protobuf::EnumValueDescriptor *data = ref->GetRepeatedEnum(message, field, j);
            ret.push_back(data);
        }
    }
    else
    {
        const google::protobuf::EnumValueDescriptor *data = ref->GetEnum(message, field);
        return data;
    }
}

std::any ParseString(const Message &message, const Reflection *ref, const FieldDescriptor *field)
{
    if (field->is_repeated()) //数组
    {
        std::vector<std::any> ret;
        int count = ref->FieldSize(message, field);
        for (int j = 0; j < count; j++)
        {
            std::any data = ref->GetRepeatedString(message, field, j);
            ret.push_back(data);
        }
    }
    else
    {
        std::any data = ref->GetString(message, field);
        return data;
    }
}

std::any ParseMessage(const Message &message, const Reflection *ref, const FieldDescriptor *field)
{
    if (field->is_repeated())
    {
        std::vector<ArgsPtr> ret; 
        int count = ref->FieldSize(message, field);
        for (int j = 0; j < count; j++)
        {
            const google::protobuf::Message &innerMsg = ref->GetRepeatedMessage(message, field, j);
            ret.push_back(ParseProtoMsg(innerMsg));
        }
    }
    else
    {
        const google::protobuf::Message &innerMsg = ref->GetMessage(message, field);
        return ParseProtoMsg(innerMsg);
    }
}
}
