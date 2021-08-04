from digraphillion import DiGraphSet
import unittest

class TestDiGraphSet(unittest.TestCase):
    def test_init(self):
        s = DiGraphSet()
        print(s)

if __name__ == '__main__':
    unittest.main()
