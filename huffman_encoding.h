#pragma once

#include "heap.h"
#include "haffman_trie.h"
#include "iostream_utils.h"
#include <string>
#include <sstream>
#include <fstream>

TrieNode<uint16_t>* HaffmanTrie(const std::map<uint16_t, size_t>& alphabet);

void PushEncoding(OutputBufferStream& buffer, const std::string& code);

void HaffmanFileEncoding(const std::map<uint16_t, size_t>& alphabet, std::string archive_name,
                         OutputBufferStream& buffer, bool last);

template <typename terminal_type>
void Add(TrieNode<terminal_type>* ptr, std::string str, uint16_t decode) {  // inserts string into the trie
    if (str.empty()) {
        ptr->SetTerminal(decode);
        return;
    }

    if (str[0] == '0') {
        str.erase(str.begin());

        if (!ptr->GetZero()) {
            ptr->SetZero();
        }
        Add(ptr->GetZero(), str, decode);
    } else {
        str.erase(str.begin());

        if (!ptr->GetOne()) {
            ptr->SetOne();
        }
        Add(ptr->GetOne(), str, decode);
    }
}

template <typename reading_blocks>
uint16_t NextCharacter(InputBufferStream<reading_blocks>& reader,
                       TrieNode<uint16_t>* ptr) {  // Decodes next char from archive
    while (ptr->GetTerminal() == TERMINAL_END) {
        if (reader.Get(1)) {
            ptr = TrieNode<uint16_t>::DescendOne(ptr);
        } else {
            ptr = TrieNode<uint16_t>::DescendZero(ptr);
        }
    }

    return ptr->GetTerminal();
}

template <typename reading_blocks>
std::string HaffmanFileDecoding(InputBufferStream<reading_blocks>& reader, bool& lastfile) {  // returns file name
    uint16_t symbols_count = reader.Get(9, false);
    std::vector<uint16_t> letters_ordering;

    for (size_t i = 0; i < symbols_count; ++i) {
        letters_ordering.push_back(reader.Get(9, false));
    }

    std::map<size_t, size_t> count;

    size_t current_count = 0;
    // reads character counts
    for (size_t i = 1; i <= symbols_count; ++i) {
        count[i] = reader.Get(9, false);

        current_count += count[i];
        if (current_count == symbols_count) {
            break;
        }
    }

    std::map<std::string, uint16_t> letters_encoding;

    // builds trie
    std::unique_ptr<TrieNode<uint16_t>> root =
        std::unique_ptr<TrieNode<uint16_t>>(new TrieNode<uint16_t>(TERMINAL_END));
    std::string path = "0";
    for (size_t i = 0; i < symbols_count;) {
        if (count[path.size()] > 0) {
            --count[path.size()];
            letters_encoding[path] = letters_ordering[i];
            Add(root.get(), path, letters_ordering[i]);
            while (!path.empty() && path.back() == '1') {
                path.pop_back();
            }

            if (!path.empty()) {
                path.pop_back();
                path.push_back('1');
            }
            ++i;
        } else {
            path.push_back('0');
        }
    }

    uint16_t scan = NextCharacter(reader, root.get());
    std::string name;

    // gets file name
    while (scan != FILENAME_END) {
        name.push_back(scan);
        scan = NextCharacter(reader, root.get());
    }

    const char* file_name = name.c_str();
    std::ofstream ofs(file_name, std::ios::out | std::ios_base::binary);

    OutputBufferStream printer(ofs);
    scan = NextCharacter(reader, root.get());

    // decodes text of the file
    while (scan != ARCHIVE_END && scan != ONE_MORE_FILE) {
        printer.Write(scan, 8, true);
        scan = NextCharacter(reader, root.get());
    }

    if (scan == ONE_MORE_FILE) {
        lastfile = false;
        return name;
    } else {
        lastfile = true;
        return name;
    }
}

std::map<uint16_t, size_t> AlphabetCount(std::istream& input, const std::string& name);
