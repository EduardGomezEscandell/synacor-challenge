#pragma once

#include <cassert>

#include "word.h"

class InstructionData
{
public:

    enum OpCode
    {
        HALT,
        SET,
        PUSH,
        POP,
        EQ,
        GT,
        JMP,
        JT,
        JF,
        ADD,
        MULT,
        MOD,
        AND,
        OR,
        NOT,
        RMEM,
        WMEM,
        CALL,
        RET,
        OUT,
        IN,
        NOOP,
        FATAL_ERROR
    };

    static constexpr OpCode Interpret(Word const& word)
    {
        // Operations   0 --   21 are correct
        // Operations  22 --   FF terminate the program
        // Operations 100 -- FFFF are truncated to the previous two
        
#ifndef DNDEBUG
        if(word.hi() != 0) return FATAL_ERROR;
#endif

        return static_cast<OpCode>(word.lo());
    }

private:
    constexpr InstructionData() = default;
};