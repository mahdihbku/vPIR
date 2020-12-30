/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ServerOps.cpp
 * Author: mahdi
 * 
 * Created on January 7, 2018, 10:22 AM
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <cstring>
#include <fstream>
#include <thread>
#include <vector>
#include <sys/mman.h>
#include <sys/stat.h>
#include <thread>
#include <chrono>
#include <libgen.h>
#include <netinet/tcp.h>

#include "ServerOps.hpp"
#include "DBManager.hpp"
#include "QueryHandler.hpp"
//#include "Wqueue.hpp"
#include "lib.hpp"

using namespace std;

const string ServerOps::DBINFO_REQUEST = "DBInfo request";
const string ServerOps::DBINFO_ACK = "DBInfo ack";
const string ServerOps::VER_REQUEST = "Version request";
const string ServerOps::DBUPDATES_REQUEST = "DBupdates request";
const string ServerOps::DBUPDATES_ACK = "DBupdates ack";
const string ServerOps::NO_UPDATES = "No new updates";
const string ServerOps::XDBFILE_REQUEST = "Sending XDB request";
const string ServerOps::XDBFILE_ACK = "XDB ack";
const string ServerOps::QUERY_REQUEST = "Query request";
const string ServerOps::CLOSE_CONNECTION = "Close Connection";

ServerOps::ServerOps() {
}

ServerOps::ServerOps(DBManager* t_DB, const int t_serverPort, const int t_max, const int t_verbose){
    m_DB=t_DB;
    m_serverPort=t_serverPort;
    MAX_PARALLEL_CLIENTS=t_max;
    m_verbose = t_verbose;
}

ServerOps::ServerOps(DBManager* t_DB, const int t_serverPort, const int t_max, const int t_queueTimeout, const int t_waitingTimeBetweenQueries, const bool t_parallelQueries, const int t_verbose, const int t_parrallelSimQueries){
    m_DB=t_DB;
    m_serverPort=t_serverPort;
    MAX_PARALLEL_CLIENTS=t_max;
    m_queueTimeout=t_queueTimeout;
    m_waitingTimeBetweenQueries=t_waitingTimeBetweenQueries;
    m_forceParallelClients = t_parallelQueries;
    m_verbose = t_verbose;
    m_parrallelSimQueries = t_parrallelSimQueries;
}

ServerOps::ServerOps(const ServerOps& orig) {
}

int ServerOps::openConnection() {
    int sockfd, newsockfd, pid;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    
    thread clientThread[MAX_CLIENTS];
    thread serverThread;
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");
    
    int yes = 1;
    setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (void*) &yes, sizeof(yes));
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(m_serverPort);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR on binding");
    listen(sockfd, MAX_CLIENTS);
    clilen = sizeof(cli_addr);
    
    cout << "db loaded to memory" << endl;
    cout << "Waiting for clients..." << endl;
    
    serverThread = thread(&ServerOps::waitForClients, this);
    
    int noClientsThread = 0;
    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) error("ERROR on accept");
        clientThread[noClientsThread] = thread(&ServerOps::serveNewClient, this, newsockfd);
        if (m_verbose > 1) cout << "-> new client thread created=" << newsockfd << endl;
        noClientsThread++;
        
        //TODO this line for test
        //this_thread::sleep_for(chrono::milliseconds(m_waitingTimeBetweenQueries));
    }
    close(sockfd);
    
    return 0;
}

