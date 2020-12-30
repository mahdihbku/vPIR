/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   DBManager.cpp
 * Author: mahdi
 * 
 * Created on August 28, 2017, 2:41 PM
 */
#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <dirent.h>
#include <experimental/filesystem>
#include <assert.h>
#include <emmintrin.h>
#include <iomanip>
#include <bitset>
#include <sys/mman.h>
#include <sys/stat.h>
#include <chrono>

#include "DBManager.hpp"
#include "lib.hpp"
#include "ProgressBar.hpp"
#include "RandomDBgenerator.hpp"

using namespace std;

DBManager::DBManager(const DBManager& orig) {
}

DBManager::DBManager(const int t_verbose) : m_DBdirectory(""), m_DBfileName(""), m_DBfileInfo(""), m_DBupdatesFileName(""),
        m_version("0.0.0"), m_DBname(""), m_N(0), m_M(0), m_verbose(t_verbose) {}

int DBManager::setDBParameters(string t_directory, string t_DBFileName, string t_DBFileInfo, 
        string t_XDBfileName, string t_DBupdatesFileName, string t_DBName, size_t t_N) {
    m_DBdirectory   = t_directory;
    m_DBfileName    = t_DBFileName;
    m_DBfileInfo    = t_DBFileInfo;
    m_XDBfileName   = t_XDBfileName;
    m_DBupdatesFileName = t_DBupdatesFileName;
    m_DBname        = t_DBName;
    m_N             = t_N;
    return 0;
}

int DBManager::setDBParameters(string t_directory, string t_DBFileName, string t_DBFileInfo, 
        string t_XDBfileName, string t_DBupdatesFileName, string t_DBName, int memUsageThreshold) {
    m_DBdirectory   = t_directory;
    m_DBfileName    = t_DBFileName;
    m_DBfileInfo    = t_DBFileInfo;
    m_XDBfileName   = t_XDBfileName;
    m_DBupdatesFileName = t_DBupdatesFileName;
    if (m_verbose) cout << "m_DBupdatesFileName= " << m_DBupdatesFileName << endl;
    m_DBname        = t_DBName;
    processDBDirectory(); //To get N and M and load files names 
    if (m_verbose) cout << "In directory " << m_DBdirectory << "(after processing), number of files M=" << m_M << ", and max file size N= " << m_N << endl;
    m_systemParm = new SystemParameters(m_N*m_M, m_N, memUsageThreshold);
    
    if (m_verbose) cout << "System parameters:" << endl;
    if (m_verbose) cout << "DBsouldBeSplitted=" << m_systemParm->DBshouldBeSplitted() << " DBsize=" << m_N*m_M/1024/1024/1024 << "GB numberOfChunks=" << m_systemParm->getNumberOfChunks() 
            << " maxChunkSize=" << m_systemParm->getMaxChunkSize()/1024/1024/1024 << "GB totalAvailRAM=" << m_systemParm->getTotalAvailRAM()/1024/1024/1024 << "GB RAMthreshold=" << m_systemParm->getRAMthreshold() << "%" << endl;
    
    return 0;
}

int DBManager::setXPIRParameters(int t_XPIRport, string t_XPIRdir, string t_XPIRip) {
    m_XPIRport = t_XPIRport;
    m_XPIRdir = t_XPIRdir;
    m_XPIRip = t_XPIRip;
    return 0;
}

