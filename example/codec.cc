#include <bbt/rpc/detail/RpcCodec.hpp>

using namespace bbt::rpc;

int main()
{
    detail::RpcCodec codec;

    // 正确的序列化和反序列化
    auto byte_buffer_1 = codec.Serialize(1, 2, 3, 4, 5);

    std::tuple<int, int, int, int, int> args; 
    auto err = codec.DeserializeWithTuple(byte_buffer_1, args);
    if (err.has_value())
    {
        std::cerr << "Deserialize failed: " << err->What() << std::endl;
        return -1;
    }
    std::cout << "Deserialized values: "
              << std::get<0>(args) << ", "
              << std::get<1>(args) << ", "
              << std::get<2>(args) << ", "
              << std::get<3>(args) << ", "
              << std::get<4>(args) << std::endl;

    // 不同类型的
    auto byte_buffer_2 = codec.Serialize(1, (uint32_t)-1, (uint64_t)2, "hello", (int64_t)-100, (uint64_t)100);
    std::tuple<int, uint32_t, uint64_t, std::string, int64_t, uint64_t> args2;

    auto err2 = codec.DeserializeWithTuple(byte_buffer_2, args2);
    if (err2.has_value())
    {
        std::cerr << "Deserialize failed: " << err2->What() << std::endl;
        return -1;
    }
    std::cout << "Deserialized values: "
              << std::get<0>(args2) << ", "
              << std::get<1>(args2) << ", "
              << std::get<2>(args2) << ", "
              << std::get<3>(args2) << ", "
              << std::get<4>(args2) << ", "
              << std::get<5>(args2) << std::endl;
              
    // 参数过多的解析：可以正确的解析出想要的参数，多余的相当于忽略了
    auto byte_buffer_4 = codec.Serialize(1, (uint32_t)-1, (uint64_t)2, "hello", (int64_t)-100, (uint64_t)100, 123);
    std::tuple<int, uint32_t, uint64_t, std::string, int64_t, uint64_t> args4;
    auto err4 = codec.DeserializeWithTuple(byte_buffer_4, args4);
    if (err4.has_value())
    {
        std::cerr << "Deserialize failed: " << err4->What() << std::endl;
        return -1;
    }
    std::cout << "Deserialized values: "
              << std::get<0>(args4) << ", "
              << std::get<1>(args4) << ", "
              << std::get<2>(args4) << ", "
              << std::get<3>(args4) << ", "
              << std::get<4>(args4) << ", "
              << std::get<5>(args4) << std::endl;

    // 参数不完整的解析：运行时报错实际上还是会将正确的参数解析出来
    auto byte_buffer_3 = codec.Serialize(1, (uint32_t)-1, (uint64_t)2, "hello");
    std::tuple<int, uint32_t, uint64_t, std::string, int64_t, uint64_t> args3;
    auto err3 = codec.DeserializeWithTuple(byte_buffer_3, args3);
    if (err3.has_value())
    {
        std::cerr << "Deserialize failed: " << err3->What() << std::endl;
    }

    // 空字符串
    auto byte_buffer_6 = codec.Serialize("", 1);
    std::tuple<std::string, int> args6;
    auto err6 = codec.DeserializeWithTuple(byte_buffer_6, args6);
    if (err6.has_value())
    {
        std::cerr << "Deserialize failed: " << err6->What() << std::endl;
        return -1;
    }
    std::cout << "Deserialized values: "
              << std::get<0>(args6) << ", "
              << std::get<1>(args6) << std::endl;

    // 不支持的类型（无法通过编译）
    // auto byte_buffer_5 = codec.Serialize(1, (uint32_t)-1, (uint64_t)2, "hello", (int64_t)-100, (uint64_t)100, 123.456);
    // std::tuple<int, uint32_t, uint64_t, std::string, int64_t, uint64_t> args5;
    // auto err5 = codec.DeserializeWithArgs(byte_buffer_5, args5);
    // if (err5.has_value())
    // {
    //     std::cerr << "Deserialize failed: " << err5->What() << std::endl;
    // }

    // 最推荐的使用方式
    typedef std::tuple<int, uint32_t, uint64_t, std::string, int64_t, uint64_t> MyProtocol;

    MyProtocol my_protocol = std::make_tuple(1, (uint32_t)-1, (uint64_t)2, "hello", (int64_t)-100, (uint64_t)100);
    auto byte_buffer_7 = codec.SerializeWithTuple(my_protocol);
    MyProtocol my_protocol2;
    auto err7 = codec.DeserializeWithTuple(byte_buffer_7, my_protocol2);
    if (err7.has_value())
    {
        std::cerr << "Deserialize failed: " << err7->What() << std::endl;
        return -1;
    }
    std::cout << "Deserialized values: "
              << std::get<0>(my_protocol2) << ", "
              << std::get<1>(my_protocol2) << ", "
              << std::get<2>(my_protocol2) << ", "
              << std::get<3>(my_protocol2) << ", "
              << std::get<4>(my_protocol2) << ", "
              << std::get<5>(my_protocol2) << std::endl;
    
    // 支持结构体，注意这里结构体是字节流对象，要注意跨平台和字节序的问题，以及最好不要使用复杂类
    struct MyStruct
    {
        int a;
        uint32_t b;
        uint64_t c;
        char d[12];
        // std::string d; // 错误的，也许可以看到结果，但是事实上把指针copy过去了
        int64_t e;
        uint64_t f;
    };

    MyStruct my_struct = {1, (uint32_t)-1, (uint64_t)2, "hello", (int64_t)-100, (uint64_t)100};
    std::tuple<MyStruct> my_struct_tuple = std::make_tuple(my_struct);
    auto byte_buffer_8 = codec.SerializeWithTuple(my_struct_tuple);
    std::tuple<MyStruct> my_struct2;
    auto err8 = codec.DeserializeWithTuple(byte_buffer_8, my_struct2);
    if (err8.has_value())
    {
        std::cerr << "Deserialize failed（structure）: " << err8->What() << std::endl;
        return -1;
    }
    std::cout << "Deserialized values: "
                << std::get<0>(my_struct2).a << ", "
                << std::get<0>(my_struct2).b << ", "
                << std::get<0>(my_struct2).c << ", "
                << std::get<0>(my_struct2).d << ", "
                << std::get<0>(my_struct2).e << ", "
                << std::get<0>(my_struct2).f << std::endl;
}