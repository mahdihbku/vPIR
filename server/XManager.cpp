/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   XManager.cpp
 * Author: mahdi
 * 
 * Created on August 29, 2017, 10:18 AM
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
#include <iomanip>

#include "XManager.hpp"
#include "lib.hpp"

using namespace std;

const string XManager::XFILE_REQUEST = "Sending X request";
const string XManager::XFILE_REQUEST_ACK = "Sending X request ACK";
const string XManager::XFILE_SEED = "Sending X seed request";
const string XManager::XFILE_SEED_ACK = "Sending X seed request ACK";

#define HEX(x) setw(2) << setfill('0') << hex << (int)(x)

XManager::XManager(size_t t_M, int t_R, string t_XDBfile, int t_verbose) {
    cout << "in XManager() t_R=" << t_R << endl;
    m_M = t_M;
    m_R = t_R;
    m_seed = (char *) malloc (m_seedSize/8 * sizeof(char));
    getRandomSeed();
    m_verbose = t_verbose;
    m_XDB = new XDBGenerator(m_R, t_XDBfile, m_verbose);    
}

XManager::XManager(size_t t_M, int t_R, char* t_seed, string t_XDBfile, int t_verbose) {
    m_M = t_M;
    m_R = t_R;
    m_seed = (char *) malloc (m_seedSize/8 * sizeof(char));
    memcpy(m_seed, t_seed, m_seedSize/8);
    m_verbose = t_verbose;
    m_XDB = new XDBGenerator(m_R, t_XDBfile, m_verbose);    
}

int XManager::getRandomSeed() {
    ifstream randomStream("/dev/random", ios::binary);
    if (!randomStream.is_open()) {
        ifstream urandomStream("/dev/urandom", ios::binary);
        if (!urandomStream.is_open()) {
            error("Unable to open random seed sources");
        } else {
            urandomStream.read(m_seed, m_seedSize/8);
            urandomStream.close();
        }
    } else {
        randomStream.read(m_seed, m_seedSize/8);
        randomStream.close();
    }
    if (m_verbose) cout << "New random seed:";
    for (int i=0; i<m_seedSize/8; i++)
        cout << HEX((uint8_t)m_seed[i]);
    cout << endl;
    return 0;
}

int XManager::sendXSeedToClient(const int t_socket) {
    int sockErr;
    char *infoBuffer;
    infoBuffer = (char *) malloc (INFO_BUFFER_SIZE * sizeof (char));

    //sending "sending X seed request"
    bzero(infoBuffer, INFO_BUFFER_SIZE);
    strcpy (infoBuffer, XFILE_SEED.c_str());
    sockErr = write(t_socket, infoBuffer, INFO_BUFFER_SIZE);
    if (sockErr < 0) error("ERROR sending XFILE_SEED");
    
    //receiving "sending X request ACK"
    bzero(infoBuffer, INFO_BUFFER_SIZE);
    sockErr = read(t_socket, infoBuffer, INFO_BUFFER_SIZE);
    if (sockErr < 0) error("ERROR while reading sending X seed request ACK");
    string reply(infoBuffer);
    if (m_verbose)
        if (reply == XFILE_SEED_ACK)
            cout << "received: " << XFILE_SEED_ACK << endl;
        else
            cout << "ERROR: " << XFILE_SEED_ACK << " hasn't been received" << endl;
    
    //sending the seed to the client
    bzero(infoBuffer, INFO_BUFFER_SIZE);
    memcpy(infoBuffer, m_seed, m_seedSize/8);
    sockErr = write(t_socket, infoBuffer, INFO_BUFFER_SIZE);
    if (sockErr < 0) error("ERROR sending X seed");
    
    //sending 'r'
    bzero(infoBuffer, INFO_BUFFER_SIZE);
    string rString = to_string(m_R);
    strcpy (infoBuffer, rString.c_str());
    sockErr = write(t_socket, infoBuffer, INFO_BUFFER_SIZE);
    if (sockErr < 0) error("ERROR sending X file size");
    
    if (m_verbose) cout << "seed sent to the client :" << m_seed << endl;
    if (m_verbose) cout << "r sent to the client :" << m_R << endl;
}

int XManager::setSeed(char* newSeed) {
    memcpy(m_seed, newSeed, m_seedSize/8);
    return 0;
}

int XManager::setR(const int t_R) {
    m_R = t_R;
    return 0;
}

int XManager::getR() {
    return m_R;
}

char * XManager::getSeed() {
    return m_seed;
}

XDBGenerator* XManager::getXDB() {
    return m_XDB;
}

XManager::XManager(const XManager& orig) {
}

XManager::~XManager() {
}

