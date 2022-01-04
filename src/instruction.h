#pragma once

#include "integer.h"

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
        NOOP
    };

    constexpr static std::size_t NumberOfArguments(OpCode op)
    {
        switch (op) {
            case HALT:  return 0;
            case SET:   return 2;
            case PUSH:  return 1;
            case POP:   return 1;
            case EQ:    return 3;
            case GT:    return 3;
            case JMP:   return 2;
            case JT:    return 2;
            case JF:    return 2;
            case ADD:   return 3;
            case MULT:  return 3;
            case MOD:   return 3;
            case AND:   return 3;
            case OR:    return 3;
            case NOT:   return 2;
            case RMEM:  return 2;
            case WMEM:  return 2;
            case CALL:  return 1;
            case RET:   return 0;
            case OUT:   return 1;
            case IN:    return 1;
            case NOOP:  return 0;
        }
    }

private:
    constexpr InstructionData() = default;
};