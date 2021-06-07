//
// Created by apridgen on 6/5/21.
//

#ifndef SIMPLE_BUFFER_BUFFER_H
#define SIMPLE_BUFFER_BUFFER_H
#include <cstddef>
#include <cstdint>
#include <assert.h>
#include <cstdio>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <boost/optional.hpp>
#include <thread>
#include <mutex>

namespace simple_buffer {
    enum class BufferPolicy { Static, Dynamic };
    const size_t INITIAL_MAX_SIZE = 1024 << 10;  // 4MB

class Buffer {

    private:
        char *_data;
        size_t _max_size;
        size_t _byte_size;
        size_t _read_pos;
        size_t _write_pos;
        BufferPolicy _buffer_type; // (BufferPolicy::Static);
        std::mutex _buffer_readable;
        std::mutex _buffer_writeable;

        void deinit() {
            _read_pos = (std::size_t) BAD_READ;
            _write_pos = (std::size_t) BAD_READ;
            _byte_size = 0;
            _data = nullptr;
        }

    public:
        const static size_t BAD_READ = (size_t) -1;
        explicit Buffer(size_t size, BufferPolicy policy=BufferPolicy::Static, size_t max_size = INITIAL_MAX_SIZE)
                : _byte_size(size), _read_pos(0), _write_pos(0), _buffer_type(policy),
                _buffer_readable(), _buffer_writeable(), _max_size(max_size){

            if (_byte_size > 0) _data = new char[_byte_size];
            else _data = nullptr;
            assert(_data != nullptr);

            if(_data != nullptr) clean();
            assert(_write_pos == 0);
            assert(_read_pos == 0);
        }
        Buffer(const Buffer&& other) noexcept
               : _data(other._data),
                 _read_pos(other._read_pos), _write_pos(other._write_pos),
                 _byte_size(other._byte_size), _buffer_type(other._buffer_type),
                 _max_size(other._max_size),
                 _buffer_readable(), _buffer_writeable()
        {}

        Buffer(const Buffer& other)
                : _read_pos(other._read_pos),
                  _write_pos(other._write_pos),
                  _byte_size(other._byte_size),
                  _buffer_type(other._buffer_type),
                  _max_size(other._max_size),
                  _buffer_readable(), _buffer_writeable()
        {
            _data = new char[_byte_size];
            memcpy(_data, other._data, _byte_size);
            assert(_write_pos == 0);
            assert(_read_pos == 0);
        }

        ~Buffer() {
            delete [] _data;
            deinit();
        }

        void swap(Buffer& rhs) {
            std::swap(_data, rhs._data);
            std::swap(_write_pos, rhs._write_pos);
            std::swap(_read_pos, rhs._read_pos);
            std::swap(_buffer_type, rhs._buffer_type);
            std::swap(_max_size, rhs._max_size);
        }

        const char *begin() const {
            return _data;
        }

        char *begin() {
            return _data;
        }

        const char *end() const {
            return _data + _byte_size;
        }

        char *end() {
            return _data + _byte_size;
        }

        bool safe_read(size_t pos) const {
            return pos < _write_pos;
        }

        bool inbounds(size_t pos) const {
            // check that end value falls in the buffer
            return (pos < this->_byte_size);
        }

        bool inbounds(size_t pos, size_t len) const {
            // check that end value falls in the buffer
            return inbounds(pos+len) && !int_overflow(pos, len);
        }

        static bool int_overflow(size_t start, size_t len) {
            return (len != 0) && (start + len <= start);
        }

        // read bytes from buffer from current read position
        // to len || this->length()
        // allocate buffer and return if its possible to read
        size_t read(char *buf, size_t len) {
            // Lock the read on potentially shared pointer
            auto bytes_read = this->read_at(buf, len, this->_read_pos);
                if (bytes_read > 0 && bytes_read != BAD_READ){
                    std::lock_guard<std::mutex> guard(_buffer_readable);
                    this->_read_pos += bytes_read;
                }
            return bytes_read;
        }