int ServerOps::waitForClients() {
    int waitingTime = 0;
    while (1) {
        if (m_serverQueue.size() >= MAX_PARALLEL_CLIENTS
                || m_serverQueue.size() > 0 && (!m_DB->getSystemParm()->DBshouldBeSplitted() && !m_forceParallelClients || waitingTime >= m_queueTimeout))
                //|| m_serverQueue.size() > 0 && (!m_DB->getSystemParm()->DBshouldBeSplitted() && !m_forceParallelClients))
        {
            if (m_verbose > 1) cout << "Serving clients in the queue:" << endl;
            if (m_verbose > 1) cout << "Current parameters:" << " WaitingTime=" << waitingTime << " ClientsInQueue=" << m_serverQueue.size() << " ForceParallelClients=" << m_forceParallelClients << endl;
//            if (m_verbose > 1) cout << "DBshouldBeSplitted=" << m_DB->getSystemParm()->DBshouldBeSplitted() << endl;
            chrono::time_point<chrono::system_clock> t0, t1;
            t0 = chrono::system_clock::now();
            serveQueuedClients();
            waitingTime = 0;
            t1 = chrono::system_clock::now();
            chrono::duration<double> totalTime = t1-t0;
            cout << "All queries have been served in: " << totalTime.count() << "s" << endl;
            //TODO
            //what will happen when new clients come and the queue is occupied
        }
        waitingTime += m_waitingTimeBetweenQueries;
        this_thread::sleep_for(chrono::milliseconds(m_waitingTimeBetweenQueries));
    }
    return 0;
}

int ServerOps::serveNewClient(int t_clientSocket) {
    char *infoBuffer;
    infoBuffer = (char *) malloc (INFO_BUFFER_SIZE * sizeof (char));
    int sockErr;
    while (1) {
        bzero(infoBuffer, INFO_BUFFER_SIZE);
        sockErr = 0;
        while (sockErr<INFO_BUFFER_SIZE)
            sockErr += read(t_clientSocket, infoBuffer+sockErr, INFO_BUFFER_SIZE-sockErr);
        //sockErr = read(t_clientSocket, infoBuffer, INFO_BUFFER_SIZE);
        if (m_verbose) cout << "->received from client (" << t_clientSocket << "): " << infoBuffer << endl;
        if (sockErr < 0) error("ERROR reading from socket");
        string reply(infoBuffer);
        if (reply == DBINFO_REQUEST) {
            cout << "Sending DB info to client (" << t_clientSocket << ")..." << endl;
            sendDBFileInfoToClient(t_clientSocket);
            if (m_verbose) cout << "DB info sent to client (" << t_clientSocket << ") successfully" << endl;
        }
//        else if (reply == XDBFILE_REQUEST) {
//            if (m_verbose) cout << "Sending XDB file to client(" << t_clientSocket << ")..." << endl;
//            m_DB->getX()->getXDB()->sendOutputFileToClient(t_clientSocket);
//            if (m_verbose) cout << "XDB sent to client (" << t_clientSocket << ") successfully" << endl;
//        }
        else if (reply == DBUPDATES_REQUEST) {
            if (m_verbose) cout << "Sending files in DBupdates file to client(" << t_clientSocket << ")..." << endl;
            sendDBUpdatesToclient(t_clientSocket);
            if (m_verbose) cout << "DB updates sent to client (" << t_clientSocket << ") successfully" << endl;
        }
        else if (reply == VER_REQUEST) {
            if (m_verbose) cout << "Sending DB version to client(" << t_clientSocket << ")..." << endl;
            sendDBversionToClient(t_clientSocket);
            if (m_verbose) cout << "DB version sent to client (" << t_clientSocket << ") successfully" << endl;
        }
        else if (reply == QUERY_REQUEST) {
            cout << "Getting Query from client (" << t_clientSocket << ")..." << endl;
            serveQuery(t_clientSocket);
        }
        else if (reply == CLOSE_CONNECTION) {
            close(t_clientSocket);
            cout << "Connection closed by the client (" << t_clientSocket << ")." << endl;
            return 0;
        }
        else {
            cout << "ERROR: " << reply << " couldn't be treated" << endl;
            close(t_clientSocket);
            cout << "Connection closed by the client (" << t_clientSocket << ")." << endl;
        }
    }
    return 0;
}

