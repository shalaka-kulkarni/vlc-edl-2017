import serial
import sys
from time import time


no_of_bytes = 0
with open('Text_file.txt') as f:
	array = []
	char=0;
	for line in f:
		print (line)
		for i in line:       
		    char = ord(i)
		    no_of_bytes = no_of_bytes + 1
		    print ('char '+i+"  "+str(char))
		    array.append("{0:08b}".format(char))
print (array)
print (no_of_bytes)

with open('Output_bin_file.txt', "w") as output:
	for char in array:
		output.write(char) 

bit_per_sym =1
no_of_bits = no_of_bytes*8

with open('outfile.txt', "w") as output:
  output.write(str('0'))
  output.write(str(no_of_bits))
  for char in array:
    for i in range(0,int(8/bit_per_sym)):
      substr =char[i*bit_per_sym:(i+1)*bit_per_sym]
      symbol=int(substr,2)
      output.write(str(symbol))
  output.write(str('2'))
  output.close()

			
conn = serial.Serial('COM5',115200)
print("Opened COM port")

filename = 'outfile.txt'

with open(filename) as f:
  while True:
    c = f.read(1)
    if not c:
      print ("File sent")
      break
    conn.write(c.encode('ascii'))
    print(c)
  conn.close()
