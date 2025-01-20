#include "QAServer.h"
#include <sstream>
#include "TEfficiency.h"

using namespace std;


                long QAServer::getNROFs (const string& RunNumber) const{
                        long nRofs = -1;

                        QA_object nROfs_obj;
                        nROfs_obj.Name = "AssociatedClusterFraction"; //NROFs?????
                        if (params.dataBaseType=="qc")  nROfs_obj.Task = "ITSTrackTask";
                        else nROfs_obj.Task = "Tracks";
                        nROfs_obj.ObjectType = "TH1";

                        TH1D *hClustersPerROF = (TH1D*) this->downloadObject(RunNumber, nROfs_obj);
                        if (hClustersPerROF != NULL)  nRofs = hClustersPerROF->Integral();
                        else cout<<"[ERROR] Problem with receiving number of ROFs for run: "<< RunNumber<< " from: "<<params.ccdb_port<< " pass:"<<params.apass<< " Task: "<< nROfs_obj.Task << " Name "<< nROfs_obj.Name<<endl;
                        return nRofs;
                }




map<pair<string,string>,string> QAServer::LoadTimeStamps(const string& objectName, const string& PeriodNameForMC){

 
   map< pair<string,string>, string > time_stamps;
   string objectlist = ccdbApi.list(objectName,false,"text/plain");
   stringstream ss(objectlist);
   string word, timestamp, runnumber, pass, periodName;

   if (isVerbose>1) std::cout<<" object name to get timestamps: " << objectName <<endl;
 
   bool isRunCorrect = false, isPassCorrect = false;
 
   while(ss>>word){

      if(word=="Validity:"){// take the one related to file creation
	
	if (PerioidNameForMC.size()>1 && periodName!= PerioidNameForMC) continue;

       if (isVerbose>2) cout<< " found new timesamp for obj"<< objectName<< " run: " <<  runnumber<< " pass:" << pass << "peroid:"<<periodName<<" MC period" << PerioidNameForMC <<" timestamp: "<<timestamp<<endl;

       if (time_stamps[std::make_pair(runnumber,pass)].size()==0)
         time_stamps[std::make_pair(runnumber,pass)] = timestamp;
       else if (stol(timestamp) > stol(time_stamps[std::make_pair(runnumber,pass)])) { time_stamps[std::make_pair(runnumber,pass)] = timestamp;}
       


         ss>>word;
         timestamp = word;
      }
    if (word=="RunNumber"){
       ss>>word;
       ss>>word;
       runnumber = word;
    }
    
    if (word=="PeriodName" && params.dataBaseType!="qc" ){
       ss>>word;
       ss>>word;
       periodName = word;

    }
    if (word=="PassName" && params.dataBaseType!="qc"){
       ss>>word;
       ss>>word;
       pass = word;
    }
     

  }

  return time_stamps;

}


TH1* QAServer::downloadObject(const string& RunNumber, const QA_object& object) const {
    	

    string fullPath = params.dataBaseType + "/ITS/MO/"+object.Task+ "/"+object.Name;
    TH1 *out = NULL;
    std::map<std::string, std::string> metadata;
    metadata["RunNumber"]=RunNumber;
    metadata["PassName"]=params.apass;

    string timestamp = getTimeStamp(RunNumber,getTaskType(object.Task));
    if (isVerbose>1) cout<<" looking for a object with timestamp: "<<timestamp <<" run: "<< RunNumber << " path: "<< fullPath<<endl;
    if (timestamp.size() < 2) {
	    cout<<"[ERROR] Can't find correct time stamp for object: "<<object.Name << " database: "<< params.dataBaseType << " in Run: "<< RunNumber<<endl; 
	    return out;	
   }

   try {
    if ( object.ObjectType=="TH2")
         out= ccdbApi.retrieveFromTFileAny<TH2>(fullPath, metadata, stol(timestamp));
    else if  (object.ObjectType=="TEfficiency") {
         TEfficiency *hEff = ccdbApi.retrieveFromTFileAny<TEfficiency>(fullPath, metadata, stol(timestamp));

         if (hEff){
            //write function to convert TEff to TH1
            TH1 *teff_Num = (TH1*)hEff->GetPassedHistogram();
            teff_Num->Divide(hEff->GetTotalHistogram());
            for(int i=1; i<=teff_Num->GetNbinsX(); i++) {
		    teff_Num->SetBinError(i, std::max(hEff->GetEfficiencyErrorLow(i), hEff->GetEfficiencyErrorUp(i)));
	    }
            out = (TH1*) teff_Num->Clone("Efficiency");
            out->SetStats(0);
         }
       }
      else
         out = ccdbApi.retrieveFromTFileAny<TH1>(fullPath, metadata, stol(timestamp));
   } catch (const std::exception& ex){
	   cout<<"[ERROR] Can't download object"<<object.Name << " database: "<< params.dataBaseType << " in Run: "<< RunNumber<<endl;
   
   
   }

   
   return out;
}

