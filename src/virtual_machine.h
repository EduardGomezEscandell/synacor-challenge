#include "address.h"
#include "instruction.h"
#include "flags.h"
#include "virtual_memory.h"
#include <ostream>

#pragma once

class VirtualMachine
{
public:
    using program_file_t = Memory::program_file_t;

    void LoadMemory(program_file_t& source);
    void Run();
    void RunDebug();

    constexpr Memory const& memory() const noexcept {return m_memory; }
    void Print() const;


private:
    constexpr void ExecuteNextInstruction();

    /**
     * @brief Obtain the register from its integer alias withput checking validity.
     */
    constexpr Word& DecodeRegisterUnsafe(Word const& w) noexcept;

    /**
     * @brief Obtain the register from its integer alias withput checking validity.
     */
    constexpr Word const& DecodeRegisterUnsafe(Word const& w) const noexcept;

    /**
     * @brief Obtain the register from its integer alias
     *        May set WRITE_ON_LITERAL, BAD_INTEGER and ERROR flags
     */
    constexpr Word& DecodeRegister(Word & w);

    /**
     * @brief Get the Word, or decode the register it aliases to.
     *        Sets ERROR and BAD_INTEGER flags if the value is not valid
     * 
     * @param arg: The word to interpret
     */
    constexpr Word const& GetValue(Word const& arg);

    /**
     * @brief Executes the operation.
     *        The instruction pointer is expected to be at the current instruction's opcode.
     *        After execution, the instruction pointer is left pointing at the next instruction. 
     */
    template<InstructionData::OpCode TOp>
    constexpr void Execute();

    /**
     * @brief Utility for instructions of type a=f(b,c).
     * @param op is expected to be of type (Word const&, Word const&) -> Word
     */
    template<typename TOperator>
    constexpr void ExecuteBinaryOp(TOperator const& op) noexcept;

    /**
     * @brief Utility for instructions of type a=f(b).
     * @param op is expected to be of type (Word const&) -> Word
     */
    template<typename TOperator>
    constexpr void ExecuteUnaryOp(TOperator const& op) noexcept;

    /**
     * @brief Sets the base and stack pointers to the first free row of memory
     */
    constexpr void StackInit() noexcept;

    /**
     * @brief Pushes a value onto the stack and advances the stack pointer
     */
    constexpr void StackPush(Word const& val) noexcept;

    /**
     * @brief Pulls a value from the stack and decreases the stack pointer
     * Note that the value will not be removed until overwritten.
     */
    constexpr Word StackPop() noexcept;

    static constexpr std::size_t num_registers = InstructionData::num_registers;

    Flags m_flags;                         // Flags indicating side-effects of instructions
    Word m_registers[num_registers];       // General-purpose registers
    Address m_instr_ptr = 0;               // Register containing the instruction pointer: points to the next instruction's opcode
    Address m_stack_base_ptr = 0;          // Register containing the base of the stack in memory
    Address m_stack_ptr = 0;               // Register containing the current top of the stack
    Word m_nul_register = 0;               // A register to read/write from when a worng adress is given.
    Memory m_memory;                       // The RAM
    std::ostream * m_ostream = &std::cout; // Stream that OUT instruction ouputs to

    class TextBuffer
    {
    public:
        TextBuffer() noexcept {}

        friend TextBuffer& operator>>(TextBuffer& tbuffer, Word& t)
        {
            if(tbuffer.ptr == tbuffer.data.size())
            {
                tbuffer.data.clear();
                std::getline(std::cin, tbuffer.data);
                tbuffer.data.push_back('\n');
                tbuffer.ptr = 0;
            }
            t.lo() = tbuffer.data[tbuffer.ptr++];
            t.hi() = 0;
            return tbuffer;
        }
    
    private:
        std::string data;
        std::size_t ptr = 0;

    } m_input_buffer; // Stream that IN instruction uses as a buffer
};