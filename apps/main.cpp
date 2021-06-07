#include "options.h"
#include "buffer.h"
#include <iostream>
#include <sstream>

int main(int argc, char** argv) {
    Options options;
    options.parse(argc, argv);
    std::string test("AAAAAAAAAAAAAAAAAAAAAA");


    simple_buffer::Buffer buf(10);
    simple_buffer::Buffer buf2(10, simple_buffer::BufferPolicy::Dynamic);
    buf2.put('A');
    buf2.put('A');
    buf2.put('A');
    buf2.put('A');
    std::cout << "BUF2 POS = " << buf2.get_write_pos() << std::endl;
    std::stringstream ss;
    for (size_t ss_s = 0; ss_s < buf2.get_max_size(); ss_s++ ) {
        ss << "B";
    }
    buf2.set_max_size(buf2.get_max_size() << 2);
    std::string test2 = ss.str();
    auto test2_ch = test2.c_str();

    auto buf_write_result = buf2.write_at(test2_ch, test2.size(), 7);
    buf_write_result = buf2.write_at(test2_ch, test2.size(), 7);
    uint32_t tes;
    buf2.read_at((char *)&tes, 4, 7);
    std::cout << "Test string size = " << test2.size() << " Buf2 size = " << buf2.get_max_size() << std::endl;
    std::cout << "Test value = " << test2_ch[20] << " Buf2 size = " << test2.size() << std::endl;
    std::cout << "Test result value = " << std::hex << tes << std::endl;
    std::cout << "Test value = " << (uint16_t)buf2.peek<uint8_t>(8).value() << " Buf2 size = " << test2.size() << std::endl;

    buf.put('A');
    buf.put('A', 1);
    buf.put(1, 3);
    buf.put('w', 8);
    auto peek_x = buf.peek<uint8_t>(0);


    auto w = buf.peek<uint8_t>(8);
    auto x = buf.peek<uint8_t>(0);
    char y, z, zz;
    buf.read(&y, 1);
    auto good_read = buf.read(&z, 1);
    auto bad_read = buf.read_at(&zz, 1, 20);


    std::cout << "Buffer size: " << buf.get_size() << std::endl;
    std::cout << "Buffer read_pos: " << buf.get_read_pos() << std::endl;
    std::cout << "Buffer write_pos: " << buf.get_write_pos() << std::endl;
    std::cout << "Buffer[0] has value " << buf.peek<uint8_t>().has_value() << std::endl;
    std::cout << "Buffer[0] has value peek<uint32_t>" << std::hex << buf.peek<uint32_t>(0).value() << std::endl;
    std::cout << "read, Buffer[0]: " << y << std::endl;
    std::cout << "read, Buffer[1] == 0 " << (z == 0) << " good read: " << good_read << std::endl;
    std::cout << "read, Buffer[20] == ?? " << zz << " bad read: " << bad_read << std::endl;
    std::cout << "Buffer[1]: " << buf.peek<uint8_t>().has_value() << std::endl;
    std::cout << "Buffer[0] has value: " << x.has_value() << " value is: "<< (x.has_value() ? x.value() : -1) << std::endl;
    std::cout << "Buffer[7] has value: " << w.has_value() << " value is: "<< (w.has_value() ? w.value() : -1) << std::endl;


    for (int i = 0; i < 20; i++) {
        auto te = buf2.peek<uint8_t>(i);
        std::cout << "Buffer2[" << i << "] = " << std::hex << te.value() << std::endl;
    }
}