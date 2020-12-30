/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: mahdi
 *
 * Created on August 23, 2017, 11:41 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <cstring>
#include <fstream>
#include <cstdlib>
#include <chrono>
#include <ctime>

#include "ClientConnector.hpp"
#include "DBDescriptor.hpp"
#include "Query.hpp"
#include "cxxopts.hpp"
#include "VersionChecker.hpp"

using namespace std;

/*
 * 
 */

int main(int argc, char** argv) {
    int filesToDownload = 70;    //number of files to download from the XPIR server
    string ip = "127.0.0.1";
    int port = 12345;
    string dbInfoName = "dbfiles/db.data";
    string outputFile = "receivedfile";
    string XfileName = "dbfiles/X.rand";
    string XDBfileName = "dbfiles/XDB.file";
    string DBupdatesFileName = "dbfiles/DBupdates.txt";
    int requestedFile = -1;
    int k = 0;
    bool displayCatalogue = true;
    int verbose = 0;
    string XPIRdir = "";
    try
    {
        cxxopts::Options options("vPIR", "Truly practical private information retrieval");
        options.add_options()
            ("l,load", "Load existing database")
            ("noCatalogue", "No db catalogue printing")
            ("ip", "Server ip -default:127.0.0.1", cxxopts::value<string>())
            ("p,port", "Server port -default:12345", cxxopts::value<int>())
            ("dbInfoName", "Database information file location -default:dbfiles/db.data", cxxopts::value<string>())
            ("outputFile", "Received file location and name -default:currentDir/remoteFileName", cxxopts::value<string>())
            ("XfileName", "X matrix file location -default:dbfiles/X.rand", cxxopts::value<string>())
            ("XDBfileName", "XDB file location -default:dbfiles/XDB.file", cxxopts::value<string>())
            ("DBupdatesFileName", "DB updates file location -default:dbfiles/DBupdates.txt", cxxopts::value<string>())
            ("r", "Number of files to download from XPIR -default:70", cxxopts::value<int>())
            ("k", "Number of random vectors to select -default:r/2=35", cxxopts::value<int>())
            ("f,file", "Requested file number", cxxopts::value<int>())
            ("simulation", "simulating query generation")
            ("XPIRdir", "XPIR client directory (including /_build/apps/client)", cxxopts::value<string>())
            ("v,verbose", "Show additional details", cxxopts::value<int>())
            ("h,help", "Print help");
        auto result = options.parse(argc, argv);
        if (result.count("help")) {
            cout << options.help();
            exit(0);
        }
        if (result.count("verbose"))
            verbose = result["verbose"].as<int>();
        if (result.count("ip"))
            ip = result["ip"].as<string>();
        if (result.count("port"))
            port = result["port"].as<int>();
        if (result.count("XPIRdir"))
            XPIRdir = result["XPIRdir"].as<string>();
        if (result.count("dbInfoName"))
            dbInfoName = result["dbInfoName"].as<string>();
        if (result.count("outputFile"))
            outputFile = result["outputFile"].as<string>();
        if (result.count("XfileName"))
            XfileName = result["XfileName"].as<string>();
        if (result.count("DBupdatesFileName"))
            DBupdatesFileName = result["DBupdatesFileName"].as<string>();
        if (result.count("XDBfileName"))
            XDBfileName = result["XDBfileName"].as<string>();
        if (result.count("k"))
            k = result["k"].as<int>();
        if (result.count("r"))
            filesToDownload = result["r"].as<int>();
        if (result.count("file"))
            requestedFile = result["file"].as<int>();
    
        ClientConnector cn(verbose);
        DBDescriptor* db;
        if (result.count("load")) {
            cout << "Loading Db info from the file..." << endl;
            db = new DBDescriptor;
            db->loadDBinfoFromFile(dbInfoName);
            cn.setServerInfo(db->getServerIP(),db->getServerPort());
            cn.setDataBase(db);
            cout << "Initiating Connection..." << endl;
            cn.initiateConnection();
            cout << "Connected to the server: " << cn.getServerIP() << ":" << cn.getServerPort() << endl;
            string localDBversion = db->getDBversion();
            VersionChecker localVersion(localDBversion);
            string remoteDBversion = cn.getDBversionFromServer();
            VersionChecker serverVersion(remoteDBversion);
            if (localVersion.getV1() < serverVersion.getV1()) {
                cout << "Local ("<< localDBversion <<") and remote("<< remoteDBversion <<") db versions are not the same" << endl;
                cout << "Downloading the new DB files..." << endl;
                //redownload db info
                cn.getDBInfoFromServer();
                db = cn.getDB();
                db->getXPIRDescriptor()->setNumberOfFiles(filesToDownload);
                db->getXPIRDescriptor()->setXPIRclientDir(XPIRdir);

                if (verbose) cout << "Generating and saving X file to: " << XfileName << endl;
                vector<size_t>* RandomlySelectedVectors = new vector<size_t>();
                cn.generateAndSaveXtoFile(XfileName, RandomlySelectedVectors);
                //getting and saving XDB
                if (verbose) cout << "Saving XDB file to: " << XDBfileName << endl;
                cn.getAndSaveXDBtoFile(XDBfileName, RandomlySelectedVectors);
                //getting and saving DB updates     //TODO better check db version. if x.0.x no need to request db updates


                if (verbose) cout << "Saving db updates to: " << DBupdatesFileName << endl;
                cn.getAndSaveDBupdatesToFile(DBupdatesFileName);
                //saving DB info
                if (verbose) cout << "Saving db info to: " << dbInfoName << endl;
                db->saveDBinfoToFile(dbInfoName);
            } else if (localVersion.getV2() < serverVersion.getV2()) {
                //redownload DB info
                cn.getDBInfoFromServer();
                db = cn.getDB();
                if (verbose) cout << "Saving db info to: " << dbInfoName << endl;
                db->saveDBinfoToFile(dbInfoName);
                //redownload DB updates
                if (verbose) cout << "Saving db updates to: " << DBupdatesFileName << endl;
                cn.getAndSaveDBupdatesToFile(DBupdatesFileName);
            } else if (localVersion.getV3() < serverVersion.getV3()) {
                //redownload DB info
                cn.getDBInfoFromServer();
                db = cn.getDB();
                if (verbose) cout << "Saving db info to: " << dbInfoName << endl;
                db->saveDBinfoToFile(dbInfoName);
            }
        } else {
            cn.setServerInfo(ip, port);
            cout << "initiating Connection..." << endl;
            cn.initiateConnection();
            cout << "Connected to the server: " << cn.getServerIP() << ":" << cn.getServerPort() << endl;
            //getting db info
            cout << "Getting db info from the main server..." << endl;
            cn.getDBInfoFromServer();
            db = cn.getDB();
            db->getXPIRDescriptor()->setNumberOfFiles(filesToDownload);
            db->getXPIRDescriptor()->setXPIRclientDir(XPIRdir);
            
            if (verbose) cout << "Generating and saving X file to: " << XfileName << endl;
            vector<size_t>* RandomlySelectedVectors = new vector<size_t>();;
            cn.generateAndSaveXtoFile(XfileName, RandomlySelectedVectors);
            //getting and saving XDB
            if (verbose) cout << "Saving XDB file to: " << XDBfileName << endl;
            cn.getAndSaveXDBtoFile(XDBfileName, RandomlySelectedVectors);
            //getting and saving DB updates     //TODO better check db version. if x.0.x no need to request db updates
            
            
            if (verbose) cout << "Saving db updates to: " << DBupdatesFileName << endl;
            cn.getAndSaveDBupdatesToFile(DBupdatesFileName);
            //saving DB info
            if (verbose) cout << "Saving db info to: " << dbInfoName << endl;
            db->saveDBinfoToFile(dbInfoName);
        }

//        cout << "***inside main save db updates NumberOfFileInDBupdates="<<db->getDBupdates()->getNumberOfFileInDBupdates()<<endl; 

        if (!result.count("noCatalogue")) {
            cout << "Files on the server:" << endl;
            db->displayDBcatalog();
        }

        if (requestedFile == -1){
            cout << " file to request:";
            cin >> requestedFile;
            cout << "-----selected file:" << requestedFile << "/" << db->getFilesNames()->size() << endl;
        }

        if(requestedFile >= db->getFilesNames()->size() + db->getDBupdates()->getNumberOfFileInDBupdates()) {
            cout << "Error: no file with such index" << endl;
            return 0;
        } else if (requestedFile >= db->getFilesNames()->size()) {  //file exists in the downloaded updates
            cout << "The file " << db->getDBupdates()->getDBupdatesfilesNamesList()->at(requestedFile-db->getFilesNames()->size()) << " has been already downloaded from the server." << endl;
            return 0;
        }
        
        chrono::time_point<chrono::system_clock> start, end, t1, t2, t3, t4, t5, t6, t7, t8;
        start = chrono::system_clock::now();

        chrono::duration<double> elapsed_seconds = start-start;
        chrono::duration<double> selectKRandomXTime = start-start;
        chrono::duration<double> calculateWTime = start-start;
        chrono::duration<double> calculateSTime = start-start;
        chrono::duration<double> getNsetSDBTime = start-start;
        chrono::duration<double> calculateDataTime = start-start;
        chrono::duration<double> writeDataToFileTime = start-start;

        t1 = chrono::system_clock::now();
        if (k==0) k=db->getXPIRDescriptor()->getNumberOfFiles()/2;
        if (outputFile.compare("receivedfile") == 0) 
            outputFile = db->getFilesNames()->at(requestedFile);
        Query query(k, outputFile, db, requestedFile, verbose);
        cout << "New query: file number:" << requestedFile << endl;
        
            if (verbose) cout << "Selecting K Random Xs..." << endl;
            query.selectKRandomX();
            t2 = chrono::system_clock::now();
            selectKRandomXTime = t2-t1;

            t1 = chrono::system_clock::now();
            if (verbose) cout << "Calculating W..." << endl;
            if (!result.count("simulation"))
                query.calculateW();
            else
                query.generateRandomWforSimulation();

            t2 = chrono::system_clock::now();
            calculateWTime = t2-t1;

            t1 = chrono::system_clock::now();
            if (verbose) cout << "Calculating S..." << endl;
            //if (!result.count("simulation"))    //TODO remove
            query.calculateS();
            char* s = query.getS();
            t2 = chrono::system_clock::now();
            calculateSTime = t2-t1;
            //TO be removed******************************************************
            /*cout << hex;
            if (db->getP()==1)
                coutS(s,db->getP(),db->getM()/8);
            else
                coutS(s,db->getP(),db->getM()*db->getP()/8);*/
            /*cout << "s : ";
            for (int i=0; i<s->size(); i++)
                cout << hex << s->at(i) << ",";
            cout << endl;*/
            //TO be removed******************************************************

            t1 = chrono::system_clock::now();
            cout << "Sending query to the server..." << endl;
            cn.sendQueryToServer(s);
    //        if (verbose) cout << "query=" << s << endl;
            cout << "Waiting for SDB from the server..." << endl;
            char* sdb = cn.getSdb();
    //        if (verbose) cout << "sdb=" << sdb << endl;
            query.setSDB(sdb);
            t2 = chrono::system_clock::now();
            getNsetSDBTime = t2-t1;
            //TO be removed******************************************************
            //coutS(sdb,db->getP(),db->getN());
            //cout << "received SDB : ";
            //for (int i=0; i<sdb->size(); i++)
            //    cout << hex << sdb->at(i) << ",";
            //cout << endl;
            //TO be removed******************************************************

            t1 = chrono::system_clock::now();
            cout << "Calculating Data..." << endl;
            query.calculateData();
            t2 = chrono::system_clock::now();
            calculateDataTime = t2-t1;

            t1 = chrono::system_clock::now();
            cout << "Writing the data to the file\"" << outputFile << "\"" << endl;

            query.writeDataToFile();
            t2 = chrono::system_clock::now();
            writeDataToFileTime = t2-t1;

            end = chrono::system_clock::now();
            elapsed_seconds = end-start;

            ofstream ofs;
            ofs.open ("ClientStats.csv", ofstream::out | ofstream::app);
            ofs << elapsed_seconds.count() << ",";
            ofs << selectKRandomXTime.count() << ",";
            ofs << calculateWTime.count() << ",";
            ofs << calculateSTime.count() << ",";
            ofs << getNsetSDBTime.count() << ",";
            ofs << calculateDataTime.count() << ",";
            ofs << outputFile << ",";
            ofs << writeDataToFileTime.count() << endl;
            ofs.close();

            cout << "Query served in : " << elapsed_seconds.count() << "s" << endl;
            if (verbose) cout << "selectKRandomXTime : " << selectKRandomXTime.count() << "s" << endl;
    //        if (verbose) 
                cout << "calculateWTime : " << calculateWTime.count() << "s" << endl;
            if (verbose) cout << "calculateSTime : " << calculateSTime.count() << "s" << endl;
            if (verbose) cout << "getNsetSDBTime : " << getNsetSDBTime.count() << "s" << endl;
            if (verbose) cout << "calculateDataTime : " << calculateDataTime.count() << "s" << endl;
            if (verbose) cout << "writeDataToFileTime : " << writeDataToFileTime.count() << "s" << endl;
            cout << "Query size : " << db->getM()/8 << "B" << endl;


            cn.closeConnection();
            cout << "Connection closed" << endl;

    
    }
    catch (const cxxopts::OptionException& e)
    {
      cout << "error parsing options: " << e.what() << endl;
      exit(1);
    }
    
    return 0;
}

