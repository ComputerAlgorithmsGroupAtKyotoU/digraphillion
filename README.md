# DiGraphillion

## how to build

- `python3 setup.py build`

## how to test

- `python3 setup.py test`

## methods

| Method                                                                     | Description                        |
| :------------------------------------------------------------------------- | :--------------------------------- |
| Returns a new DiGraphSet with directed single cycles from `gs`             | `gs.directed_cycles()`             |
| Returns a new DiGraphSet with directed single hamiltonian cycles from `gs` | `gs.directed_hamiltonian_cycles()` |

## Graphillion からの変更点 (for Developers)

- `graphillion` から `digraphillion` に変更
  - `GraphSet` は `DiGraphSet` へ
- `digraphillion/graphset.py`
  - `set_universe` で双方向の辺に対応
  - `_traverse` で双方向の辺に対応
- `src/subsetting/util/Digraph.hpp`
  - 双方向辺に対応
- python2, cygwin 等の動作に必要な部分は割愛

## References

- [TdZdd](https://github.com/kunisura/TdZdd)
- [py3c](https://github.com/encukou/py3c)

## Todo

- license 表記
- test 追加 (tox に移行?)
