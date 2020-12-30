/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   VersionUpdater.cpp
 * Author: mahdi
 * 
 * Created on February 14, 2018, 6:29 AM
 */

//VERSION format: v1.v2.v3

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>

#include "VersionUpdater.hpp"
#include "lib.hpp"

using namespace std;

VersionUpdater::VersionUpdater(DBManager* t_DB) {
    m_DB = t_DB;
    m_DBversion = t_DB->getDBVersion();
    splitDBversion();
}

int VersionUpdater::performSmallUpdate() {
    m_v3++;
    updateLocalVersion();
    updateDBversion();
    m_DB->generateDBInfo();
    return 0;
}

int VersionUpdater::performMinorUpdate() {
    m_v2++;
    updateLocalVersion();
    updateDBversion();
    m_DB->generateDBInfo();
    return 0;
}

int VersionUpdater::performMajorUpdate() {
    //Update db version
    m_v1++;
    updateLocalVersion();
    updateDBversion();
    //rewriting DBfileInfo
    m_DB->generateDBInfo();
    //recreateDBfile
    m_DB->generateDBFile();
    //recreateXDBfile
    m_DB->loadDBtoMemory();
    char* seed = m_DB->getX()->getSeed();
    m_DB->getX()->getXDB()->createXDBFileFromSeed(seed, m_DB);
    return 0;
}

int VersionUpdater::performMajorUpdateWithoutDBfile() {
    //Update db version
    m_v1++;
    updateLocalVersion();
    updateDBversion();
    //rewriting DBfileInfo
    m_DB->generateDBInfo();
    //recreateDBfile
    //m_DB->writeFilesToPIRDBPerBits();
    //recreateXDBfile
    m_DB->loadDBtoMemory();
    char* seed = m_DB->getX()->getSeed();
    m_DB->getX()->getXDB()->createXDBFileFromSeed(seed, m_DB);
    return 0;
}

int VersionUpdater::splitDBversion() {
    int pos1 = m_DBversion.find(".");
    string v1string = m_DBversion.substr(0, pos1);
    int pos2  = m_DBversion.find(".",pos1+1);
    string v2string = m_DBversion.substr(pos1+1, pos2);
    string v3string = m_DBversion.substr(pos2+1, m_DBversion.length());
    m_v1=stoi(v1string);
    m_v2=stoi(v2string);
    m_v3=stoi(v3string);
    //cout << "v1="<<m_v1<<" v2="<<m_v2<<" v3="<<m_v3<<endl;
    return 0;
}

int VersionUpdater::updateLocalVersion() {
    m_DBversion = to_string(m_v1)+"."+to_string(m_v2)+"."+to_string(m_v3);
    return 0;
}

int VersionUpdater::updateDBversion() {
    m_DB->setDBVersion(m_DBversion);
    //TODO write it to dbInfo file
    //....
    return 0;
}

VersionUpdater::VersionUpdater(const VersionUpdater& orig) {
}

VersionUpdater::~VersionUpdater() {
}

