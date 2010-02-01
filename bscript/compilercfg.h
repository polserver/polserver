/*
History
=======

Notes
=======

*/

#ifndef BSCRIPT_COMPILERCFG_H
#define BSCRIPT_COMPILERCFG_H

struct CompilerConfig
{
    vector< string > PackageRoot;
    string IncludeDirectory;
    string ModuleDirectory;
    string PolScriptRoot;
	bool GenerateListing;
	bool GenerateDebugInfo;
	bool GenerateDebugTextInfo;
    bool DisplayWarnings;
    bool GenerateDependencyInfo;
    bool CompileAspPages;
    bool AutoCompileByDefault;
    bool UpdateOnlyOnAutoCompile;
    bool OnlyCompileUpdatedScripts;
    bool DisplaySummary;
    bool DisplayUpToDateScripts;
    bool OptimizeObjectMembers;

    void Read( const string& path );
    void SetDefaults();
};

extern CompilerConfig compilercfg;

#endif
