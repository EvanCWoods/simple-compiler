#include <cctype>
#include <iostream>
#include <fstream>
#include <sstream>
#include <optional>

#include "./tokenizer.hpp"

// Function to translate the tokens to Assembly
std::string tokens_to_asm(const std::vector<Token>& tokens) {

    std::stringstream output;
    output << "global _start\n_start:\n";

    const char* exit_syscall = "0x2000001";  // macOS BSD syscall: exit

    for (int i=0; i<tokens.size(); i++) {
        const Token& token = tokens.at(i);
        if (token.type == TokenType::_return) {
            if (i + 1 < tokens.size() && tokens.at(i + 1).type == TokenType::integer_literal) {
                if (i + 2 < tokens.size() && tokens.at(i + 2).type == TokenType::semicolon) {
                    output << "    mov rax, " << exit_syscall << "\n";
                    output << "    mov rdi, " << tokens.at(i + 1).value.value() << "\n";
                    output << "    syscall\n";
                }
            }
        }
    }
    return output.str();
}

void compile_and_run() {
    // Compile the assembly code to an object file
    system("nasm -f macho64 out.asm -o out.o");
    // Link the object file to an executable
    system("clang -target x86_64-apple-macos10.15 -nostdlib -e _start -lSystem -Wl,-w -o out out.o");
    // Run the executable
    system("./out");
}


int main(int argc, char* argv[]) {

    if (argc != 2) {
        std::cerr << "Incorrect Usage" << std::endl;
        return EXIT_FAILURE;
    }

    std::string contents;
    {
        std::stringstream contents_stream;
        std::fstream input(argv[1], std::ios::in);
        contents_stream << input.rdbuf();
        contents = contents_stream.str();
    }

    Tokenizer tokenizer(std::move(contents));
    std::vector<Token> tokens = tokenizer.tokenize();
    std::string asm_code = tokens_to_asm(tokens);

    {
        std::fstream file("out.asm", std::ios::out);
        file << asm_code;
    }

    compile_and_run();

    return EXIT_SUCCESS;
}