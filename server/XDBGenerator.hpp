/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   XDBGenerator.hpp
 * Author: mahdi
 *
 * Created on August 27, 2017, 12:59 AM
 */

#ifndef XDBGENERATOR_HPP
#define XDBGENERATOR_HPP

//#include "DBManager.hpp"
class DBManager;

#include <string>

#define htonll(x) ((((uint64_t)htonl(x)) << 32) + htonl((x) >> 32));

class XDBGenerator {
    static const int INFO_BUFFER_SIZE = 1024;
    static const int DATA_BUFFER_SIZE = 16384;
    static const std::string XDBFILE_REQUEST;
    static const std::string XDBFILE_REQUEST_ACK;
    
public:
    uint8_t m_parityLookupTable[256] = {0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0};
    XDBGenerator();
    //XDBGenerator(int newR, DBManager* newDB, std::string newOutputFile);
    XDBGenerator(int t_R, std::string t_outputFile, int t_verbose) : m_R(t_R), m_XDBfileName(t_outputFile), m_verbose(t_verbose) {}
    //int setDB(DBManager* newDB);
    int createXDBFileFromSeed(char* t_seed, DBManager* t_DB);
    int sendOutputFileToClient(const int t_socket);
    std::string getOutputFile();
    int loadXDBtoMemory();
    XDBGenerator(const XDBGenerator& orig);
    virtual ~XDBGenerator();
private:
    int m_R;
    char *m_buffer;
    size_t m_bufferSize;
    size_t m_seedSize=512;  //in bits
    std::string m_XDBfileName;
    int m_verbose = 0;
};

#endif /* XDBGENERATOR_HPP */

