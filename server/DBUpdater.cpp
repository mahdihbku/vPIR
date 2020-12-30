/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   DBUpdater.cpp
 * Author: mahdi
 * 
 * Created on February 14, 2018, 6:28 AM
 */

#include "stdlib.h"
#include "stdio.h"
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>

#include "DBUpdater.hpp"
#include "VersionUpdater.hpp"
#include "lib.hpp"

using namespace std;

DBUpdater::DBUpdater() {
    m_maxNumberOfFilesInDBupdatesFile = 0;
    m_verbose = 0;
    m_DBupdatesfilesNamesList = new vector<string>();
    m_DBupdatesfilesSizesList = new vector<size_t>();
    m_numberOfNewFiles = new int(0);
    m_maxNewSize = new size_t(0);
}

DBUpdater::DBUpdater(DBManager* t_DB, int t_maxNumberOfFilesInDBupdatesFile, int t_verbose) {
    m_DB = t_DB;
    m_maxNumberOfFilesInDBupdatesFile = t_maxNumberOfFilesInDBupdatesFile;
    m_verbose = t_verbose;
    m_versionUpdater = new VersionUpdater(m_DB);
    m_DBupdatesfilesNamesList = new vector<string>();
    m_DBupdatesfilesSizesList = new vector<size_t>();
    m_numberOfNewFiles = new int(0);
    m_maxNewSize = new size_t(0);
    loadFromUpdatesFile();
}

int DBUpdater::setParameters(DBManager* t_DB, int t_maxNumberOfFilesInDBupdatesFile, int t_verbose) {
    m_DB = t_DB;
    m_maxNumberOfFilesInDBupdatesFile = t_maxNumberOfFilesInDBupdatesFile;
    m_verbose = t_verbose;
    m_versionUpdater = new VersionUpdater(m_DB);
    m_DBupdatesfilesNamesList = new vector<string>();
    m_DBupdatesfilesSizesList = new vector<size_t>();
    m_numberOfNewFiles = new int(0);
    m_maxNewSize = new size_t(0);
    loadFromUpdatesFile();
    return 0;
}

int DBUpdater::getNumberOfFileInDBupdates() {
    return m_DBupdatesfilesNamesList->size();
}

int DBUpdater::changeDBname(string t_DBname) {
    m_DB->setDBname(t_DBname);
    m_versionUpdater->performSmallUpdate();
    return 0;
}

int DBUpdater::changeFileName(int t_fileIndex, string t_newFileName) {
    vector<string> *dbFilesNamesList = m_DB->getDBfilesNamesList();
    dbFilesNamesList->at(t_fileIndex) = m_DB->getDBDirectory()+t_newFileName;
    m_versionUpdater->performSmallUpdate();
    return 0;
}

int DBUpdater::changeFileName(string t_oldFileName, string t_newFileName) {
    vector<string> *dbFilesNamesList = m_DB->getDBfilesNamesList();
    ptrdiff_t pos = find(dbFilesNamesList->begin(), dbFilesNamesList->end(), m_DB->getDBDirectory()+t_oldFileName) - dbFilesNamesList->begin();
    if (pos < dbFilesNamesList->size())
        changeFileName(pos, t_newFileName);
    else
        cout << "The file" << t_oldFileName << " doesn't exist in the DB catalog" << endl;
    return 0;
}

int DBUpdater::addFile(string t_fileName) {
    if (!fileExists(t_fileName)) {
        cout << "Error-the file \"" << t_fileName << "\" doesn't exist." << endl;
        return 0;
    }
    if (!fileExists(m_DB->getDBupdatesFileName())) {
        clearFile(m_DB->getDBupdatesFileName());
    }
    
    int numberOfFiles = (int)getNumberOfLinesInFile(t_fileName) / 2;     //1 line of file name; 1 line for file size
    if (m_verbose) cout << "numberOfFiles in updates file=" << numberOfFiles << endl;
    if (numberOfFiles+1 == m_maxNumberOfFilesInDBupdatesFile) {
        if (m_verbose) cout << "Performing major update on the DB..." << endl;
        addNewFileToDBupdatesFile(t_fileName);
        //remove any dummy file in the files list
        removeDummyFileFromFilesList();
        //load files from the Updates file to the files list as new files
        loadFromUpdatesFile();
        //merge the DBupdates files with the files list
        m_DB->getDBfilesNamesList()->insert(end(*m_DB->getDBfilesNamesList()), begin(*m_DBupdatesfilesNamesList), end(*m_DBupdatesfilesNamesList));
        m_DB->getDBfilesSizesList()->insert(end(*m_DB->getDBfilesSizesList()), begin(*m_DBupdatesfilesSizesList), end(*m_DBupdatesfilesSizesList));
        //update N, M
        m_DB->setM(m_DB->getM() + *m_numberOfNewFiles);
        m_DB->setN(max(m_DB->getN(), *m_maxNewSize));
        //performing major update
        m_versionUpdater->performMajorUpdate();
        clearFile(m_DB->getDBupdatesFileName());
    }
    else {
        addNewFileToDBupdatesFile(t_fileName);
        if (m_verbose) cout << "Adding file name \"" << t_fileName << "\" to the db updates file." << endl;
        m_versionUpdater->performMinorUpdate();
    }
    return 0;
}

