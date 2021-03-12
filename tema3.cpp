#include <bits/stdc++.h>

#include <condition_variable>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "mpi.h"

// This could be lowered in order to achieve better performance,
// if this wold have been runned on multiple systems, but this value
// will do just fin in order to beat the cheker
#define MAX_ROWS_AT_ONCE 1000
#define MASTER_PROC_ID 0

void readingTest(const std::string &fileName, const int threadId,
                 std::unordered_map<int, std::vector<std::string>> &paragraphs) {

    // Setting the key word
    std::string keyWord;
    if (threadId == 0) {
        keyWord = "horror";
    } else if (threadId == 1) {
        keyWord = "comedy";
    } else if (threadId == 2) {
        keyWord = "fantasy";
    } else if (threadId == 3) {
        keyWord = "science-fiction";
    }

    std::string fileLine;
    std::ifstream fin(fileName);

    // Counter of the paragraphs, will get to be the value of the key in map 
    int paragraphCount = 0;
    int numParagraphsLines;

    MPI_Status status;

    while (std::getline(fin, fileLine)) {
        if (fileLine == keyWord) {
            std::vector<std::string> paragraphLinesHere = std::vector<std::string>();

            char *line;
            int lineSpitter = 0;
            int numberLines = 0;
            std::string chunkLine = "";

            // Building the chunk, using string for this
            while (std::getline(fin, fileLine)) {
                if (fileLine == "") {
                    break;
                }
                ++numberLines;
                if (lineSpitter < MAX_ROWS_AT_ONCE) {
                    ++lineSpitter;
                    chunkLine += fileLine;
                    chunkLine += '\n';
                } else {
                    paragraphLinesHere.push_back(chunkLine);

                    lineSpitter = 1;
                    chunkLine = fileLine;
                    chunkLine += '\n';
                }
            }

            if (chunkLine != "") {
                paragraphLinesHere.push_back(chunkLine);
            }

            // Sending the number of chunks and the number of actual lines in the paragraph
            numParagraphsLines = paragraphLinesHere.size();
            MPI_Send(&numParagraphsLines, 1, MPI_INT, threadId + 1, 0, MPI_COMM_WORLD);
            MPI_Send(&numberLines, 1, MPI_INT, threadId + 1, 0, MPI_COMM_WORLD);

            // Adding the key and a vector list, of strings, in the map, to retain the lines
            paragraphs.insert(std::pair<int, std::vector<std::string>>(paragraphCount, std::vector<std::string>(numParagraphsLines + 1, "")));
            paragraphs[paragraphCount][0] = keyWord;
            int paragraphLineLength;

            // Seinding for process then receiving processed
            for (int i = 0; i < numParagraphsLines; ++i) {
                paragraphLineLength = paragraphLinesHere[i].size() + 1;
                MPI_Send(paragraphLinesHere[i].c_str(), paragraphLineLength, MPI_CHAR, threadId + 1, 0, MPI_COMM_WORLD);

                // Checking the status in order to know how much info is coming through
                MPI_Probe(threadId + 1, 0, MPI_COMM_WORLD, &status);
                MPI_Get_count(&status, MPI_CHAR, &paragraphLineLength);
                line = new char[paragraphLineLength];
                MPI_Recv(line, paragraphLineLength, MPI_CHAR, threadId + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                paragraphs[paragraphCount][i + 1] = line;

                delete[] line;
            }
        }

        // Inclement each time for overall paragpah evidence
        if (fileLine == "") {
            ++paragraphCount;
        }
    }
    fin.close();

    // Telling the workers that the job is done
    numParagraphsLines = 0;
    MPI_Send(&numParagraphsLines, 1, MPI_INT, threadId + 1, 0, MPI_COMM_WORLD);
}

void processHorror(unsigned int numberLines, unsigned int &currentLineIndex,
                   unsigned int &numProcessedLines,
                   unsigned int &linesUntilNow, std::string *paragraphLines,
                   std::mutex &regionMutex,
                   std::mutex &sendingMutex) {
    unsigned int localLineIndex;

    std::string consonants = "qwrtypsdfghjklzxcvbnm";

    while (1) {
        // Applies the same for each function from now on
        // Get your assigned line index
        regionMutex.lock();
        localLineIndex = currentLineIndex;
        ++currentLineIndex;
        regionMutex.unlock();

        if (localLineIndex >= numberLines) return;

        // Wait for lines to come, then check for consonants and double up
        while (localLineIndex >= linesUntilNow)
            ;

        std::string auxStr = "";
        for (auto c : paragraphLines[localLineIndex]) {
            auxStr += c;
            char auxChr = tolower(c);
            if (consonants.find_first_of(auxChr) != std::string::npos) {
                auxStr += auxChr;
            }
        }

        paragraphLines[localLineIndex] = auxStr;

        // Applies the same for each function from now on
        // Tell the sender-receiver that another line got processed
        sendingMutex.lock();
        ++numProcessedLines;
        sendingMutex.unlock();
    }
}

void processComedy(unsigned int numberLines, unsigned int &currentLineIndex,
                   unsigned int &numProcessedLines,
                   unsigned int &linesUntilNow, std::string *paragraphLines,
                   std::mutex &regionMutex,
                   std::mutex &sendingMutex) {
    unsigned int localLineIndex;

    while (1) {
        regionMutex.lock();
        localLineIndex = currentLineIndex;
        ++currentLineIndex;
        regionMutex.unlock();

        if (localLineIndex >= numberLines) return;

        // Wait for lines to come, then check each word to turn to upper case the letters on even pos
        while (localLineIndex >= linesUntilNow)
            ;

        for (unsigned int i = 0; i < paragraphLines[localLineIndex].size(); ++i) {
            while (paragraphLines[localLineIndex][i] == ' ' && i < paragraphLines[localLineIndex].size()) {
                ++i;
            }

            int localK = 0;

            while (paragraphLines[localLineIndex][i] != ' ' && i < paragraphLines[localLineIndex].size()) {
                if (localK & 1) {
                    paragraphLines[localLineIndex][i] = toupper(paragraphLines[localLineIndex][i]);
                }

                ++i;
                ++localK;
            }
        }

        sendingMutex.lock();
        ++numProcessedLines;
        sendingMutex.unlock();
    }
}

void processFantasy(unsigned int numberLines, unsigned int &currentLineIndex,
                    unsigned int &numProcessedLines,
                    unsigned int &linesUntilNow, std::string *paragraphLines,
                    std::mutex &regionMutex,
                    std::mutex &sendingMutex) {
    unsigned int localLineIndex;

    while (1) {
        regionMutex.lock();
        localLineIndex = currentLineIndex;
        ++currentLineIndex;
        regionMutex.unlock();

        if (localLineIndex >= numberLines) return;

        while (localLineIndex >= linesUntilNow)
            ;

        // Wait for lines to come, then check for each word if the right before character was ' '
        // then turn to upper
        paragraphLines[localLineIndex][0] = toupper(paragraphLines[localLineIndex][0]);

        for (unsigned int i = 1; i < paragraphLines[localLineIndex].size(); ++i) {
            if (paragraphLines[localLineIndex][i - 1] == ' ') {
                paragraphLines[localLineIndex][i] = toupper(paragraphLines[localLineIndex][i]);
            }
        }

        sendingMutex.lock();
        ++numProcessedLines;
        sendingMutex.unlock();
    }
}

void processSF(unsigned int numberLines, unsigned int &currentLineIndex,
               unsigned int &numProcessedLines,
               unsigned int &linesUntilNow, std::string *paragraphLines,
               std::mutex &regionMutex, std::mutex &sendingMutex) {
    unsigned int localLineIndex;

    while (1) {
        regionMutex.lock();
        localLineIndex = currentLineIndex;
        ++currentLineIndex;
        regionMutex.unlock();

        if (localLineIndex >= numberLines) return;

        // Wait for lines to come, start counting the words for each one then turn reverse each 7th word
        while (localLineIndex >= linesUntilNow)
            ;

        int words = 0;
        for (unsigned int i = 0; i < paragraphLines[localLineIndex].size(); ++i) {
            if (paragraphLines[localLineIndex][i] != ' ') {
                ++words;

                if (words == 7) {
                    unsigned int start = i;
                    while (i < paragraphLines[localLineIndex].size() && paragraphLines[localLineIndex][i] != ' ') {
                        ++i;
                    }
                    unsigned int stop = i - 1;

                    while (start < stop) {
                        std::swap(paragraphLines[localLineIndex][start], paragraphLines[localLineIndex][stop]);
                        ++start;
                        --stop;
                    }
                    words = 0;
                } else {
                    while (i < paragraphLines[localLineIndex].size() && paragraphLines[localLineIndex][i] != ' ') {
                        ++i;
                    }
                }
            }
        }

        sendingMutex.lock();
        ++numProcessedLines;
        sendingMutex.unlock();
    }
}

void receiveForProcess(std::function<void(unsigned int, unsigned int &, unsigned int &, unsigned int &,
                                          std::string *, std::mutex &, std::mutex &)>
                           proccesFunction) {
    int linesToRecv;
    int numberLines;
    int lineLength;

    // System call for concuret threads (C++ 11 or better)
    unsigned int numberThreads = std::thread::hardware_concurrency();
    --numberThreads;

    char *longLine;

    // Loop finishes when getting 0, JOB'S DONE!!
    do {
        MPI_Recv(&linesToRecv, 1, MPI_INT, MASTER_PROC_ID, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if (linesToRecv <= 0) break;
        MPI_Recv(&numberLines, 1, MPI_INT, MASTER_PROC_ID, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        unsigned int threadsToWork = 0;
        threadsToWork = numberLines / 20 + 1;
        threadsToWork = std::min(threadsToWork, numberThreads);

        std::thread threads[threadsToWork];

        unsigned int currentLineIndex = 0;
        unsigned int linesRecievedTillNow = 0;
        unsigned int processedLinesUntilNow = 0;

        std::mutex regionMutex;
        std::mutex sendingMutex;

        // Long chunk undecoded and decoded lines
        std::vector<std::string> longParagraphLines = std::vector<std::string>(linesToRecv, "");
        std::string *paragraphLines = new std::string[numberLines]();

        for (unsigned int i = 0; i < threadsToWork; ++i) {
            threads[i] =
                std::thread(proccesFunction, numberLines, std::ref(currentLineIndex),
                            std::ref(processedLinesUntilNow), std::ref(linesRecievedTillNow),
                            paragraphLines, std::ref(regionMutex), std::ref(sendingMutex));
        }

        MPI_Status status;
        for (int i = 0; i < linesToRecv; ++i) {
            MPI_Probe(MASTER_PROC_ID, 0, MPI_COMM_WORLD, &status);
            MPI_Get_count(&status, MPI_CHAR, &lineLength);

            // Allocating the memory
            longLine = new char[lineLength];
            MPI_Recv(longLine, lineLength, MPI_CHAR, MASTER_PROC_ID, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            longParagraphLines[i] = longLine;

            std::string::size_type posPrev = 0;
            std::string::size_type posNext = 0;
            int countAddWords = 0;

            // Decoding the chunk and telling the workers there are new lines to be processed
            while ((posNext = longParagraphLines[i].find('\n', posPrev)) != std::string::npos) {
                paragraphLines[linesRecievedTillNow + countAddWords] =
                    longParagraphLines[i].substr(posPrev, posNext - posPrev);

                posPrev = posNext + 1;
                ++countAddWords;
            }

            linesRecievedTillNow += countAddWords;
            while (processedLinesUntilNow < linesRecievedTillNow)
                ;

            // Rebuild and and deliver to master
            std::string chunkLine = "";
            do {
                chunkLine += paragraphLines[linesRecievedTillNow - countAddWords];
                chunkLine += '\n';
            } while (--countAddWords);

            int lenChunkSize = chunkLine.size() + 1;
            MPI_Send(chunkLine.c_str(), lenChunkSize, MPI_CHAR, MASTER_PROC_ID, 0, MPI_COMM_WORLD);

            delete[] longLine;
        }

        for (unsigned int i = 0; i < threadsToWork; ++i) {
            threads[i].join();
        }

        delete[] paragraphLines;

    } while (linesToRecv != 0);
}

int main(int argc, char *argv[]) {
    int mpiThreadProvided, rank;

    if (argc < 2) {
        std::cout << "arguments not valid, please give file name\n";
        return 0;
    }

    std::string fileName = argv[1];

    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &mpiThreadProvided);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (mpiThreadProvided < MPI_THREAD_MULTIPLE) {
        std::cout << "Thread not okay!\n";
        exit(1);
    }

    if (rank == MASTER_PROC_ID) {

        // Will be used for retaining the processed lines
        std::unordered_map<int, std::vector<std::string>> paragraphs = std::unordered_map<int, std::vector<std::string>>();
        std::thread threads[4];

        // Opening one thread for each genre
        for (int i = 0; i < 4; ++i) {
            threads[i] = std::thread(readingTest, fileName, i, std::ref(paragraphs));
        }

        for (int i = 0; i < 4; ++i) {
            threads[i].join();
        }

        // Serving the output
        std::ofstream fout(fileName.substr(0, 18) + "out");
        for (unsigned int i = 0; i < paragraphs.size(); ++i) {
            fout << paragraphs[i][0] << "\n";

            for (unsigned int j = 1; j < paragraphs[i].size(); ++j) {
                fout << paragraphs[i][j];
            }
            fout << '\n';
        }
        fout.close();

    } else {

        // Each main worker responsible with a genre calls the processing receiver with it's specific method as param
        if (rank == 1) {
            receiveForProcess(&processHorror);
        } else if (rank == 2) {
            receiveForProcess(&processComedy);
        } else if (rank == 3) {
            receiveForProcess(&processFantasy);
        } else if (rank == 4) {
            receiveForProcess(&processSF);
        }
    }

    MPI_Finalize();
    return 0;
}