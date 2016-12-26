# Breaking Node.js 0.13's RNG
> Marius Petcu - 343C4

An implementation of [Exploiting CSGOJackpot's Weak RNG](https://jonasnick.github.io/blog/2015/07/08/exploiting-csgojackpots-weak-rng/).

[Another good article explaining how broken Node's RNG was](https://medium.com/@betable/tifu-by-using-math-random-f1c308c4fd9d#.vj8favdol).

This was fixed in [V8 4.9.41.0](http://v8project.blogspot.ro/2015/12/theres-mathrandom-and-then-theres.html)

## Deconstructing the problem

Old versions of Node.js used the MWC1616 PRNG for `Math.random()`. This uses a
64bit state that gets mutated each iteration, then a 32bit integer is derived
from that state, which is then converted to a double by dividing it by `2^32`.

The article above provided us with a JS implementation of the RNG. We start
by splitting it in 3 standalone stages:

```js
nextState(rngstate)
randomInt32(rngstate)
int32ToDouble(x)
```

This allows us to test each function separately as we implement the C versions
of these functions.

The following invocations should output the results after each of these 3 steps
from both the JS and the C implementations with `0x0123456789ABCDEF` as its
64bit RNG state:

```bash
./random.js test 01234567 89ABCDEF
./random test 01234567 89ABCDEF
```

I wrote `./test.sh` to diff the outputs of the 2 implementations on a set of
random RNG states. There are small differences in how C and JS print doubles,
so the outputs are not always exact, but the differences don't seem to be felt
internally.

## Cracking the RNG

We can easily turn a double generated with this RNG back to its original 32bit
integer by multiplying the double with 2^32. The 32bit random integer directly
contains 32bits of the 64bit RNG state, so in order to get the full RNG state,
we need to brute force the remaining 32bits.

If we have two consecutive random numbers, we can easily check if our guess is
good by simply computing the next RNG with our guessed seed and checking it
against our known number.

If the two random numbers are an unknown (but known small) number of RNG
iterations apart, we can apply the same tactic and generate random numbers ahead
starting with our seed and see if one of them is our known number. But we run
into problems when we discover that two different seeds can generate the same
two random numbers in sequence (though maybe not the exact same number of
iterations apart). We can eliminate these false positives if we know a third
random number from the sequence.

Usage:

```bash
# For 2 consecutive known numbers:
./random crack 0.7211675397120416 0.051753338193520904 1
# For 3 known numbers spread over at most 10 iterations:
./random crack 0.7211675397120416 0.5180133141111583 0.308838497614488 10
```

## Verification

We just run the RNG over the seed we found:

```bash
./random.js generate 01234567 89ABCDEF
./random generate 01234567 89ABCDEF
```
