/*
History
=======
2015/20/12 Bodom:     creating this file to handle string/unicode

Notes
=======

*/

#pragma once

#ifndef BSCRIPT_BOBJECT_H
#include "bobject.h"
#endif

#ifndef BSCRIPT_BERROR_H
#include "berror.h"
#endif

#include <string>
#include <stack>

// Just like "this", but with explicit cast to child (see BaseString doc)
#define this_child (static_cast<D*>(this))
#define this_child_const (static_cast<const D*>(this))

namespace Pol {
  namespace Bscript {

    /**
     * This is an ABSTRACT class from which all string-like bscript objects
     * must be derived from (currently String and Unicode, can't imagine more)
     *
     * @param V type of the underlying value_ container
     * @param D type of the derived class (the class implementing this template itself)
     *          Just found this technique has a name: CRTP - Curiously Recurring Template Pattern
     * @param T ID for the object type, constant for BObjectImp constructor
     */
    template <typename V, typename D, BObjectImp::BObjectType T>
    class BaseString : public BObjectImp {

    typedef typename V::size_type V_size_type;

    public:
    // ---------------- CONSTRUCTORS ------------------------------------------

      /**
       * Creates an empty instance
       */
      BaseString() : BObjectImp( T ), value_() {}

      /**
       * Creates an instance from an object of the same class
       */
      BaseString( const D& str ) : BObjectImp( T ), value_( str.value_ ) {}

      /**
       * Creates an instance from any other object (convert to String)
       */
      explicit BaseString( BObjectImp& objimp ) :
        BObjectImp( T ),
        value_( objimp.getStringRep() )
      {}

      /**
       * Creates an instance from a parameter of the same type of the internal storage object
       */
      explicit BaseString( const V& str ) : BObjectImp( T ), value_( str ) {}

      /**
       * Creates an instance from a part of a parameter
       * of the same type of the internal storage object
       */
      explicit BaseString( const V& str, V_size_type pos, V_size_type n ) : BObjectImp( T ), value_( str, pos, n ) {}

      /**
       * Creates an instance by concatenating two instances
       */
      explicit BaseString( const D& left, const D& right ) :
        BObjectImp( T ),
        value_( left.value_ + right.value_ )
      {}


    // ----------------- SEARCH/QUERY FUNCTIONS -------------------------------

      /**
       * Returns this String length, in characters
       */
      size_t length() const
      {
        return value_.length();
      }

      /**
       * 0-based string find
       * find( "str srch", 2, "srch"):
       * 01^-- start
       *
       * @param begin: position to start from. If this is greater than the string length,
       *               the function never finds matches.
       * @param target: the string to search for
       * @return 0-based position, -1 if not found
       **/
      int find( int begin, const V &target )
      {
        V_size_type pos;
        pos = value_.find( target, begin );
        if ( pos == V::npos )
          return -1;
        else
          return static_cast<int>( pos );
      }

      /**
       * Returns the amount of alpha-numeric characters in string.
       */
      unsigned int alnumlen( void ) const
      {
        unsigned int c = 0;
        while ( iswalnum( value_[c] ) )
          c++;
        return c;
      }

      /**
       * Returns how many safe to print characters can be read
       * from string until a non-safe one is found
       */
      unsigned int SafeCharAmt() const
      {
        int strlen = static_cast<int>( this->length() );
        for ( int i = 0; i < strlen; i++ )
        {
          wchar_t tmp = value_[i];
          if ( iswalnum( tmp ) ) // a-z A-Z 0-9
            continue;
          else if ( iswpunct( tmp ) ) // !"#$%&'()*+,-./:;<=>?@{|}~
          {
            if ( tmp == '{' || tmp == '}' )
              return i;
            else
              continue;
          }
          else
          {
            return i;
          }
        }
        return strlen;
      }


    // ------------------- MANIPULATION FUNCTIONS -------------------------------

