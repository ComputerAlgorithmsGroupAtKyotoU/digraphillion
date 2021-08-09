# DiGraphillion

## Installing

### Requirements

- Linux/Mac OS
- 64-bit machines
- Python version 3.6 or later
- Python development environment (`Python.h`)
  - It is included with XCode in macOS, while it can be installed by `apt install python-dev` in Ubuntu.
- GCC or Clang
  - To build Graphillion, you need gcc version 5.4.0 or later.

### Installing from source

GitHub repository

1. Clone the Graphillion repository `git clone https://github.com/ComputerAlgorithmsGroupAtKyotoU/digraphillion.git`
2. Change directory to "digraphillion"
3. Run python `setup.py` build to build
4. (optional) Run `python setup.py test -q` to execute the tests
5. Run `sudo python setup.py install` to install

## How to use graphsets

- see [Graphillion](https://github.com/takemaru/graphillion#installing)

## Methods for digraph

| Method                                       | Description                                                                |
| :------------------------------------------- | :------------------------------------------------------------------------- |
| `gs.directed_cycles()`                       | Returns a new DiGraphSet with directed single cycles from `gs`             |
| `gs.directed_hamiltonian_cycles()`           | Returns a new DiGraphSet with directed single hamiltonian cycles from `gs` |
| `gs.directed_st_paths(s, t, is_hamiltonian)` | Returns a new DiGraphSet with directed st path from `gs`                   |
| `gs.directed_forests()`                      | Returns a new DiGraphSet with directed forests (branching) from `gs`       |
| `gs.rooted_trees(root, is_spanning`          | Returns a new DiGraphSet with rooted trees from `gs`                       |

## References

- [Graphillion](https://github.com/takemaru/graphillion)
- [TdZdd](https://github.com/kunisura/TdZdd)
- [py3c](https://github.com/encukou/py3c)
- [開発者向けドキュメント](https://github.com/ComputerAlgorithmsGroupAtKyotoU/digraphillion/blob/main/doc/developers_guide.md)
