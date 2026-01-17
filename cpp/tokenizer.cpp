#include "tokenizer.hpp"
#include "pretokenizer.hpp"
#include <algorithm>
#include <stdexcept>
#include <iostream>

using std::vector;
using std::pair;
using std::unordered_map;
using std::string;
using std::sort;
using std::cout;
using std::endl;
using std::move;


Tokenizer::Tokenizer() {
    for (int i = 0; i < 256; ++i) {
        vocab_[i] = vector<uint8_t>{static_cast<uint8_t>(i)};
    }
}

unordered_map<pair<int, int>, int, PairHash> Tokenizer::GetStats(
    const vector<vector<int>>& pretokens) {
    unordered_map<pair<int, int>, int, PairHash> stats;
    for (const auto& pretoken : pretokens) {
        if (pretoken.size() < 2) continue;
        for (size_t i = 0; i < pretoken.size() - 1; ++i) {
            std::pair<int, int> pair(pretoken[i], pretoken[i+1]);
            stats[pair]++;
        }
    }
    return stats;
}

pair<int, int> Tokenizer::GetMaxPair(
    const unordered_map<pair<int, int>, int, PairHash>& stats) {
    int max_count = 0;
    std::pair<int, int> maxPair;

    for (const auto& [pair, count] : stats) {
        if (count > max_count) {
            max_count = count;
            maxPair = pair;
        } 
    }
    return maxPair;
}

vector<int> Tokenizer::Merge(
    const vector<int>& tokens,
    pair<int, int> pair,
    int idx) {
    vector<int> new_pretokens;
    new_pretokens.reserve(tokens.size());
    size_t token_size = tokens.size();
    size_t i = 0;
    while (i < token_size) {
        if (i < token_size-1 && tokens[i] == pair.first && tokens[i+1] == pair.second) {
            new_pretokens.push_back(idx);
            i += 2;
        } else {
            new_pretokens.push_back(tokens[i]);
            i++;
        }
    }
    return new_pretokens;
}

void Tokenizer::MergeInPlace(
    vector<int>& tokens,
    pair<int, int> pair,
    int idx) {
    size_t writeIdx = 0;
    size_t readIdx = 0;
    size_t token_size = tokens.size();
    if (tokens.empty()) return;

    while (readIdx < token_size) {
        if (readIdx < token_size-1 && tokens[readIdx] == pair.first && tokens[readIdx+1] == pair.second) {
            tokens[writeIdx++] = idx;
            readIdx += 2;
        } else {
            tokens[writeIdx++] = tokens[readIdx++];
        }
    }
    tokens.resize(writeIdx);

}

void Tokenizer::MergeAll(
    vector<vector<int>>& pretokens,
    pair<int, int> pair,
    int idx) {
    
    for (auto& pretoken : pretokens) {
        // cout << "Attempting to Merge" << endl;
        // pretoken = Merge(pretoken, pair, idx);
        if (pretoken.empty()) continue;
        MergeInPlace(pretoken, pair, idx);
    }
}

vector<pair<pair<int, int>, int>> Tokenizer::GetOrderedMerges() {
    vector<pair<pair<int, int>, int>> orderedMerges;
    orderedMerges.reserve(merges_.size());

    for (const auto& [pair, idx] : merges_) {
        orderedMerges.emplace_back(pair, idx);
    }

    sort(orderedMerges.begin(), orderedMerges.end(), [](pair<pair<int, int>, int> a, pair<pair<int, int>, int> b) {
        return a.second < b.second;
    });

    return orderedMerges;
}

void Tokenizer::Train(const string& text, int vocab_size) {
    vector<vector<uint8_t>> pretokens = pretokenize_re2(text);
    cout << "Pretokens: " << pretokens.size() << endl;

    vector<vector<int>> ids;
    ids.reserve(pretokens.size());

    for (const auto& pretoken : pretokens) {
        vector<int> temp_token;
        temp_token.reserve(pretoken.size());
        for (const auto& token : pretoken) {
            temp_token.push_back(static_cast<int>(token));
        }
        ids.push_back(temp_token);
    }

    int num_merges = vocab_size - 256;
    int check_interval = vocab_size / 20;

    for (int i = 0; i < num_merges; ++i) {
        if (i % check_interval == 0) {
            cout << "Training... " << (i*100/num_merges) << "%" << endl;
        }

        unordered_map<pair<int, int>, int, PairHash> stats = GetStats(ids);

        pair<int, int> max_pair = GetMaxPair(stats);
        int idx = 256 + i;


        merges_[max_pair] = idx;
        MergeAll(ids, max_pair, idx);
        // cout << "Merged All" << endl;

        vector<uint8_t> new_token = vocab_[max_pair.first];
        const vector<uint8_t>& second_token = vocab_[max_pair.second];

        new_token.reserve(new_token.size() + second_token.size());
        new_token.insert(new_token.end(), second_token.begin(), second_token.end());

        vocab_[idx] = move(new_token);
    }
}

vector<int> Tokenizer::Encode(const string& text) {
    vector<vector<uint8_t>> pretoken_bytes = pretokenize(text);
    vector<vector<int>> pretokens;
    pretokens.reserve(pretoken_bytes.size());

    for (const vector<uint8_t>& pretoken : pretoken_bytes) {
        vector<int> temp_token;
        temp_token.reserve(pretoken.size());
        for (uint8_t byte : pretoken) {
            temp_token.push_back(static_cast<int>(byte));
        }
        pretokens.push_back(temp_token);
    }

    vector<pair<pair<int, int>, int>> ordered_merges = GetOrderedMerges();

    vector<int> tokens;
    for (vector<int>& pretoken : pretokens) {
        for (const auto& [pair, idx] : ordered_merges) {
            vector<int> new_token;
            new_token.reserve(pretoken.size());

            int j = 0;
            while (j < pretoken.size()) {
                if (j < pretoken.size()-1 && pretoken[j] == pair.first && pretoken[j+1] == pair.second) {
                    new_token.push_back(idx);
                    j += 2;
                } else {
                    new_token.push_back(pretoken[j]);
                    j += 1;
                }

            }
            pretoken = new_token;
        }
        tokens.insert(tokens.end(), pretoken.begin(), pretoken.end());
    }
    return tokens;
}

std::string Tokenizer::Decode(const std::vector<int> & token_ids) {
    vector<uint8_t> result_bytes;
    result_bytes.reserve(token_ids.size() * 4);

    for (const int& token : token_ids) {
        const vector<uint8_t>& token_bytes = vocab_[token];
        result_bytes.insert(result_bytes.end(), token_bytes.begin(), token_bytes.end());
    }
    return string(result_bytes.begin(), result_bytes.end());
}

vector<string> Tokenizer::IdsToTokens(const vector<int>& token_ids) {
    vector<string> result;
    result.reserve(token_ids.size());

    for (const auto& token : token_ids) {
        const vector<uint8_t>& token_bytes = vocab_[token];
        string token_str = string(token_bytes.begin(), token_bytes.end());
        result.push_back(token_str);
    }
    return result;
}