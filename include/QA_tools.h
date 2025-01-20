#pragma once

#include"simple_json_parser.h"

#include <TCanvas.h>
#include <TStyle.h>
#include "QAServer.h"
#include "QA_object.h"

#include<TH2.h>
#include<TH1.h>



class QA_analysis{

	public:
		QA_analysis(const string& json_file_path){
			parse_parameters(json_file_path);
			setStyle();
		}
		
		int StartQA();

	private:

		static const int NLayer = 7;
  		const int mNStaves[NLayer] = { 12, 16, 20, 24, 30, 42, 48 };
  		const int nBinsX[NLayer] = {9,9,9,16,16,28,28};
        	string data_path, Data1Type, Data2Type, Data1Pass, Data2Pass, PeriodName1, PeriodName2;

		void parse_parameters(const string& json_file_path);
		void setStyle() const;		

		vector <string> getRuns (const string& path) const;
		vector<QA_object> readObjects(const string& file_name);
		void setMinMax(TH1* obj1, TH1* obj2);
		void FormatHisto(TVirtualPad* c1, TH1* obj, const QA_object& object, const string& run, const TString apass, const long hROFs);
		string TokenizePath(const string& input, const char& token);

		TH2D* produceAverageClusterPlot(const QAServer& server, const TString& run, QA_object object );
		void performRatio(TVirtualPad* c1, TH1* obj_new, TH1* obj_old, QA_object object, TString pass_new, TString pass_old);
		void PlotHisto(TVirtualPad* c1, TH1* obj);


};


