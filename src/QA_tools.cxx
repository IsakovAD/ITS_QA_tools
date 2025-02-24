#include "QA_tools.h"
#include <filesystem>
#include <TText.h>
void QA_analysis::parse_parameters(const string &json_file_path) {

  auto json_file = parseJSON(json_file_path);
  if (!json_file.empty()) {
    const auto &params = json_file[0];
    data_path = params.at("run_list");
    Data1Type = params.at("DataType_old");
    Data2Type = params.at("DataType_new");
    Data1Pass = params.at("DataPass_old");
    Data2Pass = params.at("DataPass_new");

    PeriodName1 = params.at("MCPeriod_old");
    PeriodName2 = params.at("MCPeriod_new");

    //[TO-DO] error checks on wrong json format;
    //cout<<"Starting Analysis with: data_path= "<<data_path << " Data Type old: "<< Data1Type<< " Data Type new: "<< Data2Type << " Pass old: "<< Data1Pass << " Pass new: "<<Data2Pass << " MC period old: "<< PeriodName1 << " MC period new: "<< PeriodName2 <<endl; 
  } else {
    cout << "[ERROR] can't open .json with parameters" << endl;
    exit(1);
  }
}

void QA_analysis::setStyle() const {
  gStyle->SetLineScalePS(1);
  gStyle->SetGridStyle(1);
  gStyle->SetGridColor(17);
  gStyle->SetHatchesLineWidth(1);
  gStyle->SetPaintTextFormat("0.1f");
}

vector<string> QA_analysis::getRuns(const string &path) const {

  std::ifstream infile(path);

  vector<string> out;
  string str;
  if (!infile.is_open()) {
    cout << "[ERROR] wrong path to the file with the runs: " << path << endl;
    exit(0);
  }

  while (std::getline(infile, str)) {
     try {
        int run = stoi(str.substr(0, 6));
     } catch(...){
	cout<<"################################### something wrong during conversion string: "<< str << "to 6-digit int due to"<<endl;
    	continue;
     } 
     out.emplace_back(str.substr(0, 6));
/*
    if (str.size() >= 6)
      out.emplace_back(str.substr(0, 6));

    else {
      cout << "[ERROR] wrong line in the run list: " << str << " skipping!"
           << endl;
    }
*/    
  }
  return out;
}

vector<QA_object> QA_analysis::readObjects(const string &file_name) {

  //[to-do] stoi can throw exception
  vector<QA_object> vOut;

  auto object_json_file = parseJSON(file_name);

  for (const auto &object : object_json_file) {
    QA_object current_object;
    for (const auto &[key, item] : object) {
      if (key == "Path")
        current_object.Name = item;
      else if (key == "Task")
        current_object.Task = item;
      else if (key == "ObjectType")
        current_object.ObjectType = item;
      else if (key == "isEnabled")
        current_object.isEnabled = stoi(item);
      else if (key == "isDoROF_norm")
        current_object.isDoROF_norm = stoi(item);
      else if (key == "isLogy")
        current_object.isLogy = stoi(item);
      else if (key == "isLogx")
        current_object.isLogx = stoi(item);
      else
        cout << "[ERROR] Wrong item in the input object list: " << key << ":"
             << item << endl;
    }
    vOut.emplace_back(current_object);
  }
  return vOut;
}

TH2D *QA_analysis::produceAverageClusterPlot(const QAServer &server,
                                             const TString &run,
                                             QA_object object) {

  TString in_name = object.Name;
  TH2D *obj = new TH2D("obj", in_name.Data(), 24, -12, 12, 14, 0, 14);
  obj->SetStats(0);

  QA_object temp;
  temp.Task = object.Task;
  temp.ObjectType = "TH2";
  string ClusterObjectName = in_name.ReplaceAll("avg_", "").Data();

  for (int iLayer = 0; iLayer < NLayer; iLayer++) {

    temp.Name = Form("Layer%d/%s", iLayer, ClusterObjectName.c_str());

    TH2D *hClusterLayer = (TH2D *)server.downloadObject(run.Data(), temp);

    if (!hClusterLayer) {
      cout << "[ERROR] can't open cluster object for average plot for Layer: "
           << iLayer << endl;
      return nullptr;
    }

    for (int iStave = 0; iStave < mNStaves[iLayer]; iStave++) {

      int ybin = iStave < (mNStaves[iLayer] / 2) ? 7 + iLayer + 1 : 7 - iLayer;
      int xbin =
          12 - mNStaves[iLayer] / 4 + 1 + (iStave % (mNStaves[iLayer] / 2));
      double sum = 0;
      for (int ix = 1; ix <= nBinsX[iLayer]; ix++) {
        sum += hClusterLayer->GetBinContent(ix, iStave + 1);
      }
      obj->SetBinContent(xbin, ybin, sum / hClusterLayer->GetNbinsX());
    }
  }

  return obj;
}