int DBUpdater::removeDummyFileFromFilesList() {
    size_t i=0;
    string dummyFileName = "dummyFile.data";
    while (i < m_DB->getDBfilesNamesList()->size()) {
        if (m_DB->getDBfilesNamesList()->at(i).compare(dummyFileName) == 0) {
            m_DB->getDBfilesNamesList()->erase(m_DB->getDBfilesNamesList()->begin() + i);
            m_DB->getDBfilesSizesList()->erase(m_DB->getDBfilesSizesList()->begin() + i);
            m_DB->setM(m_DB->getM() - 1);
        }
        else
            i++;
    }
    return 0;
}

int DBUpdater::loadFromUpdatesFile() {
    m_DBupdatesfilesNamesList->clear();
    m_DBupdatesfilesSizesList->clear();
    if (!fileExists(m_DB->getDBupdatesFileName()))
        return 0;
    ifstream DBUpdatesFileStream(m_DB->getDBupdatesFileName(),ifstream::in);
    string errorMsg = "Unable to open the file: " + m_DB->getDBupdatesFileName();
    if (!DBUpdatesFileStream.is_open()) error(errorMsg.c_str());
    string line;
    //size_t oldN = m_DB->getN();
    *m_numberOfNewFiles = 0;
    *m_maxNewSize = 0;
    while (getline(DBUpdatesFileStream, line)) {
        m_DBupdatesfilesNamesList->push_back(line);
        getline(DBUpdatesFileStream, line);
        stringstream sstream(line);
        size_t fileSize;
        sstream >> fileSize;
        m_DBupdatesfilesSizesList->push_back(fileSize);
        if (fileSize > *m_maxNewSize) *m_maxNewSize = fileSize;
        *m_numberOfNewFiles++;
    }
    DBUpdatesFileStream.close();
    return 0;
}

int DBUpdater::clearFile(string t_fileName) {
    ofstream fileStream(t_fileName, ios::out | ios::trunc);
        string errorMsg = "Unable to open the file: " + t_fileName;
        if (!fileStream.is_open()) error(errorMsg.c_str());
        fileStream.close();
    return 0;
}

int DBUpdater::addNewFileToDBupdatesFile(string t_fileName) {
    ofstream dbUpdatesFileStream(m_DB->getDBupdatesFileName(), ios::app);
    string errorMsg = "Unable to open the DB updates file: " + m_DB->getDBupdatesFileName();
    if (!dbUpdatesFileStream.is_open()) error(errorMsg.c_str());
    dbUpdatesFileStream << t_fileName << endl;
    dbUpdatesFileStream << getFileSize(t_fileName) << endl;
    dbUpdatesFileStream.close();
    return 0;
}

int DBUpdater::addNewFileToDBupdatesFile(string t_fileName, size_t t_fileSize) {
    ofstream dbUpdatesFileStream(m_DB->getDBupdatesFileName(), ios::app);
    string errorMsg = "Unable to open the DB updates file: " + m_DB->getDBupdatesFileName();
    if (!dbUpdatesFileStream.is_open()) error(errorMsg.c_str());
    dbUpdatesFileStream << t_fileName << endl;
    dbUpdatesFileStream << t_fileSize << endl;
    dbUpdatesFileStream.close();
    return 0;
}

size_t DBUpdater::getNumberOfLinesInFile(string t_fileName) {
    size_t number_of_lines = 0;
    FILE *infile = fopen(t_fileName.c_str(), "r");
    int ch;
    while (EOF != (ch=getc(infile)))
        if ('\n' == ch)
            ++number_of_lines;
    number_of_lines;
    return 0;
}

vector<string>* DBUpdater::getDBupdatesfilesNamesList() { return m_DBupdatesfilesNamesList; }
vector<size_t>* DBUpdater::getDBupdatesfilesSizesList() { return m_DBupdatesfilesSizesList; }

