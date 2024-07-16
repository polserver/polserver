/** @file
 *
 * @par History
 */


#include "berror.h"
#include "contiter.h"
#include "executor.h"
#include "impstr.h"
#include "objmethods.h"

namespace Pol
{
namespace Bscript
{
unsigned int BError::creations_ = 0;
unsigned int BError::creations()
{
  return creations_;
}

BError::BError( Bscript::Executor* exec ) : BStruct( OTError )
{
  ++creations_;
  attach_stack( exec );
}

BError::BError( const BError& other ) : BStruct( other, OTError )
{
  ++creations_;
  stack_ = other.stack_;
  script_ = other.script_;
}

BError::BError( std::istream& is, unsigned size ) : BStruct( is, size, OTError )
{
  ++creations_;
}

BError::BError( const char* err, Bscript::Executor* exec ) : BStruct( OTError )
{
  ++creations_;
  addMember( "errortext", new String( err ) );
  attach_stack( exec );
}

BError::BError( const std::string& err, Bscript::Executor* exec ) : BStruct( OTError )
{
  ++creations_;
  addMember( "errortext", new String( err ) );
  attach_stack( exec );
}


BObjectImp* BError::copy( void ) const
{
  return new BError( *this );
}

char BError::packtype() const
{
  return 'e';
}

const char* BError::typetag() const
{
  return "error";
}

const char* BError::typeOf() const
{
  return "Error";
}

u8 BError::typeOfInt() const
{
  return OTError;
}

BObjectImp* BError::unpack( std::istream& is )
{
  unsigned size;
  char colon;
  if ( !( is >> size >> colon ) )
  {
    return new BError( "Unable to unpack struct elemcount" );
  }
  if ( (int)size <= 0 )
  {
    return new BError(
        "Unable to unpack struct elemcount. Length given must be positive integer!" );
  }
  if ( colon != ':' )
  {
    return new BError( "Unable to unpack struct elemcount. Bad format. Colon not found!" );
  }
  return new BError( is, size );
}


bool BError::isTrue() const
{
  return false;
}
/**
 * An error is equal to any other error or uninit
 */
bool BError::operator==( const BObjectImp& imp ) const
{
  return ( imp.isa( OTError ) || imp.isa( OTUninit ) );
}

bool BError::operator<( const BObjectImp& imp ) const
{
  if ( imp.isa( OTError ) || imp.isa( OTUninit ) )
    return false;  // Because it's equal can't be lesser

  return true;
}

ContIterator* BError::createIterator( BObject* /*pIterVal*/ )
{
  return new ContIterator();
}

BObjectRef BError::OperSubscript( const BObject& /*obj*/ )
{
  return BObjectRef( this );
}

BObjectImp* BError::array_assign( BObjectImp* /*idx*/, BObjectImp* /*target*/, bool /*copy*/ )
{
  return this;
}

BObjectImp* BError::call_method_id( const int id, Executor& ex, bool )
{
  switch ( id )
  {
  case MTH_STACKTRACE:
  {
    bool returnAsObject = false;

    if ( ex.numParams() > 0 && !ex.getParam( 0, returnAsObject ) )
    {
      return new BError( "Invalid parameter type" );
    }

    if ( returnAsObject )
    {
      if ( script_->read_dbg_file() != 0 )
      {
        return UninitObject::create();
      }

      std::unique_ptr<ObjArray> arr( new ObjArray );

      for ( const auto& PC : stack_ )
      {
        auto filename = script_->dbg_filenames[script_->dbg_filenum[PC]];
        auto line = script_->dbg_linenum[PC];
        auto dbgFunction =
            std::find_if( script_->dbg_functions.begin(), script_->dbg_functions.end(),
                          [&]( auto& i ) { return i.firstPC <= PC && PC <= i.lastPC; } );

        std::string functionName =
            dbgFunction != script_->dbg_functions.end() ? dbgFunction->name : "<program>";

        std::unique_ptr<BStruct> entry( new BStruct );
        entry->addMember( "file", new String( filename ) );
        entry->addMember( "line", new BLong( line ) );
        entry->addMember( "name", new String( functionName ) );
        arr->addElement( entry.release() );
      }

      return arr.release();
    }

    auto errortext = FindMember( "errortext" );

    std::string message = errortext == nullptr ? "Error" : errortext->getStringRep();

    if ( script_->read_dbg_file() == 0 )
    {
      for ( const auto& PC : stack_ )
      {
        auto filename = script_->dbg_filenames[script_->dbg_filenum[PC]];
        auto line = script_->dbg_linenum[PC];
        auto dbgFunction =
            std::find_if( script_->dbg_functions.begin(), script_->dbg_functions.end(),
                          [&]( auto& i ) { return i.firstPC <= PC && PC <= i.lastPC; } );

        std::string functionName =
            dbgFunction != script_->dbg_functions.end() ? dbgFunction->name : "<program>";

        message.append( fmt::format( "\n    at {} ({}:{})", functionName, filename, line,
                                     script_->dbg_filenames[script_->dbg_filenum[PC]],
                                     script_->dbg_filenames[1] ) );
      }
    }
    else
    {
      for ( const auto& pc : stack_ )
      {
        message.append( fmt::format( "\n    at {}+{}", Clib::scripts_thread_script, pc ) );
      }
    }

    return new String( message );
  }
  default:
    return base::call_method_id( id, ex, false );
  }
}

void BError::attach_stack( Bscript::Executor* exec )
{
  if ( !exec )
  {
    return;
  }

  EScriptProgram* prog = const_cast<EScriptProgram*>( exec->prog() );
  script_.set( prog );

  if ( !stack_.empty() )
  {
    return;
  }
  std::vector<ReturnContext> callStack = exec->ControlStack;

  {
    ReturnContext rc;
    rc.PC = exec->PC;
    rc.ValueStackDepth = static_cast<unsigned int>( exec->ValueStack.size() );
    callStack.push_back( rc );
  }

  while ( !callStack.empty() )
  {
    ReturnContext& rc = callStack.back();
    stack_.push_back( rc.PC );
    callStack.pop_back();
  }
}

size_t BError::sizeEstimate() const
{
  size_t size = base::sizeEstimate() + sizeof( ref_ptr<EScriptProgram> );
  size += 3 * sizeof( unsigned int* ) + stack_.capacity() * sizeof( unsigned int );
  return size;
}
}  // namespace Bscript
}  // namespace Pol
