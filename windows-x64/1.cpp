#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <windows.h>
#include <cstdlib>
#include "argagg.hpp"
using namespace std;
#define uint unsigned int



struct compiler
{
    string version = "v3.0 windows x64";
    string path = ".\\";

    string code = "";
    uint size = 0;
    bool useful = 0;
    uint bal = 0;
    uint indent = 0;

    bool onlyMiddle = 0;
    string input = "";
    string middle = "a.c";
    string output = "a.exe";
    uint cells = 30000;
    bool echo = 0;


    string getExeDir() {
        char buffer[MAX_PATH];
        GetModuleFileNameA(NULL, buffer, MAX_PATH);
        return string(buffer);
    }

    void parse(int argc, char** argv) {
        argagg::parser_results args;

        argagg::parser args_parser{{
            {"help", {"-h", "--help"}, "show this help", 0},
            {"version", {"-v", "--version"}, "show version", 0},
            {"dir", {"-d", "--dir"}, "show this compiler dir", 0},
            {"input", {"-i", "--input"}, "input file path", 1},
            {"output", {"-o", "--output"}, "output file path", 1},
            {"cells", {"-c", "--cells"}, "memory cells count", 1},
            {"echo", {"-e", "--echo"}, "echo input char", 0},
            {"midfile", {"-m", "--middle"}, "only in middle file", 0}
        }};

        try {
            args = args_parser.parse(argc, argv);
        }
        catch (...) {
            throw "arguments\n";
        }

        path = filesystem::path(getExeDir()).parent_path().string();

        if (argc == 1) {
            cout << path << '\n';
            cout << version << '\n';
            cout << args_parser;
            throw "";
        }

        if (args.has_option("dir")) {
            cout << path << '\n';
        }

        if (args.has_option("version")) {
            cout << version << '\n';
        }

        if (args.has_option("help")) {
            cout << args_parser;
        }

        if (args.has_option("cells")) {
            cells = atoi(args["cells"].as<string>("30000").c_str());
            if (cells <= 0) throw "bad cells\n";
        }

        if (args.has_option("echo")) {
            echo = 1;
        }

        if (args.has_option("midfile")) {
            onlyMiddle = 1;
        }

        if (args.has_option("output")) {
            output = args["output"].as<string>("a.exe");
        }

        if (args.has_option("input")) {
            input = args["input"].as<string>("");
        }
        else {
            if (args.pos.empty())
                throw ((args.has_option("dir") || args.has_option("help") || args.has_option("version")) && (!args.has_option("output")))
                ? "" : "no input file\n";
            else for (const char* arg : args.pos) input = string(arg);
        }
    }


    bool clean() {
        bool change = 0;
        string bcode = code;
        code = "";

        uint i = 0;
        while (i < size) {
            if (bcode[i] == '+' || bcode[i] == '-') {
                uint s = i;
                int count = 0;
                while (i < size && (bcode[i] == '+' || bcode[i] == '-' || (i + 2 < size ? (bcode[i] == '[' && (bcode[i+1] == '+' || bcode[i+1] == '-') && bcode[i+2] == ']') : 0))) {
                    if (bcode[i] == '+') count++;
                    else if (bcode[i] == '-') count--;
                    else if (i + 2 < size ? (bcode[i] == '[' && (bcode[i+1] == '+' || bcode[i+1] == '-') && bcode[i+2] == ']') : 0) {
                        uint si = i;
                        while (i + 2 < size ? (bcode[i] == '[' && (bcode[i+1] == '+' || bcode[i+1] == '-') && bcode[i+2] == ']') : 0) i += 3;
                        count = 0;
                        code += "[-]";
                        change = 1;
                        i--;
                    }
                    i++;
                }
                string opt;
                if (count > 0) opt = string((int)((unsigned char)count), '+');
                else if (count < 0) opt = string((int)((unsigned char)(-count)), '-');
                code += opt;
                if (bcode.substr(s, i-s) != opt) change = 1;
            }

            else if (bcode[i] == '>' || bcode[i] == '<') {
                uint s = i;
                int count = 0;
                while (i < size && (bcode[i] == '>' || bcode[i] == '<')) {
                    if (bcode[i] == '>') count++;
                    else count--;
                    i++;
                }
                string opt;
                if (count > 0) opt = string((int)((unsigned char)count), '>');
                else if (count < 0) opt = string((int)((unsigned char)(-count)), '<');
                code += opt;
                if (bcode.substr(s, i-s) != opt) change = 1;
            }

            else if (i + 2 < size && (bcode[i] == '[' && (bcode[i+1] == '+' || bcode[i+1] == '-') && bcode[i+2] == ']')) {
                uint s = i;
                while (i + 2 < size && (bcode[i] == '[' && (bcode[i+1] == '+' || bcode[i+1] == '-') && bcode[i+2] == ']')) i += 3;
                code += "[-]";
                if (s + 3 != i) change = 1;
            }

            else {
                code += bcode[i];
                i++;
            }
        }

        size = code.length();

        return change;
    }


