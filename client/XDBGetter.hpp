/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   XDBGetter.hpp
 * Author: mahdi
 *
 * Created on August 28, 2017, 10:07 AM
 */

#ifndef XDBGETTER_HPP
#define XDBGETTER_HPP

#include <cstring>

class XDBGetter {
    static const int INFO_BUFFER_SIZE = 1024;
    static const int DATA_BUFFER_SIZE = 16384;
public:
    static const std::string XDBFILE_REQUEST;
    static const std::string XDBFILE_REQUEST_ACK;
    int getAndSaveXDBtoFile(const int t_socket);
    XDBGetter(const std::string t_outputFile);
    XDBGetter(const XDBGetter& orig);
    virtual ~XDBGetter();
private:
    int m_clientSocket;
    std::string m_XDBfile;
    bool verbose = false;
};

#endif /* XDBGETTER_HPP */

