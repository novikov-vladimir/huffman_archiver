#include "haffman_encoding.h"
#include "haffman_trie.h"
#include "iostream_utils.h"
#include <iostream>
#include <map>
#include <queue>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>

struct LetterNode {
    size_t sz;
    TrieNode<uint16_t>* ptr;
    uint16_t letter;
};

class Compare {
public:
    bool operator()(const LetterNode& a, const LetterNode& b) {
        return std::make_pair(a.sz, a.letter) > std::make_pair(b.sz, b.letter);
    }
};

TrieNode<uint16_t>* HaffmanTrie(const std::map<uint16_t, size_t>& alphabet) {
    // We use 8 bit integers to represent chars in count array

    Heap<LetterNode, Compare> q;

    for (auto [encoding, count] : alphabet) {
        q.Push({count, new TrieNode(encoding), encoding});
    }

    while (q.Size() > 1) {
        auto v = q.Pop();
        auto u = q.Pop();

        TrieNode<uint16_t>* node = (new TrieNode<uint16_t>(v.ptr, u.ptr));
        // creates new node with u and v begin left child and right child accordingly
        LetterNode my_pair = {u.sz + v.sz, node, std::min(u.letter, v.letter)};
        q.Push(std::move(my_pair));
        // unite sizes and move pointer to new node to queue
    }

    return q.Pop().ptr;
}

void PushEncoding(OutputBufferStream& buffer, const std::string& code) {
    for (auto current_bit : code) {
        buffer.Write(current_bit - '0', 1);
    }
}

void HaffmanFileEncoding(const std::map<uint16_t, size_t>& alphabet, std::string archive_name,
                         OutputBufferStream& buffer, bool last = false) {
    std::unique_ptr<TrieNode<uint16_t>> root = std::unique_ptr<TrieNode<uint16_t>>(HaffmanTrie(alphabet));

    std::map<uint16_t, std::string> letters_encoding = TrieNode<uint16_t>::GetAlphabet(root.get());

    buffer.Write(alphabet.size(), 9);

    std::map<size_t, size_t> lengths_count;
    std::map<size_t, size_t> count;

    std::vector<std::pair<std::string, uint16_t>> letters_ordering;
    for (auto [symbol, coding] : letters_encoding) {
        ++lengths_count[coding.size()];
        letters_ordering.push_back({coding, symbol});
    }

    // reordering of letters
    sort(letters_ordering.begin(), letters_ordering.end(),
         [](const std::pair<std::string, uint16_t>& a, const std::pair<std::string, uint16_t>& b) {
             if (a.first.size() > b.first.size()) {
                 return false;
             }
             if (b.first.size() > a.first.size()) {
                 return true;
             }

             return a.second < b.second;
         });

    std::string path = "0";

    // construction of letter codes
    count = lengths_count;
    for (size_t i = 0; i < alphabet.size();) {
        if (count[path.size()] > 0) {
            --count[path.size()];
            letters_encoding[letters_ordering[i].second] = path;
            letters_ordering[i].first = path;

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

    for (const auto& [symbol, coding] : letters_ordering) {
        buffer.Write(coding, 9, true);
    }

    size_t current_count = 0;
    for (size_t len = 1; len <= letters_encoding.size(); ++len) {
        buffer.Write(lengths_count[len], 9, true);

        current_count += lengths_count[len];
        if (current_count == alphabet.size()) {
            break;
        }
    }

    for (auto symb : archive_name) {
        PushEncoding(buffer, letters_encoding[symb]);
    }

    PushEncoding(buffer, letters_encoding[FILENAME_END]);

    uint16_t current_block = 0;

    // coding file text
    std::ifstream file(archive_name, std::ios::in | std::ios_base::binary);
    file.tie(0);
    InputBufferStream reader(file);
    while (!reader.Stop()) {
        current_block = reader.Get(8, false);

        if (!reader.Stop()) {
            PushEncoding(buffer, letters_encoding[current_block]);
        }
    }

    if (last) {
        PushEncoding(buffer, letters_encoding[ARCHIVE_END]);
    } else {
        PushEncoding(buffer, letters_encoding[ONE_MORE_FILE]);
    }
}

std::map<uint16_t, size_t> AlphabetCount(std::istream& input, const std::string& name) {
    std::map<uint16_t, size_t> result;
    for (auto c : name) {
        result[c]++;
    }

    while (input.peek() != EOF) {
        result[static_cast<unsigned char>(input.get())]++;
    }

    return result;
}
