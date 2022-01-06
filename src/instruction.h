#pragma once

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
        BAD_HALT
    };

    static constexpr OpCode Interpret(Word const& word)
    {
        return static_cast<OpCode>(word.get_raw());
    }

private:
    constexpr InstructionData() = default;
};