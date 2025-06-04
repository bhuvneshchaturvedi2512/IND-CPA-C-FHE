from sage.all import *
import numpy as np

m = 0
q = 512

def solve_equation(A,B,K, m):
	Z = Zmod(q)
	A_1 = matrix(Z, m, 500, range(m*500))
	for i in range(0,m):
		A_1.set_row(i,A[i])
	b = vector(Z, m, B)
	S = A_1.solve_right(b)
	count = 0
	with open("generated_key.txt","a") as fp:
		for i in range(500):
			fp.write(str(S[i]) + " ")
			if((S[i] == 0 or S[i] == 1) and S[i] == K[i]):
				count = count + 1
			if((S[i] == 511) and S[i] - 512 == K[i]):
				count = count + 1
	print("\n" + str(count) + "-bits has been identified correctly!!")

def convert(str_lst):
	lst = []
	for string in str_lst:
		if (string != ""):
			temp = int(string)
			lst.append(temp)
	return lst

with open("gaussian_elimination.txt","r") as fp:
	lines = fp.readlines()
	m = len(lines)
	
with open("secret_key.txt","r") as fp:
	lines_1 = fp.readline()
key = lines_1.strip().split(" ")
key = convert(key)

A = []
B = []

for line in lines:
	temp = line.strip().split(" ")
	if(len(temp) == 8 and temp[0] == "recovered_error:"):
		a = temp[7].strip().split(",")
		a = convert(a)
		A.append(a)
		message = int(temp[3])
		error = int(temp[1])
		b = int(temp[5])
		b = (b - message) % q
		b = (b - error) % q
		B.append(b)
solve_equation(A,B, key, m)
