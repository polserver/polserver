#ifndef POLOBJECT_H
#define POLOBJECT_H

#ifndef BSCRIPT_BOBJECT_H
#include "../bscript/bobject.h"
#endif

namespace Pol
{
namespace Core
{
class PolObjectImp : virtual public Bscript::BObjectImp
{
public:
  PolObjectImp(){}

  virtual Bscript::BObjectImp* call_method( const char* methodname,
                                            Bscript::Executor& ex ) override;
  virtual Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex,
                                               bool forcebuiltin );
  virtual Bscript::BObjectImp* call_polmethod( const char* methodname, Core::UOExecutor& ex );
  virtual Bscript::BObjectImp* call_polmethod_id( const int id, Core::UOExecutor& ex,
                                                  bool forcebuiltin );
};


template <class T>
class PolApplicObj : virtual public Bscript::BApplicObj<T>
{
public:
  PolApplicObj<T>() {}

  virtual Bscript::BObjectImp* call_method( const char* methodname,
                                            Bscript::Executor& ex ) override;
  virtual Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex,
                                               bool forcebuiltin );
  virtual Bscript::BObjectImp* call_polmethod( const char* methodname, Core::UOExecutor& ex );
  virtual Bscript::BObjectImp* call_polmethod_id( const int id, Core::UOExecutor& ex,
                                                  bool forcebuiltin );

};
}  // namespace Core
}  // namespace Pol
#endif
