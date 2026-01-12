package main

type GoTokenizer struct {
	vocab     map[int][]byte
	merges    map[[2]byte]int
	stats     map[[2]byte]int
	vocabSize int
}

func NewGoTokenizer(vocabSize int) *GoTokenizer {
	return &GoTokenizer{
		vocab:     make(map[int][]byte),
		merges:    make(map[[2]byte]int),
		stats:     make(map[[2]byte]int),
		vocabSize: vocabSize,
	}
}

func (t * GoTokenizer) GetStats(ids [][]byte) {
	for i := 0; i < len(ids); i++ {
		for j := 0; j < len(ids[i])-1; j++ {
			pair := [2]byte{ids[i][j], ids[i][j+1]}
			t.stats[pair]++
		}
	}
}

func (t *GoTokenizer) GetMaxPair() [2]byte {
	var max_pair [2]byte
	max_count := 0

	for pair, count := range t.stats {
		if count > max_count {
			max_count = count
			max_pair = pair
		}
	}
	return max_pair
}

func mergeHelper(pretokens [][]byte, pair [2]byte, idx int) [][]byte {
	new_pretokens := make([][]byte, 0, len(pretokens))

	for i := 0; i < len(pretokens); i++ {
		temp_pretoken := make([]byte, 0, len(pretokens[i]))
		j := 0
		for j < len(pretokens[i]) {
			if j < len(pretokens[i])-1 && pretokens[i][j] == pair[0] && pretokens[i][j+1] == pair[1] {
				temp_pretoken = append(temp_pretoken, byte(idx))
				j += 2
			} else {
				temp_pretoken = append(temp_pretoken, pretokens[i][j])
				j += 1
			}
		}
		new_pretokens = append(new_pretokens, temp_pretoken)
	}
	return new_pretokens
}


func (t *GoTokenizer) Train(text string, vocabSize int) error {

	for i := 0; i < 256; i++ {
		t.vocab[i] = []byte{byte(i)}
	}
	pretokens, err := Pretokenize(text)
	if err != nil {
		return err
	}

	num_merges := vocabSize - 256
	for i := 0; i < num_merges; i++ {
		t.GetStats(pretokens)

		max_pair := t.GetMaxPair()
		idx := 256 + i
		t.merges[max_pair] = idx
		pretokens = mergeHelper(pretokens, max_pair, idx)
		t.vocab[idx] = append(t.vocab[int(max_pair[0])], t.vocab[int(max_pair[1])]...)
	}

	return nil
}