int ServerOps::sendDBUpdatesToclient(int t_clientSocket) {
    string updatesFile = m_DB->getDBDirectory()+"DBupdates.db";
    int sockErr;
    char *infoBuffer;
    infoBuffer = (char *) malloc (INFO_BUFFER_SIZE * sizeof (char));
    if (!fileExists(updatesFile)) {
        //send no updates
        bzero(infoBuffer, INFO_BUFFER_SIZE);
        strcpy (infoBuffer, NO_UPDATES.c_str());
        sockErr = write(t_clientSocket, infoBuffer, INFO_BUFFER_SIZE);
        if (sockErr < 0) error("ERROR writing to socket");
        if (m_verbose) cout << "sendDBUpdatesToclient: sending: " << NO_UPDATES << " " << endl;
    }
    else {
        //send number of files in DBupdates (list.size)
        bzero(infoBuffer, INFO_BUFFER_SIZE);
        DBUpdater* dbu = m_DB->getDBupdater();
        if (dbu == NULL) cout << "sendDBUpdatesToclient: DBUpdater==NULL  " << endl;
        int numberOfFilesInDBupdates = dbu->getNumberOfFileInDBupdates();
        string DBupdatesFileCount = to_string(numberOfFilesInDBupdates);
        strcpy (infoBuffer, DBupdatesFileCount.c_str());
        sockErr = write(t_clientSocket, infoBuffer, INFO_BUFFER_SIZE);
        if (sockErr < 0) error("ERROR writing to socket");
        if (m_verbose) cout << "sending db updates: sending: " << DBupdatesFileCount << " " << endl;
        //send files names + sizes 1 by 1
        string fileName, fileSizeString;
        size_t fileSize = 0;
        for (int i=0; i<numberOfFilesInDBupdates; i++) {
            //sending name
            //TODO send just the file name. not the path
            bzero(infoBuffer, INFO_BUFFER_SIZE);
            fileName = m_DB->getDBupdater()->getDBupdatesfilesNamesList()->at(i);
            strcpy (infoBuffer, fileName.c_str());
            sockErr = write(t_clientSocket, infoBuffer, INFO_BUFFER_SIZE);
            if (sockErr < 0) error("ERROR writing to socket");
            if (m_verbose) cout << "sending db updates: sending: " << fileName << " " << endl;
            //sending size
            bzero(infoBuffer, INFO_BUFFER_SIZE);
            fileSize = m_DB->getDBupdater()->getDBupdatesfilesSizesList()->at(i);
            fileSizeString =  to_string(fileSize);
            strcpy (infoBuffer, fileSizeString.c_str());
            sockErr = write(t_clientSocket, infoBuffer, INFO_BUFFER_SIZE);
            if (sockErr < 0) error("ERROR writing to socket");
            if (m_verbose) cout << "sending db updates: sending: " << fileSizeString << " " << endl;
        }
        //send files 1 by 1
        char *dataBuffer;
        size_t minSize = 0;
        dataBuffer = (char *) malloc (DATA_BUFFER_SIZE * sizeof (char));
        for (int i=0; i<numberOfFilesInDBupdates; i++) {
            fileName = m_DB->getDBupdater()->getDBupdatesfilesNamesList()->at(i);
            fileSize = m_DB->getDBupdater()->getDBupdatesfilesSizesList()->at(i);
            ifstream fileStream(fileName, ios::binary);
            if (m_verbose) cout << "sending db updates: sending file: " << fileName << " " << endl;
            string errorMsg = "Unable to open the file: " + fileName;
            if (!fileStream.is_open()) error(errorMsg.c_str());
            bzero(dataBuffer, DATA_BUFFER_SIZE);
            size_t fileSizeCounter = fileSize;
            do {
                minSize = min(fileSizeCounter, DATA_BUFFER_SIZE);
                fileStream.read(dataBuffer, minSize);
                sockErr = write(t_clientSocket, dataBuffer, DATA_BUFFER_SIZE);
                if (sockErr < 0) error("ERROR sending X file size");
                fileSizeCounter -= minSize;
            } while (fileSizeCounter > 0);
            fileStream.close();
        }
        
        //wait for ack from client
        bzero(infoBuffer, INFO_BUFFER_SIZE);
        sockErr = 0;
        while (sockErr<INFO_BUFFER_SIZE)
            sockErr += read(t_clientSocket, infoBuffer+sockErr, INFO_BUFFER_SIZE-sockErr);
        if (sockErr < 0) error("ERROR reading X file size");
        if (m_verbose) cout << "ACK received: " << infoBuffer << endl;
    }
}

