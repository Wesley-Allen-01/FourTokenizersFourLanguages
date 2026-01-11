# Tokenizers

A multi-language implementation of Byte Pair Encoding (BPE) tokenizers for natural language processing. This project aims to provide efficient tokenizer implementations across four different programming languages: Python, Go, C++, and Rust.

## Overview

This project implements tokenizers using the **Byte Pair Encoding (BPE)** algorithm, a subword tokenization technique commonly used in modern NLP models. BPE works by iteratively merging the most frequent pairs of bytes (or characters) in a corpus to build a vocabulary of subword units.

## Project Status

| Language | Status |
|----------|--------|
| Python   | ✅ Complete |
| Go       | 🚧 In Progress |
| C++      | 🚧 In Progress |
| Rust     | 🚧 In Progress |

## Features

- **BPE Training**: Learn subword vocabulary from training corpus
- **Encoding**: Convert text strings to token IDs
- **Decoding**: Reconstruct original text from token IDs
- **Pretokenization**: GPT-2 style regex-based pretokenization
- **Dataset Support**: Load training data from parquet files (WikiText-2)

## Python Implementation

### Requirements

- Python 3.12+
- Required packages (see `python/requirements.txt` or install via pip):
  - `regex`
  - `pandas`
  - `tqdm`

### Usage

```python
from dataset import load_wikitext2_from_parquet
from tokenizer import PythonTokenizer

# Load training data
text = load_wikitext2_from_parquet()

# Initialize and train tokenizer
tokenizer = PythonTokenizer()
tokenizer.train(text, vocab_size=5000)

# Encode text
encoded = tokenizer.encode("Hello, world!")

# Decode tokens
decoded = tokenizer.decode(encoded)

# Get token strings
tokens = tokenizer.ids_to_tokens(encoded)
```

### Project Structure

```
tokenizers/
├── python/
│   ├── tokenizer.py      # Main tokenizer class with BPE implementation
│   ├── bpe.py            # BPE algorithm utilities
│   ├── pretokenizer.py   # GPT-2 style regex pretokenization
│   ├── dataset.py        # Dataset loading utilities
│   ├── main.py           # Example usage script
│   └── notebooks/        # Jupyter notebooks for prototyping
├── go/                   # Go implementation (in progress)
├── cpp/                  # C++ implementation (in progress)
├── rust/                 # Rust implementation (in progress)
└── README.md
```

### Python Implementation Details

The Python tokenizer includes:

- **Vocabulary**: Starts with 256 base byte tokens, expands through BPE merges
- **Training**: Iteratively finds and merges most frequent byte pairs
- **Pretokenization**: Uses regex patterns similar to GPT-2 for text segmentation
- **Encoding/Decoding**: Converts between text and token IDs using learned vocabulary

## Algorithm: Byte Pair Encoding

BPE is a data compression algorithm adapted for tokenization:

1. **Initialize**: Start with vocabulary of all 256 possible byte values
2. **Count**: Count frequency of all adjacent byte pairs in the corpus
3. **Merge**: Find the most frequent pair and merge it into a new token
4. **Repeat**: Continue merging until desired vocabulary size is reached
5. **Encode**: Apply learned merges in order to encode new text


