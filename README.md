# Multi-Threaded-Hash-Tree
htree.c is driver
the .h files just have useful thread stuff in it and stuff for tracking time
this program uses a binary tree of threads to calculate the hash value from very large files
it is multithreaded so we can have a speedup in time to compute the hash values
the files are divided into n blocks and there are m thread 
each thread calculates the hash value of n/m consecutive blocks and sends it to its parent
the parent combines the hash value recived from right and left children and rehashes it
this is repeated until we have a single hash value
hash values should be combined to form a string: <computed hash value + hash value from left child + hash value from right child>
the + means concat
there is a report attached with the results of testing the program with some very large files (files were too large to upload)
