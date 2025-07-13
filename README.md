# fraglets-cpp
a very fast implementation of http://www.fraglets.net/, with some tweaks



From fraglets.net:


Introduction
What are Fraglets?
Fraglets are tiny computation fragments or tokens that flow through a computer network. There are two ways to look at Fraglets. First, Fraglets implement a chemical reaction model where computations are carried out by having fraglets "react" with each other. Alternatively, fraglets can be seen as dataflow tokens that work themselves through communication media and routing tables - conceptually, the CPU is turned inside out such that the network becomes the CPU's bus. An interesting twist (with both views) is that Fraglets blend the notion of code and data, overcoming the discrepancy between "classic networking" and "active networking".

What are they good for?
Fraglets can be used to explore new protocol engineering and implementation opportunities. Inspired by the chemical metaphor, we study for example the regulation of protocol instances as well as their internal code base. Such a system will hopefully be able to track changes in network and code configurations, adapt gracefully to unforseen situations and even evolve its own functionality. On the other hand, Fraglets are formal enough such that we can study self-healing properties of protocol implementations running in unreliable execution environments.


![alt text](https://github.com/koltafrickenfer/fraglets-cpp/blob/master/sort.png)

## Building

The project ships a small C++ extension exposing the core engine to
Python. Building simply requires a compiler, Python and the Graphviz
development libraries.

```bash
sudo apt-get install build-essential python3-dev libgraphviz-dev
git clone https://github.com/koltafrickenfer/fraglets-cpp.git
cd fraglets-cpp
python3 -m pip install .
```

This command compiles the extension and installs the `cFraglets` module
locally. You can run the included examples directly after the build:

```bash
python3 test.py
```

### Benchmarking

To measure how the sort implementation scales with different thread counts, run
the benchmark script:

```bash
python3 benchmark_sort.py
```

The script generates a random list of 100 numbers and times the sorting process
using 1, 2, 4 and 8 threads. Each run executes roughly 200k iterations of the
engine to ensure the list is fully sorted, and the elapsed time is printed.
