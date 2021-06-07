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

    bool Buffer::realloc_buffer(size_t new_size) {
        if (!LOOPER->running())
            LOOPER->run();

        unsigned int factor = 0;
        // find a good size and allocate the new buffer
        for (; (_byte_size << factor) < (new_size ); factor++) {}
        auto byte_size = _byte_size << factor;

        if (_max_size < byte_size ) return false;
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
        memset(data, 0, byte_size);

        {
            std::lock_guard<std::mutex> guard_read(_buffer_readable);
            std::lock_guard<std::mutex> guard_write(_buffer_writeable);
            std::vector<std::thread> copier;
            copier.emplace_back([start_zero_set, diff]() {
                memset(start_zero_set, 0, diff);
            });

            copier.emplace_back([this_data, data, this_byte_size]() {
                memcpy(data, this_data, this_byte_size);
            });
            // wait for these critical functions to happen
            std::for_each(copier.begin(), copier.end(), [](std::thread &t){
                t.join();
            });

            // buffer is prepped and it can be reassigned safely
            this->_data = data;
            this->_byte_size = byte_size;

        }

        // async destroy using clooper (dont need to wait for it)
        auto const destroy_old_buffer = [this_data, this_byte_size]() {
            memset(this_data, 0, this_byte_size);
            delete[] this_data;
        };
        dispatcher->post(std::move(destroy_old_buffer));
        // update / clear memory
        return true;
    }

}