from digraphillion import DiGraphSet
import unittest

"""
1 <-> 2 <-> 3
^     ^     ^
|     |     |
v     v     v
4 <-> 5 <-> 6
"""

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


class TestDigraphillion(unittest.TestCase):
    def test_init(self):
        gs = DiGraphSet()
        self.assertEqual(len(gs), 0)

    def test_directed_cycles(self):
        DiGraphSet.set_universe(universe_edges)
        gs = DiGraphSet.directed_cycles()
        self.assertEqual(len(gs), 2 * (2 + 1) + len(universe_edges) / 2)

        self.assertTrue([(2, 3), (3, 2)] in gs)
        self.assertTrue([(1, 2), (2, 3), (3, 6), (6, 5), (5, 4), (4, 1)] in gs)
        self.assertTrue([(4, 5), (5, 2), (2, 1), (1, 4)] in gs)
        self.assertTrue([(5, 4), (4, 1), (1, 2), (2, 5)] in gs)

        self.assertTrue([(1, 2), (2, 5), (5, 4), (1, 4)] not in gs)
        self.assertTrue([(1, 4), (4, 5), (5, 4), (4, 1)] not in gs)

    def test_directed_hamiltonian_cycles(self):
        DiGraphSet.set_universe(universe_edges)
        gs = DiGraphSet.directed_hamiltonian_cycles()
        cycles = DiGraphSet.directed_cycles()
        self.assertEqual(len(gs), 2)
        for c in gs:
            self.assertTrue(c in cycles)
        self.assertTrue([(1, 2), (2, 3), (3, 6), (6, 5), (5, 4), (4, 1)] in gs)
        self.assertTrue([(1, 2), (2, 3)] not in gs)

    def test_directed_st_paths(self):
        DiGraphSet.set_universe(universe_edges)
        s, t = 1, 6
        gs = DiGraphSet.directed_st_paths(s, t, False)
        self.assertTrue([(1, 4), (4, 5), (5, 6)] in gs)
        self.assertTrue([(1, 4), (4, 5), (5, 2), (2, 3), (3, 6)] in gs)
        self.assertTrue([(1, 4), (4, 5)] not in gs)
        self.assertEqual(len(gs), 4)

    def test_directed_st_hamiltonian_paths(self):
        DiGraphSet.set_universe(universe_edges)
        s, t = 1, 6
        s_to_t = [(1, 4), (4, 5), (5, 2), (2, 3), (3, 6)]
        t_to_s = [(6, 3), (3, 2), (2, 5), (5, 4), (4, 1)]

        gs = DiGraphSet.directed_st_paths(s, t, True)
        self.assertTrue(s_to_t in gs)
        self.assertTrue(t_to_s not in gs)
        self.assertEqual(len(gs), 1)

        gs = DiGraphSet.directed_st_paths(t, s, True)
        self.assertTrue(s_to_t not in gs)
        self.assertTrue(t_to_s in gs)
        self.assertEqual(len(gs), 1)

    def test_hamiltonian_path_in_paths(self):
        DiGraphSet.set_universe(universe_edges)
        for s in range(1, 7):
            for t in range(1, 7):
                path = DiGraphSet.directed_st_paths(s, t, False)
                hamiltonian = DiGraphSet.directed_st_paths(s, t, True)
                self.assertTrue(hamiltonian.issubset(path))

    def test_directed_forests(self):
        DiGraphSet.set_universe(universe_edges)
        gs = DiGraphSet.directed_forests()

        self.assertTrue([(1, 2), (2, 3), (3, 6), (6, 5), (5, 4)] in gs)
        self.assertTrue([(5, 4), (4, 1), (5, 6), (6, 3)] in gs)
        self.assertTrue([(4, 1), (4, 5), (2, 3), (3, 6)] in gs)
        self.assertTrue([(2, 1), (2, 5), (2, 3)] in gs)
        self.assertTrue([(5, 2), (6, 3)] in gs)
        self.assertTrue([(1, 4)] in gs)
        self.assertTrue([] in gs)

        self.assertTrue([(2, 1), (4, 1)] not in gs)
        self.assertTrue([(1, 2), (2, 5), (5, 2), (2, 1)] not in gs)
        self.assertTrue([(1, 2), (2, 1)] not in gs)

    def test_rooted_spanning_trees(self):
        DiGraphSet.set_universe(universe_edges)
        root = 1
        is_spanning = True

        gs = DiGraphSet.rooted_trees(root, is_spanning)
        self.assertEqual(len(gs), 15)  # det(L)
        self.assertTrue([(1, 2), (2, 3), (1, 4), (2, 5), (3, 6)] in gs)
        self.assertTrue([(1, 2), (2, 3), (4, 1), (2, 5), (3, 6)] not in gs)
        for rooted_tree in gs:
            self.assertEqual(len(rooted_tree), 5)
            self.assertTrue((1, 2) in rooted_tree or (1, 4) in rooted_tree)

    def test_rooted_trees(self):
        DiGraphSet.set_universe(universe_edges)
        root = 1
        is_spanning = False

        gs = DiGraphSet.rooted_trees(root, is_spanning)
        gs.issubset(DiGraphSet.rooted_trees(root, True))

        self.assertTrue([(1, 2)] in gs)
        self.assertTrue([(1, 2), (1, 4)] in gs)
        self.assertTrue([(1, 2), (1, 4), (4, 5)] in gs)
        self.assertTrue([(1, 2), (1, 4), (4, 5), (5, 6)] in gs)
        self.assertTrue([(1, 2), (1, 4), (4, 5), (5, 6), (6, 3)] in gs)

        self.assertTrue([(4, 1)] not in gs)
        self.assertTrue([(2, 3)] not in gs)
        self.assertTrue([(1, 2), (2, 5), (5, 4), (4, 1)] not in gs)


if __name__ == '__main__':
    unittest.main()
