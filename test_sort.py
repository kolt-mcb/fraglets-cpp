import os
import unittest
import fraglets

class SortFraTest(unittest.TestCase):
    def test_sort_fra_runs(self):
        f = fraglets.fraglets()
        sort_path = os.path.join(os.path.dirname(__file__), 'sort.fra')
        nums = []
        with open(sort_path) as fh:
            for line in fh:
                line = line.strip()
                if line and not line.startswith('#'):
                    f.parse(line)
                if line.startswith('[sort'):
                    parts = line.strip('[]').split()
                    nums = list(map(int, parts[1:]))
        f.run(10000, 10000, True)
        result = f.get_sorted()
        self.assertEqual(len(result), len(nums))
        self.assertTrue(all(result[i] <= result[i+1] for i in range(len(result)-1)))

if __name__ == '__main__':
    unittest.main()
