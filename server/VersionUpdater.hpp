/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   VersionUpdater.hpp
 * Author: mahdi
 *
 * Created on February 14, 2018, 6:29 AM
 */

//VERSION format: v1.v2.v3

#ifndef VERSIONUPDATER_HPP
#define VERSIONUPDATER_HPP

#include <string>
#include <vector>

#include "DBManager.hpp"

class VersionUpdater {
public:
    VersionUpdater(DBManager* t_DB);
    int performSmallUpdate();
    int performMinorUpdate();
    int performMajorUpdate();
    int performMajorUpdateWithoutDBfile();
    int loadFromUpdatesFile(std::vector<std::string>* DBupdatesfilesNamesList, std::vector<std::string>* DBupdatesfilesSizesList,
        int *numberOfNewFiles, size_t *maxNewSize);
    int removeDummyFileFromFilesList();
    VersionUpdater(const VersionUpdater& orig);
    virtual ~VersionUpdater();
private:
    DBManager* m_DB;
    std::string m_DBversion="";
    int m_v1=0, m_v2=0, m_v3=0;
    int splitDBversion();
    int updateLocalVersion();
    int updateDBversion();
};

#endif /* VERSIONUPDATER_HPP */

