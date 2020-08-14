#ifndef POLSERVER_CONFORMINGCHARSTREAM_H
#define POLSERVER_CONFORMINGCHARSTREAM_H

#include <antlr4-runtime.h>

namespace Pol::Bscript::Compiler
{
/*
 * Convert all symbols to lowercase because that is how the grammar expects them.
 */
class ConformingCharStream : public antlr4::CharStream
{
public:
  explicit ConformingCharStream( antlr4::CharStream* stream ) : stream( stream ) {}

  std::string getText( const antlr4::misc::Interval& interval ) override
  {
    return stream->getText( interval );
  }

  std::string toString() const override { return stream->toString(); }

  void consume() override { return stream->consume(); }
  size_t LA( ssize_t i ) override;

  ssize_t mark() override { return stream->mark(); }

  void release( ssize_t marker ) override { return stream->release( marker ); }

  size_t index() override { return stream->index(); }

  void seek( size_t index ) override { return stream->seek( index ); }

  size_t size() override { return stream->size(); }

  std::string getSourceName() const override { return stream->getSourceName(); }

private:
  antlr4::CharStream* stream;
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_CONFORMINGCHARSTREAM_H
