
SRC_DIR=".."
DST_DIR="../rpc/proto"
SRC_DIR="../message"

if [ !  -d "../proto"  ]
then
    mkdir ../proto
fi

protoc -I=${SRC_DIR} --cpp_out=${DST_DIR} ${SRC_DIR}/AddAndStr.proto
protoc -I=${SRC_DIR} --cpp_out=${DST_DIR} ${SRC_DIR}/c2s.proto
protoc -I=${SRC_DIR} --cpp_out=${DST_DIR} ${SRC_DIR}/s2c.proto


cd .. && cmake .

# (make -j4 >right.log)>&error.log
make -j4