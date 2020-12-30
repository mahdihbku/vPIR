/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   XPIRDescriptor.cpp
 * Author: mahdi
 * 
 * Created on November 26, 2018, 12:00 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

#include "XPIRDescriptor.hpp"

using namespace std;

XPIRDescriptor::XPIRDescriptor() {
}

XPIRDescriptor::XPIRDescriptor(const XPIRDescriptor& orig) {
}

XPIRDescriptor::XPIRDescriptor(string t_XPIRip, int t_XPIRport, size_t t_numberOfFiles, std::string t_XPIRclientDir, int t_verbose) : 
    m_XPIRip(t_XPIRip), m_XPIRport(t_XPIRport), m_numberOfFiles(t_numberOfFiles), m_XPIRclientDir(t_XPIRclientDir), m_verbose(t_verbose) {}

int XPIRDescriptor::getFileFromXPIRserver(size_t fileNumber) {
    string cmd = "cd " + m_XPIRclientDir + ";./pir_client -u 8000000 -d 8000000 -r LWE:.*:2048:60 -k 80 -a 1 --dmax 2 -c -i " + m_XPIRip + " -p " + to_string(m_XPIRport) + " --selected-file " + to_string(fileNumber);
    cout << cmd << endl;
    return system(cmd.c_str());
}

void XPIRDescriptor::setNumberOfFiles(size_t numberOfFiles) { m_numberOfFiles = numberOfFiles;}
size_t XPIRDescriptor::getNumberOfFiles() const { return m_numberOfFiles;}
void XPIRDescriptor::setXPIRclientDir(std::string XPIRclientDir) { m_XPIRclientDir = XPIRclientDir;}
std::string XPIRDescriptor::getXPIRclientDir() const { return m_XPIRclientDir;}
void XPIRDescriptor::setXPIRport(int XPIRport) { m_XPIRport = XPIRport;}
int XPIRDescriptor::getXPIRport() const { return m_XPIRport;}
void XPIRDescriptor::setXPIRip(std::string XPIRip) { m_XPIRip = XPIRip;}
std::string XPIRDescriptor::getXPIRip() const { return m_XPIRip;}

XPIRDescriptor::~XPIRDescriptor() {
}

