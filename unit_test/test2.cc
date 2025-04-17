#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>

#include <bbt/rpc/detail/RpcCodec.hpp>

using namespace bbt::rpc;

BOOST_AUTO_TEST_SUITE()

BOOST_AUTO_TEST_CASE(t_codec)
{
    // 对基础类型的序列化和反序列化
    std::tuple<int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t> tuple;
    auto buffer = codec::SerializeWithTuple(
        std::make_tuple<int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t>(1, 2, 3, 4, 5, 6, 7, 8));

    BOOST_ASSERT(codec::DeserializeWithTuple(buffer, tuple) == std::nullopt);
    BOOST_CHECK_EQUAL(std::get<0>(tuple), 1);
    BOOST_CHECK_EQUAL(std::get<1>(tuple), 2);
    BOOST_CHECK_EQUAL(std::get<2>(tuple), 3);
    BOOST_CHECK_EQUAL(std::get<3>(tuple), 4);
    BOOST_CHECK_EQUAL(std::get<4>(tuple), 5);
    BOOST_CHECK_EQUAL(std::get<5>(tuple), 6);
    BOOST_CHECK_EQUAL(std::get<6>(tuple), 7);
    BOOST_CHECK_EQUAL(std::get<7>(tuple), 8);

    // 对字符串类型的序列化和反序列化
    std::tuple<std::string> tuple2;
    auto buffer2 = codec::SerializeWithTuple(std::make_tuple<std::string>("hello world"));
    BOOST_ASSERT(codec::DeserializeWithTuple(buffer2, tuple2) == std::nullopt);
    BOOST_CHECK_EQUAL(std::get<0>(tuple2), "hello world");

    // 对pod类型的序列化和反序列化
    struct MyStruct
    {
        int a;
        uint32_t b;
        uint64_t c;
        char d[12];
        int64_t e;
        uint64_t f;
    };

    std::tuple<MyStruct> my_struct_tuple = std::make_tuple(MyStruct{1, (uint32_t)-1, (uint64_t)2, "hello", (int64_t)-100, (uint64_t)100});
    auto buffer3 = codec::SerializeWithTuple(my_struct_tuple);
    std::tuple<MyStruct> my_struct_tuple2;
    BOOST_ASSERT(codec::DeserializeWithTuple(buffer3, my_struct_tuple2) == std::nullopt);
    BOOST_CHECK_EQUAL(std::get<0>(my_struct_tuple2).a, 1);
    BOOST_CHECK_EQUAL(std::get<0>(my_struct_tuple2).b, -1);
    BOOST_CHECK_EQUAL(std::get<0>(my_struct_tuple2).c, 2);
    BOOST_CHECK_EQUAL(std::get<0>(my_struct_tuple2).d, "hello");
    BOOST_CHECK_EQUAL(std::get<0>(my_struct_tuple2).e, -100);
    BOOST_CHECK_EQUAL(std::get<0>(my_struct_tuple2).f, 100);
}

BOOST_AUTO_TEST_SUITE_END()