# TODO: Implement a tokenizer that can tokenize a string into a list of tokens
from pretokenizer import pretokenize
from tqdm import tqdm

class PythonTokenizer:
    def __init__(self):
        self.vocab = {idx: bytes([idx]) for idx in range(256)}
        self.merges = {}
        self.stats = {}
        
    def _getStats(self, bts):
        for pair in zip(bts, bts[1:]):
            self.stats[pair] = self.stats.get(pair, 0) + 1
        return self.stats
    
    def _merge(self, bts, pair, idx):
        i = 0
        new_bts = []
        while i < len(bts):
            if i < len(bts)-1 and bts[i] == pair[0] and bts[i+1] == pair[1]:
                new_bts.append(idx)
                i += 2
            else:
                new_bts.append(bts[i])
                i += 1
        return new_bts
    
    def _pretokenize(self, text):
        return pretokenize(text)
    
    def train(self, text, vocab_size=3000):
        pretokens = self._pretokenize(text)
        for i in tqdm(range(vocab_size - 256)):
            for chunk in pretokens:
                self._getStats(chunk)
            max_pair = max(self.stats, key=self.stats.get)
            idx = 256 + i
            self.merges[max_pair] = idx
            pretokens = [self._merge(chunk, max_pair, idx) for chunk in pretokens]
            self.vocab[idx] = self.vocab[max_pair[0]] + self.vocab[max_pair[1]]
            
    def encode(self, text):
        pretokens = self._pretokenize(text)
        merge_order = sorted(self.merges.items(), key=lambda x: x[1])
        token_ids = []
        
        for pretoken in pretokens:
            for (b1, b2), merge_idx in merge_order:
                new_tokens = []
                i = 0
                while i < len(pretoken):
                    if i < len(pretoken)-1 and pretoken[i] == b1 and pretoken[i+1] == b2:
                        new_tokens.append(merge_idx)
                        i += 2
                    else:
                        new_tokens.append(pretoken[i])
                        i += 1
                pretoken = new_tokens
            token_ids.extend(pretoken)
        return token_ids
    
    def decode(self, token_ids):
        result = [self.vocab[token_id] for token_id in token_ids]
        vals = [res.decode('utf-8') for res in result]
        val_str = ''.join(vals)
        return val_str
    
    def ids_to_tokens(self, token_ids):
        return [self.vocab[token_id] for token_id in token_ids]