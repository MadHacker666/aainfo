#ifndef XLPACK_H
#define XLPACK_H

#include <string>
#include <windows.h>

const char* libname = "xlpack.dll";

//Файловый тип (вероятно аналог FILE), создаётся и используется библиотекой.
typedef void* xlpack_File;

//Структура для поиска файлов. Создаётся вызывающей стороной.
//Точная структура не известна. 100 байт взято с запасом.
struct xlpack_findstruct{
	char raw[100];
};

//callback для лога библиотеки. Возможно сигнатура другая. Приходит всегда одна строка в первом параметре.
typedef void __cdecl (*xlpack_loghandler)(const char *val, ...);

//В первом параметре полный путь к файлу в который будет писаться лог. Во втором функция в которую будет дублироваться лог.
//Любой параметр может быть NULL
typedef void __cdecl (*p_xlpack_SetFileLogHandler)(char const* filename, xlpack_loghandler lh);
//Убирает лог. Переваривает NULL. Назначение параметра неизвестно.
typedef void __cdecl (*p_xlpack_DestroyFileLogHandler)(void *);

//Инициализирует библиотеку. Должна быть вызвана до любой файловой операции.
typedef void __cdecl (*p_xlpack_CreateFileSystem)(void);
typedef void __cdecl (*p_xlpack_DestroyFileSystem)(void);

//Подключает pak файл или каталог в файловой системе к виртуальной файловой системе библиотеки.
//Первый параметр - путь монтирования. В корень / монтировать не умеет. Путь монтирования должен быть уникальным (не встречаться в полном пути к файлу).
//Клиентом для pak файла используется точка монтирования /master
//Второй параметр путь к монтируемому pak файлу или каталогу в файловой системе.
//Третий параметр - разрешение на запись. В случае монтирования только на чтение любая попытка записи даст ошибку, зато не угробите файл.
typedef void * __cdecl(*p_xlpack_mount)(char const* filepath, char const* filename, bool writeble);
//отключает точку монтирования от виртуально ФС библиотеки. Обязательно для pak файлов.
typedef int __cdecl(*p_xlpack_umount)(const char * mountpath);

//Аналоги стандартных функций работы с файлами.
//filename - путь до файла в виртуальной ФС библиотеки.
typedef xlpack_File * __cdecl(*p_xlpack_FOpen)(const char* filename, const char* filemode);
typedef void __cdecl(*p_xlpack_FClose)(xlpack_File *f);
typedef int __cdecl(*p_xlpack_FEof)(xlpack_File *f);
typedef int __cdecl(*p_xlpack_FFlush)(xlpack_File *f);
typedef __int64 __cdecl (*p_xlpack_FRead)(xlpack_File *f, char * dest, __int64 size);
typedef int __cdecl(*p_xlpack_GetC)(xlpack_File *f);

//Функции копирования файлов и каталогов.
//Принимают пути к файлам\каталогам в виртуальной ФС библиотеки
typedef int __cdecl(*p_xlpack_copydir)(const char * from, const char * to);
typedef int __cdecl(*p_xlpack_copyfile)(const char * from, const char * to);

//Проверка существования файла. Путь в виртуальной ФС библиотеки. Для каталога вернёт false.
typedef bool __cdecl(*p_xlpack_IsFileExist)(const char* filename);

//Аналоги winapi функций работы с ФС

//начинает поиск файлов по маске. Принимает пути в ВФС библиотеки. Поведение для pak файлов и файлов в ФС отличается.
//Возвращает HANDLE - идентификатор поиска
typedef int __cdecl(*p_xlpack_findfirst)(char const * mask, xlpack_findstruct& st);
typedef int __cdecl(*p_xlpack_findnext)(int serchHandle, xlpack_findstruct& st);
typedef int __cdecl(*p_xlpack_findclose)(int serchHandle);
//Проверяет, является ли текущая позиция поиска директорией.
typedef bool __cdecl(*p_xlpack_IsDirectory)(xlpack_findstruct& st);
//Возвращает имя текущего файла в поиске. Без пути.
typedef const char * __cdecl(*p_xlpack_GetFileName)(xlpack_findstruct& st);

//библиотека
HINSTANCE xlpack;

//Сами функции.
p_xlpack_SetFileLogHandler xlpack_SetFileLogHandler;
p_xlpack_DestroyFileLogHandler xlpack_DestroyFileLogHandler;
p_xlpack_CreateFileSystem xlpack_CreateFileSystem;
p_xlpack_DestroyFileSystem xlpack_DestroyFileSystem;
p_xlpack_mount xlpack_mount;
p_xlpack_umount xlpack_umount;
p_xlpack_FOpen xlpack_FOpen;
p_xlpack_FClose xlpack_FClose;
p_xlpack_FEof xlpack_FEof;
p_xlpack_FFlush xlpack_FFlush;
p_xlpack_FRead xlpack_FRead;
p_xlpack_GetC xlpack_GetC;
p_xlpack_copydir xlpack_copydir;
p_xlpack_copyfile xlpack_copyfile;
p_xlpack_IsFileExist xlpack_IsFileExist;
p_xlpack_findfirst xlpack_findfirst;
p_xlpack_findnext xlpack_findnext;
p_xlpack_findclose xlpack_findclose;
p_xlpack_IsDirectory xlpack_IsDirectory;
p_xlpack_GetFileName xlpack_GetFileName;

