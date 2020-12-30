/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   DBDescriptor.cpp
 * Author: mahdi
 * 
 * Created on August 29, 2017, 11:12 AM
 */

#include <cmath>
#include <random> 
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <netdb.h>

#include "DBDescriptor.hpp"
#include "lib.hpp"
#include "sha512.hpp"

using namespace std;

DBDescriptor::DBDescriptor() {
}

DBDescriptor::DBDescriptor(string t_DBName, size_t t_N, size_t t_M, int t_R, string t_seed, 
        int t_verbose, string t_version, string t_serverIP, int t_serverPort, XPIRDescriptor* t_XPIRDescriptor) {
    m_serverIP      = t_serverIP;
    m_serverPort    = t_serverPort;
    m_N             = t_N;
    m_M             = t_M;
    m_R             = t_R;
    m_seed          = t_seed;
    m_dbName        = t_DBName;
    m_version       = t_version;
    m_verbose       = t_verbose;
    m_updates       = new DBupdates();
    m_XPIRDescriptor= t_XPIRDescriptor;
}

int DBDescriptor::setM(const size_t t_M) { m_M = t_M; }
int DBDescriptor::setdbName(const string t_name) { m_dbName = t_name; }
//int DBDescriptor::setDBupdates(DBupdates* t_updates) { m_updates = t_updates; }
int DBDescriptor::setXDBfileName(const string t_XDBfile) { m_XDBfileName = t_XDBfile; }
int DBDescriptor::setXfileName(const string t_Xfile) { m_XfileName = t_Xfile; }
int DBDescriptor::setDBupdatesFileName(const string t_DBupdatesFile) { m_DBupdatesFileName = t_DBupdatesFile; }

size_t DBDescriptor::getM() { return m_M; }
size_t DBDescriptor::getN() { return m_N; }
int DBDescriptor::getR() { return m_R; }
string DBDescriptor::getSeed() { return m_seed; }
string DBDescriptor::getdbName() { return m_dbName; }
string DBDescriptor::getXfileName() { return m_XfileName; }
string DBDescriptor::getXDBfileName() { return m_XDBfileName; }
string DBDescriptor::getDBupdatesFileName() { return m_DBupdatesFileName; }
vector<string>* DBDescriptor::getFilesNames() { return &m_dbFilesNamesList; }
vector<size_t>* DBDescriptor::getFilesSizes() { return &m_dbFilesSizesList; }
string DBDescriptor::getDBversion() { return m_version; }
DBupdates* DBDescriptor::getDBupdates() { return m_updates; }
XPIRDescriptor* DBDescriptor::getXPIRDescriptor()  { return m_XPIRDescriptor; }

int DBDescriptor::addFileNameToList(const string t_name) {
    m_dbFilesNamesList.push_back(t_name);
}

int DBDescriptor::addFileSizeToList(const size_t t_size) {
    m_dbFilesSizesList.push_back(t_size);
}

int DBDescriptor::displayDBcatalog() {
    size_t i=0;
    for ( ; i<m_dbFilesNamesList.size(); i++)
        cout << i << "." << m_dbFilesNamesList.at(i) << " (" << m_dbFilesSizesList.at(i) << ')' << endl;
    if (m_verbose) cout << "Number of files in the updates file: "<<m_updates->getNumberOfFileInDBupdates()<<endl;
    for (int j=0 ; j<m_updates->getNumberOfFileInDBupdates(); j++)
        cout << i+j << ".(local)" << m_updates->getDBupdatesfilesNamesList()->at(j) << " (" << m_updates->getDBupdatesfilesSizesList()->at(j) << ')' << endl;
}

int DBDescriptor::saveDBinfoToFile(const string t_dbInfoFileName) {
    ofstream dbFileStream(t_dbInfoFileName);
    string errorMsg = "Unable to open the file: " + t_dbInfoFileName;
    if (!dbFileStream.is_open()) error(errorMsg.c_str());
    dbFileStream << "server= " << m_serverIP << endl;
    dbFileStream << "port  = " << m_serverPort << endl;
    dbFileStream << "ver   = " << m_version << endl;
    dbFileStream << "DBName: " << m_dbName << endl;
    dbFileStream << "M     = " << m_M << endl;
    dbFileStream << "N     = " << m_N << endl;
    dbFileStream << "R     = " << m_R << endl;
    dbFileStream << "R2    = " << m_XPIRDescriptor->getNumberOfFiles() << endl; //added
    dbFileStream << "Seed  = " << m_seed << endl;
    dbFileStream << "X     : " << m_XfileName << endl;
    dbFileStream << "XDB   : " << m_XDBfileName << endl;
    dbFileStream << "Update: " << m_DBupdatesFileName << endl;
    dbFileStream << "XPIRip: " << m_XPIRDescriptor->getXPIRip() << endl;    //added
    dbFileStream << "XPIRport : " << m_XPIRDescriptor->getXPIRport() << endl;    //added
    dbFileStream << "XPIRdir  : " << m_XPIRDescriptor->getXPIRclientDir() << endl;    //added
    dbFileStream << "Catalog:" << endl;
    //if (m_dbFilesNamesList.size() == m_dbFilesSizesList.size())
    for (size_t i=0; i<m_dbFilesNamesList.size(); i++)
    {
        dbFileStream << m_dbFilesNamesList.at(i) << endl;
        dbFileStream << m_dbFilesSizesList.at(i) << endl;
    }
    //else
        //for (size_t i=0; i<m_dbFilesNamesList.size(); i++)
            //dbFileStream << m_dbFilesNamesList.at(i) << endl;
    dbFileStream.close();
    return 0;
}

