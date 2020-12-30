/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   QueryHandler.hpp
 * Author: mahdi
 *
 * Created on September 13, 2017, 11:32 AM
 */

#ifndef QUERYHANDLER_HPP
#define QUERYHANDLER_HPP


#include <string>

#include "DBManager.hpp"

class QueryHandler {
    static const int INFO_BUFFER_SIZE = 4096;
    static const int DATA_BUFFER_SIZE = 16384;
    static const std::string QUERY_REQUEST;
    static const std::string QUERY_REQUEST_ACK;
public:
    
    QueryHandler();
    QueryHandler(DBManager* t_DB, const int t_socket, const int t_verbose);
    int processOneQuery();
    int processQueries(char* DBstream, char* queries, size_t numberOfQueries, char* replies);
//    int processMultipleQueries(char* DBstream, char* queries, size_t numberOfQueries, char* replies);
//    int processMultipleQueriesWithTransposing(char* DBstream, char* queries, size_t numberOfQueries, char* replies);
    int sseTransose(char* inBuff, char* outBuff, size_t numberOfRows, size_t numberOfColumns);
    int sendBackReplyToClient();
    char* getReply();
    size_t getReplySize();
    size_t getQuerySize();
    QueryHandler(const QueryHandler& orig);
    virtual ~QueryHandler();
    int setQuery(char* t_query);
private:
    size_t m_C, m_M;
    DBManager* m_DB;
    int m_clientSocket;
    char* m_query;
    size_t m_querySize;
    char* m_reply;
    size_t m_replySize;
    int m_verbose = 0;
    unsigned char m_masks[8] = {0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};
};

#endif /* QUERYHANDLER_HPP */

