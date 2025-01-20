#include "QA_tools.h"



int do_main()
{

	QA_analysis my_analysis("input/analysis_params.json");
	my_analysis.StartQA();
	return 1;
}

