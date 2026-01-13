package main

import (
	"fmt"
	"log"
)

func main() {
	text, err := LoadTextFromParquet("../train-00000-of-00001.parquet", "text")
	if err != nil {
		log.Fatalf("failed to load text: %v", err)
	}

	fmt.Printf("Loaded %d characters of text\n", len(text))

	subset_size := 0.5
	preview := text[:int(float64(len(text)) * subset_size)]

	tokenizer := NewGoTokenizer(5000)
	err = tokenizer.Train(preview, true) 
	if err != nil {
		log.Fatalf("failed to train tokenizer: %v", err)
	}

	test_str := "This is a test string to see what happens, I hope it works🫢"
	encoded := tokenizer.Encode(test_str)

	decoded := tokenizer.Decode(encoded)
	fmt.Printf("Decoded: %s\n", decoded)

	tokens := tokenizer.IdsToTokens(encoded)
	for i := 0; i < len(tokens); i++ {
		fmt.Printf("%s \n", tokens[i])
	}


}
