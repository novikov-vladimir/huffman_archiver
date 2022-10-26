#include "haffman_trie.h"
#include "iostream_utils.h"
#include "heap.h"
#include "haffman_encoding.h"
#include <fstream>

int main(int argc, char** argv) {
    std::ios_base::sync_with_stdio(false);
    std::string cmd = argv[1];

    if (cmd == "-c") {
        const char* archive_name = argv[2];
        std::ofstream ofs(archive_name, std::ios::out | std::ios_base::binary);
        OutputBufferStream out(ofs);

        for (size_t i = 3; i < argc; ++i) {
            const char* name = argv[i];
            std::map<uint16_t, size_t> calc;

            {
                std::ifstream file(name, std::ios::in | std::ios_base::binary);
                calc = AlphabetCount(file, name);

                ++calc[FILENAME_END];
                ++calc[ONE_MORE_FILE];
                ++calc[ARCHIVE_END];
            }

            HaffmanFileEncoding(calc, name, out, (i + 1 == argc));
        }

        out.BufferFlush();
    } else if (cmd == "-d") {
        const char* archive_name = argv[2];
        std::ifstream input(archive_name, std::ios::in | std::ios_base::binary);
        InputBufferStream in(input);

        bool stop = false;
        while (!stop) {
            HaffmanFileDecoding(in, stop);
        }

        in.BufferFlush();
    }

    return 0;
}
