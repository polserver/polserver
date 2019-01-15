//
// Cliloc Message related functions
//

//
// CONSTANTS
//
const _DEFAULT_CLFONT  := 3;
const _DEFAULT_CLCOLOR := 0x3B2;

//
// Functions
//

// Character : Who to send packet to.
// Cliloc_Num : Cliloc # you wish to be displayed.
// UC_Text : is a String, or for backward compatibility an Array of 2-byte integers, where each integer is a Unicode character!
//            Only used when the cliloc accepts arguments for replacement.
// Font : Font
// Color : Color
//
// Argument example: 
// take number 1042762: 
// "Only ~1_AMOUNT~ gold could be deposited. A check for ~2_CHECK_AMOUNT~ gold was returned to you." 
// the arguments string may have "100 thousand\t25 hundred", which in turn would modify the string: 
// "Only 100 thousand gold could be deposited. A check for 25 hundred gold was returned to you."
SendSysMessageCL( character, cliloc_num, uc_text:="", font:=_DEFAULT_CLFONT , color:=_DEFAULT_CLCOLOR);

PrintTextAboveCL( character, cliloc_num, uc_text:="", font:=_DEFAULT_CLFONT , color:=_DEFAULT_CLCOLOR);

// Object : Object text is to appear over.
PrintTextAbovePrivateCL( character, object, cliloc_num, uc_text:="", font:=_DEFAULT_CLFONT , color:=_DEFAULT_CLCOLOR);
