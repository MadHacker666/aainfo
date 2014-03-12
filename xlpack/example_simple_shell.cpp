#include "xlpack.h"

#include <iostream>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

const char* pck_name = "game_pak";
const char* masterfs = "/master";
const char* realfs = "/fs";

string globalpath;

enum ConsoleColor
{
        Black         = 0,
        Blue          = 1,
        Green         = 2,
        Cyan          = 3,
        Red           = 4,
        Magenta       = 5,
        Brown         = 6,
        LightGray     = 7,
        DarkGray      = 8,
        LightBlue     = 9,
        LightGreen    = 10,
        LightCyan     = 11,
        LightRed      = 12,
        LightMagenta  = 13,
        Yellow        = 14,
        White         = 15
};

void SetColor(ConsoleColor text, ConsoleColor background)
{
    HANDLE hConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsoleHandle, (WORD)((background << 4) | text));
}

void __cdecl log(const char *val, ...){

   SetColor(LightCyan, Black);
   cout << val;
        SetColor(LightGray, Black);
}


bool fillMethod(string basepath){
    if (LoadLib(basepath)){

        xlpack_CreateFileSystem();
        string logfile = basepath + libname + ".log";
        xlpack_SetFileLogHandler(logfile.c_str(), &log);

        return true;
    }
    return false;
}


bool mountBase(string base){
    string full_pck_name = base + pck_name;

    cout << "mount " << base << " to " << realfs << endl;
    void * ret = xlpack_mount(realfs, base.c_str(), true);
    if (NULL == ret) return false;

    cout << "readonly mount " << full_pck_name << " to " << masterfs << endl;
    ret = xlpack_mount(masterfs, full_pck_name.c_str(), false);
    if (NULL == ret){
        SetColor(LightRed, Black);
        cout << "Bad pck file " << full_pck_name << endl;
        SetColor(LightGray, Black);
        return false;
    }

    return true;
}
void umountBase(){
    xlpack_umount(masterfs);
    xlpack_umount(realfs);
}

