def convert(str_lst):
	lst = []
	for string in str_lst:
		if (string != ""):
			temp = int(string)
			lst.append(temp)
	return lst
	
with open('vector_e_original.txt', 'r') as fp:
	lines = fp.readlines()
	temp = lines[0].strip().split(" ")
	ev_1 = convert(temp)
	temp = lines[1].strip().split(" ")
	ev_2 = convert(temp)
	
with open('recovered_errors.txt', 'r') as fp:
	lines = fp.readlines()
	temp = lines[0].strip().split(" ")
	ev_r = convert(temp)
count = 0	
n = len(ev_r) #error vector size

for i in range(n):
	ev_ac = -1 * (ev_1[i] + ev_2[i])
	if(ev_ac != ev_r[i]):
		print("Mismatch in bit: " + str(i + 1) + " actual error: " + str(ev_ac) + " recovered error: " + str(ev_r[i]) + "\n")
		count = count + 1

no_of_queries = 0 #Counts number of queries made to the conditional decryption oracle
		
with open('Number_of_decryptions.csv', 'r') as fp:
	lines = fp.readlines()
	for line in lines:
		no_of_queries = no_of_queries + int(line.strip())
		
print("Total number of queries made to the conditional decryption oracle: " + str(no_of_queries))
