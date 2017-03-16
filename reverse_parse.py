
bit_per_sym =2
char_size =8/bit_per_sym

with open('Output_symbol_file.txt') as f:
	array = []
	for line in f:
		for i in range(0,len(line)/char_size):
			encoded_char =line[i*char_size:(i+1)*char_size]
			ascii_val=0
			for j in encoded_char:
				ascii_val= (ascii_val * (2**bit_per_sym))+int(j)
			array.append(chr(ascii_val))

with open('reverse_parse_file.txt', "w") as output:
	for i in array:
		output.write(i)			
