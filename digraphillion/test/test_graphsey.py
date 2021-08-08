# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

from builtins import range
from digraphillion.graphset import DiGraphSet
from digraphillion import DiGraphSet
import tempfile
import unittest


e1 = (1, 2)
e2 = (1, 3)
e3 = (2, 4)
e4 = (3, 4)

g0 = []
g1 = [e1]
g2 = [e2]
g3 = [e3]
g4 = [e4]
g12 = [e1, e2]
g13 = [e1, e3]
g14 = [e1, e4]
g23 = [e2, e3]
g24 = [e2, e4]
g34 = [e3, e4]
g123 = [e1, e2, e3]
g124 = [e1, e2, e4]
g134 = [e1, e3, e4]
g234 = [e2, e3, e4]
g1234 = [e1, e2, e3, e4]


class TestDiGraphSet(unittest.TestCase):

    def setUp(self):
        DiGraphSet.set_universe([e1 + (.3,), e2 + (-.2,), e3 + (-.2,), e4 + (.4,)],
                                traversal='bfs', source=1)

    def tearDown(self):
        pass

    def test_init(self):
        DiGraphSet.set_universe([('i', 'ii')])
        self.assertEqual(DiGraphSet.universe(), [('i', 'ii')])

        DiGraphSet.set_universe([e1 + (.3,), e2 + (-.2,), e3 + (-.2,), e4 + (.4,)],
                                traversal='bfs', source=1)
        self.assertEqual(DiGraphSet.universe(),
                         [e1 + (.3,), e2 + (-.2,), e3 + (-.2,), e4 + (.4,)])

        DiGraphSet.set_universe([e1 + (.3,), e2 + (-.2,), e3 + (-.2,), e4 + (.4,)],
                                traversal='dfs', source=1)
        self.assertEqual(DiGraphSet.universe(),
                         [e2 + (-.2,), e4 + (.4,), e1 + (.3,), e3 + (-.2,)])

        DiGraphSet.set_universe([e1 + (.3,), e2 + (-.2,), e3 + (-.2,), e4 + (.4,)],
                                traversal='greedy', source=3)
        self.assertEqual(DiGraphSet.universe(),
                         [e2 + (-.2,), e1 + (.3,), e3 + (-.2,), e4 + (.4,)])

#        self.assertRaises(KeyError, DiGraphSet.set_universe, [(1, 2), (2, 1)])

        DiGraphSet.set_universe([(1, 2), (3, 4)])  # disconnected graph
        self.assertEqual(DiGraphSet.universe(), [(1, 2), (3, 4)])

    def test_constructors(self):
        gs = DiGraphSet()
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(len(gs), 0)

        gs = DiGraphSet([])
        self.assertEqual(len(gs), 0)

        gs = DiGraphSet([g1, [(3, 1)]])
        self.assertEqual(len(gs), 2)
        self.assertTrue(g1 in gs)
        self.assertTrue(g2 in gs)

        gs = DiGraphSet({})
        self.assertEqual(len(gs), 2**4)

        gs = DiGraphSet({'include': [e1, e2], 'exclude': [(4, 3)]})
        self.assertEqual(len(gs), 2)
        self.assertTrue(g12 in gs)
        self.assertTrue(g123 in gs)

        self.assertRaises(KeyError, DiGraphSet, [(1, 4)])
        self.assertRaises(KeyError, DiGraphSet, [[(1, 4)]])
        self.assertRaises(KeyError, DiGraphSet, {'include': [(1, 4)]})

        # copy constructor
        gs1 = DiGraphSet([g0, g12, g13])
        gs2 = gs1.copy()
        self.assertTrue(isinstance(gs2, DiGraphSet))
        gs1.clear()
        self.assertEqual(gs1, DiGraphSet())
        self.assertEqual(gs2, DiGraphSet([g0, g12, g13]))

        # repr
        gs = DiGraphSet([g0, g12, g13])
        self.assertEqual(
            repr(gs),
            "DiGraphSet([[], [(1, 2), (1, 3)], [(1, 2), (2, 4)]])")

        gs = DiGraphSet({})
        self.assertEqual(
            repr(gs),
            "DiGraphSet([[], [(1, 2)], [(1, 3)], [(2, 4)], [(3, 4)], [(1, 2), (1, 3)], [( ...")

