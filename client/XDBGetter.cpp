/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   XDBGetter.cpp
 * Author: mahdi
 * 
 * Created on August 28, 2017, 10:07 AM
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

#include "XDBGetter.hpp"
#include "lib.hpp"

using namespace std;

const string XDBGetter::XDBFILE_REQUEST = "Sending XDB request";
const string XDBGetter::XDBFILE_REQUEST_ACK = "Sending XDB request ACK";

XDBGetter::XDBGetter(const string t_outputFile) {
    m_XDBfile = t_outputFile;
}

int XDBGetter::getAndSaveXDBtoFile(const int t_socket) {
    m_clientSocket = t_socket;
    int sockErr;
    char *infoBuffer;
    infoBuffer = (char *) malloc (INFO_BUFFER_SIZE * sizeof (char));

    //receiving the XDB file size
    bzero(infoBuffer, INFO_BUFFER_SIZE);
    sockErr = read(m_clientSocket, infoBuffer, INFO_BUFFER_SIZE);
    if (sockErr < 0) error("ERROR reading X file size");
    string XDBfileSizeString(infoBuffer);
    size_t XfileSizeInt = stol(XDBfileSizeString);

    //receiving the XDB file
    ofstream XFileStream(m_XDBfile);
    string errorMsg = "Unable to open the file: " + m_XDBfile;
    if (!XFileStream.is_open()) error(errorMsg.c_str());
    char *dataBuffer;
    dataBuffer = (char *) malloc (DATA_BUFFER_SIZE * sizeof (char));
    bzero(dataBuffer, DATA_BUFFER_SIZE);
    size_t fileSizeCounter = XfileSizeInt;
    do {
        size_t minSize = min(fileSizeCounter, DATA_BUFFER_SIZE);
        sockErr = read(m_clientSocket, dataBuffer, DATA_BUFFER_SIZE);
        if (sockErr < 0) error("ERROR reading X file size");
        XFileStream.write(dataBuffer, minSize);
        fileSizeCounter -= minSize;
    }while(fileSizeCounter > 0);
    XFileStream.close();
    
    //sending "sending XDB request ACK"
    bzero(infoBuffer, INFO_BUFFER_SIZE);
    strcpy (infoBuffer, XDBFILE_REQUEST_ACK.c_str());
    sockErr = write(m_clientSocket, infoBuffer, INFO_BUFFER_SIZE);
    if (sockErr < 0) error("ERROR sending sending X file request ACK");
}

XDBGetter::XDBGetter(const XDBGetter& orig) {
}

XDBGetter::~XDBGetter() {
}

