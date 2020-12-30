/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   QueryHandler.cpp
 * Author: mahdi
 * 
 * Created on September 13, 2017, 11:32 AM
 */

#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cmath>
#include <random> 
#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string>
#include <chrono>
#include <ctime>

#include "QueryHandler.hpp"
#include "lib.hpp"

using namespace std;

QueryHandler::QueryHandler() {
}

QueryHandler::QueryHandler(DBManager* t_DB, const int t_socket, const int t_verbose) {
    m_DB = t_DB;
    m_clientSocket = t_socket;
    m_verbose = t_verbose;
    m_M = t_DB->getM();
    m_C = t_DB->getDBbufferSize()/m_DB->getN();      //C is the number of rows in the db chunk
    m_replySize = m_DB->getN();
    
    if (m_verbose > 2) cout << "New QueryHandler. parameters:";
    if (m_verbose > 2) cout << " client=" << m_clientSocket;
    if (m_verbose > 2) cout << " M=" << m_M;
    if (m_verbose > 2) cout << " m_qdbSize=" << m_replySize << endl;
    if (m_verbose > 2) cout << " N=" << m_DB->getN();
    if (m_verbose > 2) cout << " totalAvailRAM=" << t_DB->getSystemParm()->getTotalAvailRAM();
    if (m_verbose > 2) cout << " chunkSize=" << t_DB->getSystemParm()->getMaxChunkSize();
    if (m_verbose > 2) cout << " C=" << m_C << endl;;
}

int QueryHandler::processOneQuery() {
    m_C = m_DB->getDBbufferSize()/m_DB->getN();     //if DB can fit entirely to memory, then C==M else C<M
    if (m_C % 8 != 0) {
        cout << "The number of rows in the database is not multiple of 8" << endl;
        exit(0);
    }
    m_replySize = m_DB->getN();
    m_querySize = m_C/8;
    size_t m=0, n=0, dbParser=0; int mask=0;
    uint64_t* filePointer64;
    uint64_t* replyPointer64 = (uint64_t*) m_reply;
    for (m=0; m<m_C; m++) {	//M is the number of files
        filePointer64 = (uint64_t*) (m_DB->getDBbuffer()+dbParser);
        if ((uint8_t)m_query[m/8] & (uint8_t)m_masks[mask])		//queriesBuffer[a][m/8]
            for (n=0; n<m_replySize/8; n++)	//N is the file size in bytes
                replyPointer64[n] ^= filePointer64[n];
        mask=(mask+1)&7;	//mask=(mask+1)%8;
        dbParser+=m_replySize;
    }
    return 0;
}

int QueryHandler::processQueries(char* DBstream, char* queriesBuffer, size_t numberOfQueries, char* repliesBuffer){

    m_replySize = m_DB->getN();
    size_t N = m_DB->getN();
    m_querySize = m_C/8;
    
    size_t M=m_C; 
    size_t querySize = m_querySize;
    unsigned char masks[8] = {0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};
    size_t m=0, a=0, n=0; int mask=0;
    size_t mm=0, nn=0;
    size_t B1=128;		//B1 splits M (the number of files)
    size_t B2=128;		//B2 splits N, (size of 1 file) B2 should be multiple of 8 to XOR in 64bits
    uint64_t* filePointer64 = (uint64_t*) DBstream;
    uint64_t* replyPointer64;
    
    chrono::time_point<chrono::system_clock> start, end;
    chrono::duration<double> elapsed_seconds;
                    start = chrono::system_clock::now();
    
    for (mm=0; mm<M; mm+=B1) {
        if (m_verbose) cout << mm << "/" << M << endl;
        for (nn=0; nn<N/8; nn+=B2) {
            for (a=0; a<numberOfQueries; a++) {
                for (m=mm; m<min(mm+B1,M); m++) {	//M is the number of files
                    if ((uint8_t)queriesBuffer[a*querySize+m/8] & (uint8_t)masks[mask]) {		//X[a,m/8]
                        replyPointer64 = (uint64_t*) (repliesBuffer+a*N);	//Z[a,0]
                        for (n=nn; n<min(nn+B2,N/8); n++)	//N is the file size in bytes
                                replyPointer64[n] ^= filePointer64[m*N/8+n];	//Z[a,n] ^= Y[m,n]
                    }
                    //mask=(mask+1)&7;	//mask=(mask+1)%8;
                    mask=(mask+1)%8;
                }
            }
        }
    }
		end = chrono::system_clock::now();
		elapsed_seconds = end-start;
		cout << " ---------total time= " << elapsed_seconds.count();
		cout << endl;
    return 0;
}

