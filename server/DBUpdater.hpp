/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   DBUpdater.hpp
 * Author: mahdi
 *
 * Created on February 14, 2018, 6:28 AM
 */

#ifndef DBUPDATER_HPP
#define DBUPDATER_HPP

#include <string>
#include <vector>

#include "DBManager.hpp"
#include "VersionUpdater.hpp"

class DBUpdater {
public:
    DBUpdater();
    DBUpdater(DBManager* t_DB, int t_maxNumberOfFilesInDBupdatesFile, int t_verbose);
    int setParameters(DBManager* t_DB, int t_maxNumberOfFilesInDBupdatesFile, int t_verbose);
    int changeDBname(std::string t_DBname);
    int changeFileName(int t_fileIndex, std::string t_newFileName);
    int changeFileName(std::string  t_oldFileName, std::string t_newFileName);
    int addFile(std::string t_fileName);
    int addNewFileToDBupdatesFile(std::string t_fileName);
    int addNewFileToDBupdatesFile(std::string t_fileName, size_t t_fileSize);
    int deleteFile(std::string t_fileName);
    int updateFile(std::string t_oldFileName, std::string t_newFileName);
    int updateFile(int t_fileIndex);
    int updateSeed(uint64_t t_newSeed);
    int getNumberOfFileInDBupdates();
    std::vector<std::string>* getDBupdatesfilesNamesList();
    std::vector<size_t>* getDBupdatesfilesSizesList();
    DBUpdater(const DBUpdater& orig);
    virtual ~DBUpdater();
private:
    DBManager* m_DB;
    VersionUpdater* m_versionUpdater;
    int m_verbose = 0;
    int m_maxNumberOfFilesInDBupdatesFile = 0;
    std::vector<std::string>* m_DBupdatesfilesNamesList = new std::vector<std::string>();
    std::vector<size_t>* m_DBupdatesfilesSizesList = new std::vector<size_t>();
    int *m_numberOfNewFiles;
    size_t *m_maxNewSize;
    size_t getNumberOfLinesInFile(std::string t_fileName);
    int clearFile(std::string t_fileName);
    int sse8Trans(char *inBuffer, char *outBuffer, size_t numberOfRows, size_t numberOfCols);
    int removeDummyFileFromFilesList();
    int loadFromUpdatesFile();
};

#endif /* DBUPDATER_HPP */

