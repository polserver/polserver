/*
History
=======

Notes
=======

*/

#ifndef __OFILE_H
#define __OFILE_H

#ifndef __BOBJECT_H
#include "bobject.h"
#endif

#include <iosfwd>

class OFile;
class FILECont
{
    std::string filename_;

    FILE *fp;
    int nCopies;
  public:
    FILECont(FILE *iFP) : fp(iFP), nCopies(1) { }
    virtual ~FILECont() { if (fp) fclose(fp); fp = NULL; }
    friend class OFile;

    int close(void);
    int read(char *buf, int maxlen);
    int write(const char *buf);
    int seek(long posn);
    int tell(long& posn);
};
class OFile : public BObjectImp
{
    FILECont *fc;

  public:
    OFile(FILE *iFP) : BObjectImp(OTUnknown), fc(new FILECont(iFP)) { }
    OFile(FILECont *iFC) : BObjectImp(OTUnknown), fc(iFC) { fc->nCopies++; }

    virtual ~OFile();

    BObjectImp *copy(void) const;

    int close(void);
    int read(char *buf, int maxlen);
    int write(const char *buf);
    int seek(long posn);
    int tell(long& posn);

    const char *isA(void) const { return "File"; }
    virtual bool isA(const char *s) const
      {
          return (stricmp("File", s) == 0);
      }
    virtual bool isTrue(void) const { return (fc && fc->fp); }
    std::string getStringRep() const;
    void printOn(std::ostream&) const;
};
#endif
