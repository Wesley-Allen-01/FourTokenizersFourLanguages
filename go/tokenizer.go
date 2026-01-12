package main

type GoTokenizer struct {
	vocab     map[int][]byte
	merges    map[[2]int]int
	vocabSize int
}

func NewGoTokenizer(vocabSize int) *GoTokenizer {
	return &GoTokenizer{
		vocab:     make(map[int][]byte),
		merges:    make(map[[2]int]int),
		vocabSize: vocabSize,
	}
}

func (t *GoTokenizer) Train(text string) error {

	for i := 0; i < 256; i++ {
		t.vocab[i] = []byte{byte(i)}
	}
	pretokens, err := Pretokenize(text)
	if err != nil {
		return err
	}

}
