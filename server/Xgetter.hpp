/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Xgetter.hpp
 * Author: mahdi
 *
 * Created on August 24, 2017, 12:12 AM
 */

#include <string>

#ifndef XGETTER_HPP
#define XGETTER_HPP

class Xgetter {
    static const int INFO_BUFFER_SIZE = 4096;
    static const int DATA_BUFFER_SIZE = 16384;
    
public:
    static const std::string XFILE_REQUEST;
    static const std::string XFILE_REQUEST_ACK;
    static const std::string XFILE_SEED;
    static const std::string XFILE_SEED_ACK;
    Xgetter(int socket);
    int saveXtoFile(std::string file);
    unsigned long long getSeed();
    int getR();
    Xgetter(const Xgetter& orig);
    virtual ~Xgetter();
    
private:
    int clientSocket;
    std::string clientXfile;
    unsigned long long seed;
    int r;
    int error(const char *msg);
    int min(int a, int b);
};

#endif /* XGETTER_HPP */