int ServerOps::serveQuery(int t_clientSocket) {
    size_t querySize = m_DB->getM()/8;
    char* query = (char *) malloc (querySize * sizeof (char));
    char* dataBuffer = (char *) malloc (DATA_BUFFER_SIZE * sizeof (char));
    bzero((char *)dataBuffer, DATA_BUFFER_SIZE);
    size_t numberOfBytesToReceiveFromS = querySize;
    size_t minSize = 0;
    char* currentS = query;
    do {
        minSize = min(numberOfBytesToReceiveFromS, DATA_BUFFER_SIZE);
        int sockErr = 0;
        while (sockErr < DATA_BUFFER_SIZE)
            sockErr += read(t_clientSocket, dataBuffer+sockErr, DATA_BUFFER_SIZE-sockErr);
        //int sockErr = read(t_clientSocket, dataBuffer, DATA_BUFFER_SIZE);
        if (sockErr < 0) error("ERROR reading X file size");
        memcpy(currentS,dataBuffer,minSize);
        currentS += minSize;
        numberOfBytesToReceiveFromS -= minSize;
    } while(numberOfBytesToReceiveFromS > 0);

    cout << "Adding query of client (" << t_clientSocket << ") to the queue..." << endl;
//    cout << "Adding query of client. query= " << query << "" << endl;
    queueElement e;
    e.query = query;
    e.socketID = t_clientSocket;
    m_serverQueue.enqueue(&e);
    
    //adding queries for simulation
    if (m_parrallelSimQueries-1 > 0) {
        cout << "Adding queries (" << m_parrallelSimQueries-1 << ") to the queue..." << endl;
        uint8_t seed = 5;
        for (size_t q=0; q<m_parrallelSimQueries-1; q++) {
            char* query = (char *) malloc (querySize * sizeof (char));
            for (size_t i=0; i<querySize; i++) {
                seed+=3;
                query[i]=seed;
            }
            queueElement e;
            e.query = query;
            e.socketID = t_clientSocket;
            m_serverQueue.enqueue(&e);
        }
        cout << "queries added to the queue" << endl;
    }
    return 0;
}

int ServerOps::sendDBversionToClient(int t_clientSocket) {
    char *infoBuffer;
    infoBuffer = (char *) malloc (INFO_BUFFER_SIZE * sizeof (char));
    bzero(infoBuffer, INFO_BUFFER_SIZE);
    strcpy (infoBuffer, m_DB->getDBVersion().c_str());
    int sockErr = write(t_clientSocket, infoBuffer, INFO_BUFFER_SIZE);
    if (sockErr < 0) error("ERROR sending db version to client");
    return 0;
}

