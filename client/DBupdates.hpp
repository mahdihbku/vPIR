/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   DBupdates.hpp
 * Author: mahdi
 *
 * Created on February 27, 2018, 1:55 AM
 */

#ifndef DBUPDATES_HPP
#define DBUPDATES_HPP

#include <string>
#include <vector>

class DBupdates {
public:
    DBupdates();
    DBupdates(std::string t_DBupdatesfileName, int t_verbose);
    int setParameters(std::string t_DBupdatesfileName, int t_verbose);
    int getNumberOfFileInDBupdates();
    std::vector<std::string>* getDBupdatesfilesNamesList();
    std::vector<size_t>* getDBupdatesfilesSizesList();
    int writeDBupdatesTofile();
    int loadDBupdatesFromFile();
    DBupdates(const DBupdates& orig);
    virtual ~DBupdates();
private:
    std::string m_DBupdatesfileName;
    int m_verbose = 0;
    std::vector<std::string>* m_DBupdatesfilesNamesList = new std::vector<std::string>();
    std::vector<size_t>* m_DBupdatesfilesSizesList = new std::vector<size_t>();
};

#endif /* DBUPDATES_HPP */