      /**
       * In-place reverse this string
       */
      void reverse()
      {
        std::reverse(value_.begin(), value_.end());
      }

      /*
       * Copies the c-style string into this one and deletes the pointer
       */
      void set( char *newstr )
      {
        value_ = newstr;
        delete newstr;
      }

      /**
       * Looks for a substring and deletes it if found
       */
      void remove( const V &rm )
      {
        size_t len = rm.length();

        auto pos = value_.find(rm);
        if (pos != V::npos)
          value_.erase( pos, len );
      }

    // ------------------- POL Operator implementations --------------------------

      virtual BObjectImp* selfPlusObjImp( const BObjectImp& objimp ) const POL_OVERRIDE
      {
        return objimp.selfPlusObj( *this_child_const );
      }
      virtual BObjectImp* selfPlusObj( const BObjectImp& objimp ) const POL_OVERRIDE
      {
        return new D( value_ + objimp.getStringRep() );
      }
      virtual BObjectImp* selfPlusObj( const BLong& objimp ) const POL_OVERRIDE
      {
        return new D( value_ + objimp.getStringRep() );
      }
      virtual BObjectImp* selfPlusObj( const Double& objimp ) const POL_OVERRIDE
      {
        return new D( value_ + objimp.getStringRep() );
      }
      virtual BObjectImp* selfPlusObj( const String& objimp ) const POL_OVERRIDE
      {
        return new D( value_ + objimp.getStringRep() );
      }
      virtual BObjectImp* selfPlusObj( const ObjArray& objimp ) const POL_OVERRIDE
      {
        return new D( value_ + objimp.getStringRep() );
      }
      virtual void selfPlusObjImp( BObjectImp& objimp, BObject& obj ) POL_OVERRIDE
      {
        objimp.selfPlusObj( *this_child, obj );
      }
      virtual void selfPlusObj( BObjectImp& objimp, BObject& /*obj*/ ) POL_OVERRIDE
      {
        value_ += objimp.getStringRep();
      }
      virtual void selfPlusObj( BLong& objimp, BObject& /*obj*/ ) POL_OVERRIDE
      {
        value_ += objimp.getStringRep();
      }
      virtual void selfPlusObj( Double& objimp, BObject& /*obj*/ ) POL_OVERRIDE
      {
        value_ += objimp.getStringRep();
      }
      virtual void selfPlusObj( String& objimp, BObject& /*obj*/ ) POL_OVERRIDE
      {
        value_ += objimp.getStringRep();
      }
      virtual void selfPlusObj( ObjArray& objimp, BObject& /*obj*/ ) POL_OVERRIDE
      {
        value_ += objimp.getStringRep();
      }

      virtual BObjectImp* selfMinusObjImp( const BObjectImp& objimp ) const POL_OVERRIDE
      {
        return objimp.selfMinusObj( *this_child_const );
      }
      virtual BObjectImp* selfMinusObj( const BObjectImp& objimp ) const POL_OVERRIDE
      {
        D *tmp = (D *)copy();
        const std::string str( objimp.getStringRep().data() );
        tmp->remove( str );
        return tmp;
      }
      virtual BObjectImp* selfMinusObj( const BLong& objimp ) const POL_OVERRIDE
      {
        D *tmp = (D *)copy();
        const std::string str( objimp.getStringRep().data() );
        tmp->remove( str );
        return tmp;
      }
      virtual BObjectImp* selfMinusObj( const Double& objimp ) const POL_OVERRIDE
      {
        D *tmp = (D *)copy();
        const std::string str( objimp.getStringRep().data() );
        tmp->remove( str );
        return tmp;
      }
      virtual BObjectImp* selfMinusObj( const String& objimp ) const POL_OVERRIDE
      {
        D *tmp = (D *)copy();
        const std::string str( objimp.getStringRep().data() );
        tmp->remove( str );
        return tmp;
      }
      virtual BObjectImp* selfMinusObj( const ObjArray& objimp ) const POL_OVERRIDE
      {
        D *tmp = (D *)copy();
        const std::string str( objimp.getStringRep().data() );
        tmp->remove( str );
        return tmp;
      }
      virtual void selfMinusObjImp( BObjectImp& objimp, BObject& obj ) POL_OVERRIDE
      {
        objimp.selfMinusObj( *this_child, obj );
      }
      virtual void selfMinusObj( BObjectImp& objimp, BObject& /*obj*/ ) POL_OVERRIDE
      {
        remove( objimp.getStringRep().data() );
      }
      virtual void selfMinusObj( BLong& objimp, BObject& /*obj*/ ) POL_OVERRIDE
      {
        remove( objimp.getStringRep().data() );
      }
      virtual void selfMinusObj( Double& objimp, BObject& /*obj*/ ) POL_OVERRIDE
      {
        remove( objimp.getStringRep().data() );
      }
      virtual void selfMinusObj( String& objimp, BObject& /*obj*/ ) POL_OVERRIDE
      {
        remove( objimp.value_.data() );
      }
      virtual void selfMinusObj( ObjArray& objimp, BObject& /*obj*/ ) POL_OVERRIDE
      {
        remove( objimp.getStringRep().data() );
      }

