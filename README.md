# Disk-File-System-simulation

Authored by Guy Cohen

==Description==
Disk Management System - is the way in which file names and their contents are located and organized on the hard disk. Without the file system, the information stored will not be organized into individual files and it will be impossible to locate and access their contents.
The file's parts are saved on small units - the blocks.
The function of the disk management system is to map all the parts of the file that are stored on the disk.

This program is a simulation of managing the disk system by the method Indexed allocation. This program  will simulate this approach. 
 It will simulate the way that the files' names, their location, and their contents are organized on the hard disk.

There are three data structures / class:
Fsfile-> information about the files.
FileDescriptor ->pointer to the FsFile of the files and the files' name.
FsDisk-> information about the disk.
The user can:
delete files and content from the disk and exit->(0) 
print the disk contant, and some another information  (1) 
format the disk->(2) 
create file->(3)
open file->(4)
close file->(5)
write to file-> (6)
read from file-> (7)
delete file->(8)


==Program DATABASE==
There are 3 important data structures in this exercise:
1. MainDir-> The only directory in this simulation, it is a vector that save the information on all existing files, both open and closed.
2 OpenFileDescriptors-> is a vector that save the information on all existing files that are open right now.
3. sim_disk_fd-> a text file that simulate the disk. 

==functions==
As mentioned there are a number of useful functions in this simulation:

fsFormat (blockSize) -> Formats the disk and therefore accepts a variable that is the size of the block.

CreateFile (fileName) -> Getting the file name, and checking that it is unique,creates new fsFile, and updates MainDir and the OpenFileDescriptors with a new file.

WriteToFile (_fd, str_to_write, strlen (str_to_write)) -> The function finds free blocks on disk to write down the data it receives. And writes them.
In addition it will always check how much space is left in the last block before assigning a new block.

ReadFromFile (_fd, str_to_read, size_to_read) -> The function captures
From the file descriptor user of a file to be read from, plus the number of characters to be read.
The function will go to the appropriate file, from there to the appropriate blocks and finally return the characters we requested and print them.

fs->DelFile(fileName)->delete the file and all its data.


==Program Files==
code_stub.cpp

==How to compile?==
compile: g++ code_stub.cpp -o main
run: ./main

==Output:==
You can print the contents of the disk and know which of the files are open and in addition get information about them by calling the listAll function





