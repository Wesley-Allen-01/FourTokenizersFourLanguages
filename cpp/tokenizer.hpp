#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include <map>
#include <vector>
#include <string>
#include <cstdint>
#include <utility>
#include <functional>
#include <unordered_map>

struct PairHash {
    std::size_t operator()(const std::pair<int, int>& p) const {
        return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 1);
    }
};

class Tokenizer {
public:
    Tokenizer();
    
    void Train(const std::string& text, int vocab_size = 3000);
    

    std::vector<int> Encode(const std::string& text);
    
    std::string Decode(const std::vector<int>& token_ids);
    
    std::vector<std::string> IdsToTokens(const std::vector<int>& token_ids);

private:
    std::unordered_map<int, std::vector<uint8_t>> vocab_;
    std::unordered_map<std::pair<int, int>, int, PairHash> merges_;
    

    std::unordered_map<std::pair<int, int>, int, PairHash> GetStats(
        const std::vector<std::vector<int>>& pretokens);
    
    std::pair<int, int> GetMaxPair(
        const std::unordered_map<std::pair<int, int>, int, PairHash>& stats);
    
    std::vector<int> Merge(
        const std::vector<int>& tokens,
        std::pair<int, int> pair,
        int idx);

    void MergeInPlace(
        std::vector<int>& tokens,
        std::pair<int, int> pair,
        int idx);
    
    void MergeAll(
        std::vector<std::vector<int>>& pretokens,
        std::pair<int, int> pair,
        int idx);
    
    std::vector<std::pair<std::pair<int, int>, int>> GetOrderedMerges();
};

#endif // TOKENIZER_HPP
