/*
History
=======
2015/20/12 Bodom:     moved most of the methods/props to BaseString

Notes
=======

*/

#pragma once

#include "basestr.h"

namespace Pol {
  namespace Bscript {

    class String : public BaseString<std::string, String, BObjectImp::BObjectType::OTString>
    {
    protected:
      inline virtual BaseString& child() { return *this; }

    public:
      // TODO: just inherit constructors the "using" way when upgrading to Visual Studio 2015,
      //       since Visual Studio 2013 does not support the following syntax:
      // using BaseString::BaseString;
      inline String() : BaseString() {}
      inline String( const String& str ) : BaseString( str ) {}
      inline explicit String( BObjectImp& objimp ) : BaseString( objimp ) {}
      inline explicit String( const std::string& str ) : BaseString( str ) {}
      inline explicit String( const std::string& str, std::string::size_type pos, std::string::size_type n ) : BaseString( str, pos, n ) {}
      inline explicit String( const String& left, const String& right ) : BaseString( left, right ) {}
      // TODO: end of manually inherited constructors

	  String( const char *str, int nchars );
      /** Creates an instance from a c-style string */
      inline explicit String( const char *str ) { value_.append(str); };
      /** Creates an instance by concatenating two c-style strings */
      inline String( const char *left, const char *right ) { value_.append(left); value_.append(right); };

	  static BObjectImp* unpack( const char* pstr );
	  static BObjectImp* unpack( std::istream& is );
	  static BObjectImp* unpackWithLen( std::istream& is );
	  virtual std::string pack() const POL_OVERRIDE;
	  virtual void packonto( std::ostream& os ) const POL_OVERRIDE;
	  static void packonto( std::ostream& os, const std::string& value );
	  virtual BObjectImp *copy() const POL_OVERRIDE
	  {
		return new String( *this );
	  }
	  virtual size_t sizeEstimate() const POL_OVERRIDE;

	  // FIXME: Possibly eliminate this later and have [ ] operator support?
	  // Or stick to functions, overload them with other substring getting methods.
	  String *StrStr( int begin, int len );
	  // FIXME: Possibly upgrade this later with overload functions in order to support different
	  // trim methods, or add ELTrim, etc?
	  String *ETrim( const char* CRSet, int type );
	  void EStrReplace( String* str1, String* str2 );
	  void ESubStrReplace( String* replace_with, unsigned int index, unsigned int len );

	  const char *data() const { return value_.c_str(); }
	  const std::string& value() const { return value_; }
      /*
       * Only for backward compatibility
       * @deprecated
       */
      inline int find( int begin, const char* target ) { const std::string str(target); return BaseString::find( begin, str); }
	  void toUpper( void );
	  void toLower( void );

	  virtual ~String()
	  {}

	  String& operator=( const char *s ) { value_ = s; return *this; }
	  String& operator=( const String& str ) { value_ = str.value_; return *this; }
	  void copyvalue( const String& str ) { value_ = str.value_; }
	  operator const char *( ) const { return value_.data(); }

	  virtual bool isTrue() const POL_OVERRIDE { return !value_.empty(); }
	public:

	  int find( char *s, int *posn );

	  virtual std::string getStringRep() const POL_OVERRIDE { return value_; }
	  virtual std::string getFormattedStringRep() const POL_OVERRIDE { return "\"" + value_ + "\""; }
	  void printOn( std::ostream& ) const;

	protected:

	  virtual BObjectImp* call_method( const char* methodname, Executor& ex ) POL_OVERRIDE;
	  virtual BObjectImp* call_method_id( const int id, Executor& ex, bool forcebuiltin = false ) POL_OVERRIDE;

	};

  }
}
