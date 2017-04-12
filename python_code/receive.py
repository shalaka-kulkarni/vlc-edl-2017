import serial
import sys
from time import time

conn = serial.Serial('COM5',115200,timeout=0.01)
print("We have a connection")

with open('received_data.txt', "w") as outfile:
  data = ""
  while(1):
    r = conn.read(1)
    if(r!=b'1' and r!=b'0' and r!=b''):
      break
    if(r!=b''):
      print(r)
      data = data+r.decode("utf-8")
      
  print(data)
  outfile.write(data)
  outfile.close()

bit_per_sym =1
char_size =int(8/bit_per_sym)


bit_per_sym =1
char_size =int(8/bit_per_sym)

with open('received_data.txt') as f:
	array = []
	for line in f:
		for i in range(0,int(len(line)/char_size)):
			encoded_char =line[i*char_size:(i+1)*char_size]
			ascii_val=0
			for j in encoded_char:
				ascii_val= (ascii_val * (2**bit_per_sym))+int(j)
			array.append(chr(ascii_val))

with open('reverse_parse_file.txt', "w") as output:
	for i in array:
		output.write(i)			
		

  
