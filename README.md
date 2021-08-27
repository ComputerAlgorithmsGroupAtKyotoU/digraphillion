# DiGraphillion

## What is DiGraphillion?

DiGraphillion is a Python software package on search, optimization, and enumeration for a *directed* graphset, or a set of *digraphs*.

see also [Graphillion](https://github.com/takemaru/graphillion#installing).

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

#### GitHub repository

1. Clone the Graphillion repository `git clone https://github.com/ComputerAlgorithmsGroupAtKyotoU/digraphillion.git`
2. Change directory to "digraphillion"
3. Run python `setup.py build` to build
4. (optional) Run `python setup.py test -q` to execute the tests
  - `pip install -U graphillion` before testing.
5. Run `sudo python setup.py install` to install

## How to use graphsets

- see also [Graphillion](https://github.com/takemaru/graphillion#installing).

### Terminology

| Term            | Description                   | Example                                            |
|:----------------|:------------------------------|:---------------------------------------------------|
| vertex          | any hashable object           | `1`, `'v1'`, `(x, y)`                              |
| directed edge   | tuple of vertices             | `(1, 2)`, `(2, 1)`                                 |
| digraph         | list of directed edges        | `[(1, 2), (2, 1), (1, 3)]`                           |
| set of digraphs | DiGraphSet object             | `DiGraphSet([[(1, 2), (1, 3)], [(1, 2), (2, 1)]])` |

- the order of vertices is important.
  - directed edge `(1, 2)` is not equal to `(2, 1)`.
- self-loop edges and multiedges are *NOT* supported. Incorrect solutions may be obtained.
- You can use `GraphSet` with `DiGraphSet` in the same program. However, `GraphSet` and `DiGraphSet` are incompatible.
  - see also `digraphillion/test/test_digraphillion.py`.

### Methods for digraph

| Method                                                     | Description                                                                |
| :--------------------------------------------------------- | :------------------------------------------------------------------------- |
| `gs.graphs(in_degree_constraints, out_degree_constraints)` | Returns a new DiGraphSet with degree constraints from `gs`                 |
| `gs.directed_cycles()`                                     | Returns a new DiGraphSet with directed single cycles from `gs`             |
| `gs.directed_hamiltonian_cycles()`                         | Returns a new DiGraphSet with directed single hamiltonian cycles from `gs` |
| `gs.directed_st_paths(s, t, is_hamiltonian)`               | Returns a new DiGraphSet with directed st path from `gs`                   |
| `gs.rooted_forests(roots, is_spanning)`                    | Returns a new DiGraphSet with rooted forests from `gs`                     |
| `gs.rooted_trees(root, is_spanning)`                       | Returns a new DiGraphSet with rooted trees from `gs`                       |

## References

- [Graphillion](https://github.com/takemaru/graphillion)
- [TdZdd](https://github.com/kunisura/TdZdd)
- [py3c](https://github.com/encukou/py3c)
- [開発者向けドキュメント](https://github.com/ComputerAlgorithmsGroupAtKyotoU/digraphillion/blob/main/doc/developers_guide.md)
