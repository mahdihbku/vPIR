/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Xgetter.hpp
 * Author: mahdi
 *
 * Created on August 29, 2017, 10:52 AM
 */

#ifndef XGETTER_HPP
#define XGETTER_HPP

#include <string>

#include "DBDescriptor.hpp"

using namespace std;

class Xgetter {
    static const int INFO_BUFFER_SIZE = 1024;
    static const int DATA_BUFFER_SIZE = 16384;
public:
    static const string XFILE_REQUEST;
    static const string XFILE_REQUEST_ACK;
    static const string XFILE_SEED;
    static const string XFILE_SEED_ACK;
    Xgetter(int socket, DBDescriptor* newDB);
    int getSeedFromServer();
    int saveXtoFile(string file);
    unsigned long long getSeed();
    int getR();
    Xgetter(const Xgetter& orig);
    virtual ~Xgetter();
private:
    DBDescriptor* db;
    int clientSocket;
    string clientXfile;
    unsigned long long seed;
    int r;
    int error(const char *msg);
};

#endif /* XGETTER_HPP */

