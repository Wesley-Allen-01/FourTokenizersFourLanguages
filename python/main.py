# TODO: core training loop
from dataset import load_wikitext2_from_parquet
from tokenizer import PythonTokenizer

text = load_wikitext2_from_parquet()
subset_size = 0.05
text = text[:int(len(text) * subset_size)]
tokenizer = PythonTokenizer()
tokenizer.train(text, vocab_size=5000)

test_str = """
The goal of this test is that when I encode it using my tokenizer, 
and then decode it, I should get the same string back. 
"""
encoded = tokenizer.encode(test_str)
decoded = tokenizer.decode(encoded)
print(f"Initial string: {test_str}")

tokens = tokenizer.ids_to_tokens(encoded)
print(f"Tokens: {tokens}")

print(f"Decoded string: {decoded}")

if decoded == test_str:
    print("Test passed")
else:
    print("Test failed")