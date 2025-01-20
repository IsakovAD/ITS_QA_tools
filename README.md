
# Intro:

This repository contains tools for the QA of asynchronous/online data of the ITS detector.

# Structure:

The structure is as follows:
- `src`: Source files.
- `include`: Header files.
- `scripts`: Scripts for analysis. Please, execute the macro from this folder.
- `input`: Contains `.json` files for analysis configuration and `.txt` files with the run lists:
  + `analysis_params.json`: Settings for the analysis; please update it before starting a new analysis.
  + `objects_aQC.json`, `objects_Data.json`, `objects_MC.json`: Lists of QC MO objects used for QA comparison. More details are provided in the corresponding section.
  + `runlist_name.txt`: A file with the list of runs to be used for the analysis. New file can be create for new analysis. Please reference this file in `analysis_params.json`. Expected format: each line starts with a 6-digit run number, followed by any additional symbols.
- `output`: Results of the analysis, saved in folders for each new `runlist_name.txt` file:
  - `runlist_name.pdf`: The analysis results in PDF format.
  - `output.log`: A log file with any issues encountered during the analysis.
  - `runlist_name_out.txt`: A formatted list of analyzed runs.

# How to Run the Code:

1. Enter the Quality Control environment:
   ```
   alienv enter QualityControl/latest
   ```
2. Modify the input run list (`runlist_name.txt`) and adjust the analysis settings in `analysis_params.json`.
3. Run the code with:
   ```
   root -l -q scripts/do_analysis.C
   ```
4. The results will be saved in the `output/runlist_name` folder.

# Definition of `analysis_params.json`:

```json
{
  "run_list": "runs_O2_5665_LHC24as.txt",  // Path to the input file with the run list, located in the `./input` folder
  "DataType_old": "aQC",                    // Type of the "old" analysis: aQC, Data, MC
  "DataType_new": "aQC",                    // Type of the "new" analysis: aQC, Data, MC
  "DataPass_old": "cpass0",                 // Pass of the "old" analysis. Examples: "cpass0", "apass1", "apass3", etc. For "Data" type, leave it empty (" ")
  "DataPass_new": "apass1",                 // Pass of the "new" analysis. Examples: "cpass0", "apass1", "apass3", etc. For "Data" type, leave it empty (" ")
  "MCPeriod_old": " ",                      // Period to which the MC data was anchored. Example: "LHC24af"
  "MCPeriod_new": " "
}
```

# Definition of a QA Object in `objects_XXX.json`

Fill out the following structure for each file you want to compare:

```json
{
  "isEnabled": "0",          // Whether the object is ON/OFF. Can be "0" or "1".
  "Task": "ITSClusterTask",  // Name of the QC Task.
  "Path": "General/General_Occupancy",  // Name of the MO.
  "ObjectType": "TH2",       // Type of the object.
  "isDoROF_norm": "0",       // If the object should be normalized by the number of Readout Frames.
  "isLogy": "0",             // If the Y-axis should be plotted in log scale.
  "isLogx": "0"              // If the X-axis should be plotted in log scale.
}
```
