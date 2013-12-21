/*
History
=======

Notes
=======

*/

#ifndef MD5_H
#define MD5_H
namespace Pol {
  namespace Clib {
	bool MD5_Encrypt( const std::string& in, std::string& out );
	bool MD5_Compare( const std::string& a, const std::string& b );
	void MD5_Cleanup();
  }
}
#endif