//
//int QueryHandler::processMultipleQueries(char* DBstream, char* queriesBuffer, size_t numberOfQueries, char* repliesBuffer) {
//    m_C = m_DB->getDBbufferSize()/m_DB->getN();     //if DB can fit entirely to memory, then C==M else C<M
//    if (m_C % 8 != 0) {
//        cout << "The number of rows in the database is not multiple of 8" << endl;
//        exit(0);
//    }
//    m_replySize = m_DB->getN();
//    m_querySize = m_C/8;
//    size_t m=0, a=0, n=0, dbParser = 0; int mask=0;
//    uint64_t* filePointer64;
//    uint64_t* replyPointer64;
//    for (m=0; m<m_C; m++) {	//M is the number of files
//        filePointer64 = (uint64_t*) (DBstream+dbParser);
//        for (a=0; a<numberOfQueries; a++) {
//            if ((uint8_t)queriesBuffer[a*m_C/8+m/8] & (uint8_t)masks[mask]) {		//queriesBuffer[a][m/8]
//                // cout << "hit!m=" << m << " a=" << dec << a << " mask=" << hex << unsigned(mask) << " masks[mask]=" << hex << unsigned(masks[mask]) << " (uint8_t)queriesBuffer[a*M/8+m/8]=" << unsigned((uint8_t)queriesBuffer[a*M/8+m/8]) << " dbParser=" << dbParser << endl;
//                replyPointer64 = (uint64_t*) (repliesBuffer+a*m_replySize);
//                for (n=0; n<m_replySize/8; n++){	//N is the file size in bytes
//                    // if (n==0)
//                    // 	cout << "m=" << m << " before replyPointer64[n]=" << hex << replyPointer64[n] << " filePointer64[n]=" << filePointer64[n] << endl;
//                    replyPointer64[n] ^= filePointer64[n];
//                    // if (n==0)
//                    // 	cout << "m=" << m << " after replyPointer64[n]=" << hex << replyPointer64[n] << endl;
//                }
//            }
//        }
//        mask=(mask+1)&7;	//mask=(mask+1)%8;
//        dbParser+=m_replySize;
//    }
//    return 0;
//}
//
//int QueryHandler::processMultipleQueriesWithTransposing(char* DBstream, char* queriesBuffer, size_t numberOfQueries, char* repliesBuffer) {
//    m_C = m_DB->getDBbufferSize()/m_DB->getN();     //if DB can fit entirely to memory, then C==M else C<M
//    if (m_C % 8 != 0 || numberOfQueries % 8 != 0) {
//        cout << "The number of rows in the database Or the number of parallel queries is not multiple of 8" << endl;
//        exit(0);
//    }
//    m_replySize = m_DB->getN();
//    m_querySize = m_C/8;
//    size_t m=0, a=0, n=0, parser = 0; int mask=0;
//    uint64_t* filePointer64;
//    uint64_t* replyPointer64;
//    char* transQueriesBuffer = (char*) malloc (m_querySize*numberOfQueries*sizeof(char));
//    //transposing the queries buffer
//    sseTransose(queriesBuffer, transQueriesBuffer, numberOfQueries, m_querySize*8);
//    for (m=0; m<m_C; m++) {
//        filePointer64 = (uint64_t*) (DBstream+m*m_replySize);
//        for (a=0; a<numberOfQueries/8; a++) {
//            for (mask=0; mask<8; mask++) {
//                if ((uint8_t)transQueriesBuffer[parser] & (uint8_t)masks[mask]) {
//                    replyPointer64 = (uint64_t*) (repliesBuffer+(a*8+mask)*m_replySize);
//                    for (n=0; n<m_replySize/8; n++)
//                        replyPointer64[n] ^= filePointer64[n];
//                }
//            }
//            parser++;
//        }
//    }
//    return 0;
//}

