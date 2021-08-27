# 開発者向けドキュメント

## Graphillion からの変更点 (for Developers)

- `graphillion` から `digraphillion` に変更
  - `GraphSet` は `DiGraphSet` へ
- `digraphillion/graphset.py`
  - `set_universe` で双方向の辺に対応
  - `_traverse` で双方向の辺に対応
- `src/subsetting/util/Digraph.hpp`
  - 双方向辺に対応
- python2, cygwin 等の動作に必要な部分は割愛

- Graphillion では edge (1, 2) と edge (2, 1) は同じ辺として扱われていたが、DiGraphillion では異なる辺として扱われる
- 自己ループ、多重辺には未対応 (誤った解が出力される)
- Graphillion と同じプログラム中で使用可能 (see also `graphillion/test/test_graphillion.py`) だが、`GraphSet` と `DiGraphSet` に互換性は無い

## 機能追加手順

see also [Graphillion Developers Guide](https://hackmd.io/@yamazaki2021/BJbLdp5au).

1. `src/digraphillion/spec/` に spec を追加
2. `src/digraphillion/graphset.cc` に呼び出し関数を実装
3. `src/digraphillion/setset.h` に friend 関数として追加
4. `src/pydigraphillion.cc` に Python とのインターフェースを実装
5. `digrpahillion/graphset.py` に Python 側関数を実装
6. `digraphillion/test/test_digraphillion.py` にテスト追加
7. `README.md` 更新
8. `digraphillion/release.py` でバージョン番号を更新

## Tips

- Python から渡す頂点は `pickle.dumps()` で変換する
- `-DNDEBUG` を無効化したい場合は以下のマクロをファイル冒頭に追加する
```
#ifdef NDEBUG
# define NDEBUG_DISABLED
# undef NDEBUG
#endif
```

## Links

- [Python.h リファレンス](https://docs.python.org/ja/3/c-api/arg.html)
- [setup.cfg リファレンス](https://setuptools.readthedocs.io/en/latest/userguide/declarative_config.html)

## Todo

- test 追加 (tox に移行?)