    void read() {
        if (!filesystem::exists(input)) throw "input file not found\n";
        ifstream file(input);
        if (!file.is_open()) throw "cant open file\n";

        char ch = 0;
        while (file.get(ch)) {
            if (ch == '+' || ch == '-' || ch == '>' || ch == '<' || ch == '.' || ch == ',' || ch == '[' || ch == ']') {
                code += ch;
                if (ch == '.' || ch == ',') useful = 1;
                if (ch == '[') bal++;
                else if (ch == ']') {
                    bal--;
                    if (bal < 0) throw "unmatched ]\n";
                }
            }
        }
        if (bal > 0) throw "unmatched [\n";

        size = code.length();

        file.close();
    }


    void link() {
        string cmd = path + "\\cc64.exe -I" + path + " -L" + path + ' ' + middle + " -o " + output;

        if (system(cmd.c_str()) != 0) throw "link fail\n";
        filesystem::remove(middle);
    }


    void compile() {
        if (!onlyMiddle) middle = path + '\\' + middle;
        ofstream file(middle.c_str());
        if (!file.is_open()) throw "cant create middle file\n";

        file << "#include <conio.h>\n\n";
        file << "#define MC " << cells << "\n\n\n";
        file << "int main() {\n";
        file << "    unsigned char cells[MC];\n";
        file << "    unsigned char* ptr = cells;\n\n";

        uint i = 0;
        while (i < size) {
            if (code[i] == '+' || code[i] == '-') {
                int count = 0;
                while (i < size && (code[i] == '+' || code[i] == '-')) {
                    if (code[i] == '+') count++;
                    else count--;
                    i++;
                }
                if (count == 1) file << string(indent * 4, ' ') << "    (*ptr)++;\n";
                else if (count == -1) file << string(indent * 4, ' ') << "    (*ptr)--;\n";
                else if (count > 1) file << string(indent * 4, ' ') << "    *ptr += " << (int)((unsigned char)count) << ";\n";
                else if (count < -1) file << string(indent * 4, ' ') << "    *ptr -= " << (int)((unsigned char)(-count)) << ";\n";
            }

            else if (code[i] == '>' || code[i] == '<') {
                int count = 0;
                while (i < size && (code[i] == '>' || code[i] == '<')) {
                    if (code[i] == '>') count++;
                    else count--;
                    i++;
                }
                if (count == 1) file << string(indent * 4, ' ') << "    ptr++;\n";
                else if (count == -1) file << string(indent * 4, ' ') << "    ptr--;\n";
                else if (count > 1) file << string(indent * 4, ' ') << "    ptr += " << (int)((unsigned char)count) << ";\n";
                else if (count < -1) file << string(indent * 4, ' ') << "    ptr -= " << (int)((unsigned char)(-count)) << ";\n";
                if (count != 0) file << string(indent * 4, ' ') << "    ptr = cells + ((ptr - cells + MC) % MC);\n";
            }

            else if (code[i] == '.') {
                file << string(indent * 4, ' ') << "    putch(*ptr);\n";
                i++;
            }

            else if (code[i] == ',') {
                if (echo) file << string(indent * 4, ' ') << "    *ptr = getche();\n";
                else file << string(indent * 4, ' ') << "    *ptr = getch();\n";
                i++;
            }

            else if (code[i] == '[') {
                if (i + 2 < size && (code[i+1] == '+' || code[i+1] == '-') && code[i+2] == ']') {
                    while (i + 2 < size && code[i] == '[' && (code[i+1] == '+' || code[i+1] == '-') && code[i+2] == ']') i += 3;
                    if (code[i] == '+' || code[i] == '-') {
                        int count = 0;
                        while (i < size && (code[i] == '+' || code[i] == '-')) {
                            if (code[i] == '+') count++;
                            else count--;
                            i++;
                        }
                        if (count > 0) file << string(indent * 4, ' ') << "    *ptr = " << (int)((unsigned char)count) << ";\n";
                        else if (count < 0) file << string(indent * 4, ' ') << "    *ptr = " << (int)((unsigned char)count) << ";\n";
                    }
                    else file << string(indent * 4, ' ') << "    *ptr = 0;\n";
                }
                else {
                    file << string(indent * 4, ' ') << "    while (*ptr) {\n";
                    indent++;
                    i++;
                }
            }

            else if (code[i] == ']') {
                indent--;
                file << string(indent * 4, ' ') << "    }\n";
                i++;
            }

            else i++;
        }

        if (!useful) {
            file.close();
            file.open(middle, ios::trunc);
            file << "\nint main() {\n";
        }

        file << '\n';
        file << "    return 0;\n";
        file << "}\n";

        file.close();
    }
};



int main(int argc, char** argv) {
    compiler bfc;

    try {
        bfc.parse(argc, argv);
        bfc.read();
        while (bfc.clean());
        bfc.compile();
        if (!bfc.onlyMiddle) bfc.link();
    }
    catch (const char* msg) {
        if (filesystem::exists(bfc.middle)) filesystem::remove(bfc.middle);
        if (filesystem::exists(bfc.output)) filesystem::remove(bfc.output);
        cout << msg;
        return 1;
    }

    return 0;
}
