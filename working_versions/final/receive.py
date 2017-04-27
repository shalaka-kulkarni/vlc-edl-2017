import serial
import sys
from time import time

conn = serial.Serial('COM3',115200,timeout=0.01)
print("We have a connection")
flag=1
while(flag):
  data = []
  while(1):
    r = conn.read(1) #reading data from the com  port
    if(r!=b'1' and r!=b'0' and r!=b'' and r!=b'3'):
      break
    if(r!=b''):
      
      data.append(r) #adding bits received into a list
    if(r==b'3'):
      print("Link Blocked")

  data =[int(x) for x in data] # converting bits to int

 
  


  char_size=8; # each character is represented by 8 bits
  array=""                
  for i in range(0,int(len(data)/char_size)):
    encoded_char =data[i*char_size:(i+1)*char_size] # picking out 8 bits that represent a character
    ascii_val=0
    for j in encoded_char:
      ascii_val= (ascii_val * (2))+int(j)# finding the ascii value of the character from the bits.
    array=array+(chr(ascii_val)) # converting ascii code to characters and adding to the string
  print(""+array)
  print("")
  print("")
  print("")
  print("")
  print("")
  flag=1
		
		

  
