/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   XPIRDescriptor.hpp
 * Author: mahdi
 *
 * Created on November 26, 2018, 12:00 PM
 */

#ifndef XPIRDESCRIPTOR_HPP
#define XPIRDESCRIPTOR_HPP

class XPIRDescriptor {
public:
    XPIRDescriptor();
    XPIRDescriptor(std::string t_XPIRip, int t_XPIRport, size_t t_numberOfFiles, std::string t_XPIRclientDir, int t_verbose);
    XPIRDescriptor(const XPIRDescriptor& orig);
    virtual ~XPIRDescriptor();
    int getFileFromXPIRserver(size_t fileNumber);
    
    //getters and setters
    void setNumberOfFiles(size_t numberOfFiles);
    size_t getNumberOfFiles() const;
    void setXPIRclientDir(std::string XPIRclientDir);
    std::string getXPIRclientDir() const;
    void setXPIRport(int XPIRport);
    int getXPIRport() const;
    void setXPIRip(std::string XPIRip);
    std::string getXPIRip() const;
    
private:
    std::string m_XPIRip = "";
    int m_XPIRport = 12346;
    std::string m_XPIRclientDir = "";
    size_t m_numberOfFiles = 0;
    int m_verbose = 0;
};

#endif /* XPIRDESCRIPTOR_HPP */

