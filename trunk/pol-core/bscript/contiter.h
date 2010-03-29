/*
History
=======

Notes
=======

*/

#ifndef BSCRIPT_CONTITER_H
#define BSCRIPT_CONTITER_H

class ContIterator : public BObjectImp
{
public:
    ContIterator();

    virtual BObject* step();

    BObjectImp *copy( void ) const;
    unsigned int sizeEstimate() const;
    string getStringRep() const;
};


#endif
