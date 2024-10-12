#!/bin/bash

output="program.out"

cpp_files=$(find . -name "*.cpp")
c_files=$(find . -name "*.c")

cpp_objects=""
for file in $cpp_files; do
    obj_file="${file%.cpp}.o"
    g++ -g -c "$file" -o "$obj_file"
    cpp_objects="$cpp_objects $obj_file"
done

c_objects=""
for file in $c_files; do
    obj_file="${file%.c}.o"
    gcc -g -c "$file" -o "$obj_file"
    c_objects="$c_objects $obj_file"
done

g++ -g $cpp_objects $c_objects -o "$output"

echo "Compilation finished. Output: $output"
