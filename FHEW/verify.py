def convert(str_lst):
	lst = []
	for string in str_lst:
		if (string != ""):
			temp = int(string)
			lst.append(temp)
	return lst
	
with open('generated_key.txt', 'r') as fp:
	lines = fp.readlines()
	temp = lines[0].strip().split(" ")
	gk = convert(temp)
	
with open('secret_key.txt', 'r') as fp:
	lines = fp.readlines()
	temp = lines[0].strip().split(" ")
	sk = convert(temp)

no_of_queries = 0 #Counts number of queries made to the conditional decryption oracle
		
with open('Number_of_decryptions.csv', 'r') as fp:
	lines = fp.readlines()
	for line in lines:
		no_of_queries = no_of_queries + int(line.strip())
		
print("Total number of queries made to the conditional decryption oracle: " + str(no_of_queries))
