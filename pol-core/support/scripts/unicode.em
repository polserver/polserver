////////////////////////////////////////////////////////////////
//
//  CONSTANTS
//
////////////////////////////////////////////////////////////////

//
// Constants for PrintTextAbove*
//
const JOURNAL_UC_PRINT_NAME := 0x00; // Implicit.  Print's the object's description / npc's name in the journal.
const JOURNAL_UC_PRINT_YOU_SEE := 0x01; // Will print "You see:" in the journal.

// Don't use these outside this file, use FONT_* from client.inc
const _DEFAULT_UCFONT := 3;
const _DEFAULT_UCCOLOR := 0x3B2;
const _DEFAULT_UCTEXT_REQUIREDCMD := 0;

////////////////////////////////////////////////////////////////
//
//  FUNCTIONS
//
////////////////////////////////////////////////////////////////

// 'uc_text' is a String, or for backward compatibility an Array of 2-byte integers,
//           where each integer is a Unicode character!
//
// 'langcode' is a 3-character "Originating Language" code
//       e.g. ENG, ENU, CHT, DEU, FRA, JPN, RUS, KOR (etc??)

BroadcastUC( uc_text, langcode, font := _DEFAULT_UCFONT, color := _DEFAULT_UCCOLOR,
             required_cmdlevel := _DEFAULT_UCTEXT_REQUIREDCMD );
PrintTextAbovePrivateUC( above_object, uc_text, langcode, character, font := _DEFAULT_UCFONT,
                         color := _DEFAULT_UCCOLOR, journal_print := JOURNAL_UC_PRINT_NAME );
PrintTextAboveUC( above_object, uc_text, langcode, font := _DEFAULT_UCFONT, color := _DEFAULT_UCCOLOR,
                  journal_print := JOURNAL_UC_PRINT_NAME );
RequestInputUC( character, item, uc_prompt, langcode ); // item is a placeholder, pass anything
SendSysMessageUC( character, uc_text, langcode, font := _DEFAULT_UCFONT, color := _DEFAULT_UCCOLOR );
