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
    void Initialize();
    void Run();
    void RunDebug();

    constexpr Memory const& memory() const noexcept {return m_memory; }
    void Print() const;

private:
    constexpr void ExecuteNextInstruction();

    /**
     * @brief Obtain the register from its integer alias
     */
    constexpr Word& DecodeRegister(Word w);

    /**
     * @brief Obtain the register from its integer alias
     */
    constexpr Word const& DecodeRegister(Word w) const;

    /**
     * @brief Get the Word, if valid, or decode the register it aliases to
     * 
     * @param arg: The word to interpret
     */
    constexpr Word const& GetValue(Word const& arg) const;

    /**
     * @brief Executes the operation and advances the instruction pointer to 
     *        the next instruction.
     */
    template<InstructionData::OpCode TOp>
    constexpr void Execute();

    static constexpr std::size_t num_registers = InstructionData::num_registers;

    Flags m_flags;
    Address m_instr_ptr;
    Memory m_memory;
    Word m_registers[num_registers];
    std::ostream * m_ostream = &std::cout;
};