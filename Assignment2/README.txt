Alexander Chatron-Michaud, 260611509.
This work is mine and mine alone.

In order to compile and run this on trottier machines, simply do a make all.
Printer takes one argument, the size of the printer’s queue.
Client takes three arguments, it’s client number, the number of pages to print, and the duration.

I have used a semaphore to have the printer wait when the queue is empty, another to have clients wait when the queue is full, and another to create a critical section, all as specified in the assignment pdf. 

Included is a test.sh bash script which runs the program and gives the following correct and expected output when using a printer with a queue size of 4:

Printer is currently waiting for jobs.
Client 1 has to print 11 pages, puts request in Buffer[0]
Printer starts printing 11 pages from Buffer[0]
Client 2 has to print 12 pages, puts request in Buffer[1]
Client 3 has to print 13 pages, puts request in Buffer[2]
Client 4 has to print 14 pages, puts request in Buffer[3]
Client 5 has to print 15 pages, puts request in Buffer[0]
Client 6 has 16 pages to print, Printer buffer full, going to sleep
Printer finishes printing 11 pages from Buffer[0]
Printer starts printing 12 pages from Buffer[1]
Client 6 is woken up and puts request in Buffer[1]
Printer finishes printing 12 pages from Buffer[1]
Printer starts printing 13 pages from Buffer[2]
Client 7 is woken up and puts request in Buffer[2]
Printer finishes printing 13 pages from Buffer[2]
Printer starts printing 14 pages from Buffer[3]
Printer finishes printing 14 pages from Buffer[3]
Printer starts printing 15 pages from Buffer[0]
Printer finishes printing 15 pages from Buffer[0]
Printer starts printing 16 pages from Buffer[1]
Printer finishes printing 16 pages from Buffer[1]
Printer starts printing 17 pages from Buffer[2]
Printer finishes printing 17 pages from Buffer[2]
Printer is currently waiting for jobs.

If you want to run this bash script, please `pkill printer` between runs. I tested this using ssh to trottier so if the output is different during grading I’d be very surprised. If so let me know at achatr@cim.mcgill.ca if there’s something wrong with my program and I need to explain.