/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   XManager.hpp
 * Author: mahdi
 *
 * Created on August 29, 2017, 10:18 AM
 */

#ifndef XMANAGER_HPP
#define XMANAGER_HPP

#include <string>

#include "XDBGenerator.hpp"

class XManager {
    static const int INFO_BUFFER_SIZE = 1024;
    static const int DATA_BUFFER_SIZE = 16384;
public:
    static const std::string XFILE_REQUEST;
    static const std::string XFILE_REQUEST_ACK;
    static const std::string XFILE_SEED;
    static const std::string XFILE_SEED_ACK;
    XManager(size_t t_M, int t_R, std::string t_XDBfile, int t_verbose);
    XManager(size_t t_M, int t_R, char* t_seed, std::string t_XDBfile, int t_verbose);
    int getRandomSeed();
    int setSeed(char* t_seed);
    char* getSeed();
    int sendXSeedToClient(const int t_socket);
    int setR(const int t_R);
    int getR();
    XDBGenerator* getXDB();
    XManager(const XManager& orig);
    virtual ~XManager();
private:
    int m_R;
    size_t m_M;
    XDBGenerator* m_XDB; 
    char* m_seed;
    size_t m_seedSize = 512;
    int m_verbose = 0;
};

#endif /* XMANAGER_HPP */

