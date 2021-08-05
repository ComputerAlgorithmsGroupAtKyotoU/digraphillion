from digraphillion import DiGraphSet
import unittest

e1 = (1, 2)
e2 = (1, 4)
e3 = (2, 3)
e4 = (2, 5)
e5 = (3, 6)
e6 = (4, 5)
e7 = (5, 6)
e8 = (2, 1)
e9 = (4, 1)
e10 = (3, 2)
e11 = (5, 2)
e12 = (6, 3)
e13 = (5, 4)
e14 = (6, 5)
universe_edges = [e1, e2, e3, e4, e5, e6, e7, e8, e9,
                  e10, e11, e12, e13, e14]


class TestDiGraphSet(unittest.TestCase):
    """
      1 <-> 2 <-> 3
      ^     ^     ^
      |     |     |
      v     v     v
      4 <-> 5 <-> 6
    """

    def test_init(self):
        s = DiGraphSet()
        print(s)

    def test_directed_cycles(self):
        DiGraphSet.set_universe(universe_edges)
        gs = DiGraphSet.directed_cycles()
        self.assertEqual(len(gs), 2 * (2 + 1) + len(universe_edges) / 2)
        self.assertTrue([(2, 3), (3, 2)] in gs)
        self.assertTrue([(1, 2), (2, 3), (3, 6), (6, 5), (5, 4), (4, 1)] in gs)
        self.assertTrue([(4, 5), (5, 2), (2, 1), (1, 4)] in gs)
        self.assertTrue([(5, 4), (4, 1), (1, 2), (2, 5)] in gs)


    def test_directed_hamiltonian_cycles(self):
        DiGraphSet.set_universe(universe_edges)
        gs = DiGraphSet.directed_cycles()
        for gg in gs:
            print(gg)

if __name__ == '__main__':
    unittest.main()