int DBUpdater::deleteFile(string t_fileName) {
    //loading files in DBupdates to memory
    loadFromUpdatesFile();
    vector<string>::iterator pos2 = find(m_DBupdatesfilesNamesList->begin(), m_DBupdatesfilesNamesList->end(), t_fileName);
    size_t pos2Index = distance(m_DBupdatesfilesNamesList->begin() ,pos2);
    if (pos2 != m_DBupdatesfilesNamesList->end()){//file in updates file
        //removing file from the updates list
        m_DBupdatesfilesNamesList->erase(pos2);
        m_DBupdatesfilesSizesList->erase(m_DBupdatesfilesSizesList->begin()+pos2Index);
        //rewriting DBupdates file
        clearFile(m_DB->getDBupdatesFileName());
        for (int i=0; i<m_DBupdatesfilesNamesList->size(); i++)
            addNewFileToDBupdatesFile(m_DBupdatesfilesNamesList->at(i), m_DBupdatesfilesSizesList->at(i));
        //performing minor update
        m_versionUpdater->performMinorUpdate();
    }
    else {//file in DBinfo OR files list if loaded
        vector<string>::iterator pos1 = find(m_DB->getDBfilesNamesList()->begin(), m_DB->getDBfilesNamesList()->end(), t_fileName);
        size_t pos1Index = distance(m_DB->getDBfilesNamesList()->begin(), pos1);
        if (pos1 != m_DB->getDBfilesNamesList()->end()) {
            //removing file from the list
            m_DB->getDBfilesNamesList()->erase(pos1);
            m_DB->getDBfilesSizesList()->erase(m_DB->getDBfilesSizesList()->begin()+pos1Index);
            //removing any dummy files from the list
            removeDummyFileFromFilesList();
            //merge the DBupdates files with the files list
            m_DB->getDBfilesNamesList()->insert(end(*m_DB->getDBfilesNamesList()), begin(*m_DBupdatesfilesNamesList), end(*m_DBupdatesfilesNamesList));
            m_DB->getDBfilesSizesList()->insert(end(*m_DB->getDBfilesSizesList()), begin(*m_DBupdatesfilesSizesList), end(*m_DBupdatesfilesSizesList));
            //clearing the DBupdates file
            clearFile(m_DB->getDBupdatesFileName());
            //performing major update
            m_versionUpdater->performMajorUpdate();
        }
        else {
            //file name not found in the two lists
            cout << "The file name \"" << t_fileName << "\" doesn't exist in the DB!" << endl;
        }
    }
    return 0;
}

int DBUpdater::sse8Trans(char *inBuffer, char *outBuffer, size_t numberOfRows, size_t numberOfCols) {
    int i=0;
    size_t row=0, col=0;
    uint16_t transposedRow;
    union {
        __m128i x;
        uint8_t b[16];
    } tmp;
    for (i=0; i<16; i++)
        tmp.b[i] = 0;
    for (col=0; col<numberOfCols; col+=8) {
        for (i=0; i<8; i++)
            tmp.b[7-i] = inBuffer[(row+i) * numberOfCols/8 + col/8];
        for (i=0; i<8; i++) {
            transposedRow = _mm_movemask_epi8(tmp.x);
            size_t targetAddr = (col+i) * numberOfRows/8 + row/8;
            *(outBuffer + targetAddr) = transposedRow;
            tmp.x = _mm_slli_epi64(tmp.x, 1);
        }
    }
    return 0;
}

