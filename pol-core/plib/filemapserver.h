/** @file
 *
 * @par History
 */


#ifndef PLIB_FILEMAPSERVER_H
#define PLIB_FILEMAPSERVER_H

#include "../clib/compilerspecifics.h"
#include "../clib/binaryfile.h"

#include "mapblock.h"
#include "mapcell.h"
#include "mapserver.h"

namespace Pol {
  namespace Plib {

	class FileMapServer : public MapServer
	{
	public:
	  explicit FileMapServer( const RealmDescriptor& descriptor );
	  virtual ~FileMapServer() {}

	  virtual MAPCELL GetMapCell( unsigned short x, unsigned short y ) const POL_OVERRIDE;
      virtual size_t sizeEstimate( ) const POL_OVERRIDE;
	protected:
	  mutable Clib::BinaryFile _mapfile;
	  mutable int _cur_mapblock_index;
	  mutable MAPBLOCK _cur_mapblock;

	  // not implemented:
	  FileMapServer& operator=( const FileMapServer& );
	  FileMapServer( const FileMapServer& );
	};
  }
}

#endif