int DBManager::processDBDirectory() {
    if (m_verbose) cout << "Processing directory " << m_DBdirectory << "..." << endl;
    size_t numberOfFiles=0, fileSize=0, maxSize=0;
    DIR *directory;
    struct dirent *dir;
    directory = opendir(m_DBdirectory.c_str());
    string errorMsg = "Unable to open the directory: " + m_DBdirectory;
    if (!directory) error(errorMsg.c_str());
    while ((dir = readdir(directory)) != nullptr)
    {
        if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0 && 
            strcmp((m_DBdirectory+dir->d_name).c_str(), m_DBfileName.c_str()) != 0 &&
            strcmp((m_DBdirectory+dir->d_name).c_str(), m_DBfileInfo.c_str()) != 0 &&
            strcmp((m_DBdirectory+dir->d_name).c_str(), m_XDBfileName.c_str()) != 0 &&
            strcmp((m_DBdirectory+dir->d_name).c_str(), m_DBupdatesFileName.c_str()) != 0 && 
            strcmp((m_DBdirectory+dir->d_name).c_str(), (m_DBdirectory+dummyFileName).c_str()) != 0 &&
            dir->d_type != DT_DIR)
        {
            //m_dbFilesNamesList.push_back(m_DBdirectory+dir->d_name);
            m_dbFilesNamesList.push_back(dir->d_name);
            fileSize = getFileSize(m_DBdirectory+dir->d_name);
            m_dbFilesSizesList.push_back(fileSize);
            numberOfFiles++;
            if (fileSize > maxSize) maxSize = fileSize;
        }
    }
    
    if (m_verbose) cout << "Current numberOfFiles=" << numberOfFiles << endl;
    //Making N and M multiple of 8
    m_N = (maxSize%8==0) ? maxSize : (maxSize/8*8+8);   //To add padding to the files less than 8i
    if (numberOfFiles%8 != 0) {
        RandomDBgenerator randGen;
        size_t dummyFileSize = m_N;
        randGen.generateRandomFile(m_DBdirectory+dummyFileName, dummyFileSize);
        while (numberOfFiles%8 != 0) {
            m_dbFilesNamesList.push_back(dummyFileName);
            m_dbFilesSizesList.push_back(dummyFileSize);
            //addFileNameToDBInfoFile(dummyFileName);
            numberOfFiles++;
        }
    }
    m_M = numberOfFiles;
    return 0;
}

int DBManager::loadFromDBinfo(const string t_directory, int memUsageThreshold) {
    m_DBdirectory = t_directory;
    m_DBfileName = t_directory + "dbFile.db";
    m_DBfileInfo = t_directory + "dbInfo.db";
    //string XDBFileName = Directory + "XDBbytes.file";
    ifstream DBInfoFileStream(m_DBfileInfo,ifstream::in);
    string errorMsg = "Unable to open the file: " + m_DBfileInfo;
    if (!DBInfoFileStream.is_open()) error(errorMsg.c_str());
    string line;
    getline(DBInfoFileStream, line);
    m_version = line.substr(8,line.length());
    getline(DBInfoFileStream, m_DBname);
    getline(DBInfoFileStream, line);
    string Mstring = line.substr(8,line.length());
    m_M = stoi(Mstring);
    getline(DBInfoFileStream, line);
    string Nstring = line.substr(8,line.length());
    m_N = stoi(Nstring);
    getline(DBInfoFileStream, line);
    string Rstring = line.substr(8,line.length());
    int r = stoi(Rstring);
    getline(DBInfoFileStream, line);
    string XseedString = line.substr(8,line.length());
    
    getline(DBInfoFileStream, line);
    m_XPIRip = line.substr(8,line.length());
    getline(DBInfoFileStream, line);
    m_XPIRport = stoi(line.substr(11,line.length()));
    getline(DBInfoFileStream, line);
    m_XPIRdir = line.substr(8,line.length());
    
    getline(DBInfoFileStream, line);
    m_XDBfileName = line.substr(8,line.length());
    getline(DBInfoFileStream, line);
    m_DBupdatesFileName = line.substr(8,line.length());
    getline(DBInfoFileStream, line);    //bypassing the "Catalog:" line
    while (getline(DBInfoFileStream, line)) {
        m_dbFilesNamesList.push_back(line);
        getline(DBInfoFileStream, line);
        stringstream sstream(line);
        size_t fileSize;
        sstream >> fileSize;
        m_dbFilesSizesList.push_back(fileSize);
    }
    DBInfoFileStream.close();

    char *t_Xseed = (char *) malloc(512/8 * sizeof(char));
    cout << "6546546546---" << endl;
    cout << "XseedString=" << XseedString << endl;
    cout << "t_Xseed=" << t_Xseed << endl;
    hex2bin(XseedString.c_str(), t_Xseed, 512/8);
    cout << "6546546546" << endl;
    m_X = new XManager(m_M, r, t_Xseed, m_XDBfileName, m_verbose);
    cout << "6546546546" << endl;
    
    m_XPIR = new XPIRManager(m_XPIRdir, m_XPIRport, (size_t)r, m_verbose);
    
    m_systemParm = new SystemParameters(m_N*m_M, m_N, memUsageThreshold);
    cout << "6546546546" << endl;
    
    if (m_verbose) cout << "System parameters:" << endl;
    if (m_verbose) cout << "DBsouldBeSplitted=" << m_systemParm->DBshouldBeSplitted() << " DBsize=" << m_N*m_M/1024/1024/1024 << "GB numberOfChunks=" << m_systemParm->getNumberOfChunks() 
            << " maxChunkSize=" << m_systemParm->getMaxChunkSize()/1024/1024/1024 << "GB totalAvailRAM=" << m_systemParm->getTotalAvailRAM()/1024/1024/1024 << "GB RAMthreshold=" << m_systemParm->getRAMthreshold() << "%" << endl;
    
    return 0;
}

