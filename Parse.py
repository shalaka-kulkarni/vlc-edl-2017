with open('Text_file.txt') as f:
	array = []
	char=0;
	for line in f:
		print line
		for i in line:       
		    char = ord(i)
		    print 'char '+i+"  "+str(char) 
		    array.append("{0:08b}".format(char))
print array

with open('Output_bin_file.txt', "w") as output:
	for char in array:
		output.write(char) 

bit_per_sym =2
with open('Output_symbol_file.txt', "w") as output:
	for char in array:
		for i in range(0,(8/bit_per_sym)):
			substr =char[i*bit_per_sym:(i+1)*bit_per_sym]
			symbol=int(substr,2)
			output.write(str(symbol))

