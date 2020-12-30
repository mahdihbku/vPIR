/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: mahdi
 *
 * Created on August 24, 2017, 12:10 AM
 */

#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <cmath>
#include <random> 
#include <cstring>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <chrono>
#include <thread>

#include "ServerOps.hpp"
#include "DBManager.hpp"
#include "RandomDBgenerator.hpp"
#include "SystemParameters.hpp"
#include "lib.hpp"
#include "cxxopts.hpp"
#include "DBUpdater.hpp"

using namespace std;

/*
 * 
 */

void methodInFunction(char* dbBuffer, char* queriesBuffer, size_t nbrQueries, char* outputBuffer, size_t M, size_t N, unsigned char *masks) {
    chrono::time_point<chrono::system_clock> start, end;
    chrono::duration<double> elapsed_seconds;
    start = chrono::system_clock::now();
    uint64_t *filePointer64 = (uint64_t *) dbBuffer;
    uint64_t *replyPointer64;
    size_t m=0, a=0, n=0; int mask=0;
    size_t mm=0, nn=0;
    size_t B1=128;		//B1 splits M (the number of files)
    size_t B2=128;		//B2 splits N, (size of 1 file) B2 should be multiple of 8 to XOR in 64bits
    //B1*(8*B2)+(B2*8) should fit in the cache
    for (mm=0; mm<M; mm+=B1) {
        for (nn=0; nn<N/8; nn+=B2) {
            for (a=0; a<nbrQueries; a++) {
                for (m=mm; m<min(mm+B1,M); m++) {	//M is the number of files
                    if ((uint8_t)queriesBuffer[a*M/8+m/8] & (uint8_t)masks[mask]) {		//X[a,m/8]
                        // cout << "hit!m=" << m << " a=" << dec << a << " mask=" << hex << unsigned(mask) << " masks[mask]=" << hex << unsigned(masks[mask]) << " (uint8_t)queriesBuffer[a*M/8+m/8]=" << unsigned((uint8_t)queriesBuffer[a*M/8+m/8]) << " dbParser=" << dbParser << endl;
                        replyPointer64 = (uint64_t*) (outputBuffer+a*N);	//Z[a,0]
                        for (n=nn; n<min(nn+B2,N/8); n++){	//N is the file size in bytes
                            // if (n==0)
                            // 	cout << "m=" << m << " before replyPointer64[n]=" << hex << replyPointer64[n] << " filePointer64[n]=" << filePointer64[n] << endl;
                            replyPointer64[n] ^= filePointer64[m*N/8+n];	//Z[a,n] ^= Y[m,n]
                            // if (n==0)
                            // 	cout << "m=" << m << " after replyPointer64[n]=" << hex << replyPointer64[n] << endl;
                        }
                    }
                    mask=(mask+1)&7;	//mask=(mask+1)%8;
                }
            }
        }
    }
    end = chrono::system_clock::now();
    elapsed_seconds = end-start;
    cout << " total time= " << elapsed_seconds.count();
    cout << endl;
    //return 0;
}

