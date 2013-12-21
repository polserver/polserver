/*
History
=======

Notes
=======

*/

#include "stl_inc.h"

#include "iohelp.h"

// note this is only useful if ofs.exceptions( std::ios_base::failbit | std::ios_base::badbit ) is called
namespace Pol {
  namespace Clib {
	void open_file( fstream& ofs, string& filename, ios::openmode mode )
	{
	  try
	  {
		ofs.open( filename.c_str(), mode );
	  }
	  catch( exception& ex )
	  {
		string message = "Error opening file " + filename + ": " + ex.what();
		throw std::runtime_error( message );
	  }
	}
	void open_file( ofstream& ofs, string& filename, ios::openmode mode )
	{
	  try
	  {
		ofs.open( filename.c_str(), mode );
	  }
	  catch( exception& ex )
	  {
		string message = "Error opening file " + filename + ": " + ex.what();
		throw std::runtime_error( message );
	  }
	}

	void open_file( ifstream& ifs, string& filename, ios::openmode mode )
	{
	  try
	  {
		ifs.open( filename.c_str(), mode );
	  }
	  catch( exception& ex )
	  {
		string message = "Error opening file " + filename + ": " + ex.what();
		throw std::runtime_error( message );
	  }
	}
  }
}
