#include <cstddef>

#include <fstream>
#include <iostream>
#include <iomanip>

#include <iterator>
#include <vector>
#include <string_view>
#include <optional>
#include <array>
#include <charconv>

namespace assembler {

typedef uint8_t half_word_t;
typedef uint16_t word_t;

void PrintHelp()
{
    std::cout << "SC assember. Usage: \n\n";
    std::cout << "assembler INPUT [OUTPUT]\n";
}

enum OpCode {
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


struct Instruction {
    OpCode op;
    std::vector<word_t> args;
};

struct ErroneousToken : public std::exception
{
    enum struct TokenType {INSTRUCTION, BAD_ARGUMENT, REGISTER, INTEGER};
    
    ErroneousToken(
        const TokenType token,
        const std::string_view::const_iterator line_begin,
        const std::string_view::const_iterator tok_begin,
        const std::string_view::const_iterator tok_end)
        : m_token(token),
          m_start(std::distance(line_begin, tok_begin)),
          m_len(std::distance(tok_begin, tok_end))
    { }

    TokenType m_token;
    size_t m_start;
    size_t m_len;
};

struct TooManyArgs : public std::exception
{
    TooManyArgs(
        const Instruction instruction,
        const std::string_view::const_iterator line_begin,
        const std::string_view::const_iterator arg_begin)
        : m_instruction(instruction),
          m_start(std::distance(line_begin, arg_begin))
    { }

    Instruction m_instruction;
    size_t m_start;
};

struct op_data {
    OpCode code;
    std::string_view ascii;
    size_t n_args;
};

word_t GetRegister(
    const std::string_view::const_iterator begin,
    const std::string_view::const_iterator end)
{
    if(std::distance(begin, end) != 2) return 0;
    if(*begin != 'r') return 0;

    if(begin[1] < 'a' || begin[1] > 'f') return 0;

    return 32768 + begin[1] - 'a';
}

const auto& GetOpData()
{
    using op_stack_map = std::array<op_data, 22>;

    static constexpr op_stack_map op_map = {
        op_data{ OpCode::HALT, "halt",  0},
        op_data{ OpCode::SET,  "set",   2},
        op_data{ OpCode::PUSH, "push",  1},
        op_data{ OpCode::POP,  "pop",   1},
        op_data{ OpCode::EQ,   "eq",    3},
        op_data{ OpCode::GT,   "gt",    3},
        op_data{ OpCode::JMP,  "jmp",   1},
        op_data{ OpCode::JT,   "jt",    2},
        op_data{ OpCode::JF,   "jf",    2},
        op_data{ OpCode::ADD,  "add",   3},
        op_data{ OpCode::MULT, "mult",  3},
        op_data{ OpCode::MOD,  "mod",   3},
        op_data{ OpCode::AND,  "and",   3},
        op_data{ OpCode::OR,   "or",    3},
        op_data{ OpCode::NOT,  "not",   2},
        op_data{ OpCode::RMEM, "rmem",  2},
        op_data{ OpCode::WMEM, "wmem",  2},
        op_data{ OpCode::CALL, "call",  1},
        op_data{ OpCode::RET,  "ret",   0},
        op_data{ OpCode::OUT,  "out",   1},
        op_data{ OpCode::IN,   "in",    1},
        op_data{ OpCode::NOOP, "noop",  0}
    };

    return op_map;
}

[[nodiscard]]
std::optional<OpCode> ReadOp(
    const std::string_view::const_iterator begin,
    const std::string_view::const_iterator end)
{
    const std::string_view op_ascii = std::string_view(begin, end);

    const auto& op_map = GetOpData();
    const auto it = std::find_if(op_map.begin(), op_map.end(), [&](const op_data& entry)
    {
        return entry.ascii == op_ascii;
    });
    
    if(it == op_map.end()) return {};
    
    return it->code;
}

[[nodiscard]]
std::optional<ErroneousToken::TokenType> ReadArgument(
    Instruction & instruction,
    std::string_view::const_iterator& begin,
    std::string_view::const_iterator& end)
{
    word_t arg = 0;
    if(*begin == 'r') // Register
    {
        arg = GetRegister(begin, end);
        if(arg == 0) return ErroneousToken::TokenType::REGISTER;
    }
    else // String literal
    {
        if(std::from_chars(begin, end, arg).ec == std::errc::invalid_argument)
        {
            return ErroneousToken::TokenType::BAD_ARGUMENT; // Integer ill-formed
        }
        
        if(arg > 0x8000)
        {
            return ErroneousToken::TokenType::INTEGER; // Integer too large
        }
    }


    instruction.args.push_back(arg);
    return {};
}

bool is_whitespace(char const c)
{
    return c==' ' || c == '\t';
}

void NextToken(auto& begin, auto& end, const auto& str_end)
{
    while(begin != str_end && *begin != ';' && is_whitespace(*begin))
    {
        ++begin;
    }

    end = begin;
    while(end != str_end && *end != ';' && !is_whitespace(*end)) 
    {
        ++end;
    }


}

std::optional<Instruction> ReadLine(const std::string_view line)
{
    auto begin = line.cbegin();
    auto end = begin;

    NextToken(begin, end, line.end());
    if(begin==end) return {};
    auto operation = ReadOp(begin, end);
    if(!operation)
    {
        throw ErroneousToken(ErroneousToken::TokenType::INSTRUCTION, line.begin(), begin, end);
    }
    Instruction instr = { *operation, {} };

    const size_t nargs = GetOpData()[(size_t) instr.op].n_args;
    begin = end;
    while(begin != line.end() && *begin != ';')
    {
        
        NextToken(begin, end, line.end());
        if(begin == end) break;

        auto err_code = ReadArgument(instr, begin, end);
        
        if(err_code)
        {
            throw ErroneousToken(*err_code, line.begin(), begin, end);
        }

        if(instr.args.size() > nargs)
        {
            throw TooManyArgs(instr, line.begin(), begin);
        }
        begin = end;
    }

    return instr;
}

std::ostream& PrintWord(std::ostream& os, const word_t& word)
{
    char lsd = word & static_cast<word_t>(0x00FF);          // Least significant digits
    char msd = (word & static_cast<word_t>(0xFF00)) >> 8;     // Most significant digits
    os.write(&lsd, 1);
    os.write(&msd, 1);
    return os;
}

std::ostream& operator<<(std::ostream& os, const Instruction& instruction)
{
    PrintWord(os, instruction.op);
    for(const word_t & arg: instruction.args)
    {
        PrintWord(os, arg);
    }
    return os;
}

std::string GenerateOutputFileName(std::string_view input_filename)
{
    auto r_it = std::find(input_filename.rbegin(), input_filename.rend(), '.');
    
    // No extension
    if(r_it == input_filename.rend())
    {
        return std::string(input_filename) + ".bin";
    }

    // Extension
    auto f_it = input_filename.cbegin() + std::distance(r_it, input_filename.rend());
    std::string name(input_filename.begin(), f_it);

    name += "bin";
    return name == input_filename ? name+="_" : name;
}

}