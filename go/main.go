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

	preview := text[:300]
	fmt.Printf("Preview: %s\n", preview)

	pretokens, err := Pretokenize(preview)
	if err != nil {
		log.Fatalf("failed to pretokenize: %v", err)
	}

	fmt.Printf("First few pretokens: %v\n", pretokens[:5])
}
