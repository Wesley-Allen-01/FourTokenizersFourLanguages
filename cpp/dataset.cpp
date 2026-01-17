#include "dataset.hpp"
#include <arrow/api.h>
#include <arrow/io/api.h>
#include <parquet/arrow/reader.h>
#include <sstream>
#include <stdexcept>
#include <iostream>

std::string LoadTextFromParquet(const std::string& parquet_path) {
    auto file_result = arrow::io::ReadableFile::Open(parquet_path);
    if (!file_result.ok()) {
        throw std::runtime_error("Failed to open file: " + file_result.status().ToString());
    }
    std::shared_ptr<arrow::io::RandomAccessFile> input_file = file_result.ValueOrDie();

    parquet::arrow::FileReaderBuilder builder;
    arrow::Status status = builder.Open(input_file);
    if (!status.ok()) {
        throw std::runtime_error("Failed to open parquet file: " + status.ToString());
    }
    
    builder.memory_pool(arrow::default_memory_pool());
    auto reader_result = builder.Build();
    if (!reader_result.ok()) {
        throw std::runtime_error("Failed to build parquet reader: " + reader_result.status().ToString());
    }
    std::unique_ptr<parquet::arrow::FileReader> reader = std::move(reader_result).ValueOrDie();

    std::shared_ptr<arrow::Table> table;
    status = reader->ReadTable(&table);
    if (!status.ok()) {
        throw std::runtime_error("Failed to read table: " + status.ToString());
    }

    auto column = table->column(0);
    std::vector<std::string> texts;

    for (int chunk_idx = 0; chunk_idx < column->num_chunks(); ++chunk_idx) {
        auto chunk = column->chunk(chunk_idx);
        auto string_array = std::static_pointer_cast<arrow::StringArray>(chunk);

        for (int64_t i = 0; i < string_array->length(); ++i) {
            if (string_array->IsNull(i)) {
                continue;
            }

            std::string text = string_array->GetString(i);
            
            size_t start = text.find_first_not_of(" \t\n\r");
            if (start != std::string::npos) {
                texts.push_back(text);
            } 

        }
    }

    std::ostringstream oss;
    bool first = true;
    for (const auto& text : texts) {
        if (!first) {
            oss << "\n";
        }
        oss << text;
        first = false;
    }

    std::cout << "Loaded " << texts.size() << " text entries from parquet file" << std::endl;
    return oss.str();
}