        size_t read_at(char *buf, size_t len, size_t pos = 0) {
            auto _bytes_read = BAD_READ;
            // handle int_overflow case explicitly
            if (simple_buffer::Buffer::int_overflow(pos, len) || len == 0){}
            else if (this->inbounds(pos) && this->inbounds(pos, len))
                _bytes_read = len;
            else if (this->inbounds(pos))
                _bytes_read = (pos+len) - (pos+this->_byte_size);// TODO Lock here
            //if (!safe_read(pos) || !safe_read(pos+len)){}

            if (_bytes_read != BAD_READ && _bytes_read > 0 ){
                memcpy(buf, this->_data+pos, _bytes_read);
            }
            return _bytes_read;
        }

        size_t write(const char *buf, size_t len) {
            auto bytes_written = BAD_READ;
            bytes_written = this->write_at(buf, len, this->_write_pos);
            if (bytes_written != BAD_READ){
                std::lock_guard<std::mutex> guard_write(_buffer_writeable);
                this->_write_pos += bytes_written;
            }
            return bytes_written;
        }
        bool realloc_buffer(size_t new_size);
        bool need_realloc(size_t pos, size_t len) {
            return !int_overflow(pos, len) &&
                   !inbounds(pos+len) &&
                   _buffer_type == BufferPolicy::Dynamic;
        }



        size_t write_at(const char *buf, size_t len, size_t pos) {

            auto update_ptr = pos == _write_pos;
            auto bytes_written = BAD_READ;
            if (need_realloc(pos, len)) {
                auto result = realloc_buffer(pos+len);
                assert(_data!= nullptr);
                // realloc failed, so not attempting to write the data
                if (!result) return bytes_written;
                assert(pos+len < _byte_size);
                std::lock_guard<std::mutex> guard(_buffer_writeable);
            }
            // handle int_overflow case explicitly
            if (simple_buffer::Buffer::int_overflow(pos, len) || len == 0) {}
            else if (this->inbounds(pos) && this->inbounds(pos, len)) {
                bytes_written = len;
                std::lock_guard<std::mutex> guard(_buffer_writeable);
                memcpy(this->_data+pos, buf, bytes_written);
            }
            else if (this->inbounds(pos)) {
                bytes_written = (pos + len) - (pos + this->_byte_size);
                std::lock_guard<std::mutex> guard(_buffer_writeable);
                memcpy(this->_data+pos, buf, bytes_written);
            }

            return bytes_written;
        }

        void set_dynamic() {_buffer_type = BufferPolicy::Dynamic;}
        void set_static() {_buffer_type = BufferPolicy::Static;}

        size_t get_size() const { return  _byte_size;}
        size_t get_max_size() const { return  _max_size;}

        void set_max_size() { set_max_size(INITIAL_MAX_SIZE);}
        void set_max_size(size_t max_size) { _max_size = max_size;}

        size_t get_read_pos() const { return  _read_pos;}
        size_t get_write_pos() const { return  _write_pos;}
        void clean() {
            for (auto i = 0; i < this->_byte_size; i++) *(this->_data+i) = 0;
        }

        template <typename PUT_T>
        std::optional<std::size_t> put(PUT_T x){
            auto z = this->write((char *)&x, sizeof(x));;
            return (z!=BAD_READ && z != 0) ? std::optional<size_t>{z} : std::nullopt;
        }

        template <typename PUT_T>
        std::optional<std::size_t> put(PUT_T x, size_t pos){
            auto z = this->write_at((char *)&x, sizeof(x), pos);
            return (z!=BAD_READ && z != 0) ? std::optional<size_t>{z} : std::nullopt;
        }

        template<typename PEEK_T>
        std::optional<PEEK_T> peek() {
            return this->peek<PEEK_T>(_read_pos);
        }
        template<typename PEEK_T>
        std::optional<PEEK_T> peek(size_t pos){
            PEEK_T x;
            auto sz = this->read_at((char *)&x, sizeof (PEEK_T), pos);
            return sz != BAD_READ ? std::optional<PEEK_T>{x} : std::nullopt;
        }


};


}

#endif //SIMPLE_BUFFER_BUFFER_H