void QA_analysis::setMinMax(TH1 *obj1, TH1 *obj2) {

  if (!obj1 || !obj2) {
    std::cout << "[ERROR] Null pointer passed to setMinMax! histograms remain "
                 "unchanged"
              << std::endl;
    return;
  }

  Double_t max = std::max(obj1->GetMaximum(), obj2->GetMaximum());
  Double_t min = std::min(obj1->GetMinimum(), obj2->GetMinimum());

  if (obj1->InheritsFrom("TH2")) {
    obj1->GetZaxis()->SetRangeUser(min, max * 1.1);
    obj2->GetZaxis()->SetRangeUser(min, max * 1.1);
  } else {
    obj1->GetYaxis()->SetRangeUser(min, max * 1.1);
    obj2->GetYaxis()->SetRangeUser(min, max * 1.1);
  }
}

void QA_analysis::FormatHisto(TVirtualPad *c1, TH1 *obj,
                              const QA_object &object, const string &run,
                              const TString apass, const long hROFs) {

  if (!obj) {
    std::cout << "[ERROR] Null pointer passed to FormatHisto! histograms "
                 "remain unchanged"
              << std::endl;
    return;
  }

  obj->SetTitle(Form("Run%s %s ", run.c_str(),
                     apass.Length() < 2 ? "online" : apass.Data()));
  	  if ( object.Name.find("VertexZ") != string::npos) obj->Rebin(100);


          if ( object.isDoROF_norm ) obj->Scale(1./hROFs);
          if ( object.isLogy) c1->SetLogy();
          if ( object.isLogx) c1->SetLogx();
          if ( !obj->InheritsFrom("TH2")) c1->SetGridy();
  
}

string QA_analysis::TokenizePath(const string &input, const char &token) {

  string out = input;
  while (out.find(token) != std::string::npos) {
    out = out.substr(out.find(token) + 1);
  }
  return out;
}

void QA_analysis::performRatio(TVirtualPad *c1, TH1 *obj_new, TH1 *obj_old,
                               QA_object object, TString pass_new,
                               TString pass_old) {

  if (!obj_new || !obj_old) {
    std::cout << "[ERROR] Null pointer passed to performRatio! ratio "
                 "histograms will not be calculated"
              << std::endl;
    return;
  }

  TH1 *obj_ratio;
  if (obj_old->InheritsFrom("TH2"))
    obj_ratio = (TH2 *)obj_old->Clone("ratio");
  else
    obj_ratio = (TH1 *)obj_old->Clone("ratio");

  for (int ix = 1; ix <= obj_new->GetNbinsX(); ix++)
    for (int iy = 1; iy <= obj_new->GetNbinsY(); iy++) {
      if (obj_new->GetBinContent(ix, iy) == 0)
        obj_ratio->SetBinContent(ix, iy, -0.01);
      else
        obj_ratio->SetBinContent(ix, iy,
                                 obj_ratio->GetBinContent(ix, iy) /
                                     obj_new->GetBinContent(ix, iy));
    }

  obj_ratio->Scale(obj_new->Integral() / obj_old->Integral());
  if (!obj_old->InheritsFrom("TH2")) {
    c1->SetGridy();
  }

  string obj_title = TokenizePath(object.Name, '/');

  obj_ratio->SetTitle(Form("%s: %s / %s", obj_title.c_str(),
                           pass_old.Length() < 2 ? "online" : pass_old.Data(),
                           pass_new.Length() < 2 ? "online" : pass_new.Data()));
  obj_ratio->SetStats(0);

  obj_ratio->Draw("text,colz");
}

void QA_analysis::PlotHisto(TVirtualPad *c1, TH1 *obj) {

   c1->cd();	
  if (!obj) {
    std::cout << "[ERROR] Null pointer passed to PlotHisto! histograms will "
                 "not be plotted"
              << std::endl;
    TText* errorText = new TText(0.5, 0.5, "ERROR WHEN READING OBJECT");
    errorText->SetTextAlign(22); // Center alignment (horizontal and vertical)
    errorText->SetTextSize(0.05); // Adjust text size as needed
    errorText->SetTextFont(42); // Standard ROOT font
    errorText->SetTextColor(kRed); // Red color for visibility

    // Draw the text on the pad
    errorText->Draw();	



    return;
  }

  if (obj->InheritsFrom("TH2")) {
    obj->Draw("colz");
  } else {
    obj->Draw("hist");
  }
}

