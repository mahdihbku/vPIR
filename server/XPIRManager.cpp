/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   XPIRManager.cpp
 * Author: mahdi
 * 
 * Created on November 25, 2018, 9:56 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

#include "XPIRManager.hpp"

using namespace std;

XPIRManager::XPIRManager() {
}

XPIRManager::XPIRManager(string t_XPIRdir, int t_XPIRport, size_t t_numberOfFiles, int t_verbose) : 
    m_XPIRdir(t_XPIRdir), m_XPIRport(t_XPIRport), m_numberOfFiles(t_numberOfFiles), m_verbose(t_verbose) {} 

XPIRManager::XPIRManager(const XPIRManager& orig) {
}

XPIRManager::~XPIRManager() {
}

void XPIRManager::setXDBFile(string t_XDBFile) { this->m_XDBFile = t_XDBFile;}
string XPIRManager::getXDBFile() const { return m_XDBFile;}
void XPIRManager::setXPIRip(string t_XPIRip) { this->m_XPIRip = t_XPIRip;}
string XPIRManager::getXPIRip() const { return m_XPIRip;}
void XPIRManager::setXPIRdir(string t_XPIRdir) { this->m_XPIRdir = t_XPIRdir;}
string XPIRManager::getXPIRdir() const { return m_XPIRdir;}
void XPIRManager::setXPIRport(int t_XPIRport) { this->m_XPIRport = t_XPIRport;}
int XPIRManager::getXPIRport() const { return m_XPIRport;}
void XPIRManager::SetNumberOfFiles(size_t numberOfFiles) { this->m_numberOfFiles = numberOfFiles;}
size_t XPIRManager::GetNumberOfFiles() const { return m_numberOfFiles;}

int XPIRManager::startXPIRserver() {
    pid_t xpirProcess = 0;
    xpirProcess = fork();
    if (xpirProcess < 0) {
        fprintf( stderr, "process failed to fork\n" );
        return 0;
    }
    if (xpirProcess == 0) {
        string cmd = "cd " + m_XPIRdir + ";./pir_server -z -s " + to_string(m_numberOfFiles) + " -p " + to_string(m_XPIRport);
        cout << cmd << endl;
        return system(cmd.c_str());
        //return system("pwd; ls; cd /; pwd; ls");
    }
}
