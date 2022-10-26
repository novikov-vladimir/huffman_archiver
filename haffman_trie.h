#pragma once

#include <memory>
#include <map>
#include <string>

const uint16_t FILENAME_END = 256;
const uint16_t ONE_MORE_FILE = 257;
const uint16_t ARCHIVE_END = 258;
const uint16_t TERMINAL_END = 259;

template <typename terminal_type = bool>
class TrieNode {
public:
    explicit TrieNode(terminal_type terminal) : zero_edge_(nullptr), one_edge_(nullptr), terminal_(terminal) {
    }

    TrieNode(TrieNode<terminal_type>* zero_edge, TrieNode<terminal_type>* one_edge)
        : zero_edge_(std::unique_ptr<TrieNode<terminal_type>>(zero_edge)),
          one_edge_(std::unique_ptr<TrieNode<terminal_type>>(one_edge)) {
    }

    static TrieNode<terminal_type>* DescendZero(const TrieNode<terminal_type>* node) {
        if (node->zero_edge_) {
            return node->zero_edge_.get();
        } else {
            return nullptr;
        }
    }

    static TrieNode<terminal_type>* DescendOne(const TrieNode<terminal_type>* node) {
        if (node->one_edge_) {
            return node->one_edge_.get();
        } else {
            return nullptr;
        }
    }

    static std::map<terminal_type, std::string> GetAlphabet(TrieNode* root) {
        if (root == nullptr) {
            return {};
        }
        if (!(root->one_edge_) && !(root->zero_edge_)) {
            std::map<terminal_type, std::string> new_map;
            new_map[root->terminal_] = "";
            return new_map;
        }

        std::map<terminal_type, std::string> descended_zero = GetAlphabet(DescendZero(root));
        std::map<terminal_type, std::string> descended_one = GetAlphabet(DescendOne(root));
        std::map<terminal_type, std::string> final_alphabet;
        // get alphabets recursively for subtrees

        for (auto& [alpha, coding] : descended_zero) {
            coding.insert(coding.begin(), '0');
        }
        // inserts 0 as the first bit of the encoding

        final_alphabet.swap(descended_zero);
        // saves alphabet from 0 subtree to final alphabet

        for (auto& [alpha, coding] : descended_one) {
            coding.insert(coding.begin(), '1');
            final_alphabet[alpha] = coding;
        }
        // inserts 1 as the first bit of the encoding and saves to final alphabet

        return final_alphabet;
    }

    terminal_type GetTerminal() const {
        return terminal_;
    }

    TrieNode<terminal_type>* GetZero() const {
        return zero_edge_.get();
    }

    TrieNode<terminal_type>* GetOne() const {
        return one_edge_.get();
    }

    void SetTerminal(terminal_type a) {
        terminal_ = a;
    }

    void SetZero() {
        zero_edge_ = std::unique_ptr<TrieNode<terminal_type>>(new TrieNode<terminal_type>(TERMINAL_END));
    }

    void SetOne() {
        one_edge_ = std::unique_ptr<TrieNode<terminal_type>>(new TrieNode<terminal_type>(TERMINAL_END));
    }

private:
    std::unique_ptr<TrieNode<terminal_type>> zero_edge_;
    std::unique_ptr<TrieNode<terminal_type>> one_edge_;

    terminal_type terminal_ = TERMINAL_END;
    // contains value stored in terminal vertex
    // which can either refer to vertex begin a leaf or store some additional information
};
