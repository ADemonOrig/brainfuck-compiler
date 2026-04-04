#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <windows.h>
#include <cstdlib>
#include <vector>
#include <ctime>
#include "argagg.hpp"

using namespace std;
using namespace string_literals;

#define uint unsigned int


struct opcode
{
    string type = "!";
    uint count = 0;

    Command(string s = "!", uint arg = 0) {
        type = s;
        count = arg;
    }

    Command(const opcode& o) {
        type = o.type;
        stringu>< = o.count;
    }
};


struct compiler
{
    string version = "windows x64";
    string path = ".\\";
    clock_t start = 0;

    string code = "";
    string ccode = "";
    size_t size = 0;
    vector<opcode> cmds;
    int bal = 0;

    bool timer = 0;
    bool onlyC = 0;
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
            {"time", {"-t", "--time"}, "show compile time", 0},
            {"input", {"-i", "--input"}, "input file path", 1},
            {"output", {"-o", "--output"}, "output file path", 1},
            {"cells", {"-c", "--cells"}, "memory cells count", 1},
            {"echo", {"-e", "--echo"}, "echo input char", 0},
            {"cfile", {"-C"}, "only in C file", 0}
        }};

        try {
            args = args_parser.parse(argc, argv);
        }
        catch (...) {
            throw "brainfuck error\narguments\n\n";
        }

        path = filesystem::path(getExeDir()).parent_path().string();

        if (argc == 1) {
            cout << "brainfuck\n" << args_parser << '\n';
            throw "";
        }

        if (args.has_option("help")) {
            cout << "brainfuck help\n" << args_parser << '\n';
        }

        if (args.has_option("version")) {
            cout << "brainfuck version\n" << version << "\n\n";
        }

        if (args.has_option("dir")) {
            cout << "brainfuck directory\n" << path << "\n\n";
        }

        if (args.has_option("cells")) {
            cells = atoi(args["cells"].as<string>("30000").c_str());
            if (cells <= 0) throw "brainfuck error\nbad cells\n\n";
        }

        if (args.has_option("echo")) {
            echo = 1;
        }

        if (args.has_option("time")) {
            timer = 1;
            start = clock();
        }

        if (args.has_option("cfile")) {
            onlyC = 1;
        }

        if (args.has_option("output")) {
            output = args["output"].as<string>("a.exe");
        }

        if (args.pos.empty())
            throw
            ((args.has_option("dir") || args.has_option("help") || args.has_option("version"))
            && (!args.has_option("output")))
            ? "" : "brainfuck error\nno input file\n\n";
        else for (const char* arg : args.pos) input = string(arg);

        if (args.has_option("input")) {
            input = args["input"].as<string>("");
        }
    }



    void read() {
        clock_t times = clock();
        if (timer) cout << "read ";

        ifstream file(input);
        if (!file.is_open()) throw "brainfuck error\ncant open file\n\n";

        char ch = 0;
        while (file.get(ch)) {
            if (ch == '+' ||
                ch == '-' ||
                ch == '>' ||
                ch == '<' ||
                ch == '.' ||
                ch == ',' ||
                ch == '[' ||
                ch == ']'
            ) code += ch;
        }
        size = code.size();

        file.close();

        if (timer) cout << ((double)(clock() - times) / CLOCKS_PER_SEC) << " sec\n";
    }


    void save() {
        clock_t times = clock();
        if (timer) cout << "save ";

        string name = path + '\\' + middle;
        if (onlyC) name = ".\\"s + middle;

        ofstream file(name.c_str());
        if (!file.is_open()) throw "brainfuck error\ncant create middle file\n\n";

        file << ccode;

        file.close();

        if (timer) cout << ((double)(clock() - times) / CLOCKS_PER_SEC) << " sec\n";
    }


    void link() {
        clock_t times = clock();
        if (timer) cout << "link ";

        string cmd = path + "\\cc64.exe -I"s + path + " -L" + path + ' ' + path + '\\' + middle + " -o .\\" + output;

        if (system(cmd.c_str())) throw "brainfuck error\ncompile fail\n\n";
        filesystem::remove(path + '\\' + middle);

        if (timer) cout << ((double)(clock() - times) / CLOCKS_PER_SEC) << " sec\n";
    }


    void compile() {
        clock_t times = clock();
        if (timer) cout << "compile ";

        ccode = "";
        ccode += "#include <conio.h>\n\n";
        ccode += "#define MC " + to_string(cells) + "\n\n\n";
        ccode += "int main() {\n";
        ccode += "    unsigned char cells[MC];\n";
        ccode += "    unsigned char* ptr = cells;\n\n";

        uint i = 0;
        while (i < size) {
            if (code[i] == '+' || code[i] == '-') {
                int count = 0;
                while (i < size && (code[i] == '+' || code[i] == '-')) {
                    if (code[i] == '+') count++;
                    else count--;
                    i++;
                }
                if (count != 0) cmds.push_back(opcode("+-", count));
            }

            else if (code[i] == '>' || code[i] == '<') {
                int count = 0;
                while (i < size && (code[i] == '>' || code[i] == '<')) {
                    if (code[i] == '>') count++;
                    else count--;
                    i++;
                }
                if (count != 0) cmds.push_back(opcode("><", count));
            }

            else if (code[i] == '.') {
                int count = 0;
                while (i < size && code[i] == '.') {
                    count++;
                    i++;
                }
                cmds.push_back(opcode(".", count));
            }

            else if (code[i] == ',') {
                int count = 0;
                while (i < size && code[i] == ',') {
                    count++;
                    i++;
                }
                if (echo) cmds.push_back(opcode(",.", count));
                else cmds.push_back(opcode(",", count));
            }

            else if (code[i] == '[') {
                if (i + 2 < size && ((code[i+1] == '-' || code[i+1] == '+') && code[i+2] == ']')) {
                    while (i + 2 < size && ((code[i+1] == '-' || code[i+1] == '+') && code[i+2] == ']')) i += 3;
                    cmds.push_back(opcode("[-]", 0));
                }
                else {
                    int count = 0;
                    while (i < size && code[i] == '[') {
                        count++;
                        i++;
                    }
                    cmds.push_back(opcode("[", count));
                }
            }

            else if (code[i] == ']') {
                int count = 0;
                while (i < size && code[i] == ']') {
                    count++;
                    i++;
                }
                cmds.push_back(opcode("]", count));
            }

            else i++;
        }
        if (bal > 0) throw "brainfuck error\nunmatched [\n\n";

        size = cmds.size();
        i = 0;
        while (i < size) {
            if (cmds[i].type == "+-") {
                if (cmds[i].count == 1) ccode += "    *ptr++;\n";
                else if (cmds[i].count == -1) ccode += "    *ptr--;\n";
                else if (cmds[i].count > 1) ccode += "    *ptr += "s + to_string(cmds[i].count) + ";\n";
                else if (cmds[i].count < -1) ccode += "    *ptr -= "s + to_string(-(cmds[i].count)) + ";\n";
            }

            else if (cmds[i].type == "><") {
                if (cmds[i].count == 1) ccode += "    ptr++;\n";
                else if (cmds[i].count == -1) ccode += "    ptr--;\n";
                else if (cmds[i].count > 1) ccode += "    ptr += "s + to_string(cmds[i].count) + ";\n";
                else if (cmds[i].count < -1) ccode += "    ptr -= "s + to_string(-(cmds[i].count)) + ";\n";
                ccode += "    ptr = ((ptr - cells + MC) % MC) + cells;\n";
            }

            else if (cmds[i].type == ".") {
                for (int j = 0; j < cmds[i].count; j++) ccode += "    putch(*ptr);\n";
            }

            else if (cmds[i].type == ",") {
                for (int j = 0; j < cmds[i].count; j++) ccode += "    *ptr = getch();\n";
            }

            else if (cmds[i].type == ",.") {
                for (int j = 0; j < cmds[i].count; j++) {
                    ccode += "    *ptr = getch();\n";
                    ccode += "    putch(*ptr);\n";
                }
            }

            else if (cmds[i].type == "[") {
                for (int j = 0; j < cmds[i].count; j++) ccode += "    while (*ptr) {\n";
            }

            else if (cmds[i].type == "]") {
                for (int j = 0; j < cmds[i].count; j++) ccode += "    }\n";
            }

            else if (cmds[i].type == "[-]") {
                ccode += "    *ptr = 0;\n";
            }

            else i++;
        }

        ccode += "\n    return 0;\n";
        ccode += "}\n";

        if (timer) cout << ((double)(clock() - times) / CLOCKS_PER_SEC) << " sec\n";
    }
};



int main(int argc, char** argv) {
    compiler bfc;

    try {
        bfc.parse(argc, argv);
        bfc.read();
        bfc.compile();
        bfc.save();
        if (!bfc.onlyC) bfc.link();
        if (bfc.timer) cout << "total " << ((double)(clock() - bfc.start) / CLOCKS_PER_SEC) << " sec\n";
    }
    catch (const char* msg) {
        cout << msg;
        return 1;
    }

    return 0;
}
