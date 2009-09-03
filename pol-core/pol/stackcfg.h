#ifndef STACKCFG_H
#define STACKCFG_H

#include <set>
#include <string>

typedef std::set<std::string> PropSet;
extern PropSet Global_Ignore_CProps;

void load_stacking_cfg();

#endif

