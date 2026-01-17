#ifndef PRETOKENIZER_HPP
#define PRETOKENIZER_HPP

#include <vector>
#include <cstdint>
#include <string>

std::vector<std::vector<uint8_t>> pretokenize(const std::string& text);

std::vector<std::vector<uint8_t>> pretokenize_re2(const std::string& text);

#endif