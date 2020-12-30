/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ServerOps.hpp
 * Author: mahdi
 *
 * Created on January 7, 2018, 10:22 AM
 */

#ifndef SERVEROPS_HPP
#define SERVEROPS_HPP

#include <string>
#include <vector>

#include "DBManager.hpp"
#include "Wqueue.hpp"
#include "DBUpdater.hpp"

class ServerOps {
    static const int INFO_BUFFER_SIZE = 1024;
    static const int DATA_BUFFER_SIZE = 16384;
    static const std::string DBINFO_REQUEST;
    static const std::string DBINFO_ACK;
    static const std::string VER_REQUEST;
    static const std::string DBUPDATES_REQUEST;
    static const std::string DBUPDATES_ACK;
    static const std::string NO_UPDATES;
    static const std::string XDBFILE_REQUEST;
    static const std::string XDBFILE_ACK;
    static const std::string QUERY_REQUEST;
    static const std::string CLOSE_CONNECTION;
    
    
public:
    ServerOps();
    ServerOps(DBManager* t_DB, const int t_serverPort, const int t_max, const int t_verbose);
    ServerOps(DBManager* t_DB, const int t_serverPort, const int t_max, const int t_queueTimeout, const int t_waitingTimeBetweenQueries, const bool t_parallelQueries, const int t_verbose, const int t_parrallelSimQueries);
    int openConnection();
    ServerOps(const ServerOps& orig);
    int setServerPort(const int t_serverPort);
    int setMaxParallelClient(const int t_max);
    virtual ~ServerOps();
    
private:
    struct queueElement {
        int socketID;
        char* query;
    };
    int m_serverPort = 12345;
    int MAX_CLIENTS = 1000;
    int MAX_PARALLEL_CLIENTS = 5;
    Wqueue<queueElement*> m_serverQueue;
    DBManager* m_DB;
    int m_queueTimeout = 1000;
    int m_waitingTimeBetweenQueries = 10;
    int m_verbose = 0;
    bool m_forceParallelClients = false;
    int m_parrallelSimQueries=0;
    
    int serveNewClient(int t_clientSocket);
    int sendDBUpdatesToclient(int t_clientSocket);
    int waitForClients();
    int serveQueuedClients();
    int sendQueriesToClients();
    int sendDBFileInfoToClient(int t_clientSocket);
    int sendQueryReplyToClient(char* t_query, size_t querySize, int t_clientSocket);
    int sendDBversionToClient(int t_clientSocket);
    int serveQuery(int t_clientSocket);
};

#endif /* SERVEROPS_HPP */

