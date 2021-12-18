Assembler

This program assembles a assembly language with the following specifications:
- Each line is an instruction
- Comments: Everything after ';' is ignored.
- Empty lines (and comment-only lines) are ignored
- Multiple consecutive whitespace is ignored
- Opcodes are those indicated in the [Synacor Challenge](https://challenge.synacor.com/) statement.
- Unknown opcodes, or excessive number of arguments will triguer an error.
- If there is any error, the syntax parsing will continue but no executable will be generated.
- If no executable is generated, the old pre-exisiting version will be left unchanged.