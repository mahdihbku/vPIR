/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Xgetter.cpp
 * Author: mahdi
 * 
 * Created on August 24, 2017, 12:12 AM
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

#include "Xgetter.hpp"

using namespace std;

Xgetter::Xgetter(int socket)
{
	int sockErr;
	clientSocket = socket;
	char *infoBuffer;
	infoBuffer = (char *) malloc (INFO_BUFFER_SIZE * sizeof (char));
	
	//receiving "sending X request" or "Sending X seed request"
	bzero(infoBuffer, INFO_BUFFER_SIZE);
	sockErr = read(clientSocket, infoBuffer, INFO_BUFFER_SIZE);
	if (sockErr < 0) error("ERROR while reading sending X file request");
	std::string sendingXrequest(infoBuffer);
        
	if (sendingXrequest == XFILE_REQUEST) {
                std::cout << "received: " << sendingXrequest << std::endl;

            //sending "sending X request ACK"
            bzero(infoBuffer, INFO_BUFFER_SIZE);
            strcpy (infoBuffer, XFILE_REQUEST_ACK.c_str());
            sockErr = write(clientSocket, infoBuffer, INFO_BUFFER_SIZE);
            if (sockErr < 0) error("ERROR sending sending X file request ACK");

            //receiving the X file size
            bzero(infoBuffer, INFO_BUFFER_SIZE);
            sockErr = read(clientSocket, infoBuffer, INFO_BUFFER_SIZE);
            if (sockErr < 0) error("ERROR reading X file size");
            std::string XfileSizeString(infoBuffer);
            int XfileSizeInt = std::stoi(XfileSizeString);
            std::cout << "received: XfileSizeString= " << XfileSizeString << std::endl;

            //receiving 'r'
            bzero(infoBuffer, INFO_BUFFER_SIZE);
            sockErr = read(clientSocket, infoBuffer, INFO_BUFFER_SIZE);
            if (sockErr < 0) error("ERROR reading X file size");
            std::string rString(infoBuffer);
            std::cout << "received: XfileSizeString= " << rString << std::endl;
            r = stoi(rString);

            //receiving the X file
            std::string clientXfileName = "clientNameX";    //TO BE CHANGED
            ofstream XFileStream(clientXfileName);
            if (XFileStream.is_open())
            {
                char *dataBuffer;
                dataBuffer = (char *) malloc (DATA_BUFFER_SIZE * sizeof (char));
                bzero(dataBuffer, DATA_BUFFER_SIZE);
                int fileSizeCounter = XfileSizeInt;
                do {
                    size_t minSize = min(fileSizeCounter, DATA_BUFFER_SIZE);
                    sockErr = read(clientSocket, dataBuffer, DATA_BUFFER_SIZE);
                    if (sockErr < 0) error("ERROR reading X file size");
                    XFileStream.write(dataBuffer, minSize);
                    cout << "written : " << minSize << " to the file : " << clientXfileName << endl;
                    fileSizeCounter -= minSize;
                    /*int offset = 0;
                    do
                    {
                        size_t written = fwrite(&buffer[offset], 1, num-offset, f);
                        if (written < 1)
                            return false;
                        offset += written;
                    }
                    while (offset < num);*/
                }while(fileSizeCounter > 0);
                XFileStream.close();
            }
            else
                    cout << "Unable to open the file: " << clientXfileName;
        }
		
        std::cout << "received: " << sendingXrequest << std::endl;
	if (sendingXrequest == XFILE_SEED) {
            //sending "sending X seed request ACK"
            bzero(infoBuffer, INFO_BUFFER_SIZE);
            strcpy (infoBuffer, XFILE_SEED_ACK.c_str());
            sockErr = write(clientSocket, infoBuffer, INFO_BUFFER_SIZE);
            if (sockErr < 0) error("ERROR sending sending X file request ACK");

            //receiving the seed
            bzero(infoBuffer, INFO_BUFFER_SIZE);
            sockErr = read(clientSocket, infoBuffer, INFO_BUFFER_SIZE);
            if (sockErr < 0) error("ERROR reading X file size");
            std::string seedString(infoBuffer);
            seed = std::stoi(seedString);
            std::cout << "received: seedString= " << seedString << std::endl;
            
            //receiving 'r'
            bzero(infoBuffer, INFO_BUFFER_SIZE);
            sockErr = read(clientSocket, infoBuffer, INFO_BUFFER_SIZE);
            if (sockErr < 0) error("ERROR reading X file size");
            std::string rString(infoBuffer);
            std::cout << "received: XfileSizeString= " << rString << std::endl;
            r = stoi(rString);

        }
	
}

const std::string Xgetter::XFILE_REQUEST = "Sending X request";
const std::string Xgetter::XFILE_REQUEST_ACK = "Sending X request ACK";
const std::string Xgetter::XFILE_SEED = "Sending X seed request";
const std::string Xgetter::XFILE_SEED_ACK = "Sending X seed request ACK";

unsigned long long Xgetter::getSeed() {
    return seed;
}

int Xgetter::error(const char *msg)
{
	perror(msg);
	abort();
}

int Xgetter::getR() {
    return r;
}

int Xgetter::min(int a, int b)
{
	return (a>b) ? b : a;
}

int Xgetter::saveXtoFile(std::string file)
{
	//clientXfile = file;
}

Xgetter::Xgetter(const Xgetter& orig) {
}

Xgetter::~Xgetter() {
}

