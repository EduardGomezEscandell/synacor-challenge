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
        WRONG_OPCODE
    };

    enum WordType 
    {
        LITERAL,
        REGISTER,
        INVALID
    };

    static constexpr OpCode to_opcode(Word const& word)
    {
        // Operations   0 --   21 are correct
        // Operations  22 --   FF terminate the program
        // Operations 100 -- FFFF are truncated to the previous two
        
#ifndef DNDEBUG
        if(word.hi() != 0) return WRONG_OPCODE;
#endif

        return static_cast<OpCode>(word.lo());
    }

    static constexpr WordType to_wordtype(Word const& word)
    {
        if(word.to_int() < Word::max_word) return LITERAL;

        const raw_word_t reg = word.to_int() - Word::max_word;
        if(reg < num_registers) return REGISTER;

        return INVALID;
    }

    static constexpr std::string_view InstructionName(OpCode op) noexcept
    {
        switch(op)
        {
            case HALT        : return "HALT";
            case SET         : return "SET";
            case PUSH        : return "PUSH";
            case POP         : return "POP";
            case EQ          : return "EQ";
            case GT          : return "GT";
            case JMP         : return "JMP";
            case JT          : return "JT";
            case JF          : return "JF";
            case ADD         : return "ADD";
            case MULT        : return "MULT";
            case MOD         : return "MOD";
            case AND         : return "AND";
            case OR          : return "OR";
            case NOT         : return "NOT";
            case RMEM        : return "RMEM";
            case WMEM        : return "WMEM";
            case CALL        : return "CALL";
            case RET         : return "RET";
            case OUT         : return "OUT";
            case IN          : return "IN";
            case NOOP        : return "NOOP";
            case WRONG_OPCODE : return "WRONG_OPCODE";
        }
        return "INVALID";
    }


    static constexpr std::size_t num_registers = 8;

private:
    constexpr InstructionData() = default;

};