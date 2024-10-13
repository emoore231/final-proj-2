#!/bin/bash

output="main"

cpp_files=$(find . -maxdepth 1 -name "*.cpp")
c_files=$(find . -maxdepth 1 -name "*.c")

cpp_objects=""
for file in $cpp_files; do
    echo "Compiling $file"
    obj_file="${file%.cpp}.o"
    g++ -g -c "$file" -o "$obj_file"
    cpp_objects="$cpp_objects $obj_file"
done

c_objects=""
for file in $c_files; do
    echo "Compiling $file"
    obj_file="${file%.c}.o"
    gcc -g -c "$file" -o "$obj_file"
    c_objects="$c_objects $obj_file"
done

echo "Linking files"

g++ -g $cpp_objects $c_objects -lpthread -ldl -o "$output"
g++ -g $cpp_objects $c_objects -lpthread -ldl -o main2

echo "Compilation finished. Output: $output"
 