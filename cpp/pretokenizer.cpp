#include "pretokenizer.hpp"
#include <regex>
#include <string>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <re2/re2.h>


const std::string GPT2_PATTERN = R"('([sS][dDmMtT]|[Ll][lL]|[Vv][eE]|[Rr][eE])|[^\r\n\w]?\w+|\d{1,3}| ?[^\s\w]+[\r\n]*|\s+$|\s*[\r\n]|\s+)";


std::vector<std::vector<uint8_t>> pretokenize(const std::string& text) {
    std::vector<std::vector<uint8_t>> pretokens;

    std::regex pattern(GPT2_PATTERN, std::regex_constants::ECMAScript | std::regex_constants::optimize);

    std::sregex_iterator iter(text.begin(), text.end(), pattern);
    std::sregex_iterator end;

    for (; iter != end; ++iter) {
        const std::smatch& match = *iter;
        std::string token = match.str();

        std::vector<uint8_t> bytes(token.begin(), token.end());
        pretokens.push_back(bytes);
    }

    return pretokens;
}

const std::string GPT2_PATTERN_RE2 = R"(('(?:[sS][dDmMtT]|[Ll][lL]|[Vv][eE]|[Rr][eE])|[^\r\n\w]?\w+|\d{1,3}| ?[^\s\w]+[\r\n]*|\s+$|\s*[\r\n]|\s+))";


std::vector<std::vector<uint8_t>> pretokenize_re2(const std::string& text) {
    std::vector<std::vector<uint8_t>> pretokens;

    static const re2::RE2 pattern(GPT2_PATTERN_RE2);

    re2::StringPiece input(text);
    std::string match;

    while (RE2::FindAndConsume(&input, pattern, &match)) {
        std::vector<uint8_t> bytes(match.begin(), match.end());
        pretokens.push_back(bytes);
    }

    return pretokens;
}
