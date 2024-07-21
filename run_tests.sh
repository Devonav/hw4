#!/bin/bash

# Directories
input_dir="./"
output_dir="./"
expected_output_dir="./"

# Compiler executable
compiler="./hw4compiler"

# Files
declare -a input_files=( "input11.txt" "input12.txt" "input13.txt" "input14.txt" "input15.txt" "input16.txt" "input17.txt" "input18.txt")
declare -a expected_files=("output11.txt" "output12.txt" "output13.txt" "output14.txt" "output15.txt" "output16.txt" "output17.txt" "output18.txt")

# Run tests
for i in "${!input_files[@]}"; do
  input_file="${input_files[$i]}"
  expected_file="${expected_files[$i]}"
  output_file="${output_dir}/actual_${expected_file}"
  
  # Run the compiler
  $compiler "$input_dir/$input_file" > "$output_file"
  
  # Compare output to expected
  if diff -q "$output_file" "$expected_output_dir/$expected_file"; then
    echo "Test $input_file: PASSED"
  else
    echo "Test $input_file: FAILED"
    echo "Expected:"
    cat "$expected_output_dir/$expected_file"
    echo "Actual:"
    cat "$output_file"
  fi
done
