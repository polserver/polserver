/*
History
=======
2006/09/16 Shinigami: fixed Memory Overwrite Bug in convertArrayToUC

Notes
=======

*/

#include "stl_inc.h"

#include "endian.h"
#include "unicode.h"

// Warning: Make sure that your buffer if large enough! Don't forget to add space for null terminator if requested.
bool convertArrayToUC(ObjArray*& in_text, u16* out_wtext,
					  unsigned textlen, bool ConvToBE /*false*/, bool nullterm /*true*/)
{
	u16 value;
	unsigned pos = 0;
	for(unsigned i=0; i < textlen; i++) {
		BObject* bo = in_text->ref_arr[i].get();
		if (bo == NULL)
			continue;
		if (!bo->isa( BObjectImp::OTLong ))
			return false;
		BObjectImp *imp = bo->impptr();
		BLong* blong = static_cast<BLong*>(imp);
		value = (u16)(blong->value() & 0xFFFF);
		out_wtext[pos++] = ConvToBE ? ctBEu16(value) : value;
		// Jump out on a NULL (EOS) value (should stop exploits, too?)
		if ( value == 0L )
			return true;
	}
	if (nullterm)
		out_wtext[pos] = (u16)0L;
	return true;
}

bool convertUCtoArray(const u16* in_wtext, ObjArray*& out_text,
					  unsigned textlen, bool ConvFromBE /*false*/)
{
	u16 value;
	out_text = new ObjArray();
	for(unsigned i=0; i < textlen; i++) {
		value = ConvFromBE ? cfBEu16(in_wtext[i]) : in_wtext[i];
		// Jump out on a NULL (EOS) value (should stop exploits, too?)
		if ( value == 0L )
			return true;
		out_text->addElement( new BLong( value ) );
	}
	return true;
}
