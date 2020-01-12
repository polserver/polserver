#ifndef POLOBJECT_H
#define POLOBJECT_H

#ifndef BSCRIPT_BOBJECT_H
#include "../bscript/bobject.h"
#endif

namespace Pol
{
namespace Core
{
class PolObjectImp : public Bscript::BObjectImp
{
public:
  explicit PolObjectImp( Bscript::BObjectImp::BObjectType type ) : BObjectImp( type ) {}

  virtual Bscript::BObjectImp* call_method( const char* methodname,
                                            Bscript::Executor& ex ) override;
  virtual Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex,
                                               bool forcebuiltin ) override;

  virtual Bscript::BObjectImp* call_polmethod( const char* methodname, Core::UOExecutor& ex ) = 0;
  virtual Bscript::BObjectImp* call_polmethod_id( const int id, Core::UOExecutor& ex,
                                                  bool forcebuiltin ) = 0;
};

template <class T>
class PolApplicObj : public Bscript::BApplicObj<T>
{
public:
  explicit PolApplicObj( const Bscript::BApplicObjType* object_type ) : BApplicObj( object_type ) {}
  explicit PolApplicObj( const Bscript::BApplicObjType* object_type, const T& value )
      : BApplicObj( object_type, value )
  {
  }

  virtual Bscript::BObjectImp* call_method( const char* methodname,
                                            Bscript::Executor& ex ) override;
  virtual Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex,
                                               bool forcebuiltin ) override;

  virtual Bscript::BObjectImp* call_polmethod( const char* methodname, Core::UOExecutor& ex ) = 0;
  virtual Bscript::BObjectImp* call_polmethod_id( const int id, Core::UOExecutor& ex,
                                                  bool forcebuiltin ) = 0;
};

template <class T>
Bscript::BObjectImp* PolApplicObj<T>::call_method( const char* methodname, Bscript::Executor& ex )
{
  passert( ex.type() == Bscript::ExecutorType::POL );

  auto& uoex = static_cast<Core::UOExecutor&>( ex );
  return this->call_polmethod( methodname, uoex );
}

template <class T>
Bscript::BObjectImp* PolApplicObj<T>::call_method_id( const int id, Bscript::Executor& ex,
                                                      bool forcebuiltin )
{
  passert( ex.type() == Bscript::ExecutorType::POL );
  auto& uoex = static_cast<Core::UOExecutor&>( ex );
  return this->call_polmethod_id( id, uoex, forcebuiltin );
}
}  // namespace Core
}  // namespace Pol
#endif
