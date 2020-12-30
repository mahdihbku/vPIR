/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   XDBGenerator.cpp
 * Author: mahdi
 * 
 * Created on August 27, 2017, 12:59 AM
 */
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <cstdint>
#include <cmath>
#include <random> 
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string>

#include "XDBGenerator.hpp"
#include "DBManager.hpp"
#include "lib.hpp"
#include "ProgressBar.hpp"
#include "QueryHandler.hpp"
#include "sha512.hpp"

using namespace std;

const string XDBGenerator::XDBFILE_REQUEST = "Sending XDB request";
const string XDBGenerator::XDBFILE_REQUEST_ACK = "Sending XDB request ACK";

#define HEX(x) setw(2) << setfill('0') << hex << (int)(x)

XDBGenerator::XDBGenerator() {
    
}

string XDBGenerator::getOutputFile() {
    return m_XDBfileName;
}

int XDBGenerator::createXDBFileFromSeed(char* t_seed, DBManager* t_DB) {
    cout << "in createXDBFileFromSeed() t_R=" << m_R << endl;
    size_t querySize = t_DB->getM()/8;
    size_t hashSize = m_seedSize/8;     //512bits = 64bytes
    size_t XmatrixSize = m_R*querySize;
    cout << "in createXDBFileFromSeed() XmatrixSize=" << dec << XmatrixSize << endl;
    char *Xmatrix = (char *)malloc(XmatrixSize * sizeof(char));
    char *hashBin = (char *)malloc(hashSize * sizeof(char));
    string hash = "";
    size_t matrixParser = 0;
//    mt19937_64 prng((uint8_t)t_seed[0]);
//    uniform_int_distribution<uint8_t> udist(0,255U);
//    for (int k=0; k<m_R; k++)
//        for (size_t i=0; i<querySize; i++)
//            Xmatrix[k*querySize+i] = (uint8_t)udist(prng);
    
    //generating the X matrix
    hash = bin2hex(t_seed, hashSize);
    for (size_t q=0; q<m_R; q++) {
        if (q==5421 || q==0 || q==1 || 1==10) cout << "for q=" << q << ", hash=" << hash << endl;
        size_t queryParser=0;
        for ( ; queryParser+hashSize<=querySize; queryParser+=hashSize) {
            hex2bin(hash, hashBin, hashSize);
            memcpy(Xmatrix+matrixParser+queryParser, hashBin, hashSize);
            hash = sw::sha512::calculate(hash);
        }
        if (queryParser != querySize) {
            hex2bin(hash.c_str(), hashBin, querySize%hashSize);
            memcpy(&Xmatrix[matrixParser+queryParser], hashBin, querySize%hashSize);
            hash = sw::sha512::calculate(hash);
        }
        matrixParser += querySize;
        if (q==5421 || q==0 || q==1 || 1==10) cout << "for q=" << q << ", hash=" << hash << endl;
    }
    
    
    //TODO to be replaced by the above code////////////
//    for ( ; matrixParser+hashSize<=XmatrixSize; matrixParser+=hashSize ) {
//        hex2bin(hash, hashBin, hashSize);
//        memcpy(Xmatrix+matrixParser, hashBin, hashSize);
//        hash = sw::sha512::calculate(hash);
//    }
//    if (matrixParser != XmatrixSize) {
//        hex2bin(hash.c_str(), hashBin, XmatrixSize%hashSize);
//        memcpy(&Xmatrix[matrixParser], hashBin, XmatrixSize%hashSize);
//    }
    ////////////////////////////////////////////////////
    
    if (m_verbose) cout << "X generated successfully" << endl;
    
    //preparing the output file
    ofstream outputFileStream;
    outputFileStream.open(m_XDBfileName, ios::binary | ios::out | ios::trunc);
    //string errorMsg = "Unable to open the XDB file: ";
    if (!outputFileStream.is_open()) error("error");
    m_bufferSize = m_R*t_DB->getN();
    cout << "t_DB->getN()=" << t_DB->getN() << " m_R=" << dec << m_R << " XDB size=" << m_bufferSize <<endl;
    char *XDBmatrix = (char *) calloc(m_bufferSize, sizeof(char));
    
    //preparing the progress bar
    int n = 100;
    cout << "Generating the XDB..." << endl;
    //ProgressBar *bar1 = new ProgressBar(n, "");
    //bar1->Progressed(0);
    
    //declaring query handler
    cout << "declaring query handler" << endl;
    QueryHandler qh(t_DB, 0, m_verbose);    //socket_id=0 no need for client socket id
    cout << "query handler declared" << endl;
    
    if (!t_DB->getSystemParm()->DBshouldBeSplitted())
    {   //DB can be loaded entirely to memory
        cout << "!DBshouldBeSplitted" << endl;
        //qh.processMultipleQueries(t_DB->getDBbuffer(), Xmatrix, m_R, XDBmatrix);
        cout << "m_R=" << m_R << endl;
        qh.processQueries(t_DB->getDBbuffer(), Xmatrix, m_R, XDBmatrix);
        if (m_verbose) cout << "all queries have been processed" << endl;
        
//        for (int k=0; k<m_R; k++) {
//            qh.setQuery(Xmatrix+k*numberOfColInX);
//            qh.calculateQdb();
//            memcpy(XDBmatrix+k*t_DB->getN(), qh.getQdb(), t_DB->getN());
//            bar1->Progressed(k*100/m_R);
//        }
    }
    else {  //DB should be splitted
        cout << "DBshouldBeSplitted" << endl;
        int numberOfChunks = t_DB->getSystemParm()->getNumberOfChunks();
        char * queriesChunks[numberOfChunks];
        size_t C = t_DB->getDBbufferSize()/t_DB->getN();    //C is the number of files in the regular DB chunk
        size_t lastC = (t_DB->getM()%C == 0)? C : t_DB->getM()%C;
        size_t subQuerySize=0, queriesChunkSize=0;
        int j=0;
        subQuerySize = C/8;
        queriesChunkSize = subQuerySize*m_R;    //jth sub-queries chunk size
        for ( ; j<numberOfChunks-1; j++) {    //last chunk should be treated separately, its size could be different
            queriesChunks[j] = (char*) malloc (queriesChunkSize * sizeof(char));
        }
        //last chunk
        subQuerySize = lastC/8;
        queriesChunkSize = subQuerySize*m_R;
        queriesChunks[j] = (char*) malloc (queriesChunkSize * sizeof(char));
        
        //copying from queue to queries chunks
        for (size_t i=0; i<m_R; i++) {
            subQuerySize = C/8;
            size_t queryParser = 0;
            for (int j=0; j<numberOfChunks-1; j++) {    //last chunk should be treated separately, its size could be different
                memcpy(&queriesChunks[j][i*subQuerySize], &Xmatrix[i*querySize+queryParser], subQuerySize);
                queryParser += subQuerySize;
            }
            //last chunk
            subQuerySize = lastC/8;
            memcpy(&queriesChunks[j][i*subQuerySize], &Xmatrix[i*querySize+queryParser], subQuerySize);
        }
        
        //processing queries
        cout << "TREATING " << m_R << " queries in parallel" << endl;
        QueryHandler qh(t_DB, 0, m_verbose);
        //size_t transposableQueries = m_R/8*8;
        subQuerySize = C/8;
        for (int j=0; j<numberOfChunks; j++) {    //last chunk should be treated separately, its size could be different
            subQuerySize = (j==numberOfChunks-1)? lastC/8 : C/8;
            //qh.processMultipleQueriesWithTransposing(t_DB->getDBbuffer(), queriesChunks[j], transposableQueries, XDBmatrix);
            qh.processQueries(t_DB->getDBbuffer(), queriesChunks[j], m_R, XDBmatrix);
//            if (m_R%8 != 0) {
//                size_t nontTransposableQueries = m_R%8;
//                cout << "processing " << nontTransposableQueries << " queries without transposing" << endl;
//                qh.processMultipleQueries(t_DB->getDBbuffer(), &queriesChunks[j][transposableQueries*subQuerySize], nontTransposableQueries, &XDBmatrix[transposableQueries*t_DB->getN()]);
//            }
            t_DB->loadDBChunkToMemory(j+1);    //load new DB chunk
        }
        
//        size_t parser=0;
//        for (int j=0; j<t_DB->getSystemParm()->getNumberOfChunks(); j++) {
//            for (int k=0; k<m_R; k++) {
//                qh.setQuery(Xmatrix+k*numberOfColInX);
//                qh.calculateQdb();
//                memcpy(XDBmatrix+k*t_DB->getN()+parser, qh.getQdb(), qh.getQdbSize());    //getQdbSize!!! TO CHECK //DONE
//                bar1->Progressed((j*m_R+k)*100/(t_DB->getSystemParm()->getNumberOfChunks()*m_R));
//            }
//            parser += t_DB->getDBbufferSize()/t_DB->getM();
//            //load new chunk to memory
//            t_DB->loadDBChunkToMemory(j+1);
//        }
//        parser=0;
    }
    
    //writing to output file and freeing memory
    if (m_verbose) cout << "writing to outputFileStream:" <<  m_bufferSize << " number of chars." << endl;
    outputFileStream.write(XDBmatrix,m_bufferSize);
    outputFileStream.close();
    delete XDBmatrix;
    delete Xmatrix;
//    bar1->Progressed(100);
    cout << endl;
    
    return 0;
}

