#!/usr/bin/env python3

# program: Program Py
# author: Alexander Nead-Work
# class/prof: CS344, W2019, Justin Goins

# usage: ./mypython.py
# (`python3 ./mypython.py` also works, but is not recommended)

# resources used:
# - https://stackoverflow.com/a/2257449 (random string generation)
# - https://docs.python.org/3.5/library/random.html (random int)
# - https://stackoverflow.com/a/13840441 (multiplying list contents)


from functools import reduce
import os, random, string, sys


def get_random_str(n):
	"""Generate a random string of length n"""
	return ''.join(random.choice(string.ascii_lowercase) for _ in range(n))


def make_files(n, p, q):
	"""Make n files with filenames of length p, with random contents of length q"""
	files = []
	for i in range(n):
		files.append(get_random_str(p))
		with open(files[i], "w") as f:
			f.write("{}\n".format(get_random_str(q)))
	return files


def print_files(f):
	"""Prints the contents of each file in f"""
	for file in f:
		with open(file, "r") as f:
			print(''.join(f.readlines()), end='')


def get_random_int(min, max):
	"""Generate a random int in range [min, max]"""
	return random.randint(min, max)


def print_rands(n, min, max):
	"""Generates n random ints in range [min, max], prints them, and prints the product"""
	rands = []
	for _ in range(n):
		rands.append(get_random_int(min, max))

	prod = reduce(lambda a, b: a*b, rands)

	for r in rands:
		print(r)

	print(prod)


def main():
	files = make_files(3, 10, 10)

	print_files(files)

	print_rands(2, 1, 42)

	return 0

if __name__ == "__main__":
	sys.exit(main())
