/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   DBchunk.hpp
 * Author: mahdi
 *
 * Created on December 30, 2017, 8:24 PM
 */

#include <string>

#include "SystemParameters.hpp"

#ifndef DBCHUNK_HPP
#define DBCHUNK_HPP

class DBchunk {
public:
    DBchunk(std::string t_DBfile, SystemParameters* t_systemParameters);
    int initializeDBchunk(int t_order);
    int loadDBchunk();
    int freeDBchunk();
    size_t calculateChunkSize();
    DBchunk(const DBchunk& orig);
    virtual ~DBchunk();
private:
    int m_order=0;
    char *m_buffer;
    size_t m_size;
    std::string m_DBfile;
    SystemParameters* m_systemParameters;
};

#endif /* DBCHUNK_HPP */

