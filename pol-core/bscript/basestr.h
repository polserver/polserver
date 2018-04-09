/** @file
 *
 * @par History
 * - 2015/20/12 Bodom:     creating this file to handle string/unicode
 */


#pragma once



// Just like "this", but with explicit cast to child (see BaseStringTpl doc)
#define this_child (static_cast<D*>(this))
#define this_child_const (static_cast<const D*>(this))

namespace Pol {
namespace Bscript {

/**
 * This is a generic INTERFACE for the base string object
 *
 * Methods defined here are guaranteed to be implemented with the same signature
 * in all the string-like objects.
 * All methods must be PUBLIC and PURE VIRTUAL.
 */
class BaseString : public BObjectImp {
public:

  /**
   * Returns an ANSI value for this string
   * @warning precision may be lost, non-ANSI characters are replaced
   */
  virtual std::string strval() const = 0;

  /**
   * Prints a formatted version of this string into the given stream
   */
  virtual void printOn(std::ostream& os) const = 0;

};




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
class BaseStringTpl : public BaseString {

typedef typename V::size_type V_size_type;

public:
// ---------------- CONSTRUCTORS ------------------------------------------



// ----------------- SEARCH/QUERY FUNCTIONS -------------------------------


  std::string strval() const
  {
    return static_cast<std::string>(value_);
  }


// ------------------- MANIPULATION FUNCTIONS -------------------------------

};

}
}

#undef this_child
#undef this_child_const
