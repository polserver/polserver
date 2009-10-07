/*
History
=======
2005/06/01 Shinigami: Added Walking_Mounted and Running_Mounted movecosts
2007/08/20 Shinigami: fixed bug in load_movecost() - "Walking_Mounted" was ignored

Notes
=======

*/


#include "../clib/stl_inc.h"

#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/fileutil.h"
#include "../clib/stlutil.h"
#include "../clib/strutil.h"

#include "mobile/charactr.h"
#include "movecost.h"
#include "polcfg.h"
#include "ssopt.h"

#define MAX_CARRY_PERC 200

struct MovementCost
{
	double cost[ MAX_CARRY_PERC+1 ];
	double over;
};

MovementCost movecost_walking;
MovementCost movecost_running;
MovementCost movecost_walking_mounted;
MovementCost movecost_running_mounted;

//dave messed with this function 1/26/3 - remove_first_prop was reading carrying percentages in lexographic order, not 
//numerical order so it would interpolate first 1 to 100, then to 25, etc. costs were processed in this kind of 
// order: 1,10,100,2,3,4,...
// So first read into a Map<percentage,cost>, and build from that.
void read_movecost( ConfigElem& elem, MovementCost& movecost )
{
	std::map<int,double> Costs;
	std::map<int,double>::iterator itr;

	movecost.over = elem.remove_double( "over", 16.0 );
	for( int i = 0; i <= MAX_CARRY_PERC; ++i )
	{
		movecost.cost[i] = movecost.over;
	}

	int last_perc = -1;
	double last_mult = 0.0;
	string perc_str, mult_str;
	int perc;
	double mult;
	while (elem.remove_first_prop( &perc_str, &mult_str ))
	{
		ISTRINGSTREAM is1( perc_str );
		ISTRINGSTREAM is2( mult_str );
		if ( (is1 >> perc) && (is2 >> mult) )
		{
			if (perc > MAX_CARRY_PERC)
			{
				elem.throw_error( "Max carrying capacity percentage is " + decint(MAX_CARRY_PERC) );
			}
			Costs.insert(pair<int,double>(perc,mult));
		}
		else
		{
			elem.throw_error( "poorly formed movecost spec: " + perc_str + " " + mult_str );
		}
	}

	for(itr = Costs.begin(); itr != Costs.end(); ++itr)
	{
		perc = itr->first;
		mult = itr->second;
		for( int i = last_perc+1; i <= perc; ++i )
		{
			double m = ( (last_mult-mult)/(last_perc-perc) );
			double b = mult - m * perc;

			movecost.cost[i] = i * m + b;
   
				//last_end + (mult - last_mult) * 
		}
		last_perc = perc;
		last_mult = mult;
	}
}

unsigned short movecost( const Character* chr, int carry_perc, bool running, bool mounted )
{
	MovementCost* mc;
	double costmod;
	if (mounted)
	{
		mc = running ? &movecost_running_mounted : &movecost_walking_mounted;
		costmod = running ? chr->movement_cost.run_mounted : chr->movement_cost.walk_mounted;
	}
	else
	{
		mc = running ? &movecost_running : &movecost_walking;
		costmod = running ? chr->movement_cost.run : chr->movement_cost.walk;
	}
	
	double mult;
	if (carry_perc < 0)
		mult = mc->cost[0];
	else if (carry_perc > MAX_CARRY_PERC)
		mult = mc->cost[MAX_CARRY_PERC];
	else
		mult = mc->cost[ carry_perc ];

	return static_cast<unsigned short>(8 * mult * costmod);
}

void load_movecost( bool reload )
{
	if ( !ssopt.movement_uses_stamina )
		return;
	else if ( !FileExists("config/movecost.cfg") )
	{
		if ( !reload && config.loglevel > 0 )
			cout << "File config/movecost.cfg not found, skipping.\n";
		return;
	}

	ConfigFile cf("config/movecost.cfg", "MovementCost Walking Running Walking_Mounted Running_Mounted");
	ConfigElem elem;
		
	bool walking_mounted_set = false;
	bool running_mounted_set = false;
		
	while ( cf.read(elem) )
	{
		if ( elem.type_is("MovementCost") )
		{
			read_movecost(elem, movecost_running);
			memcpy(&movecost_walking, &movecost_running, sizeof movecost_walking);
		}
		else if ( elem.type_is("Walking") )
		{
			read_movecost(elem, movecost_walking);
		}
		else if ( elem.type_is("Running") )
		{
			read_movecost(elem, movecost_running);
		}
		else if ( elem.type_is("Walking_Mounted") )
		{
			read_movecost(elem, movecost_walking_mounted);
			walking_mounted_set = true;
		}
		else if ( elem.type_is("Running_Mounted") )
		{
			read_movecost(elem, movecost_running_mounted);
			running_mounted_set = true;
		}
	}

	if ( !walking_mounted_set )
		memcpy(&movecost_walking_mounted, &movecost_walking, sizeof movecost_walking);
	if ( !running_mounted_set )
		memcpy(&movecost_running_mounted, &movecost_running, sizeof movecost_running);
}
