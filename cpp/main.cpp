#include <iostream>
#include <vector>
#include <string>
#include "pretokenizer.hpp"
#include "dataset.hpp"
#include "tokenizer.hpp"
#include <chrono>


int main() {
    std::string text = LoadTextFromParquet("../../train-00000-of-00001.parquet");
    std::cout << "Loaded " << text.length() << " characters" << std::endl;

    double subset_size = 0.5;
    size_t subset_length = static_cast<size_t>(text.length() * subset_size);
    std::string preview = text.substr(0, subset_length);
    std::cout << "Using subset of " << preview.length() << " characters for training" << std::endl;

    Tokenizer tokenizer;
    auto start = std::chrono::high_resolution_clock::now();
    std::cout << "Beginning training..." << std::endl;
    tokenizer.Train(preview, 3000);
    std::cout << "Training completed!" << std::endl;
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << "Training took " << duration.count() << " seconds" << std::endl;

    std::string test_str = "This is a test string to see what happens, I hope it works🫢";
    std::cout << "Initial string: " << test_str << std::endl;
    
    auto encoded = tokenizer.Encode(test_str);
    std::cout << "Encoded to " << encoded.size() << " tokens" << std::endl;

    std::string decoded = tokenizer.Decode(encoded);
    std::cout << "Decoded string: " << decoded << std::endl;

    auto tokens = tokenizer.IdsToTokens(encoded);
    std::cout << "Tokens:" << std::endl;
    for (const auto& token : tokens) {
        std::cout << "[" << token << "] ";
    }
    std::cout << std::endl;

    if (decoded == test_str) {
        std::cout << "✓ Test passed: decoded string matches original!" << std::endl;
    } else {
        std::cout << "✗ Test failed: decoded string does not match original" << std::endl;
    }

    return 0;
}
