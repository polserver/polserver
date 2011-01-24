/*
History
=======

Notes
=======

*/

#ifndef BSCRIPT_EXECUTORTYPE_H
#define BSCRIPT_EXECUTORTYPE_H

class Executor;
class Instruction;

typedef void (Executor::*ExecInstrFunc)(const Instruction&);

#endif