int QueryHandler::sseTransose(char* inBuff, char* outBuff, size_t numberOfRows, size_t numberOfColumns) {
    if (numberOfRows % 8 != 0) {
        cout << "numberOfRows % 8 != 0" << endl;
        exit(0);
    }
    #define HEX(x) setw(2) << setfill('0') << hex << (int)(x)
    size_t row=0, col=0;
    int i;
    uint16_t transposedRow;
    union { __m128i x; uint8_t b[16]; } tmp;
    
    //The main body in 16x8 blocks
    for (row=0; row+16<=numberOfRows; row+=16) {
        //load 16 rows to memory
        for (col=0; col<numberOfColumns; col+=8) {
            for (i=0; i<8; i++)
                tmp.b[7-i] = inBuff[(row+i)*numberOfColumns/8+col/8];
            for (i=8; i<16; i++)
                tmp.b[15-i+8] = inBuff[(row+i)*numberOfColumns/8+col/8];
            for (i=0; i<8; i++) {
                transposedRow = _mm_movemask_epi8(tmp.x);
                size_t targetAddr = ((col+i)*numberOfRows/8+row/8);
                *reinterpret_cast<uint16_t*>(outBuff+targetAddr) = transposedRow;
                tmp.x = _mm_slli_epi64(tmp.x, 1);
            }
        }
    }
    if (row==numberOfRows) return 0;

    //8 rows remaining, Process the remaining rows in 8x8 units
    for (i=0; i<16; i++)
        tmp.b[i] = 0;
    for (col=0; col<numberOfColumns; col+=8) {
        for (i=0; i<8; i++)
            tmp.b[7-i] = inBuff[(row+i)*numberOfColumns/8+col/8];
        for (i=0; i<8; i++) {
            transposedRow = _mm_movemask_epi8(tmp.x);
            size_t targetAddr = ((col+i)*numberOfRows/8 + row/8);
            *(outBuff + targetAddr) = transposedRow;
            tmp.x = _mm_slli_epi64(tmp.x, 1);
        }
    }
}

int QueryHandler::sendBackReplyToClient() {
    char *dataBuffer;
    dataBuffer = (char *) malloc (DATA_BUFFER_SIZE * sizeof (char));
    bzero(dataBuffer, DATA_BUFFER_SIZE);
    size_t numberOfBytesToSendFromSdb = m_C;
    char* currentSdb = m_reply;
    size_t minSize = 0;
    do {
        minSize = min(numberOfBytesToSendFromSdb, DATA_BUFFER_SIZE);
        memcpy(dataBuffer,currentSdb,minSize);
        currentSdb += minSize;
        int sockErr = write(m_clientSocket, dataBuffer, DATA_BUFFER_SIZE);
        if (sockErr < 0) error("ERROR sending X file size");
        numberOfBytesToSendFromSdb -= minSize;
   } while(numberOfBytesToSendFromSdb > 0);
   return 0;
}

int QueryHandler::setQuery(char* t_query) { 
    m_query = t_query; 
    m_replySize = m_DB->getN();
    m_reply = (char*) calloc (m_replySize,sizeof(char));
}

char* QueryHandler::getReply() { return m_reply; }
size_t QueryHandler::getReplySize() { return m_replySize; }
size_t QueryHandler::getQuerySize() { return m_querySize; }

QueryHandler::QueryHandler(const QueryHandler& orig) {
}

QueryHandler::~QueryHandler() {
}