vector<string> &split(const string &s, char delim, vector<string> &elems) {
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

vector<string> split(const string &s, char delim) {
    vector<string> elems;
    split(s, delim, elems);
    return elems;
}

string extractFilePath(const string base){
    if (base.compare("/") == 0) return base;
    unsigned found = base.find_last_of("/\\");
    return base.substr(0, found + 1);
}
string extractFileDir(const string base){
    if (base.compare("/") == 0) return base;
    unsigned found = base.find_last_of("/\\");
    return base.substr(0, found);
}


string absolutePath(const string path){
    if (path.size() == 0) return globalpath;
    if (path.at(0) == '/' || path.at(0) == '\\') return path;
    if (path.size() == 1 && path.at(0) == '.') return globalpath;
    string base, relative;
    base = extractFileDir(globalpath);
    relative = path;
    while (relative.substr(0,2).compare("..") == 0){
        base = extractFileDir(base);
        if (relative.size() < 3) relative = "";
        else relative = relative.substr(3, relative.size() - 3);
    }
    return base + "/" + relative;
}

void cd(string path){
    globalpath = absolutePath(path);
    if (globalpath.at(globalpath.size() - 1) != '/' && globalpath.at(globalpath.size() - 1) != '\\') globalpath += '/';
}

void ls(string path){
    if (path.compare("/") == 0)
    {
        SetColor(LightBlue, Black);
        cout << masterfs << " " << realfs << endl;
        SetColor(LightGray, Black);
        return;
    }
    path = absolutePath(path);// + "*";
    cout << path << endl;
    if (path.at(path.size() - 1) == '/') path += "*";
    xlpack_findstruct fs;
    int findhandle = xlpack_findfirst(path.c_str(), fs);
    if (findhandle != -1){
        int find;
        do{
           if (xlpack_IsDirectory(fs)) SetColor(LightBlue, Black);
            else SetColor(White, Black);
            cout << xlpack_GetFileName(fs) << " ";
           find = xlpack_findnext(findhandle, fs);
        }while(find != -1);
        xlpack_findclose(findhandle);
    } else cout << "empty";
    cout << endl;
    SetColor(LightGray, Black);
}

void checkPath(string absolutePath, bool &exist, bool &dir){


    exist = xlpack_IsFileExist(absolutePath.c_str());
    if (exist){
        dir = false;
        return;
    }
    xlpack_findstruct fs;
    int findhandle = xlpack_findfirst(absolutePath.c_str(), fs);
    exist = (findhandle != -1);
    dir = exist; // не сработает для пустых каталогов.
    xlpack_findclose(findhandle);
}

void cp(string src, string dest){
    src = absolutePath(src);
    dest = absolutePath(dest);

    SetColor(White, Black);
    bool isExist = false, isDir = false;
    checkPath(src, isExist, isDir);

    if (!isExist){
        cout << "bad source path: " << src << endl;
        SetColor(LightGray, Black);
        return;
    }
    if (isDir){
        cout << "copy catalog " << src << " to " << dest << endl;
        xlpack_copydir(src.c_str(), dest.c_str());
    }else{
        cout << "copy file " << src << " to " << dest << endl;
        xlpack_copyfile(src.c_str(), dest.c_str());
    }

    SetColor(LightGray, Black);
}

void cat(string path){
    path = absolutePath(path);
    bool isExist, isDir;
    checkPath(path, isExist, isDir);

    SetColor(White, Black);
    if (!isExist){
        cout << "bad path" << endl;
        SetColor(LightGray, Black);
        return;
    }
    if (isDir){
        cout << "directory" << endl;
        SetColor(LightGray, Black);
        return;
    }

    xlpack_File *f = xlpack_FOpen(path.c_str(), "r");
    while (!xlpack_FEof(f)){
        cout << (char) xlpack_GetC(f);
    }
    cout <<endl;
    SetColor(LightGray, Black);
}

void help(){
    cout << "Arhe age game arhive shell" << endl;
    SetColor(White, Black);
    cout << "\thelp\t";
    SetColor(LightGray, Black);
    cout << "show this message" << endl;
    SetColor(White, Black);
    cout << "\tls <path>\t";
    SetColor(LightGray, Black);
    cout << "show file list" << endl;
    SetColor(White, Black);
    cout << "\tcd path\t";
    SetColor(LightGray, Black);
    cout << "change catalog" << endl;
    SetColor(White, Black);
    cout << "\tcp src dest\t";
    SetColor(LightGray, Black);
    cout << "copy file or catalog. Destination only /fs/" << endl;
    SetColor(White, Black);
    cout << "\tq\t";
    SetColor(LightGray, Black);
    cout << "exit" << endl;
}

void parsecmd(const string &cmd){
    std::vector<std::string> x;
    split(cmd, ' ', x);
    if (x.size() >= 1){
        if (x.at(0).compare("help") == 0) help();
        if (x.at(0).compare("ls") == 0){
           if (x.size() >= 2) ls(x.at(1));
            else ls(globalpath);
        }
        if (x.at(0).compare("cd") == 0 && (x.size() >= 2)) cd(x.at(1));
        if (x.at(0).compare("cp") == 0 ){
            if (x.size() == 3) cp(x.at(1), x.at(2));
            else {
                SetColor(White, Black);
                cout << "usage: cp source destination \ndestination only in " << realfs << endl;
                SetColor(LightGray, Black);
            }
        }
        if (x.at(0).compare("cat") == 0 && (x.size() >= 2)) cat(x.at(1));

    }

}

int main(int argc, char** argv)
{
    globalpath = string(masterfs) + "/";
    string workdir = extractFilePath(string(argv[0]));


    if (fillMethod(workdir)){
        //createPCK(workdir);
        if (mountBase(workdir)){
            help();
            string cmd;
            for (;;){
                cout << globalpath << " :> ";
                getline(cin, cmd);
                if (cmd.compare("q") == 0) break;
                parsecmd(cmd);
            }
            umountBase();
        }

        freelib();
    }

    return 0;
}
