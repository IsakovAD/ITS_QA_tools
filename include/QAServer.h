#pragma once

#include<TString.h>
#include<iostream>
#include<TH1.h>
#include<TH2.h>
#include<map>
#include "CCDB/CcdbApi.h"
#include "TEfficiency.h"
#include <sstream>
#include "QA_object.h"


using namespace o2;

struct QAParameters{

   QAParameters(): ccdb_port("ali-qcdb-gpn.cern.ch:8083"), apass(string("apass1") ), dataBaseType("qc_async"), isMC(false){};

   QAParameters(const string& type, const string& apass_in ): apass(apass_in),isMC(false){

        if ( type == "MC"){
                ccdb_port  = "ali-qcdbmc-gpn.cern.ch:8083";
                dataBaseType = "qc_mc";
                isMC = true;
		AnalysisType = 2;
        }else{
                ccdb_port = "ali-qcdb-gpn.cern.ch:8083";
                if ( type== "Data"){ 
                        dataBaseType = "qc";
			AnalysisType = 1;
		}
                else if (type== "aQC") { 
			dataBaseType = "qc_async";
			AnalysisType = 0;
		}
                else {
			std::cout<<"[ERROR] Wrong Data Type! "<< type<< " is not supported, expected (MC, Data, aQC)"<<std::endl;
                        exit(1);
                }
        }
   }

 
   string dataBaseType;
   string ccdb_port;
   string apass;
   int AnalysisType;
   bool isMC = false;
};

enum Task{
	Cluster,
	Track,
	MC
};

class QAServer{

	public:
		QAServer(const string& type, const string& apass, const string& MC_PeriodName): PerioidNameForMC(MC_PeriodName){

			params = QAParameters(type, apass );
			
			try {
				ccdbApi.init(params.ccdb_port);
			} catch  (const std::exception& ex){
                                cout<<"[ERROR] Problem with intializing CCDB api with ccdb_port: "<<params.ccdb_port<<" due to " <<ex.what()<<endl;

                        }

			int AnalysisType = -1;
			/*
			if(params.dataBaseType=="qc"){
                                testName = "/ITS/MO/ITSClusterTask/General/General_Occupancy";
                        }*/
			time_stamps[Cluster]=LoadTimeStamps(params.dataBaseType + "/ITS/MO/" + taskName[params.AnalysisType][Cluster] + "/"+ testName[Cluster],MC_PeriodName);
			time_stamps[Track]=LoadTimeStamps(params.dataBaseType + "/ITS/MO/" + taskName[params.AnalysisType][Track] + "/"+ testName[Track],MC_PeriodName);
			if (params.AnalysisType == 2)
				time_stamps[MC]=LoadTimeStamps(params.dataBaseType + "/ITS/MO/" + taskName[params.AnalysisType][MC] + "/"+ testName[MC],MC_PeriodName);
		}
		string getTimeStamp(const string& run, const int& TaskType) const {

			string out ="";
			try{
				auto temp =  make_pair(run,  string(params.apass) );
				out = time_stamps[TaskType].at(  temp );
			} catch (const std::exception& ex){
				cout<<"Problem with "<<params.dataBaseType << ": cant find timestamp for Run: "<<run << " due to:  "<<ex.what()<<endl;
			        	
			}
			return out;
		}


		string getApass() const {return params.apass;}
		string getPort() const {return params.ccdb_port;}
		bool getIsMC() const {return params.isMC;}
		int getVerbosityLevel() const {return isVerbose;}

		TH1* downloadObject (const string& RunNumber, const QA_object& object) const;
		long getNROFs (const string& RunNumber) const;

	private:
		int getTaskType(const string& name) const{
		   int TaskType = -1;
			if (name.find("Cluster")!=string::npos) TaskType = 0;
		       	   else if (name.find("Mc")!=string::npos) TaskType = 2;
			else TaskType = 1;	   
		   return TaskType;
		
		
		}
		string taskName[3][3]={  {"Clusters","Tracks", ""},  { "ITSClusterTask", "ITSTrackTask", ""}, {"Clusters","Tracks", "TracksMc"}};
		//string taskName[3][3]={  {"Clusters","ITSClusterTask", "Clusters"},  { "Tracks", "ITSTrackTask", "Tracks"}, {"","", "TracksMc"}};
		o2::ccdb::CcdbApi ccdbApi;
		map<pair<string,string>,string> time_stamps[3];
		QAParameters params;
		map<pair<string,string>,string> LoadTimeStamps(const string& objectName, const string& PeriodNameForMC);
		string testName[3] = { "/General/General_Occupancy", "AngularDistribution", "efficiency_pt"};
		int isVerbose = 3;
		string PerioidNameForMC;
		
};
