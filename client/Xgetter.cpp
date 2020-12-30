/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Xgetter.cpp
 * Author: mahdi
 * 
 * Created on August 29, 2017, 10:52 AM
 */

#include <cmath>
#include <random> 
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
#include <chrono>

#include "Xgetter.hpp"
#include "DBDescriptor.hpp"

using namespace std;

const string Xgetter::XFILE_REQUEST = "Sending X request";
const string Xgetter::XFILE_REQUEST_ACK = "Sending X request ACK";
const string Xgetter::XFILE_SEED = "Sending X seed request";
const string Xgetter::XFILE_SEED_ACK = "Sending X seed request ACK";

Xgetter::Xgetter(int socket, DBDescriptor* newDB) {
    clientSocket = socket;
    db = newDB;
}

int Xgetter::getSeedFromServer() {
    int sockErr;
    char *infoBuffer;
    infoBuffer = (char *) malloc (INFO_BUFFER_SIZE * sizeof (char));

    //receiving "sending X request" or "Sending X seed request"
    bzero(infoBuffer, INFO_BUFFER_SIZE);
    sockErr = read(clientSocket, infoBuffer, INFO_BUFFER_SIZE);
    if (sockErr < 0) error("ERROR while reading sending X file request");
    string reply(infoBuffer);
    if (reply == XFILE_SEED)
        cout << "received: " << reply << endl;
    else
        cout << "ERROR: " << XFILE_SEED << " hasn't been received" << endl;
    
    //sending "sending X seed request ACK"
    bzero(infoBuffer, INFO_BUFFER_SIZE);
    strcpy (infoBuffer, XFILE_SEED_ACK.c_str());
    sockErr = write(clientSocket, infoBuffer, INFO_BUFFER_SIZE);
    if (sockErr < 0) error("ERROR sending sending X file request ACK");

    //receiving the seed
    bzero(infoBuffer, INFO_BUFFER_SIZE);
    sockErr = read(clientSocket, infoBuffer, INFO_BUFFER_SIZE);
    if (sockErr < 0) error("ERROR reading X file size");
    string seedString(infoBuffer);
    seed = stoull(seedString);
    cout << "received: seedString= " << seedString << endl;

    //receiving 'r'
    bzero(infoBuffer, INFO_BUFFER_SIZE);
    sockErr = read(clientSocket, infoBuffer, INFO_BUFFER_SIZE);
    if (sockErr < 0) error("ERROR reading X file size");
    string rString(infoBuffer);
    cout << "received: XfileSizeString= " << rString << endl;
    r = stoi(rString);
    return 0;
}

int Xgetter::saveXtoFile(string file){
    mt19937_64 prng(seed);
    uniform_int_distribution<unsigned long long> udist(0,pow(2,1));
    ofstream XFileStream(file.c_str());
    if (XFileStream.is_open())
    {
        for (int i=0; i<r; i++)
        {
            for (int j=0; j<db->getM(); j++)
            {
                XFileStream << udist(prng) << ' ';
            }
            XFileStream << endl;
        }
    }
}

int Xgetter::error(const char *msg) {
    perror(msg);
    abort();
}

unsigned long long Xgetter::getSeed() {
    return seed;
}

int Xgetter::getR() {
    return r;
}

Xgetter::Xgetter(const Xgetter& orig) {
}

Xgetter::~Xgetter() {
}

