# Name: Wei Huang
# Date: 11/11/2019
# Description: Samll ramdom program
#reference: https://github.com/TylerC10/CS344/blob/master/Small%20Python%20Project/mypython.py
#           https://pythontips.com/2013/07/28/generating-a-random-string/

#!/user/bin/python
# -*- coding: utf-8 -*-
import os
import random
import string

# get the random number(whose range is from 1 to 42) and  the product of the two numbers
def random_num():
    num1 =random.randint(1,42)
    print (num1)
    
    num2 =random.randint(1,42)
    print (num2)

    print (num1 * num2)
       #print product

# create the files, and random the string in the files
def bulit_files():
    file1 = open("myfile1", 'w')   #open files to write
    file2 = open("myfile2", 'w')
    file3 = open("myfile3", 'w')
    f = [file1,file2,file3]  # file list
    for file in f:
        letters = ''.join(random.sample(string.ascii_lowercase,10)) #get random string into the file
        letters = letters + "\n"
        file.write(letters)
        file.close()

    file1 = open("myfile1", 'r') #open the files to read
    file2 = open("myfile2", 'r')
    file3 = open("myfile3", 'r')
    f =[file1,file2,file3] #file list
    for file in f:
        print(file.read(10)) #print the files


    # main function
def main():
    bulit_files()
    random_num()

main()
