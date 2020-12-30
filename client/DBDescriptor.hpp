/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   DBDescriptor.hpp
 * Author: mahdi
 *
 * Created on August 29, 2017, 11:12 AM
 */

#ifndef DBDESCRIPTOR_HPP
#define DBDESCRIPTOR_HPP

#include "DBupdates.hpp"
#include "XPIRDescriptor.hpp"

#include <string>
#include <vector>

class DBDescriptor {
public:
    DBDescriptor();
    DBDescriptor(std::string t_DBName, size_t t_N, size_t t_M, int t_R, std::string t_seed,
            int t_verbose, std::string t_version, std::string t_serverIP, int t_serverPort, XPIRDescriptor* t_XPIRDescriptor);
    DBDescriptor(const DBDescriptor& orig);
    int addFileNameToList(const std::string t_Name);
    int addFileSizeToList(const size_t t_size);
    int saveXtoFile(const std::string t_file);
    int saveDBinfoToFile(const std::string t_dbInfoFileName);
    int loadDBinfoFromFile(const std::string t_dbInfoFileName);
    int displayDBcatalog();
    virtual ~DBDescriptor();
    
    //getters and setters
    int setM(const size_t t_M);
    int setXDBfileName(const std::string t_XDBfile);
    int setXfileName(const std::string t_Xfile);
    int setDBupdatesFileName(const std::string t_DBupdatesFile);
    int setdbName(const std::string t_name);
    int setDBupdates(DBupdates* t_updates);
    size_t getN();
    size_t getM();
    int getR();
    std::vector<std::string> *getFilesNames();
    std::vector<size_t> *getFilesSizes();
    std::string getdbName();
    std::string getServerIP();
    int getServerPort();
    std::string getSeed();
    std::string getXfileName();
    std::string getXDBfileName();
    std::string getDBupdatesFileName();
    std::string getDBversion();
    DBupdates* getDBupdates();
    XPIRDescriptor* getXPIRDescriptor();
private:
    std::string m_serverIP;
    int m_serverPort;
    size_t m_N, m_M, m_R;
    std::string m_seed;
    std::string m_dbName;
    std::string m_XfileName;
    std::string m_XDBfileName;
    std::string m_DBupdatesFileName;
    std::string m_version;
    std::vector<std::string> m_dbFilesNamesList;
    std::vector<size_t> m_dbFilesSizesList;
    DBupdates* m_updates;
    XPIRDescriptor* m_XPIRDescriptor;
    int m_verbose = 0;
};

#endif /* DBDESCRIPTOR_HPP */

