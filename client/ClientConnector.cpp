/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ClientConnector.cpp
 * Author: mahdi
 * 
 * Created on August 23, 2017, 11:43 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <cstring>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <math.h> 
#include <netinet/tcp.h>
#include <vector>
#include <random>
#include <algorithm>

#include "ClientConnector.hpp"
#include "DBDescriptor.hpp"
#include "DBupdates.hpp"
#include "lib.hpp"
#include "sha512.hpp"

using namespace std;

const string ClientConnector::DBINFO_REQUEST = "DBInfo request";
const string ClientConnector::DBINFO_ACK = "DBInfo ack";
const string ClientConnector::VER_REQUEST = "Version request";
const string ClientConnector::XDBFILE_REQUEST = "Sending XDB request";
const string ClientConnector::XDBFILE_ACK = "XDB ack";
const string ClientConnector::DBUPDATES_REQUEST = "DBupdates request";
const string ClientConnector::DBUPDATES_ACK = "DBupdates ack";
const string ClientConnector::NO_UPDATES = "No new updates";
const string ClientConnector::QUERY_REQUEST = "Query request";
const string ClientConnector::CLOSE_CONNECTION = "Close Connection";

ClientConnector::ClientConnector(int t_verbose) {
    m_serverIP = "0.0.0.0";
    m_serverPort = 0;
    m_verbose = t_verbose;
}

ClientConnector::ClientConnector(const ClientConnector& orig) {
}

int ClientConnector::setServerInfo(const string t_ipAddr, const int t_portNumber) {
	m_serverIP = t_ipAddr;
	m_serverPort = t_portNumber;
}
int ClientConnector::setDataBase(DBDescriptor* t_DB) { m_DB = t_DB; }

string ClientConnector::getServerIP() { return m_serverIP; }
int ClientConnector::getServerPort() { return m_serverPort; }

