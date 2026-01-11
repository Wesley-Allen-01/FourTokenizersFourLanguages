import os
from pathlib import Path
import pandas as pd

def load_wikitext2_from_parquet(parquet_path="train-00000-of-00001.parquet", text_column="text"):
    parquet_file = Path(parquet_path)
    
    if not parquet_file.exists():
        raise FileNotFoundError(f"Parquet file not found: {parquet_path}")
    
        
    df = pd.read_parquet(parquet_file)
    print(f"Loaded parquet file with {len(df)} rows using pandas")
    
    if text_column not in df.columns:
        raise ValueError(f"Column '{text_column}' not found. Available columns: {df.columns.tolist()}")
    

    texts = df[text_column].dropna().astype(str)
    texts = texts[texts.str.strip() != '']  
    text = '\n'.join(texts)
    
    return text