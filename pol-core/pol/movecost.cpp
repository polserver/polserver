/*
History
=======
2005/06/01 Shinigami: Added Walking_Mounted and Running_Mounted movecosts
2007/08/20 Shinigami: fixed bug in load_movecost() - "Walking_Mounted" was ignored

Notes
=======

*/


#include "movecost.h"

#include "mobile/charactr.h"

#include "globals/uvars.h"

#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/fileutil.h"
#include "../clib/stlutil.h"
#include "../clib/strutil.h"
#include "../clib/logfacility.h"

#include "../plib/systemstate.h"

namespace Pol {
  namespace Core {

	//dave messed with this function 1/26/3 - remove_first_prop was reading carrying percentages in lexographic order, not 
	//numerical order so it would interpolate first 1 to 100, then to 25, etc. costs were processed in this kind of 
	// order: 1,10,100,2,3,4,...
	// So first read into a Map<percentage,cost>, and build from that.
	void read_movecost( Clib::ConfigElem& elem, MovementCost& movecost )
	{
	  std::map<int, double> Costs;
	  std::map<int, double>::iterator itr;

	  movecost.over = elem.remove_double( "over", 16.0 );
	  for ( int i = 0; i <= MAX_CARRY_PERC; ++i )
	  {
		movecost.cost[i] = movecost.over;
	  }

	  int last_perc = -1;
	  double last_mult = 0.0;
	  std::string perc_str, mult_str;
	  int perc;
	  double mult;
	  while ( elem.remove_first_prop( &perc_str, &mult_str ) )
	  {
		ISTRINGSTREAM is1( perc_str );
		ISTRINGSTREAM is2( mult_str );
		if ( ( is1 >> perc ) && ( is2 >> mult ) )
		{
		  if ( perc > MAX_CARRY_PERC )
		  {
			elem.throw_error( "Max carrying capacity percentage is " + Clib::decint( MAX_CARRY_PERC ) );
		  }
		  Costs.insert( std::pair<int, double>( perc, mult ) );
		}
		else
		{
		  elem.throw_error( "poorly formed movecost spec: " + perc_str + " " + mult_str );
		}
	  }

	  for ( itr = Costs.begin(); itr != Costs.end(); ++itr )
	  {
		perc = itr->first;
		mult = itr->second;
		for ( int i = last_perc + 1; i <= perc; ++i )
		{
		  double m = ( ( last_mult - mult ) / ( last_perc - perc ) );
		  double b = mult - m * perc;

		  movecost.cost[i] = i * m + b;

		  //last_end + (mult - last_mult) * 
		}
		last_perc = perc;
		last_mult = mult;
	  }
	}

	unsigned short movecost( const Mobile::Character* chr, int carry_perc, bool running, bool mounted )
	{
	  MovementCost* mc;
	  double costmod;
	  if ( mounted )
	  {
		mc = running ? &gamestate.movecost_running_mounted : &gamestate.movecost_walking_mounted;
		costmod = running ? chr->movement_cost.run_mounted : chr->movement_cost.walk_mounted;
	  }
	  else
	  {
		mc = running ? &gamestate.movecost_running : &gamestate.movecost_walking;
		costmod = running ? chr->movement_cost.run : chr->movement_cost.walk;
	  }

	  double mult;
	  if ( carry_perc < 0 )
		mult = mc->cost[0];
	  else if ( carry_perc > MAX_CARRY_PERC )
		mult = mc->cost[MAX_CARRY_PERC];
	  else
		mult = mc->cost[carry_perc];

	  return static_cast<unsigned short>( 8 * mult * costmod );
	}

	void load_movecost( bool reload )
	{
	  if ( !gamestate.ssopt.movement_uses_stamina )
		return;
	  else if ( !Clib::FileExists( "config/movecost.cfg" ) )
	  {
		if ( !reload && Plib::systemstate.config.loglevel > 0 )
          INFO_PRINT << "File config/movecost.cfg not found, skipping.\n";
		return;
	  }

      Clib::ConfigFile cf( "config/movecost.cfg", "MovementCost Walking Running Walking_Mounted Running_Mounted" );
      Clib::ConfigElem elem;

	  bool walking_mounted_set = false;
	  bool running_mounted_set = false;

	  while ( cf.read( elem ) )
	  {
		if ( elem.type_is( "MovementCost" ) )
		{
		  read_movecost( elem, gamestate.movecost_running );
		  memcpy( &gamestate.movecost_walking, &gamestate.movecost_running, sizeof gamestate.movecost_walking );
		}
		else if ( elem.type_is( "Walking" ) )
		{
		  read_movecost( elem, gamestate.movecost_walking );
		}
		else if ( elem.type_is( "Running" ) )
		{
		  read_movecost( elem, gamestate.movecost_running );
		}
		else if ( elem.type_is( "Walking_Mounted" ) )
		{
		  read_movecost( elem, gamestate.movecost_walking_mounted );
		  walking_mounted_set = true;
		}
		else if ( elem.type_is( "Running_Mounted" ) )
		{
		  read_movecost( elem, gamestate.movecost_running_mounted );
		  running_mounted_set = true;
		}
	  }

	  if ( !walking_mounted_set )
		memcpy( &gamestate.movecost_walking_mounted, &gamestate.movecost_walking, sizeof gamestate.movecost_walking );
	  if ( !running_mounted_set )
		memcpy( &gamestate.movecost_running_mounted, &gamestate.movecost_running, sizeof gamestate.movecost_running );
	}
  }
}