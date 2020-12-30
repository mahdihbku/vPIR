/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   SystemParameters.hpp
 * Author: mahdi
 *
 * Created on December 25, 2017, 7:35 PM
 */

#ifndef SYSTEMPARAMETERS_HPP
#define SYSTEMPARAMETERS_HPP

class SystemParameters {
public:
    SystemParameters(const size_t t_DBsize, const size_t t_N, const int t_RAMthreshold);
    SystemParameters(const SystemParameters& orig);
    bool DBshouldBeSplitted();
    size_t getMaxChunkSize();
    size_t getMaxChunkLength();
    size_t getTotalAvailRAM();
    int getNumberOfChunks();
    int getRAMthreshold();
    virtual ~SystemParameters();
private:
    bool m_DBsouldBeSplitted=false;     //If true, the DB size is larger than m_chunkSize
    size_t m_totalAvailRAM=0;           //The available ram on the system
    int m_RAMthreshold=0;               //The percentage of maximum free ram that could be allocated
    size_t m_maxChunkSize=0;            //The maximum buffer size that could be allocated
    size_t m_DBsize=0;                  //The database file size
    int m_defaultRAMThreshold=20;       //The default value of the threshold
    int m_numberOfChunks=1;             //Number of chunks needed to load the entire DB
};

#endif /* SYSTEMPARAMETERS_HPP */