int ServerOps::sendDBFileInfoToClient(int t_clientSocket) {
    ifstream DBInfoFileStream(m_DB->getDBFileInfo(),ifstream::in);
    if (DBInfoFileStream.is_open()) {
        int sockErr;
        char *infoBuffer;
        infoBuffer = (char *) malloc (INFO_BUFFER_SIZE * sizeof (char));
        string line;
        for (int j=0; j<8; j++){
            getline(DBInfoFileStream, line);
            bzero(infoBuffer, INFO_BUFFER_SIZE);
            strcpy (infoBuffer, line.c_str());
            sockErr = write(t_clientSocket, infoBuffer, INFO_BUFFER_SIZE);
            if (sockErr < 0) error("ERROR sending db name from the db file");
        }
        getline(DBInfoFileStream, line);    //bypass XPIRdir
        getline(DBInfoFileStream, line);    //bypass XDB
        getline(DBInfoFileStream, line);    //bypass DB updates
        getline(DBInfoFileStream, line);    //bypass "Catalog:"
        bool nameLine = true;
        for(string line; getline(DBInfoFileStream, line); ) {    //TODO remove the boolean test. for loop(read write, read write)
            bzero(infoBuffer, INFO_BUFFER_SIZE);
            char* filename = basename((char *)line.c_str());
            if (nameLine) {
                strcpy (infoBuffer, filename);
                nameLine = false;
            } else {    //sizeLine
                strcpy (infoBuffer, line.c_str());
                nameLine = true;
            }
            sockErr = write(t_clientSocket, infoBuffer, INFO_BUFFER_SIZE);
            if (sockErr < 0) error("ERROR sending db name from the db file");
        }
        DBInfoFileStream.close();
        //wait for ack from client
        bzero(infoBuffer, INFO_BUFFER_SIZE);
        sockErr = 0;
        while (sockErr<INFO_BUFFER_SIZE)
            sockErr += read(t_clientSocket, infoBuffer+sockErr, INFO_BUFFER_SIZE-sockErr);
        //sockErr = read(t_clientSocket, infoBuffer, INFO_BUFFER_SIZE);
        if (sockErr < 0) error("ERROR reading X file size");
        if (m_verbose) cout << "ACK received: " << infoBuffer << endl;
    }
    else
        cout << "Unable to open the db file: " << m_DB->getDBFileInfo() << endl;
    return 0;
}

