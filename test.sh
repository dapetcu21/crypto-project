#!/bin/bash
set -e

randi32() {
  dd if=/dev/random bs=4 count=1 2>/dev/null | hexdump -e '/1 "%02X"'
}

for i in $(seq 1 10); do
  MSB=$(randi32)
  LSB=$(randi32)
  node random.js test $MSB $LSB > node.out
  ./random test $MSB $LSB > c.out
  if diff node.out c.out; then
    echo "Passed test ${i}"
  else
    echo "Failed test ${i}: ${MSB} ${LSB}"
  fi
done

rm -f node.out
rm -f c.out