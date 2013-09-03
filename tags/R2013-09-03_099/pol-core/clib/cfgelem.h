/*
History
=======

Notes
=======

*/

#ifndef CLIB_CFGELEM_H
#define CLIB_CFGELEM_H

#include "maputil.h"

class ConfigProperty
{
public:
    ConfigProperty();
	ConfigProperty( const char *name, const char *value );
    ConfigProperty( const std::string& name, const std::string& value );
    ConfigProperty( std::string* pname, std::string* pvalue );

	~ConfigProperty();
protected:
    std::string name_;
    std::string value_;
	friend class ConfigElem;
    friend class VectorConfigElem;
};

class ConfigSource;

class ConfigElemBase
{
public:
    bool type_is( const char* name ) const;
    const char *type() const;
	const char *rest() const;
protected:
    ConfigElemBase();
protected:
    std::string type_;
    std::string rest_;

    const ConfigSource* _source;
};

class ConfigElem : public ConfigElemBase
{
public:
	ConfigElem();
	~ConfigElem();

    friend class ConfigFile;

    bool has_prop( const char* propname ) const;

    std::string remove_string( const char *propname );
    std::string remove_string( const char *propname, const char *dflt );

    unsigned short remove_ushort( const char *propname );
    unsigned short remove_ushort( const char *propname, unsigned short dflt );

    int remove_int( const char *propname );
    int remove_int( const char *propname, int dflt );

    unsigned remove_unsigned( const char *propname );
    unsigned remove_unsigned( const char *propname, int dflt );

    unsigned int remove_ulong( const char *propname );
    unsigned int remove_ulong( const char *propname, unsigned int dflt );

    bool remove_bool( const char *propname );
    bool remove_bool( const char *propname, bool dflt );

    float remove_float( const char *propname, float dflt );
    double remove_double( const char *propname, double dflt );

    void clear_prop( const char *propname );

    bool remove_first_prop( std::string* propname, std::string* value );
    bool remove_prop( const char *propname, std::string* value );
	bool remove_prop( const char *propname, unsigned int *plong );
	bool remove_prop( const char *propname, unsigned short *pushort );

    bool read_prop( const char *propname, std::string* value ) const;
    
    // get_prop calls: don't remove, and throw if not found.
    void get_prop( const char *propname, unsigned int* plong ) const;
    
    std::string read_string( const char *propname ) const;
    std::string read_string( const char *propname, const char *dflt ) const;


	void add_prop( const char *propname, const char *str );
	void add_prop( const char *propname, unsigned int lval );
	void add_prop( const char *propname, unsigned short sval );
	void add_prop( const char *propname, short sval );

    void throw_error( const std::string& errmsg ) const;
    void warn( const std::string& errmsg ) const;
	void warn_with_line( const std::string& errmsg ) const;

    void set_rest( const char* newrest );
	void set_type( const char* newtype );
    void set_source( const ConfigElem& elem );
    void set_source( const ConfigSource* source );
protected:
    void prop_not_found( const char *propname ) const;
    typedef std::multimap<std::string,std::string,ci_cmp_pred> Props;
    Props properties;
};

class VectorConfigElem : public ConfigElemBase
{
public:
	VectorConfigElem();
	~VectorConfigElem();

    friend class ConfigFile;
    bool type_is( const char* name ) const;
    const char *type() const;
	const char *rest() const;

    bool has_prop( const char* propname ) const;

    std::string remove_string( const char *propname );
    std::string remove_string( const char *propname, const char *dflt );

    unsigned short remove_ushort( const char *propname );
    unsigned short remove_ushort( const char *propname, unsigned short dflt );

    int remove_int( const char *propname );
    int remove_int( const char *propname, int dflt );

    unsigned int remove_ulong( const char *propname );
    unsigned int remove_ulong( const char *propname, unsigned int dflt );

    bool remove_bool( const char *propname );
    bool remove_bool( const char *propname, bool dflt );

    float remove_float( const char *propname, float dflt );
    double remove_double( const char *propname, double dflt );

    void clear_prop( const char *propname );

    bool remove_first_prop( std::string* propname, std::string* value );
    bool remove_prop( const char *propname, std::string* value );
	bool remove_prop( const char *propname, unsigned int *plong );
	bool remove_prop( const char *propname, unsigned short *pushort );

    bool read_prop( const char *propname, std::string* value ) const;
    std::string read_string( const char *propname ) const;
    std::string read_string( const char *propname, const char *dflt ) const;


	void add_prop( const char *propname, const char *str );
	void add_prop( const char *propname, unsigned int lval );
	void add_prop( const char *propname, unsigned short sval );

    void throw_error( const std::string& errmsg ) const;
    void warn( const std::string& errmsg ) const;

    void set_rest( const char* newrest );
    void set_source( const ConfigElem& elem );
protected:
    void prop_not_found( const char *propname ) const;
    std::string type_;
    std::string rest_;

    typedef std::vector<ConfigProperty*> Props;
    Props properties;

    ConfigSource* source_;
};

#endif