      virtual bool isEqual( const BObjectImp& objimp ) const POL_OVERRIDE
      {
        if ( objimp.isa( T ) )
          return ( value_ == static_cast<const D&>( objimp ).value_ );
        else
          return false;
      }

      /**
       * Since non-Strings show up here as not equal, we make them less than.
       *
       * @todo TODO: Change this behavior? It doesn't make much sense - 12-27-2015 Bodom
       */
      virtual bool isLessThan( const BObjectImp& objimp ) const POL_OVERRIDE
      {
        if ( objimp.isa( T ) )
          return ( value_ < static_cast<const D&>( objimp ).value_ );
        else if ( objimp.isa( OTUninit ) || objimp.isa( OTError ) )
          return false;
        else
          return true;
      }


      virtual BObjectImp* array_assign( BObjectImp* idx, BObjectImp* target, bool /*copy*/ ) POL_OVERRIDE
      {
        V_size_type pos, len;

        // first, determine position and length.
        if ( idx->isa( T ) )
        {
          D& rtstr = (D&)*idx;
          pos = value_.find( rtstr.value_ );
          len = rtstr.length();
        }
        else if ( idx->isa( OTLong ) )
        {
          BLong& lng = (BLong&)*idx;
          pos = lng.value() - 1;
          len = 1;
        }
        else if ( idx->isa( OTDouble ) )
        {
          Double& dbl = (Double&)*idx;
          pos = static_cast<std::string::size_type>(dbl.value());
          len = 1;
        }
        else
        {
          return UninitObject::create();
        }

        if (pos != V::npos)
        {
          if ( target->isa( OTString ) )
          {
            D* target_str = (D*)target;
            value_.replace( pos, len, target_str->value_ );
          }
          return this_child;
        }
        else
        {
          return UninitObject::create();
        }
      }

      virtual BObjectRef OperMultiSubscriptAssign(std::stack<BObjectRef>& indices, BObjectImp* target) POL_OVERRIDE
      {
        BObjectRef start_ref = indices.top();
        indices.pop();
        BObjectRef length_ref = indices.top();
        indices.pop();

        BObject& length_obj = *length_ref;
        BObject& start_obj = *start_ref;

        BObjectImp& length = length_obj.impref();
        BObjectImp& start = start_obj.impref();

        // first deal with the start position.
        unsigned index;
        if ( start.isa( OTLong ) )
        {
          BLong& lng = (BLong&)start;
          index = (unsigned)lng.value();
          if ( index == 0 || index > value_.size() )
            return BObjectRef( new BError( "Subscript out of range" ) );

        }
        else if ( start.isa( T ) )
        {
          D& rtstr = (D&)start;
          V_size_type pos = value_.find(rtstr.value_);
          if (pos != V::npos)
            index = static_cast<unsigned int>( pos + 1 );
          else
            return BObjectRef( new UninitObject );
        }
        else
        {
          return BObjectRef( copy() );
        }

        // now deal with the length.
        int len;
        if ( length.isa( OTLong ) )
        {
          BLong& lng = (BLong &)length;

          len = (int)lng.value();
        }
        else if ( length.isa( OTDouble ) )
        {
          Double& dbl = (Double &)length;

          len = (int)dbl.value();
        }
        else
        {
          return BObjectRef( copy() );
        }

        if ( target->isa( T ) )
        {
          D* target_str = (D*)target;
          value_.replace( index - 1, len, target_str->value_ );
        }
        else
        {
          return BObjectRef( UninitObject::create() );
        }

        return BObjectRef( this_child );
      }


