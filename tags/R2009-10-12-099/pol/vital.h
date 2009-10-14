/*
History
=======
2005/09/14 Shinigami: regen_while_dead implemented

Notes
=======

*/


#ifndef VITAL_H
#define VITAL_H

#include <string>
#include <vector>

class ConfigElem;
class ExportedFunction;
class Package;

class Vital
{
public:
    Vital( const Package* pkg, ConfigElem& elem );
	~Vital();

    const Package* pkg;
    std::string name;
    std::vector< std::string > aliases; // aliases[0] is always name
    unsigned vitalid;
    Vital* next;

    ExportedFunction* get_regenrate_func;
    ExportedFunction* get_maximum_func;
    ExportedFunction* underflow_func;
	bool regen_while_dead;
};

Vital* FindVital( const std::string& vitalname );
Vital* FindVital( unsigned vitalid );
void clean_vitals();
extern std::vector< Vital* > vitals;
extern unsigned numVitals;

const int VITAL_LOWEST_REGENRATE = -30000;
const int VITAL_HIGHEST_REGENRATE = 30000;

const unsigned VITAL_MIN_VALUE = 0;
const unsigned VITAL_MAX_VALUE = 100000L;

const unsigned long VITAL_MAX_HUNDREDTHS = 10000000L; // 10,000,000 hundredths = 100,000.00

const unsigned long VITAL_LOWEST_MAX_HUNDREDTHS = 100L; // 100 hundredths = 1.00
const unsigned long VITAL_HIGHEST_MAX_HUNDREDTHS = 10000000L; // 10,000,000 hundredths = 100,000.00

// max vital: 

extern const Vital* pVitalLife;
extern const Vital* pVitalStamina;
extern const Vital* pVitalMana;

#endif
