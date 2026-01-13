package main

import (
	"github.com/dlclark/regexp2"
)

const gpt2Pattern = `'(?i:[sdmt]|ll|ve|re)|[^\r\n\p{L}\p{N}]?\p{L}+|\p{N}{1,3}| ?[^\s\p{L}\p{N}]+[\r\n]*|\s+$|\s*[\r\n]|\s+(?!\S)|\s`

func Pretokenize(text string) ([][]int, error) {
	re := regexp2.MustCompile(gpt2Pattern, regexp2.None)

	var pretokens [][]int
	match, err := re.FindStringMatch(text)
	if err != nil {
		return nil, err
	}

	for match != nil {
		token := match.String()
		bytes := []byte(token)

		ints := make([]int, len(bytes))
		for i, b := range bytes {
			ints[i] = int(b)
		}
		pretokens = append(pretokens, ints)

		match, err = re.FindNextMatch(match)
		if err != nil {
			return nil, err
		}
	}

	return pretokens, nil
}
