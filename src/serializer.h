#pragma once

#include "virtual_machine.h"

class Serializer
{
public:
    constexpr Serializer() {};

    static void Write(std::ostream&, VirtualMachine const&);
    static void Load(std::istream&, VirtualMachine &);

    static void Write(std::ostream&, Memory const&);
    static void Load(std::istream&, Memory &);

    static void Write(std::ostream&, Word const&);
    static void Load(std::istream&, Word &);
};
