#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <windows.h>
#include <cstdlib>
#include <ctime>
#include <regex>
#include "argagg.hpp"

using namespace std;
using namespace string_literals;

#define uint unsigned int



struct compiler
{
    string version = "v2.0 windows x64";
    string path = ".\\";
    clock_t start = 0;

    string code = "";
    string mcode = "";
    size_t size = 0;
    bool useful = 0;
    int bal = 0;

    bool timer = 0;
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
            {"time", {"-t", "--time"}, "show compile time", 0},
            {"input", {"-i", "--input"}, "input file path", 1},
            {"output", {"-o", "--output"}, "output file path", 1},
            {"cells", {"-c", "--cells"}, "memory cells count", 1},
            {"echo", {"-e", "--echo"}, "echo input char", 0},
            {"mfile", {"-m"}, "only in middle file", 0}
        }};

        try {
            args = args_parser.parse(argc, argv);
        }
        catch (...) {
            throw "error > arguments\n";
        }

        path = filesystem::path(getExeDir()).parent_path().string();

        if (argc == 1) {
            cout << "help >\n" << args_parser << '\n';
            cout << "version > " << version << '\n';
            cout << "directory > " << path << '\n';
            throw "";
        }

        if (args.has_option("help")) {
            cout << "help >\n" << args_parser << '\n';
        }

        if (args.has_option("version")) {
            cout << "version > " << version << '\n';
        }

        if (args.has_option("dir")) {
            cout << "directory > " << path << '\n';
        }

        if (args.has_option("cells")) {
            cells = atoi(args["cells"].as<string>("30000").c_str());
            if (cells <= 0) throw "error > bad cells\n";
        }

        if (args.has_option("echo")) {
            echo = 1;
        }

        if (args.has_option("time")) {
            timer = 1;
            start = clock();
        }

        if (args.has_option("mfile")) {
            onlyMiddle = 1;
        }

        if (args.has_option("output")) {
            output = args["output"].as<string>("a.exe");
        }

        if (args.has_option("input")) {
            input = args["input"].as<string>("");
        }
        else if (!args.has_option("input")) {
            if (args.pos.empty())
                throw ((args.has_option("dir") || args.has_option("help") || args.has_option("version")) && (!args.has_option("output")))
                ? "" : "error > no input file\n";
            else for (const char* arg : args.pos) input = string(arg);
        }
    }


    void read() {
        clock_t times = clock();
        if (timer) cout << "read > ";

        if (!filesystem::exists(input)) throw "error > input file not found\n";
        ifstream file(input);
        if (!file.is_open()) throw "error > cant open file\n";

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
        size = code.length();

        file.close();

        if (timer) cout << filesystem::file_size(input) << "B > ";
        if (timer) cout << ((double)(clock() - times) / CLOCKS_PER_SEC) << " sec\n";
    }


    void save() {
        clock_t times = clock();
        if (timer) cout << "save > ";

        string filePath = path + '\\' + middle;
        if (onlyMiddle) filePath = middle;

        ofstream file(filePath.c_str());
        if (!file.is_open()) throw "error > cant create middle file\n";

        file << mcode;

        file.close();

        if (timer) cout << filesystem::file_size(filePath) << "B > ";
        if (timer) cout << ((double)(clock() - times) / CLOCKS_PER_SEC) << " sec\n";
    }


    void link() {
        clock_t times = clock();
        if (timer) cout << "link > ";

        string cmd = path + "\\cc64.exe -I"s + path + " -L" + path + ' ' + path + '\\' + middle + " -o " + output;

        if (system(cmd.c_str())) throw "error > compile fail\n";
        filesystem::remove(path + '\\' + middle);

        if (timer) cout << filesystem::file_size(output) << "B > ";
        if (timer) cout << ((double)(clock() - times) / CLOCKS_PER_SEC) << " sec\n";
    }


    void compile() {
        clock_t times = clock();
        if (timer) cout << "compile > ";

        mcode = "#include <conio.h>\n\n";
        mcode += "#define MC " + to_string(cells) + "\n\n\n";
        mcode += "int main() {\n";
        mcode += "    unsigned char cells[MC];\n";
        mcode += "    unsigned char* ptr = cells;\n\n";

        regex rmath(R"((\+|-)+)");
        regex rmov(R"((>|<)+)");
        regex rput(R"(\.+)");
        regex rget(R"(,+)");
        regex rwhile(R"(\[+)");
        regex rend(R"(\]+)");

        regex rcycle(R"(\[(\+|-)*\])");
        regex rmul(R"(\[(((>|<)+(\+|-)+)+(>|<)+-|-((>|<)+(\+|-)+)+(>|<)+)\])");

        smatch match;

        size_t i = 0;
        while (i < size) {
            // ++++++----+++--
            if (regex_search(code.begin() + i, text.end(), match, rmul)) {
                int count = 0;
                string cmd = match[0].str();
                i += cmd.length();
            }

            // >>>>>><<<>>><<
            else if (code[i] == '>' || code[i] == '<') {
                int count = 0;
                while (i < size && (code[i] == '>' || code[i] == '<')) {
                    if (code[i] == '>') count++;
                    else count--;
                    i++;
                }
                if (count == 1) mcode += "    ptr++;\n";
                else if (count == -1) mcode += "    ptr--;\n";
                else if (count > 1) mcode += "    ptr += "s + to_string(count) + ";\n";
                else if (count < -1) mcode += "    ptr -= "s + to_string(-count) + ";\n";
                if (count != 0) mcode += "    ptr = cells + ((ptr - cells + MC) % MC);\n";
            }

            // ........
            else if (code[i] == '.') {
                useful = 1;
                while (i < size && code[i] == '.') {
                    mcode += "    putch(*ptr);\n";
                    i++;
                }
            }

            // ,,,,,,,
            else if (code[i] == ',') {
                useful = 1;
                while (i < size && code[i] == ',') {
                    if (echo) {
                        mcode += "    *ptr = getch();\n";
                        mcode += "    putch(*ptr);\n";
                    }
                    else mcode += "    *ptr = getch();\n";
                    i++;
                }
            }

            // [[[[[[[[[
            // [-][+]
            else if (code[i] == '[') {
                // [-][+]
                if (i + 2 < size && ((code[i+1] == '-' || code[i+1] == '+') && code[i+2] == ']')) {
                    while (i + 2 < size && ((code[i+1] == '-' || code[i+1] == '+') && code[i+2] == ']')) i += 3;
                    mcode += "    *ptr = 0;\n";
                }
                // [[[[[[[[[[[[[[
                else {
                    while (i < size && code[i] == '[') {
                        mcode += "    while (*ptr) {\n";
                        i++;
                        bal++;
                    }
                }
            }

            // ]]]]]]]
            else if (code[i] == ']') {
                while (i < size && code[i] == ']') {
                    if (bal <= 0) throw "error > unmatched ]\n";
                    mcode += "    }\n";
                    i++;
                    bal--;
                }
            }
        }
        if (bal > 0) throw "error > unmatched [\n";

        if (!useful) mcode = "int main() {\n";

        mcode += "\n    return 0;\n";
        mcode += "}\n";

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
        if (!bfc.onlyMiddle) bfc.link();
        if (bfc.timer) cout << "total > " << ((double)(clock() - bfc.start) / CLOCKS_PER_SEC) << " sec\n";
    }
    catch (const char* msg) {
        cout << msg;
        return 1;
    }

    return 0;
}
