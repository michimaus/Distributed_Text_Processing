# Distributed_Text_Processing

DUICAN MIHNEA - IONUȚ
334CA
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ README TEMA 3 APD ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Master:
    - There is one function for the paragraph reading fork, where at first it does is
    to set the keyword for the paragraphs, according to the id of the calling thread.
    Afterwards, it starts to read the whole text, and to check for the keyword
    in order to start comunicating with the workers.
    
    - Then, it stores the whole paragraph in a buffer (string) and sends chunks of
    1000 rows to the workers, concatenated(used '\n' as separator). Each time a
    chunk gets sent, the master-thread imediatly awaits for the processed line
    and stores it into an unordered map.
     
    - The storing key for the map it's a local variable specific for each thread,
    but the map is referanced in the siganture of the function, comming from main.

Workers:
    - The workers have the ranks 1, 2, 3, 4, assigned, meanwhile 0 is for master. The rank
    is check then it gets called a function witch takes as paramether.. well yes, 
    another function (the second one is specific for the assigned genre to precess).
    
    - Right afer the call the local variables to use are initailised, it's made
    a systemcall to get the number of concurent threads available, then it's getting
    into the loop, to receive the number of lines about to come for processing
    in order to know how many threads should get on working for the incoming paragaph.

    - One thread receives the input and starts deconding the lines to be proecessed,
    as the chunk conatines multiple lines concatenated as discussed before. Meanwhile the
    varaibles and the lines array are referenced in the genre specific functions, which are
    called for creating new threads, in order to work on the same memory addres at once.

    - As soon as a chunk gets processed, it's sent back to master imediatly.

Techinque for workers:
    - Producer - consumer procedure was the way to go for me, with busy waiting (wait/notify
    was quite tricky to use with c++).
    
    - Used busy waiting for those threads waiting for
    lines to come to be processed and backwards respectively, for the thread responsible
    with sending the processed chunk back to master.




