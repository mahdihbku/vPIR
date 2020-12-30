/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Query.cpp
 * Author: mahdi
 * 
 * Created on September 10, 2017, 11:54 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <random> 
#include <fstream>
#include <iostream>
#include <cstring>
#include <string>
#include <sstream>
#include <iomanip>
#include <netdb.h>
#include <bitset>

#include "Query.hpp"
#include "DBDescriptor.hpp"
#include "lib.hpp"

//TODO change all r to m_XPIRDescriptor->getNumberOfFiles()

using namespace std;

Query::Query() {
}

Query::Query(int t_K, string t_outputFile, DBDescriptor* t_DB, int t_requestedFile, int t_verbose) {
    m_DB = t_DB;
    m_fileNumber = t_requestedFile;
    m_K = t_K;
    m_outputFile = t_outputFile;
    m_verbose = t_verbose;
}

int Query::selectKRandomX() {   //TODO SHOULD BE RE-CHECKED
    m_selectedXid = (int *) malloc (m_K * sizeof (int));
    random_device ranDev("/dev/urandom");
    int selectedRow = 0;
    for (int i=0; i<m_K; i++) {
        selectedRow = ranDev()%(m_DB->getXPIRDescriptor()->getNumberOfFiles()-1);
        m_selectedXid[i] = selectedRow; //TODO REMOVE THE COMMENT
        //m_selectedXid[i]=9;
    }
    if (m_verbose > 1) {
        cout << "Randomly selected rows: ";
        for (int i=0; i<m_K; i++)
            cout << m_selectedXid[i] << ",";
        cout << endl;
    }
    return 0;
}

int Query::generateRandomWforSimulation() {
    size_t sizeW = m_DB->getM()/8;
    size_t sizeWDB = m_DB->getN();
    random_device ranDev("/dev/urandom");
    m_w = (char *) malloc (sizeW * sizeof (char));
    m_wDB = (char *) malloc (sizeWDB * sizeof (char));
    for(size_t j=0; j<sizeW; j++)
        m_w[j] = (uint8_t)ranDev()%255;
    for(size_t j=0; j<sizeWDB; j++)
        m_wDB[j] = (uint8_t)ranDev()%255;
    return 0;
}

int Query::calculateW() {
    size_t m = m_DB->getM();
    size_t n = m_DB->getN();
    int r = m_DB->getXPIRDescriptor()->getNumberOfFiles();
    size_t sizeW=0, XfileBufferSize=0;
    sizeW = m/8;
    XfileBufferSize = m/8*r;
    m_w = (char *) malloc (sizeW * sizeof (char));
    size_t sizeWDB = n;
    m_wDB = (char *) malloc (sizeWDB * sizeof (char));
    for(size_t j=0; j<sizeW; j++) {
        m_w[j]=(uint8_t)0;
    }
    for(size_t j=0; j<sizeWDB; j++) {
        m_wDB[j]=(uint8_t)0;
    }
    ifstream XFileStream(m_DB->getXfileName().c_str());
    char *XFileBuffer;
    XFileBuffer = (char *) malloc (XfileBufferSize * sizeof (char));
    XFileStream.read(XFileBuffer, XfileBufferSize);
    XFileStream.close();
    ifstream XDBFileStream(m_DB->getXDBfileName());
    char *XDBFileBuffer;
    size_t XDBfileBufferSize = n*r;
    XDBFileBuffer = (char *) malloc (XDBfileBufferSize * sizeof (char));
    XDBFileStream.read(XDBFileBuffer, XDBfileBufferSize);
    XDBFileStream.close();
    
//    if (m_verbose) cout << "calculating W: m=" << m << " n=" << n << " p=" << p << " r=" << r << " k=" << m_K << endl;
//    if (m_verbose) cout << "sizeW=" << sizeW << " XfileBufferSize=" << XfileBufferSize << " numberOfBytes=" << numberOfBytes << endl;

    uint8_t* XFileBufferInt = (uint8_t*)XFileBuffer;
    uint8_t* XDBFileBufferInt = (uint8_t*)XDBFileBuffer;
    uint8_t* wInt = (uint8_t*)m_w;
    uint8_t* wDBInt = (uint8_t*)m_wDB;
    for (int i=0; i<m_K; i++) {
        uint8_t* currentX = XFileBufferInt + m_selectedXid[i] * m/8;
        for(size_t j=0; j<m/8; j++) {
            wInt[j] ^= currentX[j];
//                    cout << "i=" << i << " w[" << j << "]^=" << currentX[j] << " w[" << j << "]=" << wInt[j] << endl;
        }
        uint8_t* currentXdb = XDBFileBufferInt + m_selectedXid[i] * n;
        for(size_t j=0; j<n; j++) {
            wDBInt[j] ^= currentXdb[j];
//                    if (j>80&&j<100) cout << "i=" << i << " wDBInt[" << j << "]^=" << currentXdb[j] << " w[" << j << "]=" << wDBInt[j] << endl;
        }
    }
//            if (m_verbose) cout << "query(w)=" << m_w << endl;
//            for (int i=0; i<sizeW; i++)
//                cout << m_w[i];
//            cout << endl;
//            if (m_verbose) cout << "(wdb)=" << m_wDB << endl;
//            for (int i=80; i<100; i++)
//                cout << m_wDB[i];
//            cout << endl;

    return 0;
}

int Query::calculateS() {
    size_t sizeS = m_DB->getM()/8;
    //m_s = (char *) malloc (sizeS * sizeof (char));
    m_s=m_w;
    uint8_t* sInt = (uint8_t*)m_s;
    sInt[m_fileNumber/8] ^= 1<<(7-m_fileNumber%8);
}

char* Query::getS() {
    return m_s;
}

int Query::setSDB(char* t_sdb) {
    m_sDB = t_sdb;
}

int Query::calculateData() {
    m_data = (char *) malloc (m_DB->getN() * sizeof (char));
    
    uint8_t* dataInt = (uint8_t*)m_data;
    uint8_t* wDBInt = (uint8_t*)m_wDB;
    uint8_t* sDBInt = (uint8_t*)m_sDB;
    for(size_t j=0; j<m_DB->getN(); j++)
        dataInt[j] = (uint8_t)(wDBInt[j]^sDBInt[j]);
            
    return 0;
}

int Query::writeDataToFile() {
    
//    cout << "before writing to file, m_fileNumber=" << m_fileNumber << " and "
//            << "m_DB->getFilesSizes()->at(m_fileNumber)=" << m_DB->getFilesSizes()->at(m_fileNumber) << endl;
    ofstream outputFileStream(m_outputFile, ofstream::out | ofstream::trunc);
    string errorMsg = "Unable to open the file: " + m_outputFile;
    if (!outputFileStream.is_open()) error(errorMsg.c_str());
    outputFileStream.close();
    outputFileStream.open(m_outputFile, ios::binary | ios::app);
//    cout << "writeDataToFile: before writing to file, m_fileNumber=" << m_fileNumber << " and "
//            << "m_DB->getFilesSizes()->at(m_fileNumber)=" << m_DB->getFilesSizes()->at(m_fileNumber) << endl;
//    cout << "writeDataToFile: writing:" << m_data << endl;
    outputFileStream.write(m_data, m_DB->getFilesSizes()->at(m_fileNumber));
    return 0;
}

Query::Query(const Query& orig) {
}

Query::~Query() {
}