# def test_graphs(self):

#    def test_linear_constraints(self):

    def test_show_messages(self):
        a = DiGraphSet.show_messages()
        b = DiGraphSet.show_messages(True)
        self.assertTrue(b)
        c = DiGraphSet.show_messages(False)
        self.assertTrue(c)
        d = DiGraphSet.show_messages(a)
        self.assertFalse(d)

    def test_comparison(self):
        gs = DiGraphSet([g12])
        self.assertEqual(gs, DiGraphSet([g12]))
        self.assertNotEqual(gs, DiGraphSet([g13]))

        # __nonzero__
        self.assertTrue(gs)
        self.assertFalse(DiGraphSet())

        v = [g0, g12, g13]
        gs = DiGraphSet(v)
        self.assertTrue(gs.isdisjoint(DiGraphSet([g1, g123])))
        self.assertFalse(gs.isdisjoint(DiGraphSet([g1, g12])))

        self.assertTrue(gs.issubset(DiGraphSet(v)))
        self.assertFalse(gs.issubset(DiGraphSet([g0, g12])))
        self.assertTrue(gs <= DiGraphSet(v))
        self.assertFalse(gs <= DiGraphSet([g0, g12]))
        self.assertTrue(gs < DiGraphSet([g0, g1, g12, g13]))
        self.assertFalse(gs < DiGraphSet(v))

        self.assertTrue(gs.issuperset(DiGraphSet(v)))
        self.assertFalse(gs.issuperset(DiGraphSet([g1, g12])))
        self.assertTrue(gs >= DiGraphSet(v))
        self.assertFalse(gs >= DiGraphSet([g1, g12]))
        self.assertTrue(gs > DiGraphSet([[], g12]))
        self.assertFalse(gs > DiGraphSet(v))

    def test_unary_operators(self):
        gs = DiGraphSet([g0, g1, g12, g123, g1234, g134, g14, g4])

        self.assertTrue(isinstance(~gs, DiGraphSet))
        self.assertEqual(~gs, DiGraphSet(
            [g124, g13, g2, g23, g234, g24, g3, g34]))

        self.assertTrue(isinstance(gs.smaller(3), DiGraphSet))
        self.assertEqual(gs.smaller(3), DiGraphSet([g0, g1, g12, g14, g4]))
        self.assertTrue(isinstance(gs.larger(3), DiGraphSet))
        self.assertEqual(gs.larger(3), DiGraphSet([g1234]))
        self.assertTrue(isinstance(gs.graph_size(3), DiGraphSet))
        self.assertEqual(gs.graph_size(3), DiGraphSet([g123, g134]))
        self.assertTrue(isinstance(gs.len(3), DiGraphSet))
        self.assertEqual(gs.len(3), DiGraphSet([g123, g134]))

        gs = DiGraphSet([g12, g123, g234])
        self.assertTrue(isinstance(gs.minimal(), DiGraphSet))
        self.assertEqual(gs.minimal(), DiGraphSet([g12, g234]))
        self.assertTrue(isinstance(gs.maximal(), DiGraphSet))
        self.assertEqual(gs.maximal(), DiGraphSet([g123, g234]))

        gs = DiGraphSet([g12, g14, g23, g34])
        self.assertTrue(isinstance(gs.blocking(), DiGraphSet))
        self.assertEqual(
            gs.blocking(), DiGraphSet([g123, g1234, g124, g13, g134, g234, g24]))

    def test_binary_operators(self):
        u = [g0, g1, g12, g123, g1234, g134, g14, g4]
        v = [g12, g14, g23, g34]

        gs = DiGraphSet(u) | DiGraphSet(v)
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(
            gs, DiGraphSet([g0, g1, g12, g123, g1234, g134, g14, g23, g34, g4]))
        gs = DiGraphSet(u).union(DiGraphSet(u), DiGraphSet(v))
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(
            gs, DiGraphSet([g0, g1, g12, g123, g1234, g134, g14, g23, g34, g4]))

        gs = DiGraphSet(u)
        gs |= DiGraphSet(v)
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(
            gs, DiGraphSet([g0, g1, g12, g123, g1234, g134, g14, g23, g34, g4]))
        gs = DiGraphSet(u)
        gs.update(DiGraphSet(u), DiGraphSet(v))
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(
            gs, DiGraphSet([g0, g1, g12, g123, g1234, g134, g14, g23, g34, g4]))

        gs = DiGraphSet(u) & DiGraphSet(v)
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g12, g14]))
        gs = DiGraphSet(u).intersection(DiGraphSet(u), DiGraphSet(v))
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g12, g14]))

        gs = DiGraphSet(u)
        gs &= DiGraphSet(v)
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g12, g14]))
        gs = DiGraphSet(u)
        gs.intersection_update(DiGraphSet(u), DiGraphSet(v))
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g12, g14]))

        gs = DiGraphSet(u) - DiGraphSet(v)
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g0, g1, g123, g1234, g134, g4]))
        gs = DiGraphSet(u).difference(DiGraphSet(), DiGraphSet(v))
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g0, g1, g123, g1234, g134, g4]))

        gs = DiGraphSet(u)
        gs -= DiGraphSet(v)
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g0, g1, g123, g1234, g134, g4]))
        gs = DiGraphSet(u)
        gs.difference_update(DiGraphSet(), DiGraphSet(v))
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g0, g1, g123, g1234, g134, g4]))

        gs = DiGraphSet(u) ^ DiGraphSet(v)
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet(
            [g0, g1, g123, g1234, g134, g23, g34, g4]))
        gs = DiGraphSet(u).symmetric_difference(DiGraphSet(), DiGraphSet(v))
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet(
            [g0, g1, g123, g1234, g134, g23, g34, g4]))

        gs = DiGraphSet(u)
        gs ^= DiGraphSet(v)
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet(
            [g0, g1, g123, g1234, g134, g23, g34, g4]))
        gs = DiGraphSet(u)
        gs.symmetric_difference_update(DiGraphSet(), DiGraphSet(v))
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet(
            [g0, g1, g123, g1234, g134, g23, g34, g4]))

        v = [g12]
        gs = DiGraphSet(u) / DiGraphSet(v)
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g0, g3, g34]))
        gs = DiGraphSet(u).quotient(DiGraphSet(v))
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g0, g3, g34]))

        gs = DiGraphSet(u)
        gs /= DiGraphSet(v)
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g0, g3, g34]))
        gs = DiGraphSet(u)
        gs.quotient_update(DiGraphSet(v))
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g0, g3, g34]))

        gs = DiGraphSet(u) % DiGraphSet(v)
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g0, g1, g134, g14, g4]))
        gs = DiGraphSet(u).remainder(DiGraphSet(v))
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g0, g1, g134, g14, g4]))

        gs = DiGraphSet(u)
        gs %= DiGraphSet(v)
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g0, g1, g134, g14, g4]))
        gs = DiGraphSet(u)
        gs.remainder_update(DiGraphSet(v))
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g0, g1, g134, g14, g4]))

        gs = DiGraphSet(u).complement()
        self.assertEqual(gs, DiGraphSet(
            [g0, g123, g1234, g2, g23, g234, g34, g4]))

        v = [g12, g14, g23, g34]
        gs = DiGraphSet(u).join(DiGraphSet(v))
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(
            gs, DiGraphSet([g12, g123, g124, g1234, g134, g14, g23, g234, g34]))

        gs = DiGraphSet(u).meet(DiGraphSet(v))
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet(
            [g0, g1, g12, g14, g2, g23, g3, g34, g4]))

        v = [g12, g14, g23, g34]
        gs = DiGraphSet(u).subgraphs(DiGraphSet(v))
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g0, g1, g12, g14, g4]))

        gs = DiGraphSet(u).supergraphs(DiGraphSet(v))
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g12, g123, g1234, g134, g14]))

        gs = DiGraphSet(u).non_subgraphs(DiGraphSet(v))
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g123, g1234, g134]))

        gs = DiGraphSet(u).non_supergraphs(DiGraphSet(v))
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g0, g1, g4]))

        gs1 = DiGraphSet({}) - DiGraphSet([g1, g34])

        gs2 = gs1.including(DiGraphSet([g1, g2]))
        self.assertTrue(isinstance(gs2, DiGraphSet))
        self.assertEqual(len(gs2), 11)

        gs2 = gs1.including(g1)
        self.assertTrue(isinstance(gs2, DiGraphSet))
        self.assertEqual(len(gs2), 7)

        gs2 = gs1.including((2, 1))
        self.assertTrue(isinstance(gs2, DiGraphSet))
        self.assertEqual(len(gs2), 7)

        gs2 = gs1.including(1)
        self.assertTrue(isinstance(gs2, DiGraphSet))
        self.assertEqual(len(gs2), 11)

        self.assertRaises(KeyError, gs1.including, (1, 4))
        self.assertRaises(KeyError, gs1.including, 5)

        gs2 = gs1.excluding(DiGraphSet([g1, g2]))
        self.assertTrue(isinstance(gs2, DiGraphSet))
        self.assertEqual(len(gs2), 3)

        gs2 = gs1.excluding(g1)
        self.assertTrue(isinstance(gs2, DiGraphSet))
        self.assertEqual(len(gs2), 7)

        gs2 = gs1.excluding(e2)
        self.assertTrue(isinstance(gs2, DiGraphSet))
        self.assertEqual(len(gs2), 6)

        gs2 = gs1.excluding(1)
        self.assertTrue(isinstance(gs2, DiGraphSet))
        self.assertEqual(len(gs2), 3)

        self.assertRaises(KeyError, gs1.excluding, (1, 4))
        self.assertRaises(KeyError, gs1.excluding, 5)

        v = [g12, g14, g23, g34]
        gs = DiGraphSet(u).included(DiGraphSet(v))
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g0, g1, g12, g14, g4]))

        gs = DiGraphSet(u).included(g12)
        self.assertTrue(isinstance(gs, DiGraphSet))
        self.assertEqual(gs, DiGraphSet([g0, g1, g12]))

    def capacity(self):
        gs = DiGraphSet()
        self.assertFalse(gs)

        gs = DiGraphSet([g0, g12, g13])
        self.assertTrue(gs)

        self.assertEqual(len(gs), 3)
        self.assertEqual(gs.len(), 3)

    def test_iterators(self):
        gs1 = DiGraphSet([g0, g12, g13])
        gs2 = DiGraphSet()
        for g in gs1:
            self.assertTrue(isinstance(g, list))
            gs2 = gs2 | DiGraphSet([g])
        self.assertEqual(gs1, DiGraphSet([g0, g12, g13]))
        self.assertEqual(gs1, gs2)

        gs2 = DiGraphSet()
        for g in gs1:
            self.assertTrue(isinstance(g, list))
            gs2 = gs2 | DiGraphSet([g])
        self.assertEqual(gs1, gs2)

        gs1 = DiGraphSet([g0, g12, g13])
        gs2 = DiGraphSet()
        for g in gs1.rand_iter():
            self.assertTrue(isinstance(g, list))
            gs2 = gs2 | DiGraphSet([g])
        self.assertEqual(gs1, gs2)

        gen = gs1.rand_iter()
        self.assertTrue(isinstance(next(gen), list))

        gs = DiGraphSet([g0, g1, g12, g123, g1234, g134, g14, g4])
        r = []
        for g in gs.max_iter():
            self.assertTrue(isinstance(g, list))
            r.append(g)
        self.assertEqual(len(r), 8)
        self.assertEqual(r[0], g14)
        self.assertEqual(r[1], g134)
        self.assertEqual(r[2], g4)

        r = []
        for g in gs.max_iter({e1: -.3, e2: .2, e3: .2, e4: -.4}):
            self.assertTrue(isinstance(g, list))
            r.append(g)
        self.assertEqual(len(r), 8)
        self.assertEqual(r[0], g123)
        self.assertEqual(r[1], g0)
        self.assertEqual(r[2], g12)

        r = []
        for g in gs.min_iter():
            self.assertTrue(isinstance(g, list))
            r.append(g)
        self.assertEqual(len(r), 8)
        self.assertEqual(r[0], g123)
        self.assertEqual(r[1], g0)
        self.assertEqual(r[2], g12)

        r = []
        for g in gs.min_iter({e1: -.3, e2: .2, e3: .2, e4: -.4}):
            self.assertTrue(isinstance(g, list))
            r.append(g)
        self.assertEqual(len(r), 8)
        self.assertEqual(r[0], g14)
        self.assertEqual(r[1], g134)
        self.assertEqual(r[2], g4)

        gs = DiGraphSet([[]])
        self.assertEqual(list(gs.min_iter()), [[]])

    def test_lookup(self):
        gs1 = DiGraphSet([g1, g12])

        self.assertTrue(g12 in gs1)
        self.assertTrue(g2 not in gs1)
        self.assertTrue(e1 in gs1)
        self.assertTrue(e4 not in gs1)
        self.assertTrue(1 in gs1)
        self.assertTrue(4 not in gs1)

    def test_modifiers(self):
        v = [g0, g12, g13]
        gs = DiGraphSet(v)
        gs.add(g1)
        self.assertTrue(g1 in gs)

        gs.remove(g1)
        self.assertTrue(g1 not in gs)
        self.assertRaises(KeyError, gs.remove, g1)

        gs.add(g0)
        gs.discard(g0)
        self.assertTrue(g0 not in gs)
        gs.discard(g0)  # no exception raised

        gs = DiGraphSet(v)
        gs.add(e2)
        self.assertEqual(gs, DiGraphSet([g12, g123, g2]))

        gs = DiGraphSet(v)
        gs.remove(e2)
        self.assertEqual(gs, DiGraphSet([g0, g1, g13]))
        self.assertRaises(KeyError, gs.remove, e4)

        gs = DiGraphSet(v)
        gs.discard(e2)
        self.assertEqual(gs, DiGraphSet([g0, g1, g13]))
        gs.discard(e4)  # no exception raised

        v = [g1, g12, g13]
        gs = DiGraphSet(v)
        g = gs.pop()
        self.assertTrue(isinstance(g, list))
        self.assertTrue(g not in gs)
        self.assertEqual(gs | DiGraphSet([g]), DiGraphSet(v))

        self.assertTrue(gs)
        gs.clear()
        self.assertFalse(gs)

        self.assertRaises(KeyError, gs.pop)

        self.assertRaises(KeyError, gs.add, [(1, 4)])
        self.assertRaises(KeyError, gs.remove, [(1, 4)])
        self.assertRaises(KeyError, gs.discard, [(1, 4)])

        self.assertRaises(KeyError, gs.add, (1, 4))
        self.assertRaises(KeyError, gs.remove, (1, 4))
        self.assertRaises(KeyError, gs.discard, (1, 4))

        u = [g0, g1, g12, g123, g1234, g134, g14, g4]
        gs = DiGraphSet(u)
        gs.flip(e1)
        self.assertEqual(gs, DiGraphSet([g0, g1, g14, g2, g23, g234, g34, g4]))

#    def test_probability(self):

    def test_io(self):
        gs = DiGraphSet()
        st = gs.dumps()
        self.assertEqual(st, "B\n.\n")
        gs = DiGraphSet.loads(st)
        self.assertEqual(gs, DiGraphSet())

        gs = DiGraphSet([g0])
        st = gs.dumps()
        self.assertEqual(st, "T\n.\n")
        gs = DiGraphSet.loads(st)
        self.assertEqual(gs, DiGraphSet([g0]))

        v = [g0, g1, g12, g123, g1234, g134, g14, g4]
        gs = DiGraphSet(v)
        st = gs.dumps()
        gs = DiGraphSet.loads(st)
        self.assertEqual(gs, DiGraphSet(v))

        # skip this test, becasue string is treated as an element
#        gs = DiGraphSet(st)
#        self.assertEqual(gs, DiGraphSet(v))

        with tempfile.TemporaryFile() as f:
            gs.dump(f)
            f.seek(0)
            gs = DiGraphSet.load(f)
            self.assertEqual(gs, DiGraphSet(v))


if __name__ == '__main__':
    unittest.main()
