package main

import (
	"fmt"
	"log"
	"runtime"
	"slices"
	"sync"
)

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

func GetStats(pretokens [][]int) map[[2]int]int {
	stats := make(map[[2]int]int)
	for i := 0; i < len(pretokens); i++ {
		for j := 0; j < len(pretokens[i])-1; j++ {
			pair := [2]int{pretokens[i][j], pretokens[i][j+1]}
			stats[pair]++
		}
	}
	return stats
}

func GetStatsParallel(pretokens [][]int, numWorkers int) map[[2]int]int {
	chunkSize := (len(pretokens) + numWorkers - 1) / numWorkers

	results := make(chan map[[2]int]int, numWorkers)

	var wg sync.WaitGroup
	for w := 0; w < numWorkers; w++ {
		start := w * chunkSize
		end := start + chunkSize
		if end > len(pretokens) {
			end = len(pretokens)
		}
		if start >= len(pretokens) {
			break
		}

		wg.Add(1)
		go func(chunk [][]int) {
			defer wg.Done()
			localStats := make(map[[2]int]int)
			for _, ids := range chunk {
				for j := 0; j < len(ids)-1; j++ {
					pair := [2]int{ids[j], ids[j+1]}
					localStats[pair]++
				}
			}
			results <- localStats
		}(pretokens[start:end])
	}

	go func() {
		wg.Wait()
		close(results)
	}()

	stats := make(map[[2]int]int)
	for partial := range results {
		for pair, count := range partial {
			stats[pair] += count
		}
	}

	return stats
}

func getMaxPair(stats map[[2]int]int) [2]int {
	var maxPair [2]int
	maxCount := 0
	for pair, count := range stats {
		if count > maxCount {
			maxCount = count
			maxPair = pair
		}
	}
	return maxPair
}

func mergeHelper(pretokens [][]int, pair [2]int, idx int) [][]int {
	new_pretokens := make([][]int, 0, len(pretokens))

	for i := 0; i < len(pretokens); i++ {
		temp_pretoken := make([]int, 0, len(pretokens[i]))
		j := 0

		for j < len(pretokens[i]) {
			if j < len(pretokens[i])-1 && pretokens[i][j] == pair[0] && pretokens[i][j+1] == pair[1] {
				temp_pretoken = append(temp_pretoken, idx)
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

func mergeHelperParallel(pretokens [][]int, pair [2]int, idx int, numWorkers int) [][]int {
	result := make([][]int, len(pretokens))

	var wg sync.WaitGroup
	chunkSize := (len(pretokens) + numWorkers - 1) / numWorkers

	for w := 0; w < numWorkers; w++ {
		start := w * chunkSize
		end := start + chunkSize
		if end > len(pretokens) {
			end = len(pretokens)
		}
		if start >= len(pretokens) {
			break
		}

		wg.Add(1)
		go func(start, end int) {
			defer wg.Done()
			for i := start; i < end; i++ {
				newTokens := make([]int, 0, len(pretokens[i]))
				j := 0
				for j < len(pretokens[i]) {
					if j < len(pretokens[i])-1 && pretokens[i][j] == pair[0] && pretokens[i][j+1] == pair[1] {
						newTokens = append(newTokens, idx)
						j += 2
					} else {
						newTokens = append(newTokens, pretokens[i][j])
						j += 1
					}
				}
				result[i] = newTokens
			}
		}(start, end)
	}

	wg.Wait()
	return result
}

func (t *GoTokenizer) Train(text string, parallel bool) error {
	fmt.Printf("Beginning Training...\n")
	for i := 0; i < 256; i++ {
		t.vocab[i] = []byte{byte(i)}
	}
	pretokens, err := Pretokenize(text)
	if err != nil {
		return err
	}

	num_merges := t.vocabSize - 256
	check_interval := t.vocabSize / 20
	numWorkers := runtime.NumCPU()
	if parallel {
		fmt.Printf("Using %d workers\n", numWorkers)
	}

	for i := 0; i < num_merges; i++ {
		if i%check_interval == 0 {
			fmt.Printf("Training... %d%%\n", i*100/num_merges)
		}

		var max_pair [2]int
		var stats map[[2]int]int
		if parallel {
			stats = GetStatsParallel(pretokens, numWorkers)
		} else {
			stats = GetStats(pretokens)
		}
		max_pair = getMaxPair(stats)

		idx := 256 + i
		t.merges[max_pair] = idx

		if parallel {
			pretokens = mergeHelperParallel(pretokens, max_pair, idx, numWorkers)
		} else {
			pretokens = mergeHelper(pretokens, max_pair, idx)
		}

		t.vocab[idx] = slices.Concat(t.vocab[max_pair[0]], t.vocab[max_pair[1]])
	}

	return nil
}

func (t *GoTokenizer) GetOrderedMerges() ([][2]int, []int) {
	keys := make([][2]int, 0, len(t.merges))
	for key := range t.merges {
		keys = append(keys, key)
	}

	slices.SortFunc(keys, func(a, b [2]int) int {
		return t.merges[a] - t.merges[b]
	})

	values := make([]int, len(keys))
	for i, key := range keys {
		values[i] = t.merges[key]
	}

	return keys, values
}

func (t *GoTokenizer) Encode(text string) []int {
	pretokens, err := Pretokenize(text)
	if err != nil {
		log.Fatalf("failed to pretokenize: %v", err)
	}
	var vocab_ids []int
	merged_pairs, idxs := t.GetOrderedMerges()

	for i := 0; i < len(pretokens); i++ {

		for k := 0; k < len(merged_pairs); k++ { // right here!
			new_tokens := make([]int, 0, len(pretokens[i]))
			j := 0
			for j < len(pretokens[i]) {
				if j < len(pretokens[i])-1 && pretokens[i][j] == merged_pairs[k][0] && pretokens[i][j+1] == merged_pairs[k][1] {
					new_tokens = append(new_tokens, idxs[k])
					j += 2
				} else {
					new_tokens = append(new_tokens, pretokens[i][j])
					j += 1
				}

			}
			pretokens[i] = new_tokens
		}
		vocab_ids = append(vocab_ids, pretokens[i]...)
	}

	return vocab_ids
}

func (t *GoTokenizer) Decode(ids []int) string {
	result := make([]byte, 0, len(ids))
	for i := 0; i < len(ids); i++ {
		result = append(result, t.vocab[ids[i]]...)
	}
	return string(result)
}

func (t *GoTokenizer) IdsToTokens(ids []int) []string {
	result := make([]string, 0, len(ids))
	for i := 0; i < len(ids); i++ {
		result = append(result, string(t.vocab[ids[i]]))
	}
	return result
}
