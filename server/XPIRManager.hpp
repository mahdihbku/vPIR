/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   XPIRManager.hpp
 * Author: mahdi
 *
 * Created on November 25, 2018, 9:56 AM
 */

#ifndef XPIRMANAGER_HPP
#define XPIRMANAGER_HPP

class XPIRManager {
public:
    XPIRManager();
    XPIRManager(std::string t_XPIRdir, int t_XPIRport, size_t t_numberOfFiles, int t_verbose);
    XPIRManager(const XPIRManager& orig);
    virtual ~XPIRManager();
    int startXPIRserver();
    
    //getters and setters
    void setXDBFile(std::string t_XDBFile);
    std::string getXDBFile() const;
    void setXPIRip(std::string t_XPIRip);
    std::string getXPIRip() const;
    void setXPIRdir(std::string t_XPIRdir);
    std::string getXPIRdir() const;
    void setXPIRport(int t_XPIRport);
    int getXPIRport() const;
    void SetNumberOfFiles(size_t numberOfFiles);
    size_t GetNumberOfFiles() const;
    
private:
    int m_XPIRport = 12346;
    std::string m_XPIRdir = "";
    std::string m_XPIRip = "";
    std::string m_XDBFile = "XDBbytes.file";
    size_t m_numberOfFiles = 0;
    int m_verbose = 0;
};

#endif /* XPIRMANAGER_HPP */