//обнуляет методы и освобождает библиотеку в случае неудачной загрузки.
bool bad_load()
{
	xlpack_CreateFileSystem = NULL;
	xlpack_SetFileLogHandler = NULL;
	xlpack_DestroyFileLogHandler = NULL;
	xlpack_DestroyFileSystem = NULL;
	xlpack_mount = NULL;
	xlpack_umount = NULL;
	xlpack_copydir = NULL;
	xlpack_copyfile = NULL;
	xlpack_FOpen = NULL;
	xlpack_FClose = NULL;
	xlpack_FEof = NULL;
	xlpack_GetC = NULL;
	xlpack_findfirst = NULL;
	xlpack_findnext = NULL;
	xlpack_findclose = NULL;
	xlpack_IsDirectory = NULL;
	xlpack_IsFileExist = NULL;
	xlpack_GetFileName = NULL;

	FreeLibrary(xlpack);
	xlpack = NULL;

	return false;
}

//basepath - путь до каталога с библиотекой с завершающим слешем на конце.
//Истина, если все методы успешно загружены.
bool LoadLib(std::string basepath)
{
	std::string fulllibname = basepath + libname;
	xlpack = LoadLibrary(fulllibname.c_str());
	if (xlpack){
		xlpack_CreateFileSystem = (p_xlpack_CreateFileSystem) GetProcAddress(xlpack, "?CreateFileSystem@@YA_NXZ");
        if (NULL == xlpack_CreateFileSystem) return bad_load();
        xlpack_SetFileLogHandler = (p_xlpack_SetFileLogHandler) GetProcAddress(xlpack, "?SetFileLogHandler@@YAPAXPBDP6AX0ZZ@Z");
        if (NULL == xlpack_SetFileLogHandler) return bad_load();

        xlpack_DestroyFileLogHandler = (p_xlpack_DestroyFileLogHandler) GetProcAddress(xlpack, "?DestroyFileLogHandler@@YAXPAX@Z");
        if (NULL == xlpack_DestroyFileLogHandler) return bad_load();
        xlpack_DestroyFileSystem = (p_xlpack_DestroyFileSystem) GetProcAddress(xlpack, "?DestroyFileSystem@@YAXXZ");
        if (NULL == xlpack_DestroyFileSystem) return bad_load();

        xlpack_mount = (p_xlpack_mount) GetProcAddress(xlpack, "?Mount@@YAPAXPBD0_N@Z");
        if (NULL == xlpack_mount) return bad_load();
        xlpack_umount = (p_xlpack_umount) GetProcAddress(xlpack, "?Unmount@@YA_NPBD@Z");
        if (NULL == xlpack_umount) return bad_load();

        xlpack_copydir = (p_xlpack_copydir) GetProcAddress(xlpack, "?CopyDir@@YA_NPBD0@Z");
        if (NULL == xlpack_copydir) return bad_load();
        xlpack_copyfile = (p_xlpack_copyfile) GetProcAddress(xlpack, "?Copy@@YA_NPBD0@Z");
        if (NULL == xlpack_copyfile) return bad_load();

        xlpack_FOpen = (p_xlpack_FOpen) GetProcAddress(xlpack, "?FOpen@@YAPAUFile@@PBD0@Z");
        if (NULL == xlpack_FOpen) return bad_load();
        xlpack_FClose = (p_xlpack_FClose) GetProcAddress(xlpack, "?FClose@@YAXAAPAUFile@@@Z");
        if (NULL == xlpack_FClose) return bad_load();
        xlpack_FEof = (p_xlpack_FEof) GetProcAddress(xlpack, "?FEof@@YAHPAUFile@@@Z");
        if (NULL == xlpack_FEof) return bad_load();
        xlpack_GetC = (p_xlpack_GetC) GetProcAddress(xlpack, "?Getc@@YAHPAUFile@@@Z");
        if (NULL == xlpack_GetC) return bad_load();

        xlpack_findfirst = (p_xlpack_findfirst) GetProcAddress(xlpack, "?FindFirst@@YAHPBDPAUafs_finddata@@@Z");
        if (NULL == xlpack_findfirst) return bad_load();
        xlpack_findnext = (p_xlpack_findnext) GetProcAddress(xlpack, "?FindNext@@YAHHPAUafs_finddata@@@Z");
        if (NULL == xlpack_findnext) return bad_load();
        xlpack_findclose = (p_xlpack_findclose) GetProcAddress(xlpack, "?FindClose@@YAHH@Z");
        if (NULL == xlpack_findclose) return bad_load();

        xlpack_IsDirectory = (p_xlpack_IsDirectory) GetProcAddress(xlpack, "?IsDirectory@@YA_NPBUafs_finddata@@@Z");
        if (NULL == xlpack_IsDirectory) return bad_load();
        xlpack_IsFileExist = (p_xlpack_IsFileExist) GetProcAddress(xlpack, "?IsFileExist@@YA_NPBD@Z");
        if (NULL == xlpack_IsFileExist) return bad_load();
        xlpack_GetFileName = (p_xlpack_GetFileName) GetProcAddress(xlpack, "?GetFileName@@YAPBDPBUafs_finddata@@@Z");
        if (NULL == xlpack_GetFileName) return bad_load();

		return true;
	}
	return false;
}

void freelib()
{
    //отключаем лог
    xlpack_DestroyFileLogHandler(NULL);
    //разрушаем ВФС
    xlpack_DestroyFileSystem();

    FreeLibrary(xlpack);
}

#endif // XLPACK_H
