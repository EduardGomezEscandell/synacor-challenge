#include "assembler.h"

using namespace assembler;

int main(int argc, char**argv)
{

	if(argc < 2 || argc > 3) 
	{
		PrintHelp();
		exit(EXIT_FAILURE);
	}

	std::string infile_name = argv[1];
	std::string outfile_name = argc == 3 ? argv[2] : GenerateOutputFileName(argv[1]);

	std::fstream infile (infile_name, infile.in);
	std::fstream outfile("vm_synacor_assembler_tmp_1.bin", outfile.out | outfile.binary);

	std::string line;
	std::size_t ln = 1;
	bool no_errors = true;

	while(std::getline(infile, line))
	{
		try 
		{
			auto instruction = ReadLine(line);
			if(instruction)
				outfile << *instruction;
		}
		catch(TooManyArgs& e)
		{
			no_errors = false;
			std::cerr << "Too many arguments: (expected " << GetOpData()[(size_t) e.m_instruction.op].n_args << ") in " 
					  << argv[1] <<":" << ln << ":" << e.m_start+1 << '\n' << line << '\n';
			size_t col=0;
			for(; col < e.m_start; ++col) std::cerr << " ";
			std::cerr << "^";
			for(++col; col < line.size(); ++col) std::cerr << "~";
			std::cerr << '\n';
		}
		catch(ErroneousToken& e)
		{
			no_errors = false;
			std::cerr << "Erroneous";
			switch (e.m_token) {
				case assembler::ErroneousToken::TokenType::INSTRUCTION: std::cerr << " instruction"; break;
				case assembler::ErroneousToken::TokenType::INTEGER:     std::cerr << " integer";     break;
				case assembler::ErroneousToken::TokenType::REGISTER:    std::cerr << " register";    break;
			}
			std::cerr << ": " << argv[1] 
			<<":" << ln << ":" << e.m_start+1 << '\n' << line << '\n';
			size_t col=0;
			for(; col < e.m_start; ++col) std::cerr << " ";
			std::cerr << "^";
			for(++col; col < e.m_start+e.m_len; ++col) std::cerr << "~";
			for(++col; col < line.size(); ++col) std::cerr << " ";
			std::cerr << '\n';
		}
		++ln;
	}

	if(!no_errors) {
		remove("vm_synacor_assembler_tmp_1.bin");
		return EXIT_FAILURE;
	}

	if(rename(outfile_name.c_str(), "vm_synacor_assembler_tmp_2.bin") != 0)
	{
		std::cerr << "Failed to cache old executable" << std::endl;
	}

	if(rename("vm_synacor_assembler_tmp_1.bin", outfile_name.c_str()) != 0)
	{
		std::cerr << "Failed to create new executable" << std::endl;

		if(rename(argv[2], "vm_synacor_assembler_tmp_2.bin") != 0)
			std::cerr << "Failed to reload old cached executable" << std::endl;

		return EXIT_FAILURE;
	} else {
		std::cout << "New executable stored as " << outfile_name << std::endl;
	}

	if(remove("vm_synacor_assembler_tmp_2.bin") != 0)
	{
		std::cerr << "Failed to remove old executable" << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}