#include <cctype>
#include <iostream>
#include <fstream>
#include <sstream>
#include <optional>

enum class TokenType {
    _return,
    integer_literal,
    semicolon,
};

struct Token {
    TokenType type;
    std::optional<std::string> value {};
};


std::vector<Token> tokenize(const std::string& str) {
    
    std::vector<Token> tokens;

    std::string buf;

    for (int i = 0; i < str.length(); i++) {
        const char& c = str[i];
        if (std::isalpha(c)) {
            buf.push_back(c);
            i++;
            while (std::isalnum(str[i])) {
                buf.push_back(str[i]);
                i++;
            }
            i--;
            if (buf == "return") {
                tokens.push_back({.type = TokenType::_return});
                buf.clear();
                continue;
            } else {
                std::cerr << "You messed up!! " << buf << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        else if (std::isdigit(c)) {
            buf.push_back(c);
            i++;
            while(std::isdigit(str.at(i))) {
                buf.push_back(str.at(i));
                i++;
            }
            i--;
            tokens.push_back({.type = TokenType::integer_literal, .value = buf});
            buf.clear();
            continue;
        }
        else if (c == ';') {
            tokens.push_back({.type = TokenType::semicolon});
            continue;
        }
        else if (std::isspace(c)) {
            continue;
        } else {
            std::cerr << "You messed up!! " << buf << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    return tokens;
}

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

    std::vector<Token> tokens = tokenize(contents);
    std::string asm_code = tokens_to_asm(tokens);

    std::cout << asm_code << std::endl;

    {
        std::fstream file("out.asm", std::ios::out);
        file << asm_code;
    }


    system("nasm -f macho64 out.asm -o out.o");
    system("clang -arch x86_64 -nostdlib -e _start -lSystem -Wl,-macosx_version_min,10.15 -o out out.o");

    return EXIT_SUCCESS;
}