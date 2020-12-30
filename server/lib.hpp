/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   lib.hpp
 * Author: mahdi
 *
 * Created on December 12, 2017, 11:41 AM
 */

#ifndef LIB_HPP
#define LIB_HPP

int error(const char *t_msg);
int min(const size_t t_a, const size_t t_b);
int max(const size_t t_a, const size_t t_b);
bool fileExists(const std::string& t_fileName);
int coutBuff(const char* t_sToDisplay, const int t_P, const size_t t_sizeInBytes);
size_t getFileSize(const std::string fileName);
int readFileToBuffer(const std::string fileName, char *buffer, size_t size);
size_t getMemorySize();
int char2int(char input);
int hex2bin(std::string src, char* target, size_t binSize);
std::string bin2hex(const char* src, size_t binSize);

#endif /* LIB_HPP */
                              
        