int XDBGenerator::sendOutputFileToClient(int socket) {
    int sockErr;
    char *infoBuf;
    infoBuf = (char *) malloc (INFO_BUFFER_SIZE * sizeof (char));
    
    //sending the XDB file size
    bzero(infoBuf, INFO_BUFFER_SIZE);
    size_t fileSizeInt = getFileSize(m_XDBfileName);
    string fileSize = to_string(fileSizeInt);
    //TOBE removed**************************************************************
//    cout << "m_XDBfileName=" << m_XDBfileName << endl;
//    cout << "fileSizeInt=" << fileSizeInt << endl;
//    cout << "XDB size=" << fileSize << endl;
    //**************************************************************************
    strcpy (infoBuf, fileSize.c_str());
    if (m_verbose) cout << "sending XDB file size: " << infoBuf << endl;
    sockErr = write(socket, infoBuf, INFO_BUFFER_SIZE);
    if (sockErr < 0) error("ERROR sending XDB file size");

    //sending the XDB file
    ifstream XFileStream(m_XDBfileName, ios::binary);
    string errorMsg = "Unable to open the file: " + m_XDBfileName;
    if (!XFileStream.is_open()) error(errorMsg.c_str());
    char *dataBuf;
    dataBuf = (char *) malloc (DATA_BUFFER_SIZE * sizeof (char));
    bzero(dataBuf, DATA_BUFFER_SIZE);
    size_t fileSizeCounter = fileSizeInt;
    do {
        size_t minSize = min(fileSizeCounter, DATA_BUFFER_SIZE);
        XFileStream.read(dataBuf, minSize);
        sockErr = write(socket, dataBuf, DATA_BUFFER_SIZE);
        if (sockErr < 0) error("ERROR sending X file size");
        fileSizeCounter -= minSize;
    } while (fileSizeCounter > 0);
    XFileStream.close();
    
    //wait for ack from client
    bzero(infoBuf, INFO_BUFFER_SIZE);
    sockErr = 0;
    while (sockErr<INFO_BUFFER_SIZE)
        sockErr += read(socket, infoBuf+sockErr, INFO_BUFFER_SIZE-sockErr);
    //sockErr = read(socket, infoBuf, INFO_BUFFER_SIZE);
    if (sockErr < 0) error("ERROR reading X file size");
    if (m_verbose) cout << "ACK received: " << infoBuf << endl;
    return 0;
}

int XDBGenerator::loadXDBtoMemory() {
    ifstream XFileStream(m_XDBfileName, ios::binary);
    string errorMsg = "Unable to open the file: " + m_XDBfileName;
    if (!XFileStream.is_open()) error(errorMsg.c_str());
    m_buffer = (char *) malloc (m_bufferSize * sizeof (char));
    XFileStream.read(m_buffer, m_bufferSize);
}

XDBGenerator::XDBGenerator(const XDBGenerator& orig) {
}

XDBGenerator::~XDBGenerator() {
}