int main(int argc, char** argv) {
    string dbDir = "../../10GB/";
    //str ing dbDir = "resources/3000x1x1/";
    int parallelSimQueries = 0;
    int aggeragation = 1;
    int m = 3000, n = 1048576, r = 100;
    int memUsageThreshold = 90;
    int portNumber = 12345;
    int maxParallelClient = 5;
    int queueTimeout = 1000;
    int waitingTimeBetweenQueries = 10;
    int maxNumberOfFilesInDBupdatesFile = r*11/100/8*8;
    string newDBFile = "";
    string newDBFileInfo = "";
    string newDBUpdatesFile = "";
    string newDBName = "db1";
    int verbose = 0;
    bool parallelQueries = false;
    
    //XPIR server parameters
    int XPIRport = 1234;
    string XPIRdir = "/home/mahdi/workspace/XPIR2/XPIR-nocatalog/_build/apps/server";
    string XPIRip = "127.0.0.1";
    string newXDBFile = "XDBbytes.file";
    
    try
    {
        cxxopts::Options options("vPIR", "Truly practical private information retrieval");
        options.add_options()
            //Mandatory:
            ("d,directory", "Working DB directory -default:../../10GB", cxxopts::value<string>())
            ("XPIRdir", "XPIR server directory (including /_build/apps/server)", cxxopts::value<string>())
                ("xdb", "XDB file name (XPIRdir/db/arg) -default:XDBbytes.file", cxxopts::value<string>())
                ("XPIRdbFile", "Same as --xdb", cxxopts::value<string>())
                ("XPIRip", "XPIR server IP -default:localhost", cxxopts::value<string>())
                ("XPIRport", "XPIR server port -default:12346", cxxopts::value<int>())
            //Optional:
            ("m", "Number of files in DB", cxxopts::value<int>())
            ("n", "Size of one file (in bytes)", cxxopts::value<int>())
            ("generateRandom", "Generate random DB files (needs n,m)")  //needs n, m
            ("createDBfiles", "Creating new DB files from directory")   //needs also p, s, r, DBName
                ("DBName", "The database name -default:db1", cxxopts::value<string>())
                ("r", "Number of random vectors -default:2*max(n,m)", cxxopts::value<int>())
//                ("a", "Number of subfiles in one file -default:1 (no aggregation)", cxxopts::value<int>())
            ("threshold", "Memory usage threshold (RAM percentage) -default:90 (%)", cxxopts::value<int>())
            ("port", "Server port number -default:12345", cxxopts::value<int>())
            ("max", "Number of maximum parallel clients -default:5", cxxopts::value<int>())
            ("t,timeout", "Queue timeout (in ms) -default:1000", cxxopts::value<int>())
            ("w,waiting", "Waiting time between queries (in ms) -default:10", cxxopts::value<int>())
            ("parallelQueries", "Serve queries in parallel")
            ("maxUpdates", "Max number of files in updates file -default:11%", cxxopts::value<int>())
            //Updating:
            ("updateFileName", "Name of the file to change", cxxopts::value<string>())
            ("updateDBname", "Update DB name", cxxopts::value<string>())
            ("addFile", "File to add", cxxopts::value<string>())
            ("deleteFile", "File to delete", cxxopts::value<string>())
            ("updateFile", "File to update", cxxopts::value<string>())
            ("updateSeed", "Regenerate a new random seed")
            ("new", "New file name", cxxopts::value<string>())
            //General:
            ("simulation", "Simulate a number of parallel queries", cxxopts::value<int>())
            ("v,verbose", "Show additional details", cxxopts::value<int>())
            ("h,help", "Print help")
            ("test", "Test program features...TODO to be removed"); //TODO to be removed
        auto result = options.parse(argc, argv);
        if (result.count("help")) {
            cout << options.help();
            exit(0);
        }
        if (result.count("verbose"))
            verbose = result["verbose"].as<int>();
        if (!result.count("directory")) {
            cout << "Error, usage: vPIR -d DBdirectory" << endl;
            exit(1);
        } else
            dbDir = result["directory"].as<string>();
        if (result.count("DBName"))
            newDBName = result["DBName"].as<string>();
        if (result.count("m"))
            m = result["m"].as<int>();
        if (result.count("simulation"))
            parallelSimQueries = result["simulation"].as<int>();
        if (result.count("n"))
            n = result["n"].as<int>();
//        if (result.count("a"))
//            aggeragation = result["a"].as<int>();
        if (result.count("r")) {
            r = result["r"].as<int>();
            cout << "r=" << r << endl;
            maxNumberOfFilesInDBupdatesFile = r*11/100/8*8; //11% of the number of files in XDB
        }
        if (result.count("port"))
            portNumber = result["port"].as<int>();
        if (result.count("XPIRport"))
            XPIRport = result["XPIRport"].as<int>();
        if (result.count("XPIRdir"))
            XPIRdir = result["XPIRdir"].as<string>();
        if (result.count("XPIRip"))
            XPIRip = result["XPIRip"].as<string>();
        if (result.count("threshold"))
            memUsageThreshold = result["threshold"].as<int>();
        if (result.count("max"))
            maxParallelClient = result["max"].as<int>();
        if (result.count("timeout"))
            queueTimeout = result["timeout"].as<int>();
        if (result.count("waiting"))
            waitingTimeBetweenQueries = result["waiting"].as<int>();
        if (result.count("maxUpdates"))
            maxNumberOfFilesInDBupdatesFile = result["maxUpdates"].as<int>()/8*8;
        
        if (result.count("generateRandom")) {
            if (!result.count("m") || !result.count("n")) {
                cout << "Error, usage: vPIR -generateRandom -m numberOfFiles -n oneFileSize -d DBdirectory" << endl;
                exit(1);
            }
            RandomDBgenerator randomDB;
            randomDB.setParameters(dbDir, m, n, verbose);
            size_t dbSize = (size_t)n * (size_t)m;
            if (verbose) cout << "dbSize=" << dbSize << endl;
            if (dbSize < 1024*1024*1024) randomDB.fastGenerate();   //TODO to change to:  if (dbSize < 1024*1024*1024) randomDB.generate();
            else randomDB.fastGenerate();
            if (!result.count("createDBfiles")) exit(1);
        }
        
        DBManager newDB(verbose);
        if (result.count("createDBfiles")) {
            newXDBFile = result.count("xdb") ? XPIRdir+"db/"+result["xdb"].as<string>() : XPIRdir+"db/XDBbytes.file";
            newDBFile = dbDir+"dbFile.db"; newDBFileInfo = dbDir+"dbInfo.db"; newDBUpdatesFile = dbDir+"DBupdates.db";
            
            newDB.setDBParameters(dbDir, newDBFile, newDBFileInfo, newXDBFile, newDBUpdatesFile, newDBName, memUsageThreshold);
            newDB.setXPIRParameters(XPIRport, XPIRdir, XPIRip);
            //SystemParameters systemParm(newDB.getN()*newDB.getM(), newDB.getM(), memUsageThreshold);
            //newDB.setSystemParm(&systemParm);
            //if (verbose) cout << "Available memory= " << newDB.getSystemParm()->getTotalAvailRAM() << endl;
            
            r = 2*max(newDB.getM(), 8*newDB.getN());
            cout << dec << "r=" << r << " m=" << newDB.getM() << " 8n=" << 8*newDB.getN() << endl;
            newDB.generateNewDBFiles(r, newXDBFile);
            if (verbose) cout << "The DB files :" << newDBFileInfo << "," << newDBFile << "," << newXDBFile <<" have been created successfully." << endl;
            else cout << "All DB files have been created successfully." << endl;
        }
        else {
//            cout << "--------" << endl;
//            cout << "dbDir= " << dbDir << endl;
//            cout << "memUsageThreshold= " << memUsageThreshold << endl;
            newDB.loadFromDBinfo(dbDir, memUsageThreshold);
//            cout << "--------" << endl;
            newDB.loadDBtoMemory();
//            cout << "--------" << endl;
            
            DBUpdater du;
            newDB.setDBupdater(&du);
            du.setParameters(&newDB, maxNumberOfFilesInDBupdatesFile, verbose);
            //TODO check DB updater-new matrix structure
            
//            newDB.getDBupdater()->changeDBname("the db name has been changed 22222222222222");
            //cin.get();
            //du.changeFileName(0, "newFileName2.data");
            //cin.get();
            //du.changeFileName("file9855.data","newFileName3.data");
//            newDB.getDBupdater()->addFile("filetoadd.txt");
            
            if (verbose) cout << "The DB info has been loaded successfully from the file \"" << newDB.getDBFileName() << "\"" << endl;
            else cout << "The DB info have been loaded successfully." << endl;
            
            //Starting XPIR server
            if (verbose) cout << "Starting XPIR server..." << endl;
            newDB.getXPIR()->startXPIRserver();
            cout << "XPIR server has been started on port " << XPIRport << "." << endl;
            
            if (result.count("parallelQueries"))
                parallelQueries = true;
            ServerOps serverOperator(&newDB, portNumber, maxParallelClient, queueTimeout, waitingTimeBetweenQueries, parallelQueries, verbose, parallelSimQueries);
            serverOperator.openConnection();
            
        }
                
    } 
    catch (const cxxopts::OptionException& e) {
        cout << "error parsing options: " << e.what() << endl;
        exit(1);
    }
    return 0;
}