int DBDescriptor::loadDBinfoFromFile(const string t_dbInfoFileName) {
    ifstream DBInfoFileStream(t_dbInfoFileName,ifstream::in);
    string errorMsg = "Unable to open the file: " + t_dbInfoFileName;
    if (!DBInfoFileStream.is_open()) error(errorMsg.c_str());
    
    string line;
    getline(DBInfoFileStream, line);
    m_serverIP = line.substr(8,line.length());
    getline(DBInfoFileStream, line);
    string portString = line.substr(8,line.length());
    m_serverPort = stoi(portString);
    getline(DBInfoFileStream, line);
    m_version = line.substr(8,line.length());
    getline(DBInfoFileStream, line);
    m_dbName = line.substr(8,line.length());
    getline(DBInfoFileStream, line);
    string Mstring = line.substr(8,line.length());
    m_M = stoi(Mstring);
    getline(DBInfoFileStream, line);
    string Nstring = line.substr(8,line.length());
    m_N = stoi(Nstring);
    getline(DBInfoFileStream, line);
    string Rstring = line.substr(8,line.length());
    m_R = stoi(Rstring);
    getline(DBInfoFileStream, line);
    string R2string = line.substr(8,line.length());
    int R2 = stoi(R2string);
    getline(DBInfoFileStream, line);
    string seedString = line.substr(8,line.length());
    m_seed = seedString;
    getline(DBInfoFileStream, line);
    m_XfileName = line.substr(8,line.length());
    getline(DBInfoFileStream, line);
    m_XDBfileName = line.substr(8,line.length());
    getline(DBInfoFileStream, line);
    m_DBupdatesFileName = line.substr(8,line.length());
    getline(DBInfoFileStream, line);
    string XPIRip = line.substr(8,line.length());
    getline(DBInfoFileStream, line);
    string XPIRportString = line.substr(11,line.length());
    int XPIRport = stoi(XPIRportString);
    getline(DBInfoFileStream, line);
    string XPIRclientDir = line.substr(11,line.length());
    
    m_XPIRDescriptor = new XPIRDescriptor(XPIRip, XPIRport, R2, XPIRclientDir, m_verbose);
            
    getline(DBInfoFileStream, line);    //to bypass the "catalog:" line
    while (getline(DBInfoFileStream, line)) {
        m_dbFilesNamesList.push_back(line);
        getline(DBInfoFileStream, line);
        stringstream sstream(line);
        size_t fileSize;
        sstream >> fileSize;
        m_dbFilesSizesList.push_back(fileSize);
    }
    DBInfoFileStream.close();
    //loading files names if any from DBupdates.data
    m_updates = new DBupdates(m_DBupdatesFileName, m_verbose);
    m_updates->loadDBupdatesFromFile();
    return 0;
}

int DBDescriptor::saveXtoFile(const string t_file) {
//    m_XfileName = t_file;
//    mt19937_64 prng(m_seed);
//    size_t outputFileParser = 0;
//    char *outputBuffer;
//    size_t xFileSize = m_R*m_M/8;
//    outputBuffer = (char *) malloc (xFileSize * sizeof (char));
//    ofstream XFileStream(t_file.c_str());
//    string errorMsg = "Unable to open the file: " + t_file;
//    if (!XFileStream.is_open()) error(errorMsg.c_str());
//    uniform_int_distribution<uint8_t> udist(0,255U);  //pow(2,8)-1
//    for (int k=0; k<m_R; k++) {
//        for (size_t j=0; j<m_M/8; j++) {
//            uint8_t rd = udist(prng);
//            outputBuffer[outputFileParser] = (char)rd;
//            outputFileParser++;
//        }
//    }
//    XFileStream.write(outputBuffer,xFileSize);
    m_XfileName = t_file;
    size_t querySize = m_M/8;
    size_t hashSize = m_seed.size()/2;     //512bits = 64bytes
        cout << "hash Size = " << hashSize << endl; 
    size_t XmatrixSize = m_R*querySize;
    char *Xmatrix = (char *)malloc(XmatrixSize * sizeof(char));
    char *hashBin = (char *)malloc(hashSize * sizeof(char));
    size_t matrixParser=0;
    //generating the X matrix
    string hash = m_seed;
    for ( ; matrixParser+hashSize<=XmatrixSize; matrixParser+=hashSize ) {
        hex2bin(hash, hashBin, hashSize);
        memcpy(Xmatrix+matrixParser, hashBin, hashSize);
        hash = sw::sha512::calculate(hash);
    }
    if (matrixParser != XmatrixSize) {
        hex2bin(hash.c_str(), hashBin, XmatrixSize%hashSize);
        memcpy(&Xmatrix[matrixParser], hashBin, XmatrixSize%hashSize);
    }
    ofstream XFileStream(t_file.c_str());
    string errorMsg = "Unable to open the file: " + t_file;
    if (!XFileStream.is_open()) error(errorMsg.c_str());
    XFileStream.write(Xmatrix,XmatrixSize);
    XFileStream.close();
    if (m_verbose) cout << "X generated successfully" << endl;
}

std::string DBDescriptor::getServerIP() { return m_serverIP; }
int DBDescriptor::getServerPort() { return m_serverPort; }

DBDescriptor::DBDescriptor(const DBDescriptor& orig) {
}

DBDescriptor::~DBDescriptor() {
}

