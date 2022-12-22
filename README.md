# Multi-Threaded-Hash-Tree
htree.c is driver <br />
the .h files just have useful thread stuff in it and stuff for tracking time<br />
this program uses a binary tree of threads to calculate the hash value from very large files<br />
it is multithreaded so we can have a speedup in time to compute the hash values<br />
the files are divided into n blocks and there are m thread <br />
each thread calculates the hash value of n/m consecutive blocks and sends it to its parent<br />
the parent combines the hash value recived from right and left children and rehashes it<br />
this is repeated until we have a single hash value<br />
hash values should be combined to form a string: <computed hash value + hash value from left child + hash value from right child><br />
the + means concat<br />
there is a report attached with the results of testing the program with some very large files (files were too large to upload)<br />