#define HEX(x) setw(2) << setfill('0') << hex << (int)(x)

int DBManager::generateNewDBFiles(const int t_R, const string t_XDBfileName) {
    cout << "in generateNewDBFiles t_R=" << t_R << endl;
    m_XDBfileName = t_XDBfileName;
    if (m_verbose) cout << "Creating db(" << m_DBname << ") from dir:" << m_DBdirectory << " to file: " << m_DBfileName << endl;
    
    m_X = new XManager(m_M, t_R, m_XDBfileName, m_verbose);
    
    generateDBInfo();
    
    //TODO UNCOMMENT
    generateDBFile();
    
    loadDBtoMemory();   //TODO //TOCHECK    //checked! correct
    m_X->getXDB()->createXDBFileFromSeed(m_X->getSeed(), this);
}

int DBManager::generateDBInfo() {
    if (m_verbose) cout << "Writing to " << m_DBfileInfo << "..." << endl;
    ofstream DBInfoFileStream(m_DBfileInfo);
    string errorMsg = "Unable to open the file: " + m_DBfileInfo;
    if (!DBInfoFileStream.is_open()) error(errorMsg.c_str());
    DBInfoFileStream << "ver   = " << m_version << endl;
    DBInfoFileStream << m_DBname << endl;
    DBInfoFileStream << "M     = " << m_M << endl;
    DBInfoFileStream << "N     = " << m_N << endl;
    DBInfoFileStream << "R     = " << getX()->getR() << endl;
    string XseedString = bin2hex(getX()->getSeed(), 512/8);
    DBInfoFileStream << "Seed  = " << XseedString << endl;
    DBInfoFileStream << "XPIRip: " << m_XPIRip << endl;
    DBInfoFileStream << "XPIRport = " << m_XPIRport << endl;
    DBInfoFileStream << "XPIRdir:" << m_XPIRdir << endl;
    DBInfoFileStream << "XDB   : " << getX()->getXDB()->getOutputFile() << endl;
    DBInfoFileStream << "Update: " << m_DBupdatesFileName << endl;
    DBInfoFileStream << "Catalog:" << endl;
    for (size_t i=0; i<m_dbFilesNamesList.size(); i++) {
        DBInfoFileStream << m_dbFilesNamesList.at(i) << endl;
        DBInfoFileStream << m_dbFilesSizesList.at(i) << endl;
    }
    DBInfoFileStream.close();
    return 0;
}

int DBManager::addFileNameToDBInfoFile(const string fileName) {
    ofstream dbFileStream(m_DBfileInfo, ios::app);
    string errorMsg = "Unable to open the file: " + m_DBfileInfo;
    if (!dbFileStream.is_open()) error(errorMsg.c_str());
    dbFileStream << fileName << endl;
    dbFileStream << getFileSize(fileName) << endl;
    dbFileStream.close();
}

int DBManager::generateDBFile() {
    if (m_verbose) cout << "Writing from files to DBfile..." << endl;
    string errorMsg;
    clearFile(m_DBfileName);
    ofstream dbFileStream(m_DBfileName, ios::binary | ios::app);
    char *fileBuff = (char *) malloc (m_N * sizeof (char));
    for (size_t k=0; k<m_dbFilesNamesList.size(); k++) {
        ifstream fileStream(m_DBdirectory+m_dbFilesNamesList[k], ios::binary);
        if (!fileStream.is_open()) {
            errorMsg = "Unable to open the file: " + m_dbFilesNamesList[k];
            error(errorMsg.c_str());
        }
        //Reading from one file
        fileStream.read(fileBuff, m_dbFilesSizesList[k]);
        for (size_t l=m_dbFilesSizesList[k]; l<m_N; l++)      //adding padding
            fileBuff[l]='\0';
        fileStream.close();
        //writing to DBfile
        dbFileStream.write(fileBuff,m_N);
        //if (m_verbose) cout << "written " << m_N << "B to " << m_DBfileName <<" k=" << k << "..." << endl;
    }
    return 0;
}

int DBManager::clearFile(const string fileName) {
    ofstream dbFileStream(fileName, ios::out | ios::trunc);
    string errorMsg = "Unable to open the directory: " + fileName;
    if (!dbFileStream) error(errorMsg.c_str());
    dbFileStream.close();
    return 0;
}

