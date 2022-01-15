#pragma once

#include <cstdlib>
#include <istream>
#include <ostream>
#include <memory>
#include <sstream>
#include <vector>

#include "address.h"
#include "instruction.h"
#include "flags.h"
#include "virtual_memory.h"
#include "processes.h"

class Serializer;

class VirtualMachine
{
public:
    using program_file_t = Memory::program_file_t;

    class InputBuffer
    {
    public:
        InputBuffer(VirtualMachine& vm, std::istream& stream = std::cin) noexcept
            : m_parent_vm(vm), m_istream(&stream) {}
        friend InputBuffer& operator>>(InputBuffer& tbuffer, Word& t);
    private:
        VirtualMachine& m_parent_vm;
        std::istream* m_istream;
        std::string data;
        std::size_t ptr = 0;
    };

    class OutputBuffer
    {
    public:
        OutputBuffer(std::ostream& stream = std::cout) noexcept 
            : m_ostream(&stream) {}
        std::ostream* m_ostream;
        
        friend OutputBuffer& operator<<(OutputBuffer& buffer, auto const& t)
        {
            (*buffer.m_ostream) << t;
            return buffer;
        }
    protected:
    };

    VirtualMachine(std::istream& is = std::cin, std::ostream& os = std::cout)
        : m_input_buffer(*this, is), m_output_buffer(os)
    {
    }
    
    template<typename TProcess>
    constexpr void ToggleProcess()
    {
        auto it = std::find_if(m_attached_processes.begin(), m_attached_processes.end(), 
            [](auto const& process_ptr) { return process_ptr->Name() == TProcess::name; }
        );

        if(it == m_attached_processes.end())
        {
            m_attached_processes.emplace_back(std::make_unique<TProcess>(*this));
        } else {
            m_attached_processes.erase(it);
        }
    }

    void LoadMemory(program_file_t& source);
    void Run();
    void Pause();

    constexpr auto& flags() noexcept { return m_flags; }
    constexpr auto const& memory() const noexcept {return m_memory; }

    constexpr auto& output_stream() noexcept { return m_output_buffer; }
    constexpr auto& input_stream() noexcept { return m_input_buffer; }

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

    InputBuffer m_input_buffer;            // Stream that IN instruction uses as a buffer
    OutputBuffer m_output_buffer;          // Stream that OUT instruction ouputs to

    Flags m_flags;                         // Flags indicating side-effects of instructions
    std::array<Word, num_registers> m_registers;       // General-purpose registers
    Address m_instr_ptr = 0;               // Register containing the instruction pointer: points to the next instruction's opcode
    Address m_stack_base_ptr = 0;          // Register containing the base of the stack in memory
    Address m_stack_ptr = 0;               // Register containing the current top of the stack
    Word m_nul_register = 0;               // A register to read/write from when a worng adress is given.
    Memory m_memory;                       // The RAM

    friend class Serializer;

    std::vector<std::unique_ptr<BaseProcess>> m_attached_processes;
};