int ServerOps::serveQueuedClients() {
    chrono::time_point<chrono::system_clock> t0, t1, t2, t3, t4;
    chrono::duration<double> totalQDBTime;
    chrono::duration<double> totalTime;
    
    if (!m_DB->getSystemParm()->DBshouldBeSplitted())
    {   //DB can be loaded entirely to memory
        if (m_verbose) cout << "Force Serving Clients in Parallel: " << m_forceParallelClients << endl;
        if (!m_forceParallelClients) {
            if (m_verbose) cout << "m_serverQueue.size(): " << m_serverQueue.size() << endl;
            while (m_serverQueue.size()!=0)
            {
                t0 = chrono::system_clock::now();
                queueElement* e;
                e = m_serverQueue.dequeue();
                if (m_verbose) cout << "Serving client (" << e->socketID << ")" << endl;
                QueryHandler qh(m_DB, e->socketID, m_verbose);
                qh.setQuery(e->query);
                if (m_verbose) cout << "Calculating QDB for client (" << e->socketID << ")..." << endl;
                t1 = chrono::system_clock::now();
                qh.processOneQuery();
                t2 = chrono::system_clock::now();
                totalQDBTime = t2 - t1;
                totalTime = t2- t0;
                //qh.sendBackQdbToClient();
                cout << "Sending Query Reply to client (" << e->socketID << ")..." << endl;
                
                //TODO to be removed just a test///////////////////////////////////////////////
//                t3 = chrono::system_clock::now();
//                char *reply = (char*) calloc (m_DB->getN(), sizeof(char));
//                qh.processMultipleQueries(m_DB->getDBbuffer(), e->query, 1, reply);
//                t4 = chrono::system_clock::now();
//                chrono::duration<double> totalQDBTime2 = t4-t3;
                //TODO to be removed just a test///////////////////////////////////////////////
                
                sendQueryReplyToClient(qh.getReply(), m_DB->getN(), e->socketID);
                //cout << "Calculating QDB: qdb sent: " << qh.getQdb() << "" << endl;
                if (m_verbose) cout << "TotalQDBTime: " << totalQDBTime.count() << "s totalComputationTime: " << totalTime.count() << endl;
                
                //TODO to remove///////////////////////////////////////////////////////////////
                //if (m_verbose) cout << "totalQDBTime2: " << totalQDBTime2.count() << endl;
                ///////////////////////////////////////////////////////////////////////////////
            }
        }
        else {
            if (m_serverQueue.size()!=0) {
                t0 = chrono::system_clock::now();
                //getting queries from the queue
                int numberOfParallelClients = m_serverQueue.size();
                size_t querySize = m_DB->getM()/8;
                size_t queriesBufferSize = numberOfParallelClients*querySize;
                char *parallelQueries = (char *) malloc (queriesBufferSize * sizeof(char));//parallelQueries[numberOfParallelClients][querySize]
                //char parallelQueries[numberOfParallelClients][querySize];
                int parallelClients[numberOfParallelClients];
                for (int i=0; i<numberOfParallelClients; i++)
                {
                    queueElement* e;
                    e = m_serverQueue.dequeue();
                    parallelClients[i] = e->socketID;
                    memcpy(&parallelQueries[i*querySize], e->query, querySize); //parallelQueries[i][0]
                }
                size_t replySize = m_DB->getN();
                size_t repliesBufferSize = numberOfParallelClients*replySize;
                char *repliesBuffer = (char *) calloc (repliesBufferSize, sizeof (char));
                cout << "TREATING " << numberOfParallelClients << " queries in parallel" << endl;
                //resolving queries
                QueryHandler qh(m_DB, 0, m_verbose);
                t1 = chrono::system_clock::now();
//                size_t transposableQueries = numberOfParallelClients/8*8;
//                cout << "processing " << transposableQueries << " queries with transposing" << endl;
//                qh.processMultipleQueriesWithTransposing(m_DB->getDBbuffer(), parallelQueries, transposableQueries, repliesBuffer);
//                if (numberOfParallelClients%8 != 0) {
//                    size_t nontTransposableQueries = numberOfParallelClients%8;
//                cout << "processing " << nontTransposableQueries << " queries without transposing" << endl;
//                    qh.processMultipleQueries(m_DB->getDBbuffer(), &parallelQueries[transposableQueries*querySize], nontTransposableQueries, &repliesBuffer[transposableQueries*replySize]);
//                }
                char *DBstream = m_DB->getDBbuffer();
                qh.processQueries(DBstream, parallelQueries, numberOfParallelClients, repliesBuffer);
                t2 = chrono::system_clock::now();
                if (!m_parrallelSimQueries) {    //if not a simulation, send back the files to the queued clients
                    for (int i=0; i<numberOfParallelClients; i++)
                    {
                        if (m_verbose) cout << "Sending reply to client (" << parallelClients[i] << ")..." << endl;
                        sendQueryReplyToClient(&repliesBuffer[i*m_DB->getN()], m_DB->getN(), parallelClients[i]);
                    }
                }
                totalQDBTime = t2 - t1;
                totalTime = t2- t0;
                if (m_verbose) cout << "totalQDBTime: " << totalQDBTime.count() << "s totalComputationTime: " << totalTime.count() << endl;
            }
        }
    }
    else {  //DB should be splitted
        
        t0 = chrono::system_clock::now();
        totalQDBTime = t0-t0;
        //initializing the queries chunks
        int numberOfChunks = m_DB->getSystemParm()->getNumberOfChunks();
        char * queriesChunks[numberOfChunks];
        size_t numberOfParallelClients = m_serverQueue.size();
        size_t C = m_DB->getDBbufferSize()/m_DB->getN();    //C is the number of files in a regular DB chunk (excluding the last one)
        size_t lastC = (m_DB->getM()%C == 0)? C : m_DB->getM()%C;
        size_t subQuerySize=0, queriesChunkSize=0;
        int j=0;
        subQuerySize = C/8;
        queriesChunkSize = subQuerySize*numberOfParallelClients;    //jth sub-queries chunk size
        for ( ; j<numberOfChunks-1; j++) {    //last chunk should be treated separately, its size could be different
            queriesChunks[j] = (char*) malloc (queriesChunkSize * sizeof(char));
        }
        //last chunk
        subQuerySize = lastC/8;
        queriesChunkSize = subQuerySize*numberOfParallelClients;
        queriesChunks[j] = (char*) malloc (queriesChunkSize * sizeof(char));
        
        //copying from queue to queries chunks
        if (m_verbose > 1) cout << "Splitting the queries into " << numberOfChunks << " subqueries" << endl;
        int parallelClients[numberOfParallelClients];
        for (size_t i=0; i<numberOfParallelClients; i++) {
            queueElement* e;
            e = m_serverQueue.dequeue();
            parallelClients[i] = e->socketID;
            
//            cout << "e->query=" << e->query << endl;
//            cout << "e->query[553]=" << e->query[553] << endl;
//            cout << "e->query[1106]=" << e->query[1106] << endl;
            
            subQuerySize = C/8;
            size_t queryParser = 0;
            for (int j=0; j<numberOfChunks-1; j++) {    //last chunk should be treated separately, its size could be different
                memcpy(&queriesChunks[j][i*subQuerySize], &e->query[queryParser], subQuerySize);
                queryParser += subQuerySize;
//                cout << "queryParser=" << queryParser << " j=" << j << " i=" << i << " queriesChunks[" << j << "]=" << queriesChunks[j] << endl;
            }
            //last chunk
            subQuerySize = lastC/8;
            memcpy(&queriesChunks[j][i*subQuerySize], &e->query[queryParser], subQuerySize);
//            cout << "queryParser=" << queryParser << " j=" << j << " i=" << i << " queriesChunks[" << j << "]=" << queriesChunks[j] << endl;
        }
        if (m_verbose > 1) cout << "Splitting done" << endl;
        
        //processing queries
        cout << "TREATING " << numberOfParallelClients << " queries in parallel" << endl;
        if (m_verbose > 2) cout << " C=" << C << " lastC=" << lastC << " C/8=" << C/8 << " lastC/8=" << lastC/8 << endl;
        t0 = chrono::system_clock::now();
        size_t replySize = m_DB->getN();
        size_t repliesBufferSize = numberOfParallelClients*replySize;
        char *repliesBuffer = (char *) calloc (repliesBufferSize, sizeof (char));
        QueryHandler qh(m_DB, 0, m_verbose);
        //size_t transposableQueries = numberOfParallelClients/8*8;
        subQuerySize = C/8;
        for (int j=0; j<numberOfChunks; j++) {    //last chunk should be treated separately, its size could be different
            t2 = chrono::system_clock::now();
            subQuerySize = (j==numberOfChunks-1)? lastC/8 : C/8;
            //cout << "processing " << transposableQueries << " queries with transposing" << endl;
//            qh.processMultipleQueriesWithTransposing(m_DB->getDBbuffer(), queriesChunks[j], transposableQueries, repliesBuffer);
//            if (numberOfParallelClients%8 != 0) {
//                size_t nontTransposableQueries = numberOfParallelClients%8;
//                cout << "processing " << nontTransposableQueries << " queries without transposing" << endl;
//                qh.processMultipleQueries(m_DB->getDBbuffer(), &queriesChunks[j][transposableQueries*subQuerySize], nontTransposableQueries, &repliesBuffer[transposableQueries*replySize]);
//            }
            qh.processQueries(m_DB->getDBbuffer(), queriesChunks[j], numberOfParallelClients, repliesBuffer);
            t3 = chrono::system_clock::now();
            totalQDBTime += t3 - t2;
            if (j+1 < numberOfChunks) m_DB->loadDBChunkToMemory(j+1);    //load new DB chunk
        }
        t4 = chrono::system_clock::now();
        totalTime = t4 - t0;
        if (m_verbose) cout << "totalQDBTime: " << totalQDBTime.count() << "s totalComputationTime: " << totalTime.count() << endl;
        
        //sending replies to clients
        if (!m_parrallelSimQueries) {    //if not a simulation, send back the files to the queued clients
            for (int i=0; i<numberOfParallelClients; i++)
            {
                if (m_verbose) cout << "Sending reply to client (" << parallelClients[i] << ")..." << endl;
                sendQueryReplyToClient(&repliesBuffer[i*m_DB->getN()], m_DB->getN(), parallelClients[i]);
            }
        }
        
        //loading the first db chunk after sending the replies
        m_DB->loadDBChunkToMemory(0);
        
//        cout << "TREATING " << numberOfParallelClients << " queries in parallel" << endl;
//        //resolving queries
//        t0 = chrono::system_clock::now();
//        size_t parser=0;
//        size_t outputFilesBufferSize = numberOfParallelClients*m_DB->getN();
//        char *outputFilesBuffer = (char *) malloc (outputFilesBufferSize * sizeof (char));
//        //char outputFiles[numberOfParallelClients][m_DB->getN()];
//        for (int j=0; j<m_DB->getSystemParm()->getNumberOfChunks(); j++) {
//            for (int i=0; i<numberOfParallelClients; i++) {
//                if (m_verbose) cout << "Serving client (" << parallelClients[i] << "):" << endl;
//                QueryHandler qh(m_DB, parallelClients[i], m_verbose);
//                qh.setQuery(&parallelQueries[i][0]);
//                if (m_verbose) cout << "Calculating QDB for client (" << parallelClients[i] << ")..." << endl;
//                t2 = chrono::system_clock::now();
//                qh.calculateQdb();
////                cout << "outputFilesBuffer[" << numberOfParallelClients <<","<<m_DB->getN() << "] has been allocated" << endl;
////                cout << "qh.getQdbSize()=" << qh.getQdbSize() << endl;
//                if (m_verbose) cout << "Copying qdb to outputFile[" << i << "," <<  parser << "] writing " << qh.getQdbSize() << " of data." << endl;
////                if (m_verbose) cout << "qdb= " << qh.getQdb() << endl;
//                memcpy(&outputFilesBuffer[i*m_DB->getN()+parser], qh.getQdb(), qh.getQdbSize());
//                t3 = chrono::system_clock::now();
//                totalQDBTime += t3 - t2;
////                if (m_verbose) cout << "writing data to output file " << i << "," <<  parser << " writing " << qh.getQdbSize() << " of data." << endl;
////                if (m_verbose) cout << "------qh.getQdbSize()= " << qh.getQdbSize() << endl;
////                if (m_verbose) cout << "------outputFilesSize()= " << m_DB->getN() << endl;
//            }
//            parser += m_DB->getDBbufferSize()/m_DB->getM();
//            //load new chunk to memory
//            if (j+1 < m_DB->getSystemParm()->getNumberOfChunks()) m_DB->loadDBChunkToMemory(j+1);
//        }
//        t4 = chrono::system_clock::now();
//        totalTime = t4 - t0;
//        if (m_verbose) cout << "totalQDBTime: " << totalQDBTime.count() << "s totalComputationTime: " << totalTime.count() << endl;
//        parser=0;
//        //sending files to clients
//        for (int i=0; i<numberOfParallelClients; i++)
//        {
//            if (m_verbose) cout << "Sending reply to client (" << parallelClients[i] << ")..." << endl;
//            sendQueryReplyToClient(&outputFilesBuffer[i*m_DB->getN()], m_DB->getN(), parallelClients[i]);
//        }
//        m_DB->loadDBChunkToMemory(0);
    }
    return 0;
}

int ServerOps::sendQueryReplyToClient(char* t_query, size_t querySize, int t_clientSocket) {
    char *dataBuffer;
    dataBuffer = (char *) malloc (DATA_BUFFER_SIZE * sizeof (char));
    bzero(dataBuffer, DATA_BUFFER_SIZE);
    size_t numberOfBytesToSendFromSdb = querySize;
    char* currentSdb = t_query;
    size_t minSize = 0;
    do {
        minSize = min(numberOfBytesToSendFromSdb, DATA_BUFFER_SIZE);
        memcpy(dataBuffer,currentSdb,minSize);
        currentSdb += minSize;
        int sockErr = write(t_clientSocket, dataBuffer, DATA_BUFFER_SIZE);
        if (sockErr < 0) error("ERROR sending X file size");
        numberOfBytesToSendFromSdb -= minSize;
   } while(numberOfBytesToSendFromSdb > 0);
   return 0;
}

int ServerOps::setServerPort(int t_serverPort) {m_serverPort = t_serverPort;}
int ServerOps::setMaxParallelClient(int t_max) {MAX_PARALLEL_CLIENTS = t_max;}
    
ServerOps::~ServerOps() {
}

