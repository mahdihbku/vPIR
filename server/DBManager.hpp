/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   DBManager.hpp
 * Author: mahdi
 *
 * Created on August 28, 2017, 2:41 PM
 */

#ifndef DBMANAGER_HPP
#define DBMANAGER_HPP

#include <vector>

#include "XManager.hpp"
#include "XPIRManager.hpp"
#include "SystemParameters.hpp"

class XDBGenerator;
class DBUpdater;

class DBManager {
public:
    DBManager(const int t_verbose);
    DBManager(const DBManager& orig);
    int setDBParameters(const std::string t_directory, const std::string t_DBFileName, const std::string t_DBFileInfo,
        const std::string t_XDBfileName, const std::string t_DBupdatesFileName, const std::string t_DBName, const size_t t_N);
    int setDBParameters(const std::string t_directory, const std::string t_DBFileName, const std::string t_DBFileInfo,
        const std::string t_XDBfileName, const std::string t_DBupdatesFileName, const std::string t_DBName, const int memUsageThreshold);
    int setXPIRParameters(int t_XPIRport, std::string t_XPIRdir, std::string t_XPIRip);
    int generateNewDBFiles(const int t_R, const std::string t_XDBfileName);
    int loadFromDBinfo(const std::string t_directory, const int memUsageThreshold);
    int processDBDirectory();
    virtual ~DBManager();
    
    //Class setters and getters
    int setXManager(XManager* t_X);
    int setDBupdater(DBUpdater* t_DBupdater);
    int setSystemParm(SystemParameters* t_systemParm);
    int setDBVersion(const std::string t_version);
    int setDBname(const std::string t_newDBname);
    int setM(const size_t t_M);
    int setN(const size_t t_N);
    size_t getN();
    size_t getM();
    char *getDBbuffer();
    size_t getDBbufferSize();
    std::string getDBFileName();
    std::string getDBFileInfo();
    std::string getDBDirectory();
    std::string getDBName();
    std::string getDBVersion();
    std::string getDBupdatesFileName();
    XManager* getX();
    XPIRManager* getXPIR();
    SystemParameters* getSystemParm();
    DBUpdater* getDBupdater();
    std::vector<std::string>* getDBfilesNamesList();
    std::vector<std::size_t>* getDBfilesSizesList();
    
    //Public methods
    int loadEntireDBtoMemory();
    int allocateChunkMemory();
    int loadDBChunkToMemory(int chunkOrder);
    int loadDBtoMemory();
    int generateDBFile();
    int generateDBInfo(const int t_R, char *t_Xseed, char *t_Yseed, const std::string t_XDBfileName);
    int generateDBInfo();
private:
    //members
    size_t m_N, m_M;            //N:file size, M:number of files
    char *m_buffer;             //buffer is one block of (chunkLength X m/8)
    size_t m_bufferSize;        //db buffer (db chunk) size in number of bytes
    std::string m_DBfileName, m_DBfileInfo, m_DBdirectory, m_DBname, m_XDBfileName, m_DBupdatesFileName, m_version;
    std::vector<std::string> m_dbFilesNamesList;
    std::vector<size_t> m_dbFilesSizesList;
    XManager* m_X;
    SystemParameters* m_systemParm;
    DBUpdater* m_DBupdater;
    int m_verbose = 0;
    std::string dummyFileName = "dummyFile.data";
    
    //XPIR parameters
    int m_XPIRport = 12346;
    std::string m_XPIRdir = "";
    std::string m_XPIRip = "";
    XPIRManager* m_XPIR;
    
    //Private methods
    int clearFile(const std::string t_fileName);
    int addFileNameToDBInfoFile(const std::string t_fileName);
};

#endif /* DBMANAGER_HPP */

