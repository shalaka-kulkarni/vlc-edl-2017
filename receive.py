import serial
import sys
from time import time

conn = serial.Serial('COM3',115200,timeout=0.01)
print("We have a connection")
while(1):

  data = []
  num_char=0
  while(1):
    r = conn.read(1)
    if(r==b'2'):
      break
    elif(r==b'0' or r==b'1'):
      num_char=num_char+1
      #print(num_char,r)
      data = data+[r]
        
  
  
  #letter_v_array=['0','1','0','1','0','1','1','0']
  #final_data=""
  #data=[ int(x) for x in data ]
  #print(data)
  #print (len(data))
  #print(data[0:7])
  #for i in range(0,96):
   #       if((data[i:i+7])==letter_v_array):
    #              final_data =data[i:i+23]
   #               break
          
  #print(final_data)
 
  char_size =8
  array=""

  for i in range(0,int(len(data)/char_size)):
          encoded_char =data[i*char_size:(i+1)*char_size]
          #print(len(encoded_char))
          #print(encoded_char)
          ascii_val=0
          for j in encoded_char:
            ascii_val= (ascii_val * (2))+ int(j)
            #array.append(chr(ascii_val))
          array = array+chr(ascii_val)
          
  print(array)		
		

  
