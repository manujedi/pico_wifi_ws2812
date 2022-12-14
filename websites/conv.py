#!/usr/bin/python

import sys
import glob

f = open("websites.h", "w")

f.write("#ifndef _FILES_H\n")
f.write("#define _FILES_H\n\n")


#--------------http header------------------
#pre_len = "HTTP/1.1 200 OK\nContent-Type: text/html; charset=utf-8\nContent-Length: 00000\n\n"
#arraystring = "char httpheader [] = {"
#file_len = 0
#
#for c in pre_len:
#    arraystring += hex(ord(c)) + ", "
#    file_len+=1
#
#arraystring = arraystring[:-2]
#arraystring += "};\n"
#
#f.write("#define httpheader_LEN " + str(file_len) + "\n")
#f.write(arraystring + "\n")


for arg in glob.glob('*.html'):
    arraystring = "static char  __attribute__((unused)) " + arg[:arg.index(".")] + "[] = {"
    file_len = 0
    with open(arg, 'rb') as rfile:
        content = rfile.read()

        for c in content:
            arraystring += hex(c) + ", "
            file_len+=1

        arraystring += "0x0"    #add EOF
        file_len+=1;

        arraystring += "};\n"

    f.write("#define " + arg[:arg.index(".")] + "_LEN " + str(file_len) + "\n")
    f.write(arraystring)

f.write("\n#endif\n")
f.close()