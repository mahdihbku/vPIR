/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   VersionChecker.cpp
 * Author: mahdi
 * 
 * Created on February 27, 2018, 1:01 AM
 */
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>

#include "VersionChecker.hpp"

using namespace std;

VersionChecker::VersionChecker(string t_version) {
    m_version = t_version;
    splitDBversion();
}

int VersionChecker::splitDBversion() {
    int pos1 = m_version.find(".");
    string v1string = m_version.substr(0, pos1);
    int pos2  = m_version.find(".",pos1+1);
    string v2string = m_version.substr(pos1+1, pos2);
    string v3string = m_version.substr(pos2+1, m_version.length());
    m_v1=stoi(v1string);
    m_v2=stoi(v2string);
    m_v3=stoi(v3string);
    //cout << "v1="<<m_v1<<" v2="<<m_v2<<" v3="<<m_v3<<endl;
    return 0;
}

int VersionChecker::getV1() { return m_v1; }
int VersionChecker::getV2() { return m_v2; }
int VersionChecker::getV3() { return m_v3; }
    
VersionChecker::VersionChecker(const VersionChecker& orig) {
}

VersionChecker::~VersionChecker() {
}

