# TODO: Implement a pretokenizer leveraging regex similar to gpt2
import regex as re

def pretokenize(text):
    rule = re.compile(r"""'(?i:[sdmt]|ll|ve|re)|[^\r\n\p{L}\p{N}]?+\p{L}++|\p{N}{1,3}+| ?[^\s\p{L}\p{N}]++[\r\n]*+|\s++$|\s*[\r\n]|\s+(?!\S)|\s""")
    pretokens = re.findall(rule, text)
    ids = [list(ch.encode('utf-8')) for ch in pretokens]
    return ids
