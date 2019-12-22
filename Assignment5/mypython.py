# Branden Holloway
# CS344, ProgramPy
# July 25, 2019

import os
import random
import string

#https://pynative.com/python-generate-random-string/
#https://www.geeksforgeeks.org/reading-writing-text-files-python/
#https://www.pythoncentral.io/how-to-generate-a-random-number-in-python/

LENGTH = 10

#Make File 1, populate it with random string of lowercase characters, print them to screen
F1 = open("File1","w+")
def RanLetters1(LENGTH):
    letters = string.ascii_lowercase
    return ''.join(random.choice(letters) for i in range(LENGTH))
    #str1 = RanLetters1(LENGTH)
F1.write(RanLetters1(LENGTH) + "\n")
F1.close()
F2 = open("File1", "r+")
print F2.read(10)

#Make File 2, populate it with random string of lowercase characters, print them to screen
F2 = open("File2","w+")
def RanLetters1(LENGTH):
    letters = string.ascii_lowercase
    return ''.join(random.choice(letters) for i in range(LENGTH))
    #str1 = RanLetters1(LENGTH)
F2.write(RanLetters1(LENGTH) + "\n")
F2.close()
F2 = open("File2", "r+")
print F2.read(10)

#Make File 3, populate it with random string of lowercase characters, print them to screen
F3 = open("File3","w+")
def RanLetters1(LENGTH):
    letters = string.ascii_lowercase
    return ''.join(random.choice(letters) for i in range(LENGTH))
    #str1 = RanLetters1(LENGTH)
F3.write(RanLetters1(LENGTH) + "\n")
F3.close()
F3 = open("File3", "r+")
print F3.read(10)

#Pick two random numbers within the given range, print them to the screen, then print the product of the two
def RanNum1():
    for x in range(1):
        num1 = random.randint(1,42)
        print num1
    for y in range(1):
        num2 = random.randint(1,42)
        print num2
    print num1 * num2

RanNum1()


