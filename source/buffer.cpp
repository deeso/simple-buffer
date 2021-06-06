//
// Created by apridgen on 6/5/21.
//

#include "buffer.h"
#include "util.h"
#include <cstddef>
#include <cstdint>
#include <optional>
#include "clooper.h"

namespace simple_buffer {
    auto LOOPER = std::make_unique<CLooper>();

    bool Buffer::realloc_buffer(size_t how_much_more) {
        if (!LOOPER->running())
            LOOPER->run();

        unsigned int factor = 0;
        // find a good size and allocate the new buffer
        for (; (_byte_size << factor) < (how_much_more + _byte_size); factor++) {}
        auto byte_size = _byte_size << factor;

        if (_max_size < byte_size ) {
            return false;
        }

        auto data = new char[byte_size];

        // sanity checks
        assert(data!= nullptr);
        assert(_byte_size < byte_size);

        // copy buffers
        auto diff = byte_size - _byte_size;

        auto dispatcher = LOOPER->getDispatcher();
        auto this_byte_size = this->_byte_size;
        auto this_data = this->_data;
        auto start_zero_set = data + this_byte_size;

        {
            std::lock_guard<std::mutex> guard_read(_buffer_readable);
            std::lock_guard<std::mutex> guard_write(_buffer_writeable);
            std::vector<std::thread> copier;
            copier.emplace_back([start_zero_set, diff]() {
                memset(start_zero_set, 0, diff);
            });

            copier.emplace_back([this_data, data, this_byte_size]() {
                memcpy(this_data, data, this_byte_size);
            });
            // wait for these critical functions to happen
            std::for_each(copier.begin(), copier.end(), [](std::thread &t){
                t.join();
            });

            // buffer is prepped and it can be reassigned safely
            _data = data;
            _byte_size = byte_size;

        }

        // async destroy using clooper (dont need to wait for it)
        auto const destroy_old_buffer = [this_data, data, this_byte_size]() {

            memset(this_data, 0, this_byte_size);
            delete[] this_data;
        };
        dispatcher->post(std::move(destroy_old_buffer));
        // update / clear memory
        return true;
    }

    std::optional<std::size_t>  Buffer::put(uint8_t x) {
        auto z = this->write((char *)&x, 1);;
        return (z!=BAD_READ && z != 0) ? std::optional<size_t>{z} : std::nullopt;
    }
    std::optional<std::size_t>  Buffer::put(uint8_t x, size_t pos) {
        auto z = this->write_at((char *)&x, 1, pos);
        return (z!=BAD_READ && z != 0) ? std::optional<size_t>{z} : std::nullopt;
    }
//    std::size_t Buffer::put(uint16_t x, bool lendian=false) {
//        char buf[2] = {0};
//        if (lendian) {
//            convert((char *)&x, buf, 2);
//        }
//        return this->write(buf, 2);
//    }

    std::optional<uint8_t> Buffer::peak_uint8() {
        return this->peak_uint8(_read_pos);
    }
    std::optional<uint8_t> Buffer::peak_uint8(size_t pos) {
        // TODO return boost::optional
        uint8_t x = 0;
        auto sz = BAD_READ;
        if (this->inbounds(pos))
            sz = this->read_at((char *)&x, 1, pos);
        return sz > 0 && sz != BAD_READ ? std::optional<uint8_t>{x} : std::nullopt;
    }

//    uint16_t Buffer::peak_short(bool lendian=false) {
//        return this->peak_short(_read_pos, lendian);
//    }
//    uint16_t Buffer::peak_short(size_t pos, bool lendian=false) {
//        uint16_t x = 1
//        if (lendian) {
//            x = 0x0100;
//        }
//        return x;
//    }
}