int ClientConnector::initiateConnection() {
    struct sockaddr_in serv_addr;
    struct in_addr addr = { 0 };
    struct hostent *server;
    m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_sockfd < 0) error("ERROR opening socket");
    
    int yes = 1;
    //setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    setsockopt(m_sockfd, IPPROTO_TCP, TCP_NODELAY, (void*) &yes, sizeof(yes));
    
    addr.s_addr = inet_addr(m_serverIP.c_str());
    if (addr.s_addr == INADDR_NONE) {
        printf("The IPv4 address entered must be a legal address\n");
        abort();
    }
    //server = gethostbyaddr((char *) &addr, 4, AF_INET);
    server = gethostbyname(m_serverIP.c_str());
    if (server == nullptr) {
        fprintf(stderr,"ERROR, no such host\n");
        abort();
    }
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(m_serverPort);
    if (connect(m_sockfd, (struct sockaddr *) & serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    return 0;
}

string ClientConnector::getDBversionFromServer() {
    int sockErr;
    char *infoBuffer;
    infoBuffer = (char *) malloc (INFO_BUFFER_SIZE * sizeof (char));
    bzero(infoBuffer, INFO_BUFFER_SIZE);
    strcpy (infoBuffer, VER_REQUEST.c_str());
    sockErr = write(m_sockfd, infoBuffer, INFO_BUFFER_SIZE);
    if (sockErr < 0) error("ERROR writing to socket");
    bzero(infoBuffer, INFO_BUFFER_SIZE);
    sockErr = 0;
    while (sockErr<INFO_BUFFER_SIZE)
        sockErr += read(m_sockfd, infoBuffer+sockErr, INFO_BUFFER_SIZE-sockErr);
    if (sockErr < 0) error("ERROR while reading the db version");
    string version(infoBuffer);
    return version;
}

int ClientConnector::getDBInfoFromServer() {
    int sockErr;
    char *infoBuffer;
    infoBuffer = (char *) malloc (INFO_BUFFER_SIZE * sizeof (char));

    //sending DBInfo request
    bzero(infoBuffer, INFO_BUFFER_SIZE);
    strcpy (infoBuffer, DBINFO_REQUEST.c_str());
    sockErr = write(m_sockfd, infoBuffer, INFO_BUFFER_SIZE);
    if (sockErr < 0) error("ERROR writing to socket");
    if (m_verbose) cout << "DBINFO_REQUEST sent: " << infoBuffer << endl;
    
    //receiving DBInfo:
    //db version
    bzero(infoBuffer, INFO_BUFFER_SIZE);
    sockErr = 0;
    while (sockErr<INFO_BUFFER_SIZE) {
        sockErr += read(m_sockfd, infoBuffer+sockErr, INFO_BUFFER_SIZE-sockErr);
//        if (m_verbose) cout << "received:" << sockErr << "/" << INFO_BUFFER_SIZE << endl;
    }
    if (sockErr < 0) error("ERROR while reading db version");
    string versionLine(infoBuffer);
    string versionString = versionLine.substr(8,versionLine.length());
    if (m_verbose > 1) cout << "version=" << versionString << endl;
    
    //dbname
    bzero(infoBuffer, INFO_BUFFER_SIZE);
    sockErr = 0;
    while (sockErr<INFO_BUFFER_SIZE) {
        sockErr += read(m_sockfd, infoBuffer+sockErr, INFO_BUFFER_SIZE-sockErr);
//        if (m_verbose) cout << "received:" << sockErr << "/" << INFO_BUFFER_SIZE << endl;
    }
    if (sockErr < 0) error("ERROR while reading dbname");
    string DataBaseName(infoBuffer);
    if (m_verbose > 1) cout << "DataBase Name=" << DataBaseName << endl;

    //m
    bzero(infoBuffer, INFO_BUFFER_SIZE);
    sockErr = 0;
    while (sockErr<INFO_BUFFER_SIZE) {
        sockErr += read(m_sockfd, infoBuffer+sockErr, INFO_BUFFER_SIZE-sockErr);
//        if (m_verbose) cout << "received:" << sockErr << "/" << INFO_BUFFER_SIZE << endl;
    }
    if (sockErr < 0) error("ERROR while reading m");
    string Mline(infoBuffer);
    string Mstring = Mline.substr(8,Mline.length());
    size_t M = stoull(Mstring);
    if (m_verbose > 1) cout << "M=" << M << endl;

    //n
    bzero(infoBuffer, INFO_BUFFER_SIZE);
    sockErr = 0;
    while (sockErr<INFO_BUFFER_SIZE) {
        sockErr += read(m_sockfd, infoBuffer+sockErr, INFO_BUFFER_SIZE-sockErr);
//        if (m_verbose) cout << "received:" << sockErr << "/" << INFO_BUFFER_SIZE << endl;
    }
//    if (m_verbose) cout << "sockErr=" << sockErr << "received=" << infoBuffer << endl;
    if (sockErr < 0) error("ERROR while reading n");
    string Nline(infoBuffer);
    string Nstring = Nline.substr(8,Nline.length());
    size_t N = stoull(Nstring);
    if (m_verbose > 1) cout << "N=" << N << endl;

    //r
    bzero(infoBuffer, INFO_BUFFER_SIZE);
    sockErr = 0;
    while (sockErr<INFO_BUFFER_SIZE) {
        sockErr += read(m_sockfd, infoBuffer+sockErr, INFO_BUFFER_SIZE-sockErr);
//        if (m_verbose) cout << "received:" << sockErr << "/" << INFO_BUFFER_SIZE << endl;
    }
    if (sockErr < 0) error("ERROR while reading r");
    string Rline(infoBuffer);
    string Rstring = Rline.substr(8,Rline.length());
    int R = stoi(Rstring);
    if (m_verbose > 1) cout << "R=" << R << endl;
    
    //seed
    bzero(infoBuffer, INFO_BUFFER_SIZE);
    sockErr = 0;
    while (sockErr<INFO_BUFFER_SIZE) {
        sockErr += read(m_sockfd, infoBuffer+sockErr, INFO_BUFFER_SIZE-sockErr);
//        if (m_verbose) cout << "received:" << sockErr << "/" << INFO_BUFFER_SIZE << endl;
    }
    if (sockErr < 0) error("ERROR while reading seed");
    string seedLine(infoBuffer);
    string seedString = seedLine.substr(8,seedLine.length());
    if (m_verbose > 1) cout << "seed=" << seedString << endl;
    
    //XPIRip
    bzero(infoBuffer, INFO_BUFFER_SIZE);
    sockErr = 0;
    while (sockErr<INFO_BUFFER_SIZE) {
        sockErr += read(m_sockfd, infoBuffer+sockErr, INFO_BUFFER_SIZE-sockErr);
//        if (m_verbose) cout << "received:" << sockErr << "/" << INFO_BUFFER_SIZE << endl;
    }
    if (sockErr < 0) error("ERROR while reading XPIRip");
    string XPIRipLine(infoBuffer);
    string XPIRip = XPIRipLine.substr(8, XPIRipLine.length());
    if (m_verbose > 1) cout << "XPIRip=" << XPIRip << endl;
    
    //XPIRport
    bzero(infoBuffer, INFO_BUFFER_SIZE);
    sockErr = 0;
    while (sockErr<INFO_BUFFER_SIZE) {
        sockErr += read(m_sockfd, infoBuffer+sockErr, INFO_BUFFER_SIZE-sockErr);
//        if (m_verbose) cout << "received:" << sockErr << "/" << INFO_BUFFER_SIZE << endl;
    }
    if (sockErr < 0) error("ERROR while reading XPIRport");
    string XPIRportline(infoBuffer);
    string XPIRportString = XPIRportline.substr(11, XPIRportline.length());
    int XPIRport = stoi(XPIRportString);
    if (m_verbose > 1) cout << "XPIRport=" << XPIRport << endl;
    
    //Setting the XPIR client parameters
    m_XPIR = new XPIRDescriptor(XPIRip, XPIRport, 0, "", m_verbose);
    
    m_DB = new DBDescriptor(DataBaseName, N, M, R, seedString, m_verbose, versionString, m_serverIP, m_serverPort, m_XPIR);
    
    //getting files names and sizes
    for (size_t i=0; i<m_DB->getM()*2; i+=2)
    {
        bzero(infoBuffer, INFO_BUFFER_SIZE);
        sockErr = 0;
        while (sockErr<INFO_BUFFER_SIZE) {
            sockErr += read(m_sockfd, infoBuffer+sockErr, INFO_BUFFER_SIZE-sockErr);
//            if (m_verbose) cout << "reading file name: received:" << sockErr << "/" << INFO_BUFFER_SIZE << endl;
        }
        if (sockErr < 0) error("ERROR while reading a file name");
        string fileNameline(infoBuffer);
        m_DB->addFileNameToList(fileNameline);
        bzero(infoBuffer, INFO_BUFFER_SIZE);
        sockErr = 0;
        while (sockErr<INFO_BUFFER_SIZE) {
            sockErr += read(m_sockfd, infoBuffer+sockErr, INFO_BUFFER_SIZE-sockErr);
//            if (m_verbose) cout << "reading file size: received:" << sockErr << "/" << INFO_BUFFER_SIZE << endl;
        }
        if (sockErr < 0) error("ERROR while reading a file size");
        string fileSizeline(infoBuffer);
        stringstream sstream(fileSizeline);
        size_t fileSize;
        sstream >> fileSize;
        m_DB->addFileSizeToList(fileSize);
    }

    //sending ack to the server
    bzero(infoBuffer, INFO_BUFFER_SIZE);
    strcpy (infoBuffer, DBINFO_ACK.c_str());
    sockErr = write(m_sockfd, infoBuffer, INFO_BUFFER_SIZE);
    if (sockErr < 0) error("ERROR sending db name from the db file");
    if (m_verbose) cout << "ACK sent: " << infoBuffer << endl;
}

int ClientConnector::generateAndSaveXtoFile(string t_XfileName, vector<size_t> *t_selectedVectors) {
    m_DB->setXfileName(t_XfileName);
    //generating random indexes (in order)
    random_device ranDev("/dev/urandom");
    size_t selectedVector = ranDev()%(m_DB->getR()-1);
    t_selectedVectors->insert(t_selectedVectors->begin(), selectedVector);
    for (int i=0; i<m_XPIR->getNumberOfFiles()-1; i++) {
        selectedVector = ranDev()%(m_DB->getR()-1);
        vector<size_t>::iterator it = upper_bound(t_selectedVectors->begin(), t_selectedVectors->end(), selectedVector);
        t_selectedVectors->insert(it, selectedVector);
    }
    if (m_verbose > 1) {
        cout << "Randomly selected vectors: ";
        for (int i=0; i<m_XPIR->getNumberOfFiles(); i++)
            cout << t_selectedVectors->at(i) << ",";
        cout << endl;
    }
    
    //generating the matrix X
    cout << "generating the matrix X" << endl;
    size_t querySize = m_DB->getM()/8;
    size_t hashSize = 512/8;     //512bits = 64bytes
    size_t XmatrixSize = m_XPIR->getNumberOfFiles()*querySize;
    char *Xmatrix = (char *)malloc(XmatrixSize * sizeof(char));
    char *hashBin = (char *)malloc(hashSize * sizeof(char));
    string hash = "";
    size_t matrixParser = 0;
    cout << "m_DB->getSeed()=" << m_DB->getSeed() << endl;
    //hash = bin2hex(m_DB->getSeed().c_str(), hashSize);
    hash = m_DB->getSeed();
    int currentSelectedVectorIndex = 0;
    size_t currentSelectedVector = 0;
    cout << "generating the matrix X-----" << endl;
    for (size_t q=0; q<=t_selectedVectors->at(m_XPIR->getNumberOfFiles()-1); q++) {  //No need to seek the last query in the random queries
        if (q==5421 || q==0 || q==1 || 1==10) cout << "for q=" << q << ", hash=" << hash << endl;
        size_t queryParser=0;
        currentSelectedVector = t_selectedVectors->at(currentSelectedVectorIndex);
        if (q != currentSelectedVector) {
            for ( ; queryParser+hashSize<=querySize; queryParser+=hashSize) {
                hex2bin(hash, hashBin, hashSize);
                hash = sw::sha512::calculate(hash);
            }
            if (queryParser != querySize) {
                hex2bin(hash.c_str(), hashBin, querySize%hashSize);
                hash = sw::sha512::calculate(hash);
            }
        }
        else {
            cout << "hit! q=" << q << " currentSelectedVectorIndex=" << currentSelectedVectorIndex << " currentSelectedVector=" << currentSelectedVector << endl;
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
            currentSelectedVectorIndex++;
            matrixParser += querySize;
        }
        if (q==5421 || q==0 || q==1 || 1==10) cout << "for q=" << q << ", hash=" << hash << endl;
    }
    
    //storing X into a file
    cout << "storing X into a file. XmatrixSize=" << XmatrixSize/1024 << "kB" << endl;
    ofstream XFileStream(t_XfileName.c_str());
    string errorMsg = "Unable to open the file: " + t_XfileName;
    if (!XFileStream.is_open()) error(errorMsg.c_str());
    XFileStream.write(Xmatrix, XmatrixSize);
    XFileStream.close();
    if (m_verbose) cout << "X generated successfully" << endl;
    return 0;
}

int ClientConnector::getAndSaveXDBtoFile(const string t_XDBfileName, vector<size_t> * t_selectedVectors) {
    m_DB->setXDBfileName(t_XDBfileName);
    
//    pid_t xpirProcess = 0;
//    xpirProcess = fork();
//    if (xpirProcess < 0) {
//        fprintf( stderr, "process failed to fork\n" );
//        return 0;
//    }
//    if (xpirProcess == 0) {
        for (int i=0; i<t_selectedVectors->size(); i++) {
            m_XPIR->getFileFromXPIRserver(t_selectedVectors->at(i));
        }
//    }
    
    cout << "After downloading files from XPIR server----" << endl;
    //saving the individual files to one XDB file
    ofstream XDBfileStream(t_XDBfileName);
    size_t fileBufferSize = m_DB->getN();
    string errorMsg = "Unable to open the file: " + t_XDBfileName;
    if (!XDBfileStream.is_open()) error(errorMsg.c_str());    //TODO open and erase not append
    for (int i=0; i<t_selectedVectors->size(); i++) {
        string receivedFileName = m_XPIR->getXPIRclientDir()+"reception/" + to_string(t_selectedVectors->at(i));
        ifstream fileStream(receivedFileName);
        string errorMsg = "Unable to open the file: " + receivedFileName;
        if (!fileStream.is_open()) error(errorMsg.c_str());
        char *fileBuffer;
        fileBuffer = (char *) malloc (fileBufferSize * sizeof (char));
        fileStream.read(fileBuffer, fileBufferSize);
        fileStream.close();
        XDBfileStream.write(fileBuffer, fileBufferSize);
        //cout << "file:" << receivedFileName << " has been written to:" <<  t_XDBfileName << endl;
    }
    XDBfileStream.close();
    if (m_verbose) cout << "XDB file has been received successfully" << endl;
}

int ClientConnector::getAndSaveDBupdatesToFile(const string t_DBupdatesFileName) {
    if (m_verbose) cout << "getting db updates from server:" << endl;
    m_DB->setDBupdatesFileName(t_DBupdatesFileName);
    m_DB->getDBupdates()->setParameters(t_DBupdatesFileName, m_verbose);
    //m_DB->setDBupdates(&updates);
    int sockErr=0;
    char *infoBuffer = (char *) malloc (INFO_BUFFER_SIZE * sizeof (char));
    
//    cout << "***inside getand updates.getNumberOfFileInDBupdates()="<<m_DB->getDBupdates()->getNumberOfFileInDBupdates()<<endl;
//    cout << "***inside getand m_DB->getDBupdates()->getNumberOfFileInDBupdates()="<<m_DB->getDBupdates()->getNumberOfFileInDBupdates()<<endl;
    //sending DB updates request
    bzero(infoBuffer, INFO_BUFFER_SIZE);
    strcpy (infoBuffer, DBUPDATES_REQUEST.c_str());
    sockErr = write(m_sockfd, infoBuffer, INFO_BUFFER_SIZE);
    if (sockErr < 0) error("ERROR writing to socket");
    if (m_verbose) cout << " sent " << DBUPDATES_REQUEST << " to the server" << endl;
    
    //receiving the number of files in DB updates
    bzero(infoBuffer, INFO_BUFFER_SIZE);
    sockErr = 0;
    while (sockErr<INFO_BUFFER_SIZE)
        sockErr += read(m_sockfd, infoBuffer+sockErr, INFO_BUFFER_SIZE-sockErr);
    //sockErr = read(m_sockfd, infoBuffer, INFO_BUFFER_SIZE);
    if (sockErr < 0) error("ERROR reading X file size");
    string DBupdatesFilesCountString(infoBuffer);
    if (m_verbose) cout << " received " << DBupdatesFilesCountString << " to the server" << endl;
    if (DBupdatesFilesCountString.compare(NO_UPDATES) == 0) 
        return 0;
    if (m_verbose) cout << "DB updates contains: " << DBupdatesFilesCountString << " files" << endl;
    size_t DBupdatesFilesCount = stoll(DBupdatesFilesCountString);

    //receiving files names + sizes 1 by 1
    for (int i=0; i<DBupdatesFilesCount; i++) {
        //getting file name
        bzero(infoBuffer, INFO_BUFFER_SIZE);
        sockErr = 0;
        while (sockErr<INFO_BUFFER_SIZE)
            sockErr += read(m_sockfd, infoBuffer+sockErr, INFO_BUFFER_SIZE-sockErr);
        if (sockErr < 0) error("ERROR while reading the database name");
        string fileNameline(infoBuffer);
        m_DB->getDBupdates()->getDBupdatesfilesNamesList()->push_back(fileNameline);
        //getting file size
        bzero(infoBuffer, INFO_BUFFER_SIZE);
        sockErr = 0;
        while (sockErr<INFO_BUFFER_SIZE)
            sockErr += read(m_sockfd, infoBuffer+sockErr, INFO_BUFFER_SIZE-sockErr);
        if (sockErr < 0) error("ERROR while reading the database name");
        string fileSizeline(infoBuffer);
        stringstream sstream(fileSizeline);
        size_t fileSize;
        sstream >> fileSize;
        m_DB->getDBupdates()->getDBupdatesfilesSizesList()->push_back(fileSize);
        if (m_verbose) cout << " received file name " << fileNameline << " with size " << fileSize << endl;
    }
    
    //writing files names to the db updates file
    m_DB->getDBupdates()->writeDBupdatesTofile();
    
    //receiving the DB updates files 1 by 1
    if (m_verbose) cout << " receiving files from the server. count= " << DBupdatesFilesCount << " " << endl;
    char *dataBuffer = (char *) malloc (DATA_BUFFER_SIZE * sizeof (char));
    for (int i=0; i<DBupdatesFilesCount; i++ ) {
        if (m_verbose) cout << " receiving and writing to file: " << m_DB->getDBupdates()->getDBupdatesfilesNamesList()->at(i) << " " << endl;
        ofstream fileStream(m_DB->getDBupdates()->getDBupdatesfilesNamesList()->at(i));
        string errorMsg = "Unable to open the file: " + m_DB->getDBupdates()->getDBupdatesfilesNamesList()->at(i);
        if (!fileStream.is_open()) error(errorMsg.c_str());
        bzero(dataBuffer, DATA_BUFFER_SIZE);
        size_t fileSizeCounter = m_DB->getDBupdates()->getDBupdatesfilesSizesList()->at(i);
        do {
            size_t minSize = min(fileSizeCounter, DATA_BUFFER_SIZE);
            sockErr = 0;
            while (sockErr<DATA_BUFFER_SIZE)
                sockErr += read(m_sockfd, dataBuffer+sockErr, DATA_BUFFER_SIZE-sockErr);
            //sockErr = read(m_sockfd, dataBuffer, DATA_BUFFER_SIZE);
            if (sockErr < 0) error("ERROR reading X file size");
            fileStream.write(dataBuffer, minSize);
            fileSizeCounter -= minSize;
        } while(fileSizeCounter > 0);
        fileStream.close();
    }
    
    //sending ack to the server
    bzero(infoBuffer, INFO_BUFFER_SIZE);
    strcpy (infoBuffer, DBUPDATES_ACK.c_str());
    sockErr = write(m_sockfd, infoBuffer, INFO_BUFFER_SIZE);
    if (sockErr < 0) error("ERROR sending db name from the db file");
    if (m_verbose) cout << "ACK sent: " << infoBuffer << endl;
}

int ClientConnector::sendQueryToServer(char* t_s) {
    int sockErr;
    unsigned char *infoBuffer;
    infoBuffer = (unsigned char *) malloc (INFO_BUFFER_SIZE * sizeof (unsigned char));
    bzero(infoBuffer, INFO_BUFFER_SIZE);
    strcpy ((char *)infoBuffer, QUERY_REQUEST.c_str());
    sockErr = write(m_sockfd, infoBuffer, INFO_BUFFER_SIZE);
    if (sockErr < 0) error("ERROR writing to socket");
    
    //sending the query
    char* dataBuffer;
    dataBuffer = (char *) malloc (DATA_BUFFER_SIZE * sizeof (char));
    bzero(dataBuffer, DATA_BUFFER_SIZE);
    size_t numberOfBytesToSendFromS = m_DB->getM()/8;
    char* currentS = t_s;
    size_t minSize = 0;
    do {
        minSize = min(numberOfBytesToSendFromS, DATA_BUFFER_SIZE);
        memcpy(dataBuffer,currentS,minSize);
        currentS += minSize;
        int sockErr = write(m_sockfd, dataBuffer, DATA_BUFFER_SIZE);
        if (sockErr < 0) error("ERROR sending X file size");
        numberOfBytesToSendFromS -= minSize;
   } while(numberOfBytesToSendFromS > 0);
   return 0;
}

char* ClientConnector::getSdb() {
    char* sdb = (char *) malloc (m_DB->getN() * sizeof (char));
    char* dataBuffer = (char *) malloc (DATA_BUFFER_SIZE * sizeof (char));
    bzero(dataBuffer, DATA_BUFFER_SIZE);
    size_t numberOfBytesToReceiveFromSdb = m_DB->getN();
    size_t minSize = 0;
    char* currentSdb = sdb;
    do {
        minSize = min(numberOfBytesToReceiveFromSdb, DATA_BUFFER_SIZE);
        int sockErr = 0;
        while (sockErr<DATA_BUFFER_SIZE)
            sockErr += read(m_sockfd, dataBuffer+sockErr, DATA_BUFFER_SIZE-sockErr);
        //int sockErr = read(m_sockfd, dataBuffer, DATA_BUFFER_SIZE);
        if (sockErr < 0) error("ERROR reading X file size");
        memcpy(currentSdb,dataBuffer,minSize);
        currentSdb += minSize;
        numberOfBytesToReceiveFromSdb -= minSize;
    } while(numberOfBytesToReceiveFromSdb > 0);
    return sdb;
}

int ClientConnector::closeConnection() {
    int sockErr;
    char *infoBuffer;
    infoBuffer = (char *) malloc (INFO_BUFFER_SIZE * sizeof (char));
    bzero(infoBuffer, INFO_BUFFER_SIZE);
    strcpy (infoBuffer, CLOSE_CONNECTION.c_str());
    sockErr = write(m_sockfd, infoBuffer, INFO_BUFFER_SIZE);
    if (sockErr < 0) error("ERROR writing to socket");
    close(m_sockfd);
    return 0;
}

int ClientConnector::getClientSocket() { return m_sockfd;}
DBDescriptor* ClientConnector::getDB() { return m_DB;}
XPIRDescriptor* ClientConnector::getXPIR() { return m_XPIR;}

ClientConnector::~ClientConnector() {
}

