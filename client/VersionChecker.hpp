/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   VersionChecker.hpp
 * Author: mahdi
 *
 * Created on February 27, 2018, 1:01 AM
 */

#ifndef VERSIONCHECKER_HPP
#define VERSIONCHECKER_HPP

#include <string>

class VersionChecker {
public:
    VersionChecker(std::string t_version);
    int getV1();
    int getV2();
    int getV3();
    VersionChecker(const VersionChecker& orig);
    virtual ~VersionChecker();
private:
    std::string m_version="";
    int m_v1=0, m_v2=0, m_v3=0;
    int splitDBversion();
};

#endif /* VERSIONCHECKER_HPP */

