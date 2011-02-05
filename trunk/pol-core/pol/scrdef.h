/*
History
=======


Notes
=======

*/

#ifndef SCRDEF_H
#define SCRDEF_H

#include <string>

class Package;

class ScriptDef
{
public:
    ScriptDef() : name_(""), pkg_(NULL) {}
    ScriptDef( const std::string& name, const Package* pkg, const char* mainpfx );
    ScriptDef( const std::string& name, const Package* pkg );
    // ScriptDef( const ScriptDef& ); default copy ctor is okay
    // ScriptDef& operator=( const ScriptDef& ); default operator= is okay

    bool operator==( const ScriptDef& other ) const;
    void config( const std::string& name, 
                 const Package* pkg, 
                 const char* mainpfx,
                 bool warn_if_not_found = true );
    void config( const std::string& name, 
                 const Package* pkg );
    bool config_nodie( const std::string& name, 
                       const Package* pkg, 
                       const char* mainpfx );
    void quickconfig( const Package* pkg, const std::string& name_ecl );
    void quickconfig( const std::string& name_ecl );

    bool empty() const { return name_.empty(); }
    const char* c_str() const { return name_.c_str(); }
    void clear();
    
    const std::string& name() const { return name_; }
    const Package* pkg() const { return pkg_; }

    bool exists() const;

    std::string relativename( const Package* pkg = NULL ) const;

private:
    std::string localname_;
    std::string name_;
    const Package* pkg_;

private: // not implemented
};


#endif
