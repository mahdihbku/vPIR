/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   DBchunk.cpp
 * Author: mahdi
 * 
 * Created on December 30, 2017, 8:24 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <iostream>

#include "DBchunk.hpp"
#include "lib.hpp"
#include "SystemParameters.hpp"

using namespace std;

DBchunk::DBchunk(std::string t_DBfile, SystemParameters* t_systemParameters) : m_DBfile(t_DBfile), m_systemParameters(t_systemParameters)
{}

int DBchunk::initializeDBchunk(int t_order) {
    m_order = t_order;
    ifstream chunkStream(m_DBfile, ios::binary);
    if (!chunkStream.is_open()) error("Unable to open the database file");
    m_size = calculateChunkSize();
    chunkStream.close();
}

int DBchunk::loadDBchunk() {
    ifstream chunkStream(m_DBfile, ios::binary);
    size_t maxChunkSize = m_systemParameters->getMaxChunkSize();
    chunkStream.seekg(m_order*maxChunkSize);
    m_buffer = (char *) malloc (m_size * sizeof (char));
    chunkStream.read(m_buffer, m_size);
    //cout << "read from file, chunk size= " << m_size << " Starting from position: " << m_order*maxChunkSize << endl;
    chunkStream.close();
}

size_t DBchunk::calculateChunkSize() {
    size_t maxChunkSize = m_systemParameters->getMaxChunkSize();
    size_t DBfileSize = getFileSize(m_DBfile);
    size_t fileIndex = m_order*maxChunkSize;
    return (fileIndex+maxChunkSize <= DBfileSize) ? maxChunkSize : DBfileSize-fileIndex;
}

int DBchunk::freeDBchunk() {
    delete m_buffer;
}

DBchunk::DBchunk(const DBchunk& orig) {
}

DBchunk::~DBchunk() {
}

