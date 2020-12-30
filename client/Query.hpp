/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Query.hpp
 * Author: mahdi
 *
 * Created on September 10, 2017, 11:54 AM
 */

#ifndef QUERY_HPP
#define QUERY_HPP

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include "DBDescriptor.hpp"

#define htonll(x) ((((uint64_t)htonl(x)) << 32) + htonl((x) >> 32));

class Query {
public:
    Query();
    Query(int t_K, std::string t_outputFile, DBDescriptor* t_DB, int t_requestedFile, int t_verbose);
    int selectKRandomX();
    int calculateW();
    int generateRandomWforSimulation();
    int calculateS();
    char* getS();
    int setSDB(char* t_sdb);
    int calculateData();
    int writeDataToFile();
    Query(const Query& orig);
    virtual ~Query();
private:
    DBDescriptor* m_DB;
    int m_fileNumber;
    int m_K;
    std::string m_outputFile;
    int* m_selectedXid;
    char* m_w;
    char* m_wDB;
    char* m_s;
    char* m_sDB;
    char* m_data;
    int m_verbose = 0;
};

#endif /* QUERY_HPP */