int DBUpdater::updateFile(string t_oldFileName, string t_newFileName) {
    //loading files in DBupdates to memory
    loadFromUpdatesFile();
    vector<string>::iterator pos2 = find(m_DBupdatesfilesNamesList->begin(), m_DBupdatesfilesNamesList->end(), t_oldFileName);
    size_t pos2Index = distance(m_DBupdatesfilesNamesList->begin() ,pos2);
    if (pos2 != m_DBupdatesfilesNamesList->end()){//file in updates file
        //removing file from the updates list
        m_DBupdatesfilesNamesList->erase(pos2);
        m_DBupdatesfilesSizesList->erase(m_DBupdatesfilesSizesList->begin()+pos2Index);
        //inserting the new file to the updates list
        m_DBupdatesfilesNamesList->push_back(t_newFileName);
        m_DBupdatesfilesSizesList->push_back(getFileSize(t_newFileName));
        //removing file from the updates list
        clearFile(m_DB->getDBupdatesFileName());
        for (int i=0; i<m_DBupdatesfilesNamesList->size(); i++)
            addNewFileToDBupdatesFile(m_DBupdatesfilesNamesList->at(i), m_DBupdatesfilesSizesList->at(i));
        //performing minor update to rewrite DBinfo + DBupdates
        m_versionUpdater->performMinorUpdate();
    }
    else {
        vector<string>::iterator pos1 = find(m_DB->getDBfilesNamesList()->begin(), m_DB->getDBfilesNamesList()->end(), t_oldFileName);
        size_t pos1Index = distance(m_DB->getDBfilesNamesList()->begin(), pos1);
        if (pos1 != m_DB->getDBfilesNamesList()->end()) {
            if (!m_DBupdatesfilesNamesList->empty()) {    //rewrite the entire DB file. adding new file + files in DBupdates
                //removing the old file from the list
                m_DB->getDBfilesNamesList()->erase(pos1);
                m_DB->getDBfilesSizesList()->erase(m_DB->getDBfilesSizesList()->begin()+pos1Index);
                removeDummyFileFromFilesList();
                //adding the new file to the list
                m_DB->getDBfilesNamesList()->push_back(t_newFileName);
                m_DB->getDBfilesSizesList()->push_back(getFileSize(t_newFileName));
                //merging the files list with the DBupdates list
                m_DB->getDBfilesNamesList()->insert(end(*m_DB->getDBfilesNamesList()), begin(*m_DBupdatesfilesNamesList), end(*m_DBupdatesfilesNamesList));
                m_DB->getDBfilesSizesList()->insert(end(*m_DB->getDBfilesSizesList()), begin(*m_DBupdatesfilesSizesList), end(*m_DBupdatesfilesSizesList));
                clearFile(m_DB->getDBupdatesFileName());
                //performing major update to rewrite DBinfo + DBfile + XDB
                m_versionUpdater->performMajorUpdate();
            }
            else {  //change just the file bits of the file in DBfile
                m_DB->getDBfilesNamesList()->at(pos1Index) = t_newFileName;
                m_DB->getDBfilesSizesList()->at(pos1Index) = getFileSize(t_newFileName);
                //updating directly on DBfile
                fstream DBfileStream(m_DB->getDBFileName(), ios::binary);
                string errorMsg = "Unable to open the file: " + m_DB->getDBFileName();
                if (!DBfileStream.is_open()) error(errorMsg.c_str());
                //load the file "i" + 7others to transpose
                char *inputBuff = (char *) malloc (8 * m_DB->getN() * sizeof (char));
                size_t firstFileIndex = pos1Index/8*8;    //index of the first file
                for (int i=0; i<8; i++) {
                    readFileToBuffer(m_DB->getDBfilesNamesList()->at(firstFileIndex+1), &inputBuff[m_DB->getN() * i], m_DB->getN());
                }
                //transposing the 8 files in memory
                char *outputBuff = (char *) malloc (8 * m_DB->getN() * sizeof (char));
                sse8Trans(inputBuff, outputBuff, 8, 8 * m_DB->getN());
                //copying from memory the the DB file
                for (size_t col=0; col<8 * m_DB->getN(); col++) {
                    //size_t fileTargetAddr = transposedFilesBufferSize*currentChunkNumber + col*numberOfBlocksInFilesBuffer*2;
                    size_t fileTargetAddr = col*m_DB->getM()/8 + pos1Index/8;
                    size_t buffTargetAddr = col*(m_DB->getM())/8;
                    DBfileStream.seekp(fileTargetAddr);
                    DBfileStream.write(&outputBuff[buffTargetAddr],1);
            //            if (col==0||col==1||col==2) cout <<"wrinting from TFB["<<col*numberOfBlocksInFilesBuffer*2<<
            //                    "] to outputfile[transposedFilesBufferSize*"<<currentChunkNumber<<","<<col*numberOfBlocksInFilesBuffer*2<<"]"<<endl;
                    //if (col==0||col==1||col==2) cout << "from buffTargetAddr:"<<buffTargetAddr<<" to fileTargetAddr:"<<fileTargetAddr
                            //<<". wiriting:"<<(m_M-row)/8<<"chars"<<endl;
                }
                //major update without updating DBfile
                m_versionUpdater->performMajorUpdateWithoutDBfile();
            }
        }
        else {
            //file name not found in the two lists
            cout << "The file name \"" << t_oldFileName << "\" doesn't exist in the DB!" << endl;
        }
    }
    return 0;
}

int DBUpdater::updateFile(int t_fileIndex) {
    //...
    return 0;
}

int DBUpdater::updateSeed(uint64_t t_newSeed) {
    //...
    return 0;
}

DBUpdater::DBUpdater(const DBUpdater& orig) {
}

DBUpdater::~DBUpdater() {
}

