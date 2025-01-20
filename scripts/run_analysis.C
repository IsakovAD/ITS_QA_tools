{
    gSystem->AddIncludePath("-I./include");

	gSystem->CompileMacro("src/QA_tools.cxx", "k");
	gSystem->CompileMacro("src/QAServer.cxx", "k");
	gSystem->CompileMacro("src/main.cxx", "k");

    	do_main(); 

}
