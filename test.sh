#!/bin/bash

echo "Testing program with test input"

cat ./test.txt | tr -d '\r' | ./main