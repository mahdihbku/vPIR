/*
 * MIT License
 * Copyright (c) 2016 Hemant Tailor
 * https://github.com/htailor/cpp_progress_bar
 */

#ifndef PROGRESSBAR_HPP
#define PROGRESSBAR_HPP

#ifdef _WINDOWS
#include <windows.h>
#else
#include <sys/ioctl.h>
#endif

#include <iostream>
#include <iomanip>
#include <cstring>

#define TOTAL_PERCENTAGE 100.0
#define CHARACTER_WIDTH_PERCENTAGE 4

class ProgressBar{

public: 

    ProgressBar();
    ProgressBar(unsigned long n_, const char *description_="", std::ostream& out_=std::cerr);

    void SetFrequencyUpdate(unsigned long frequency_update_);
    void SetStyle(const char* unit_bar_, const char* unit_space_);		

    void Progressed(unsigned long idx_);

private:
	
    unsigned long n;
    unsigned int desc_width;
    unsigned long frequency_update;
    std::ostream* out;
		
    const char *description;
    const char *unit_bar;
    const char *unit_space;
		
    void ClearBarField();
    int GetConsoleWidth();
    int GetBarLength();

};

#endif /* PROGRESSBAR_HPP */

