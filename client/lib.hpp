/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   lib.hpp
 * Author: mahdi
 *
 * Created on December 12, 2017, 12:25 PM
 */

#ifndef LIB_HPP
#define LIB_HPP

int error(const char *t_msg);
int min(const size_t t_a, const size_t t_b);
int coutBuff(const char* t_sToDisplay, const int t_P, const size_t t_sizeInBytes);
size_t getFileSize(const std::string fileName);
int char2int(char input);
int hex2bin(std::string src, char* target, size_t binSize);
std::string bin2hex(const char* src, size_t binSize);

#endif /* LIB_HPP */

