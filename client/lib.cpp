/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   lib.cpp
 * Author: mahdi
 * 
 * Created on December 12, 2017, 12:25 PM
 */
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#include "lib.hpp"

using namespace std;

int error(const char *t_msg) {
    perror(t_msg);
    abort();
}

int min(const size_t t_a, const size_t t_b) {
    return (t_a>t_b) ? t_b : t_a;
}

size_t getFileSize(const string fileName) {
    size_t size = 0;
    ifstream fileStream(fileName, ios::binary);
    string errorMsg = "Unable to open the file: " + fileName;
    if (!fileStream.is_open()) error(errorMsg.c_str());
    fileStream.seekg (0, fileStream.end);
    size = fileStream.tellg();
    fileStream.close();
    return size;
}

int coutBuff(const char* t_sToDisplay, const int t_P, const size_t t_sizeInBytes) {
    cout << "Buff= ";
    switch (t_P) {
        case 1: {
            uint8_t* sInt = (uint8_t*) t_sToDisplay;
            for (size_t i=0; i<t_sizeInBytes; i++)
                cout << hex << (int)sInt[i] << ", ";
            break;}
        case 8: {
            uint8_t* sInt = (uint8_t*) t_sToDisplay;
            for (size_t i=0; i<t_sizeInBytes; i++)
                cout << (int)sInt[i] << ", ";
            break;}
        case 16: {
            uint16_t* sInt = (uint16_t*) t_sToDisplay;
            for (size_t i=0; i<t_sizeInBytes/2; i++)
                cout << (int)sInt[i] << ", ";
            break;}
        case 32: {
            uint32_t* sInt = (uint32_t*) t_sToDisplay;
            for (size_t i=0; i<t_sizeInBytes/4; i++)
                cout << (int)sInt[i] << ", ";
            break;}
        case 64: {
            uint64_t* sInt = (uint64_t*) t_sToDisplay;
            for (size_t i=0; i<t_sizeInBytes/8; i++)
                cout << (int)sInt[i] << ", ";
            break;}
    }
    cout << endl;
}

#define HEX(x) setw(2) << setfill('0') << hex << (int)(x)

int char2int(char input)
{
  if(input >= '0' && input <= '9')
    return input - '0';
  if(input >= 'A' && input <= 'F')
    return input - 'A' + 10;
  if(input >= 'a' && input <= 'f')
    return input - 'a' + 10;
  throw std::invalid_argument("Invalid input string");
}

// This function assumes src to be a zero terminated sanitized string with
// an even number of [0-9a-f] characters, and target to be sufficiently large

int hex2bin(string src, char* target, size_t binSize)
{
    for (size_t i=0; i<binSize; i++)
        target[i] = char2int(src.at(i*2))*16 + char2int(src.at(i*2+1));
}
    
string bin2hex(const char* src, size_t binSize) {
    stringstream seedSS;
    for (int i=0; i<binSize; i++)
        seedSS << HEX((uint8_t)src[i]);
    return seedSS.str();
}
