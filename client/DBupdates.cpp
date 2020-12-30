/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   DBupdates.cpp
 * Author: mahdi
 * 
 * Created on February 27, 2018, 1:55 AM
 */
#include "stdlib.h"
#include "stdio.h"
#include <string>
#include <vector>
#include <fstream>
#include <iostream>

#include "DBupdates.hpp"
#include "lib.hpp"

using namespace std;

DBupdates::DBupdates() {
    m_DBupdatesfilesNamesList = new vector<string>();
    m_DBupdatesfilesNamesList->clear();
    m_DBupdatesfilesSizesList = new vector<size_t>();
    m_DBupdatesfilesSizesList->clear();
}

DBupdates::DBupdates(std::string t_DBupdatesfileName, int t_verbose) {
    m_DBupdatesfileName = t_DBupdatesfileName;
    m_verbose = t_verbose;
    m_DBupdatesfilesNamesList = new vector<string>();
    m_DBupdatesfilesNamesList->clear();
    m_DBupdatesfilesSizesList = new vector<size_t>();
    m_DBupdatesfilesSizesList->clear();
}

int DBupdates::setParameters(std::string t_DBupdatesfileName, int t_verbose) {
    m_DBupdatesfileName = t_DBupdatesfileName;
    m_verbose = t_verbose;
}

int DBupdates::writeDBupdatesTofile() {
    if (m_verbose) cout << " writing db updates files names to " << m_DBupdatesfileName << " " << endl;
    ofstream dbFileStream(m_DBupdatesfileName);
    string errorMsg = "Unable to open the file: " + m_DBupdatesfileName;
    if (!dbFileStream.is_open()) error(errorMsg.c_str());
    for (size_t i=0; i<m_DBupdatesfilesNamesList->size(); i++)
        dbFileStream << m_DBupdatesfilesNamesList->at(i) << endl;
    dbFileStream.close();
    return 0;
}

int DBupdates::loadDBupdatesFromFile() {
    m_DBupdatesfilesNamesList->clear();
    m_DBupdatesfilesSizesList->clear();
    ifstream DBUpdatesFileStream(m_DBupdatesfileName,ifstream::in);
    string errorMsg = "Unable to open the file: " + m_DBupdatesfileName;
    //if (!DBUpdatesFileStream.is_open()) error(errorMsg.c_str());
    if (!DBUpdatesFileStream.is_open()) return 1;
    string line;
    while (getline(DBUpdatesFileStream, line)) {
        m_DBupdatesfilesNamesList->push_back(line);
        m_DBupdatesfilesSizesList->push_back(getFileSize(line));
    }
    DBUpdatesFileStream.close();
    return 0;
}

int DBupdates::getNumberOfFileInDBupdates() { return m_DBupdatesfilesNamesList->size(); }
vector<string>* DBupdates::getDBupdatesfilesNamesList() { return m_DBupdatesfilesNamesList; }
vector<size_t>* DBupdates::getDBupdatesfilesSizesList() { return m_DBupdatesfilesSizesList; }

DBupdates::DBupdates(const DBupdates& orig) {
}

DBupdates::~DBupdates() {
}

