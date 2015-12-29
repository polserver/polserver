/*
History
=======
2015/20/12 Bodom:     moved most of the methods/props to BaseStringTpl

Notes
=======

*/

#pragma once

#include "basestr.h"

namespace Pol {
  namespace Bscript {

    /**
     * An e-script String, with no specific encoding (usually ANSI)
     * Uses std::string for its internal storage
     * bscript ID: OTString
     */
    class String : public BaseStringTpl<std::string, String, BObjectImp::BObjectType::OTString>
    {
    public:

    // ---------------------- INHERITED CONSTRUCTORS ------------------------
      // TODO: just inherit constructors the "using" way when upgrading to Visual Studio 2015,
      //       since Visual Studio 2013 does not support the following syntax:
      // using BaseStringTpl::BaseStringTpl;
      inline String() : BaseStringTpl() {}
      inline String( const String& str ) : BaseStringTpl( str ) {}
      inline explicit String( BObjectImp& objimp ) : BaseStringTpl( objimp ) {}
      inline explicit String( const std::string& str ) : BaseStringTpl( str ) {}
      inline explicit String( const std::string& str, std::string::size_type pos, std::string::size_type n ) : BaseStringTpl( str, pos, n ) {}
      inline explicit String( const String& left, const String& right ) : BaseStringTpl( left, right ) {}
      // TODO: end of manually inherited constructors

    // ---------------------- PERSONALIZED EXTRA CONSTRUCTORS ---------------
      String( const char* str, int nchars );
      /** Creates an instance from a c-style string */
      inline explicit String( const char *str ) { value_.append(str); };
      /** Creates an instance by concatenating two c-style strings */
      inline String( const char *left, const char *right ) { value_.append(left); value_.append(right); };

    // ---------------------- PACK/UNPACK STUFF -----------------------------
	  static BObjectImp* unpack( const char* pstr );
	  static BObjectImp* unpack( std::istream& is );
	  static BObjectImp* unpackWithLen( std::istream& is );
	  virtual std::string pack() const POL_OVERRIDE;
	  virtual void packonto( std::ostream& os ) const POL_OVERRIDE;
	  static void packonto( std::ostream& os, const std::string& value );

    // -------------------------- MISC ----------------------------------------
	  virtual size_t sizeEstimate() const POL_OVERRIDE;

      /*
       * Only for backward compatibility
       * @deprecated
       */
      inline int find( int begin, const char* target ) { const std::string str(target); return BaseStringTpl::find( begin, str); }

	  void toUpper( void );
	  void toLower( void );

      inline unsigned long intval() const { return strtoul( value_.c_str(), NULL, 0 ); }
      inline double dblval() const { return strtod( value_.c_str(), NULL ); }

      inline String& operator=( const char *s ) { value_ = s; return *this; }
      inline operator const char *( ) const { return value_.data(); }

      inline virtual std::string getStringRep() const POL_OVERRIDE { return value_; }
      inline virtual std::string getFormattedStringRep() const POL_OVERRIDE { return "\"" + value_ + "\""; }
      virtual void printOn(std::ostream& os) const;

    };

  }
}
