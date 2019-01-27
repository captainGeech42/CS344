#!/usr/bin/env python36

# program: Program Py
# author: Alexander Nead-Work
# class/prof: cs344, w2019, Justin Goins

# usage: ./mypython.py
# (`python3 ./mypython.py` also works, but is not recommended)

# resources used:
# - https://stackoverflow.com/a/2257449 (random string generation)

import os, random, string, sys

def get_random_str(n: int) -> str:
	"""Generate a random string of length n"""
	return ''.join(random.choices(string.ascii_lowercase, k=n))

def main():
	for _ in range(3):
		with open(get_random_str(10), "w") as f:
			f.write("{}\n".format(get_random_str(10)))

	return 0

if __name__ == "__main__":
	sys.exit(main())