int QA_analysis::StartQA() {

  TString outname = ((string)data_path).substr(0, data_path.size() - 4);
  string folder_name = "output/"+(string) outname;

  if (!std::filesystem::exists(folder_name))
    std::filesystem::create_directory(folder_name);

  std::ofstream log_file(folder_name + "/output.log", ofstream::out);
  std::streambuf *original_cout_buffer = std::cout.rdbuf();
  std::cout.rdbuf(log_file.rdbuf());

  QAServer server_new(Data2Type, Data2Pass, PeriodName2);
  QAServer server_old(Data1Type, Data1Pass, PeriodName1);

  vector<string> runs = getRuns("input/"+data_path);

  vector<QA_object> vObjects_old =
      readObjects(Form("input/objects_%s.json", Data1Type.c_str()));
  vector<QA_object> vObjects_new =
      readObjects(Form("input/objects_%s.json", Data2Type.c_str()));

  //[TO-DO] make class for TCanvass
  int nObjects = 0, nCurrentPosition = 0, nRows = 10;
  TCanvas *c1 = new TCanvas("c1", "c1", 0, 0, 1440, nRows * 480);
  c1->Divide(3, nRows);
  for (int i = 1; i <= 3 * nRows; i++)
    c1->cd(i)->SetRightMargin(0.15);

  for (string run : runs) {

    cout << " ------------- run is " << run << endl;

    long nROFs_old = server_old.getNROFs(run);
    long nROFs_new = server_new.getNROFs(run);


    for (QA_object object_new : vObjects_new) {

      if (!object_new.isEnabled)    continue;

      QA_object object_old;
      const auto it = find_if(vObjects_old.begin(), vObjects_old.end(),
                              [&object_new](const QA_object &obj) {
                                return object_new.Name == obj.Name;
                              });
      if (it != vObjects_old.end()) object_old = *it;
      

      TH1 *obj_old, *obj_new;
      if (object_new.Name.find("avg") != string::npos) {
        obj_old = produceAverageClusterPlot(server_old, run, object_old);
        obj_new = produceAverageClusterPlot(server_new, run, object_new);
      } else {
        obj_old = server_old.downloadObject(run, object_old);
        obj_new = server_new.downloadObject(run, object_new);
      }

      FormatHisto(c1->cd(nCurrentPosition * 3 + 2), obj_new, object_new, run, server_new.getApass(), nROFs_new);
      FormatHisto(c1->cd(nCurrentPosition * 3 + 1), obj_old, object_old, run, server_old.getApass(), nROFs_old);
      setMinMax(obj_old, obj_new);

      PlotHisto(c1->cd(nCurrentPosition * 3 + 2), obj_new);
      PlotHisto(c1->cd(nCurrentPosition * 3 + 1), obj_old);

      performRatio(c1->cd(nCurrentPosition * 3 + 3), obj_new, obj_old, object_new, server_new.getApass(), server_old.getApass());



      //------------- operation with Canvases:
      c1->SetTitle(Form("Run %s, %s", run.c_str(), object_new.Name.c_str()));
      if (((nObjects + 1) % nRows == 0) || (nObjects == vObjects_new.size() * runs.size() - 1)) {
        if ((nObjects + 1) == nRows) {
          c1->Print(Form("%s/%s.pdf(", folder_name.c_str(), outname.Data()), "pdf");
        } else if (nObjects == vObjects_new.size() * runs.size() - 1) continue;
              else c1->Print(Form("%s/%s.pdf", folder_name.c_str(), outname.Data()), "pdf");
        
        nCurrentPosition = 0;
        c1->Clear();
        c1->Divide(3, nRows);
        for (int i = 1; i <= 3 * nRows; i++)
          c1->cd(i)->SetRightMargin(0.15);

      } else {
        nCurrentPosition++;
      }

      nObjects++;
    }
  }

  c1->Print(Form("%s/%s.pdf)", folder_name.c_str(), outname.Data()), "pdf");

  std::ofstream outfile(
      Form("%s/%s_out.txt", folder_name.c_str(), outname.Data()));
  for (const string &run : runs) {

    outfile << run << endl;
  }

  std::cout.rdbuf(original_cout_buffer);
  return 1;
}
