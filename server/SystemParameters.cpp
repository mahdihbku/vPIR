/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   SystemParameters.cpp
 * Author: mahdi
 * 
 * Created on December 25, 2017, 7:35 PM
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>

#include "SystemParameters.hpp"
#include "lib.hpp"

using namespace std;

SystemParameters::SystemParameters(const size_t t_DBsize, const size_t t_N, const int t_RAMthreshold) {
    m_DBsize = t_DBsize;
    m_RAMthreshold = t_RAMthreshold;
    m_DBsouldBeSplitted = false;
    m_numberOfChunks = 1;
    size_t memorySize = getMemorySize();
    if (memorySize==0L)
    {
        cout << "Unable to get the system available memory" << endl;
    }
    else
    {
        m_totalAvailRAM = memorySize;
        size_t maxSize = m_totalAvailRAM*m_RAMthreshold/100;
        if (m_DBsize < maxSize)  //DB fits entirely to memory
        {
            m_maxChunkSize = m_DBsize;
            m_DBsouldBeSplitted = false;
        }
        else    //DB should be splitted
        {
            m_DBsouldBeSplitted = true;
            m_maxChunkSize = maxSize / t_N / 8 * 8 * t_N;
            //m_maxChunkSize = (maxSize%t_M==0) ? maxSize : (maxSize/t_M*t_M+t_M);
            m_numberOfChunks = (m_DBsize%m_maxChunkSize==0) ? m_DBsize/m_maxChunkSize : m_DBsize/m_maxChunkSize+1;
        }
    }
}

bool SystemParameters::DBshouldBeSplitted() { return m_DBsouldBeSplitted; }
size_t SystemParameters::getMaxChunkSize(){ return m_maxChunkSize; }
int SystemParameters::getNumberOfChunks(){ return m_numberOfChunks; }
size_t SystemParameters::getTotalAvailRAM() {return m_totalAvailRAM; }
int SystemParameters::getRAMthreshold() { return m_RAMthreshold; }

SystemParameters::SystemParameters(const SystemParameters& orig) {
}

SystemParameters::~SystemParameters() {
}

