#pragma once

#include <cstdio>
#include <cstring>
#include <iostream>
#include <sstream>

class BufferStream {
public:
    static uint64_t BitsReverse(uint64_t n, size_t len) {
        // reverses binary representation of first len bits of integer
        uint64_t rev = 0;

        for (size_t i = 0; i < len; ++i) {
            rev <<= 1;

            if ((n & 1) == 1) {
                rev ^= 1;
            }

            n >>= 1;
        }

        return rev;
    }

    virtual void BufferFlush() = 0;

protected:
    size_t count_ = 0;
    uint64_t buffer_ = 0;
};

class OutputBufferStream : public BufferStream {
public:
    explicit OutputBufferStream(std::ostream& stream) : stream_(stream) {
    }

    void Push() {
        // Pushes elements to stream from buffer
        while (count_ >= 8) {
            unsigned char out = (buffer_ & ((1ll << 8) - 1));
            out = BitsReverse(out, 8);

            stream_.put(out);

            buffer_ >>= 8;
            count_ -= 8;
        }
    }

    void WriteBit(uint64_t bit) {
        // Writes bit
        buffer_ |= (bit << count_);

        count_ += 1;

        Push();
    }

    void Write(uint64_t value, size_t output_len, bool big_endian = true) {
        // Writes integer to buffer
        if (big_endian) {
            value = BitsReverse(value, output_len);
        }

        buffer_ |= (value << count_);

        count_ += output_len;

        Push();
    }

    void BufferFlush() override {
        while (count_ != 0) {
            WriteBit(0);
        }
    }

private:
    std::ostream& stream_;
};

template <typename reading_blocks = uint64_t>
class InputBufferStream : public BufferStream {
public:
    explicit InputBufferStream(std::istream& stream) : stream_(stream) {
    }

    bool Stop() {
        // checks whenever we reached EOF
        return stopped_;
    }

    void Pull() {
        // Pulls bits from stream to buffer
        int new_byte = stream_.get();
        if (new_byte == std::char_traits<char>::eof()) {
            stopped_ = true;
            return;
        }

        unsigned char current = static_cast<unsigned char>(new_byte);
        current = BitsReverse(current, 8);

        buffer_ |= (current << count_);
        count_ += 8;
    }

    reading_blocks GetBit() {
        // Gets single bit from buffer
        reading_blocks result;
        while (count_ < 1 && !stopped_) {
            Pull();
            if (stopped_) {
                return 0;
            }
        }

        result = (buffer_ & 1);

        buffer_ >>= 1;
        count_ -= 1;

        return result;
    }

    reading_blocks Get(size_t input_len, bool big_endian = true) {
        // Gets integer from buffer
        reading_blocks result = 0;

        while (count_ < input_len) {
            Pull();
            if (stopped_) {
                return 0;
            }
        }

        result = (buffer_ & ((1ll << input_len) - 1));
        buffer_ >>= input_len;
        count_ -= input_len;

        if (!big_endian) {
            return BitsReverse(result, input_len);
        } else {
            return result;
        }
    }

    void BufferFlush() override {
        buffer_ >>= 8;
        count_ = 0;
    }

private:
    bool stopped_ = false;
    std::istream& stream_;
};