int DBManager::loadDBtoMemory() {
    
    if (m_systemParm->DBshouldBeSplitted()) {
        if (m_verbose) cout << "the DB " << m_DBfileName << " has been splitted into " << m_systemParm->getNumberOfChunks() << " chunks" << endl;
        allocateChunkMemory();
        //leave this line
        loadDBChunkToMemory(0);
    }
    else {
        if (m_verbose) cout << "Loading the entire DB to memory..." << endl;
        loadEntireDBtoMemory();
        cout << "the DB " << m_DBfileName << " has been loaded entirely to the memory. (" << m_bufferSize/1024/1024/1024 << "GB)" << endl;
    }
}

int DBManager::loadEntireDBtoMemory() {
    assert (!m_systemParm->DBshouldBeSplitted());
    ifstream dbFileStream(m_DBfileName, ios::binary);
    if (!dbFileStream.is_open()) error("Unable to open the database file");
    m_bufferSize = m_N*m_M;
    m_buffer = (char *) malloc (m_bufferSize * sizeof (char));
    dbFileStream.read(m_buffer, m_bufferSize);
    dbFileStream.close();
}

int DBManager::allocateChunkMemory() {
    size_t maxBufferSize =  m_systemParm->getMaxChunkSize();
    m_buffer = (char *) malloc (maxBufferSize * sizeof (char));
}

int DBManager::loadDBChunkToMemory(int chunkOrder) {
    assert (m_systemParm->DBshouldBeSplitted());
    if (chunkOrder >= m_systemParm->getNumberOfChunks())
        chunkOrder = 0;
    cout << "Loading DB chunk number " << chunkOrder+1 << "/" << m_systemParm->getNumberOfChunks() << " to memory..." << endl;
    ifstream dbFileStream(m_DBfileName, ios::binary);
    if (!dbFileStream.is_open()) error("Unable to open the database file");
    if (chunkOrder < m_systemParm->getNumberOfChunks()-1)
        m_bufferSize =  m_systemParm->getMaxChunkSize();
    else {
        if (chunkOrder == m_systemParm->getNumberOfChunks()-1) {
            m_bufferSize = m_N*m_M % m_systemParm->getMaxChunkSize();
        }
        else {
            error("trying to access non existing chunk");
        }
    }
    dbFileStream.seekg(chunkOrder*m_systemParm->getMaxChunkSize());
    dbFileStream.read(m_buffer, m_bufferSize);
    dbFileStream.close();
    if (m_verbose) cout << "Chunk number " << chunkOrder << " has been loaded successfully. (" << m_bufferSize << " Bytes)" << endl;
}

string DBManager::getDBFileName() { return m_DBfileName;}
string DBManager::getDBFileInfo() { return m_DBfileInfo;}
string DBManager::getDBDirectory() { return m_DBdirectory;}
string DBManager::getDBName() { return m_DBname;}
string DBManager::getDBVersion() { return m_version;}
string DBManager::getDBupdatesFileName() { return m_DBupdatesFileName;}
XManager* DBManager::getX() { return m_X;}
XPIRManager* DBManager::getXPIR() { return m_XPIR;}
SystemParameters* DBManager::getSystemParm() { return m_systemParm;}
DBUpdater* DBManager::getDBupdater() { return m_DBupdater;}
size_t DBManager::getN() { return m_N;}
size_t DBManager::getM(){ return m_M;}
char* DBManager::getDBbuffer() { return m_buffer;}
size_t DBManager::getDBbufferSize() { return m_bufferSize;}
vector<string>* DBManager::getDBfilesNamesList() { return &m_dbFilesNamesList;}
vector<size_t>* DBManager::getDBfilesSizesList() { return &m_dbFilesSizesList;}

int DBManager::setXManager(XManager* t_X) {
    m_X = t_X;
    return 0;
}
int DBManager::setDBupdater(DBUpdater* t_DBupdater) {
    m_DBupdater = t_DBupdater;
    return 0;
}
int DBManager::setSystemParm(SystemParameters* t_systemParm) {
    m_systemParm = t_systemParm;
    return 0;
}
int DBManager::setDBVersion(const string t_version) {
    m_version = t_version;
    return 0;
}
int DBManager::setDBname(const string t_newDBname) {
    m_DBname = t_newDBname;
    return 0;
}
int DBManager::setM(const size_t t_M) {
    m_M = t_M;
    return 0;
}
int DBManager::setN(const size_t t_N) {
    m_N = t_N;
    return 0;
}

DBManager::~DBManager() {
}
