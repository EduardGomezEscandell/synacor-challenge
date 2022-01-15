#pragma once

#include <ostream>
#include <string>
#include <sstream>

class VirtualMachine;

class BaseProcess
{
public:
    BaseProcess(VirtualMachine & vm) : m_parent_vm(vm) { };
    virtual bool operator()() = 0; // Returns true if it has to be removed. False otherwise
    static inline std::string name = "BaseProcess";
    virtual std::string const& Name() const noexcept = 0;
protected:
    VirtualMachine& m_parent_vm;
};

class DumpContentsProcess : public BaseProcess
{
public:
    DumpContentsProcess(VirtualMachine & vm) : BaseProcess(vm) { }
    bool operator()() override;
    static inline std::string name = "DumpContentsProcess";
    std::string const& Name() const noexcept override { return name; }
};

class DebugProcess : public BaseProcess
{
public:
    DebugProcess(VirtualMachine & vm);
    ~DebugProcess() noexcept;
    bool operator()() override;
    static inline std::string name = "DebugProcess";
    std::string const& Name() const noexcept override { return name; }
protected:
    std::stringstream m_outstream;
    std::ostream * m_original_ostream;
    std::size_t m_step_count;
};