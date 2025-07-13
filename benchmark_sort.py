import os
import random
import time
import fraglets

SORT_FILE = os.path.join(os.path.dirname(__file__), 'sort.fra')


def load_sort_core():
    lines = []
    with open(SORT_FILE) as fh:
        for line in fh:
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            # skip the example sort invocation
            if line.startswith('[sort '):
                continue
            lines.append(line)
    return lines


CORE_LINES = load_sort_core()


def build_sort_lines(nums):
    return CORE_LINES + ["[sort " + " ".join(map(str, nums)) + "]"]


def run_once(nums, threads):
    """Run the sort once and return (duration, sorted_nums)."""
    f = fraglets.fraglets()
    for line in build_sort_lines(nums):
        f.parse(line)
    start = time.perf_counter()
    # Roughly tuned iteration count: larger lists require more
    # iterations to complete. We found 200k iterations gives reliable
    # results when sorting 100 numbers across thread counts.
    f.run_threads(200000, 10000, threads, True)
    dur = time.perf_counter() - start
    sorted_nums = list(map(int, f.get_sorted()))
    return dur, sorted_nums


if __name__ == '__main__':
    nums = [random.randint(-1000, 1000) for _ in range(100)]
    for t in [1, 2, 4, 8]:
        dur, out = run_once(nums, t)
        assert out == sorted(nums)
        print(f'Threads: {t}, time: {dur:.3f}s')
