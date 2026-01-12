package main

import (
	"context"
	"fmt"
	"os"
	"strings"

	"github.com/apache/arrow/go/v17/arrow/array"
	"github.com/apache/arrow/go/v17/arrow/memory"
	"github.com/apache/arrow/go/v17/parquet/file"
	"github.com/apache/arrow/go/v17/parquet/pqarrow"
)

func LoadTextFromParquet(path string, textColumn string) (string, error) {
	f, err := os.Open(path)
	if err != nil {
		return "", fmt.Errorf("failed to open file: %w", err)
	}
	defer f.Close()

	pf, err := file.NewParquetReader(f)
	if err != nil {
		return "", fmt.Errorf("failed to create parquet reader: %w", err)
	}
	defer pf.Close()

	allocater := memory.NewGoAllocator()
	reader, err := pqarrow.NewFileReader(pf, pqarrow.ArrowReadProperties{}, allocater)
	if err != nil {
		return "", fmt.Errorf("failed to create arrow reader: %w", err)
	}

	table, err := reader.ReadTable(context.Background())
	if err != nil {
		return "", fmt.Errorf("failed to read table: %w", err)
	}
	defer table.Release()

	colIndex := -1
	for i := 0; i < int(table.NumCols()); i++ {
		if table.Schema().Field(i).Name == textColumn {
			colIndex = i
			break
		}
	}

	if colIndex == -1 {
		return "", fmt.Errorf("column %s not found in parquet file", textColumn)
	}

	col := table.Column(colIndex)
	var texts []string

	for _, chunk := range col.Data().Chunks() {
		stringArr := chunk.(*array.String)
		for i := 0; i < stringArr.Len(); i++ {
			if stringArr.IsNull(i) {
				continue
			}
			text := stringArr.Value(i)
			if strings.TrimSpace(text) != "" {
				texts = append(texts, text)
			}
		}
	}

	return strings.Join(texts, "\n"), nil
}