      virtual BObjectRef OperMultiSubscript(std::stack<BObjectRef>& indices) POL_OVERRIDE
      {
        BObjectRef start_ref = indices.top();
        indices.pop();
        BObjectRef length_ref = indices.top();
        indices.pop();

        BObject& length_obj = *length_ref;
        BObject& start_obj = *start_ref;

        BObjectImp& length = length_obj.impref();
        BObjectImp& start = start_obj.impref();

        // first deal with the start position.
        unsigned index;
        if ( start.isa( OTLong ) )
        {
          BLong& lng = (BLong&)start;
          index = (unsigned)lng.value();
          if ( index == 0 || index > value_.size() )
            return BObjectRef( new BError( "Subscript out of range" ) );

        }
        else if ( start.isa( T ) )
        {
          D& rtstr = (D&)start;
          V_size_type pos = value_.find(rtstr.value_);
          if (pos != V::npos)
            index = static_cast<unsigned int>( pos + 1 );
          else
            return BObjectRef( new UninitObject );
        }
        else
        {
          return BObjectRef( copy() );
        }

        // now deal with the length.
        int len;
        if ( length.isa( OTLong ) )
        {
          BLong& lng = (BLong &)length;

          len = (int)lng.value();
        }
        else if ( length.isa( OTDouble ) )
        {
          Double& dbl = (Double &)length;

          len = (int)dbl.value();
        }
        else
        {
          return BObjectRef( copy() );
        }

        auto str = new D( value_, index - 1, len );
        return BObjectRef( str );
      }

      virtual BObjectRef OperSubscript( const BObject& rightobj ) POL_OVERRIDE
      {
        const BObjectImp& right = rightobj.impref();
        if ( right.isa( OTLong ) )
        {
          BLong& lng = (BLong&)right;

          unsigned index = (unsigned)lng.value();

          if ( index == 0 || index > value_.size() )
            return BObjectRef( new BError( "Subscript out of range" ) );

          return BObjectRef( new BObject( new String( value_.c_str() + index - 1, 1 ) ) );
        }
        else if ( right.isa( OTDouble ) )
        {
          Double& dbl = (Double&)right;

          unsigned index = (unsigned)dbl.value();

          if ( index == 0 || index > value_.size() )
            return BObjectRef( new BError( "Subscript out of range" ) );

          return BObjectRef( new BObject( new String( value_.c_str() + index - 1, 1 ) ) );
        }
        else if ( right.isa( T ) )
        {
          D& rtstr = (D&)right;
          auto pos = value_.find(rtstr.value_);
          if (pos != V::npos)
            return BObjectRef( new BObject( new D( value_, pos, 1 ) ) );
          else
            return BObjectRef( new UninitObject );
        }
        else
        {
          return BObjectRef( new UninitObject );
        }
      }


    // ------------------- INTERNAL ----------------------------------------------

    protected:
      V value_;

      /**
       * Returns a pointer to a substring of this string
       */
      D* midstring( int begin, int len ) const
      {
        return new D( value_.substr( begin - 1, len ) );
      }

    };

  }
}

#undef this_child
#undef this_child_const
