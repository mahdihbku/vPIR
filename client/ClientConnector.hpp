/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ClientConnector.hpp
 * Author: mahdi
 *
 * Created on August 23, 2017, 11:43 PM
 */

#ifndef CLIENTCONNECTOR_HPP
#define CLIENTCONNECTOR_HPP

#include <string>
#include <vector>

#include "DBDescriptor.hpp"
#include "XPIRDescriptor.hpp"

class ClientConnector {
    static const int INFO_BUFFER_SIZE = 1024;
    static const int DATA_BUFFER_SIZE = 16384;
public:
    static const std::string DBINFO_REQUEST;
    static const std::string DBINFO_ACK;
    static const std::string VER_REQUEST;
    static const std::string XDBFILE_REQUEST;
    static const std::string XDBFILE_ACK;
    static const std::string DBUPDATES_REQUEST;
    static const std::string DBUPDATES_ACK;
    static const std::string NO_UPDATES;
    static const std::string QUERY_REQUEST;
    static const std::string CLOSE_CONNECTION;
    ClientConnector(int t_verbose);
    ClientConnector(const ClientConnector& orig);
    virtual ~ClientConnector();
    int setServerInfo(const std::string t_ipAddr, const int t_portNumber);
    int setDataBase(DBDescriptor* t_DB);
    int initiateConnection();
    int getDBInfoFromServer();
    std::string getDBversionFromServer();
    std::string getServerIP();
    int getServerPort();
    int getClientSocket();
    int closeConnection();
    int generateAndSaveXtoFile(std::string t_XfileName, std::vector<size_t>* t_RandomlySelectedVectors);
    int getAndSaveXDBtoFile(const std::string t_XDBfileName, std::vector<size_t>* t_selectedVectors);
    int getAndSaveDBupdatesToFile(const std::string t_DBupdatesFileName);
    int sendQueryToServer(char* t_s);
    char* getSdb();
    DBDescriptor* getDB();
    XPIRDescriptor* getXPIR();
private:
    int m_sockfd;
    double m_Q;
    DBDescriptor* m_DB;
    XPIRDescriptor* m_XPIR;
    std::string m_serverIP;
    int m_serverPort;
    int m_verbose = 0;
};

#endif /* CLIENTCONNECTOR_HPP */

