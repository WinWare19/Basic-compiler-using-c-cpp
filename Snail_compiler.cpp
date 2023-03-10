#ifndef _UNICODE
	#define _UNICODE
#endif

// global definitions
#define ERROR_INVALID_ID 0x1
#define ERROR_INVALID_SEPARATOR 0x2
#define ERROR_UNEXPECTED_SEPARATOR 0x3
#define ERROR_EXPECTED_ID 0x4
#define VAR_TYPE_INTEGER 0x5
#define VAR_TYPE_REAL 0x6
#define VAR_TYPE_STRING 0x7
#define VAR_TYPE_UNKNOWN 0x8
                                      
#include <Windows.h>
#include <memoryapi.h>
#include <processthreadsapi.h>
#include <WinUser.h>
#include <wingdi.h>
#include <iostream>
#include <strsafe.h>
#include <shellapi.h>
#include <vector>

#pragma comment (lib, "user32.lib")
#pragma comment (lib, "gdi32.lib")
#pragma comment (lib, "comdlg32.lib")
#pragma comment (lib, "shell32.lib")

typedef struct _WINDOWINFOW {
	LPWSTR caption;
	HANDLE cwf_handle;
	HWND main_window_handle;
	HWND edit_control_handle;
	HWND cmp_window_handle;
	std::vector<std::vector<std::pair<LPCSTR, BYTE> > > cmp_vector;
	WORD current_view;
} WINDOWINFOW, * PWINDOWINFOW, * LPWINDOWINFOW;
typedef struct _SNAIL_VAR {
	LPSTR name;
	WORD type;
	UINT line;
	UINT line_position;
	LPSTR value;
} SNAIL_VAR, * PSNAIL_VAR, * LPSNAIL_VAR;

INT __stdcall wWinMain(HINSTANCE, HINSTANCE, LPWSTR, INT);

LRESULT __stdcall MainWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT __stdcall CompileWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT __stdcall Custom_BTN_Proc(HWND, UINT, WPARAM, LPARAM);

HRESULT __stdcall Init(INT);
HRESULT __stdcall FillAndRegisterWndClass(LPCWSTR, LPCWSTR, LPCWSTR, COLORREF, WNDPROC);
HRESULT __stdcall ErrorHandler(HWND, LPCWSTR, LPDWORD);
BOOL __stdcall GetItemIndex(HWND, ULONGLONG*);
BOOL __stdcall EnableMenuItems(HMENU, std::pair<WORD, BOOL>*, SIZE_T);
std::vector<WINDOWINFOW> __stdcall RemoveItemFromVector(std::vector<WINDOWINFOW>, HWND);
DWORD __stdcall GetFilenameFromCaption(LPWSTR, LPWSTR*);
DWORD __stdcall GetFileExtension(LPWSTR, LPWSTR*);

DWORD __stdcall GUI_ThreadStartingExePoint(LPVOID);

DWORD __stdcall ReadLine(LPSTR, LPSTR*, LPDWORD, LPDWORD);
ULONG _stdcall AnalyseLexicale(LPSTR, std::vector<std::pair<LPCSTR, BYTE> >*);
ULONG __stdcall AnalyseSyntaxique(LPSTR, std::vector<std::pair<LPCSTR, BYTE> >*, std::vector<SNAIL_VAR>*);
ULONG __stdcall  AnalyseSemantique(LPSTR, std::vector<std::pair<LPCSTR, BYTE> >*, std::vector<SNAIL_VAR>);
BOOL __stdcall Generate_CPP_PseudoCode(LPSTR, LPDWORD);

BOOL __stdcall Is_ID(LPSTR, LPDWORD);
BOOL __stdcall Is_IntegerNumber(LPSTR, LPDWORD);
BOOL __stdcall Is_FloatNumber(LPSTR, LPDWORD);
BOOL __stdcall Is_CmpOpeartor(LPSTR, LPDWORD);
BOOL __stdcall Is_ArtOperator(LPSTR);
BOOL __stdcall Is_Comment(LPSTR, LPDWORD);
BOOL __stdcall Is_String(LPSTR, LPDWORD);

std::vector<WINDOWINFOW> windows_list;
HINSTANCE image_base = (HINSTANCE)0x0;
LPWSTR init_caption = (LPWSTR)0x0;
LONG DefaultMessageHandler = 0x0;
HACCEL accel_handle = (HACCEL)0x0;
CRITICAL_SECTION critical_section = { 0x0 };

INT __stdcall wWinMain(HINSTANCE current_instance, HINSTANCE previous_instance, LPWSTR cmdline, INT show_flag) {
	UNREFERENCED_PARAMETER(previous_instance);
	UNREFERENCED_PARAMETER(show_flag);
	UNREFERENCED_PARAMETER(cmdline);
	DWORD args_count = 0x0;
	LPWSTR* cmdline_args = CommandLineToArgvW(GetCommandLineW(), (int*)&args_count);
	if (args_count < 0x2) {
		init_caption = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lstrlenW(L"Untitled - Snail Compiler") * 0x2 + 0x2);
		StringCchCopyW((STRSAFE_LPWSTR)init_caption, lstrlenW(L"Untitled - Snail Compiler") + 0x1, (STRSAFE_LPCWSTR)L"Untitled - Snail Compiler");
	}
	else if (args_count == 0x2) {
		init_caption = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lstrlenW(cmdline_args[1]) * 0x2 + lstrlenW(L" - Snail Compiler") * 0x2 + 0x2);
		StringCchCopyW((STRSAFE_LPWSTR)init_caption, lstrlenW(cmdline_args[1]) + 0x1, (STRSAFE_LPCWSTR)cmdline_args[1]);
		StringCchCatW((STRSAFE_LPWSTR)init_caption, lstrlenW(cmdline_args[1]) + lstrlenW(L" - Snail Compiler") + 0x1, (STRSAFE_LPCWSTR)L" - Snail Compiler");
	}
	else return MessageBoxW(GetDesktopWindow(), L"To much command line arguments", L"Error", MB_ICONERROR | MB_OKCANCEL);
	InitializeCriticalSection(&critical_section);
	windows_list = std::vector<WINDOWINFOW>();
	image_base = current_instance;
	if (FAILED(Init(0x0))) {
		LocalFree((HLOCAL)cmdline_args);
		HeapFree(GetProcessHeap(), 0x0, init_caption);
		DeleteCriticalSection(&critical_section);
		if(accel_handle) DestroyAcceleratorTable(accel_handle);
		return EXIT_FAILURE;
	}
	MSG window_message = { 0x0 };
	while (GetMessageW(&window_message, NULL, 0x0, 0x0)) {
		if (!TranslateAcceleratorW(windows_list.at(0x0).main_window_handle, accel_handle, &window_message)) {
			TranslateMessage(&window_message);
			DispatchMessageW(&window_message);
		}
	}
	while (true) {
		EnterCriticalSection(&critical_section);
		if (windows_list.size() <= 0x0) break;
		LeaveCriticalSection(&critical_section);
	}
	LocalFree((HLOCAL)cmdline_args);
	HeapFree(GetProcessHeap(), 0x0, init_caption);
	DestroyAcceleratorTable(accel_handle);
	DeleteCriticalSection(&critical_section);
	return window_message.wParam;
}

LRESULT __stdcall MainWndProc(HWND window_handle, UINT window_message, WPARAM word_param, LPARAM long_param) {
	static HMENU menu_bar = (HMENU)0x0;
	ULONGLONG index = 0x0;
	static LPWSTR caption = (LPWSTR)0x0;
	GetItemIndex(window_handle, &index);
	switch (window_message) {
		case WM_SHOWWINDOW: {
			DefWindowProcA(window_handle, window_message, word_param, long_param);
			if (word_param == 0x1 && long_param == 0x0) {
				if (lstrcmpW(caption, L"Untitled - Snail Compiler") != 0x0) {
					LPWSTR filename = (LPWSTR)0x0;
					if (GetFilenameFromCaption(caption, &filename) != 0x0) {
						LPWSTR file_ext = (LPWSTR)0x0;
						windows_list.at(index).cwf_handle = CreateFileW(filename, GENERIC_WRITE | GENERIC_READ, 0x0, (LPSECURITY_ATTRIBUTES)0x0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE)0x0);
						if (windows_list.at(index).cwf_handle == INVALID_HANDLE_VALUE || (GetFileExtension(filename, &file_ext) == 0x0 || (lstrcmpW(file_ext, L"snl") != 0x0 && lstrcmpW(file_ext, L"Snl") != 0x0 && lstrcmpW(file_ext, L"sNl") != 0x0 && lstrcmpW(file_ext, L"snL") != 0x0 && lstrcmpW(file_ext, L"SNl") != 0x0 && lstrcmpW(file_ext, L"SnL") != 0x0 && lstrcmpW(file_ext, L"sNL") != 0x0 && lstrcmpW(file_ext, L"SNL") != 0x0))) {
							HeapFree(GetProcessHeap(), 0x0, caption);
							SetWindowTextW(window_handle, L"Untitled - Snail Compiler");
							caption = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, GetWindowTextLengthW(window_handle) * 0x2 + 0x2);
							GetWindowTextW(window_handle, caption, GetWindowTextLengthW(window_handle) + 0x1);
							std::pair<WORD, BOOL> menu_items[0x2] = { std::make_pair(1003, FALSE), std::make_pair(1005, FALSE) };
							EnableMenuItems(GetMenu(windows_list.at(index).main_window_handle), menu_items, 0x2);
							ErrorHandler(window_handle, L"CreateFileW() failed : the specified file does not exist or it is not valid !!", (LPDWORD)0x0);
						}
						else {
							LARGE_INTEGER file_size = { 0x0 };
							GetFileSizeEx(windows_list.at(index).cwf_handle, &file_size);
							LPSTR filedata_buffer = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, file_size.QuadPart + 0x1);
							if (!filedata_buffer) {
								HeapFree(GetProcessHeap(), 0x0, filename);
								SetWindowTextW(window_handle, L"Untitled - Snail Compiler");
								caption = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, GetWindowTextLengthW(window_handle) * 0x2 + 0x2);
								GetWindowTextW(window_handle, caption, GetWindowTextLengthW(window_handle) + 0x1);
								std::pair<WORD, BOOL> menu_items[0x2] = { std::make_pair(1003, FALSE), std::make_pair(1005, FALSE) };
								EnableMenuItems(GetMenu(windows_list.at(index).main_window_handle), menu_items, 0x2);
								ErrorHandler(window_handle, L"HeapAllloc() failed !!", (LPDWORD)0x0);
								return 0x0L;
							}
							DWORD bytes_read = 0x0;
							if (!ReadFile(windows_list.at(index).cwf_handle, (LPVOID)filedata_buffer, (DWORD)file_size.QuadPart, &bytes_read, (LPOVERLAPPED)0x0) || bytes_read != file_size.QuadPart) {
								HeapFree(GetProcessHeap(), 0x0, filename);
								HeapFree(GetProcessHeap(), 0x0, (LPVOID)filedata_buffer);
								SetWindowTextW(window_handle, L"Untitled - Snail Compiler");
								caption = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, GetWindowTextLengthW(window_handle) * 0x2 + 0x2);
								GetWindowTextW(window_handle, caption, GetWindowTextLengthW(window_handle) + 0x1);
								std::pair<WORD, BOOL> menu_items[0x2] = { std::make_pair(1003, FALSE), std::make_pair(1005, FALSE) };
								EnableMenuItems(GetMenu(windows_list.at(index).main_window_handle), menu_items, 0x2);
								ErrorHandler(window_handle, L"ReadFile() failed !!", (LPDWORD)0x0);
								return 0x0L;
							}
							filedata_buffer[bytes_read] = '\0';
							if (!windows_list.at(index).edit_control_handle) windows_list.at(index).edit_control_handle = CreateWindowExW(0x0, L"EDIT", (LPCWSTR)0x0, WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | WS_VSCROLL | WS_HSCROLL, 0x0, 0x0, 783, 490, window_handle, (HMENU)0x0, image_base, (LPVOID)0x0);
							SetWindowTextA(windows_list.at(index).edit_control_handle, filedata_buffer);
							std::pair<WORD, BOOL> menu_items[0x2] = { std::make_pair(1003, TRUE), std::make_pair(1005, TRUE) };
							EnableMenuItems(GetMenu(windows_list.at(index).main_window_handle), menu_items, 0x2);
							HeapFree(GetProcessHeap(), 0x0, (LPVOID)filedata_buffer);
							windows_list.at(index).cmp_vector.at(0).clear();
							windows_list.at(index).cmp_vector.at(1).clear();
							windows_list.at(index).cmp_vector.at(2).clear();
						}
						HeapFree(GetProcessHeap(), 0x0, filename);
						HeapFree(GetProcessHeap(), 0x0, file_ext);
						StringCchCopyW((STRSAFE_LPWSTR)windows_list.at(index).caption, lstrlenW(caption) + 0x1, (STRSAFE_LPCWSTR)caption);
					}
				}
			}
			return 0x0L;
		}
		case WM_CREATE: {
			caption = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, GetWindowTextLengthW(window_handle) * 0x2 + 0x2);
			GetWindowTextW(window_handle, caption, GetWindowTextLengthW(window_handle) + 0x1);
			menu_bar = CreateMenu();
			if (!menu_bar) {
				HRESULT returned_value = ErrorHandler(window_handle, L"CreateMenu() failed !!", (LPDWORD)0x0);
				SendMessageW(window_handle, WM_CLOSE, 0x0, 0x0);
				return returned_value;
			}
			const wchar_t* menu_items_text_content[] = { L"File", L"Debug", L"Help"};
			const wchar_t* file_menu_items_text_content[] = { L"New							Ctrl+N", L"New window			Ctrl+Shift+N", L"Open					Ctrl+O", L"Save		Ctrl+S", L"Exit					Ctrl+X"};
			const wchar_t* debug_menu_items_text_content[] = { L"Compile			 Ctrl+B", L"Generate a c++ pseudo code			 Ctrl+F5" };
			for (UINT i = 0x0; i < 0x3; i++) {
				MENUITEMINFOW menu_item = { 0x0 };
				menu_item.cbSize = sizeof MENUITEMINFOW;
				menu_item.fMask = MIIM_SUBMENU | MIIM_STATE | MIIM_STRING;
				menu_item.fState = MFS_ENABLED;
				menu_item.cch = lstrlenW(menu_items_text_content[i]);
				menu_item.dwTypeData = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, menu_item.cch * 0x2 + 0x2);
				if (!menu_item.dwTypeData) {
					ErrorHandler(window_handle, L"CreateMenu() failed !!", (LPDWORD)0x0);
					continue;
				}
				if (FAILED(StringCchCopyW((STRSAFE_LPWSTR)menu_item.dwTypeData, menu_item.cch + 0x1, (STRSAFE_LPCWSTR)menu_items_text_content[i]))) {
					RtlZeroMemory(menu_item.dwTypeData, menu_item.cch * 0x2);
					HeapFree(GetProcessHeap(), 0x0, (LPVOID)menu_item.dwTypeData);
					ErrorHandler(window_handle, L"StringCchCopyW() failed !!", (LPDWORD)0x0);
					continue;
				}
				menu_item.hSubMenu = CreatePopupMenu();
				if (!menu_item.hSubMenu) {
					RtlZeroMemory(menu_item.dwTypeData, menu_item.cch * 0x2);
					HeapFree(GetProcessHeap(), 0x0, (LPVOID)menu_item.dwTypeData);
					ErrorHandler(window_handle, L"StringCchCopyW() failed !!", (LPDWORD)0x0);
					continue;
				}
				if (i == 0x0) {
					for (UINT j = 0; j < 0x5; j++) {
						MENUITEMINFOW sub_menu_item = { 0x0 };
						sub_menu_item.cbSize = sizeof MENUITEMINFOW;
						sub_menu_item.fMask = MIIM_STATE | MIIM_STRING | MIIM_ID;
						if(j != 0x3) sub_menu_item.fState = MFS_ENABLED;
						else if (lstrcmpW(caption, L"Untitled - Snail Compiler") == 0x0) sub_menu_item.fState = MFS_DISABLED;
						sub_menu_item.wID = j + 1000;
						sub_menu_item.cch = lstrlenW(file_menu_items_text_content[j]);
						sub_menu_item.dwTypeData = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sub_menu_item.cch * 0x2 + 0x2);
						if (FAILED(StringCchCopyW((STRSAFE_LPWSTR)sub_menu_item.dwTypeData, sub_menu_item.cch + 0x1, (STRSAFE_LPCWSTR)file_menu_items_text_content[j]))) {
							RtlZeroMemory(sub_menu_item.dwTypeData, sub_menu_item.cch * 0x2);
							HeapFree(GetProcessHeap(), 0x0, (LPVOID)sub_menu_item.dwTypeData);
							ErrorHandler(window_handle, L"StringCchCopyW() failed !!", (LPDWORD)0x0);
							continue;
						}
						InsertMenuItemW(menu_item.hSubMenu, j, TRUE, &sub_menu_item);
					}
				}
				else if (i == 0x1) {
					for (UINT j = 0; j < 0x2; j++) {
						MENUITEMINFOW sub_menu_item = { 0x0 };
						sub_menu_item.cbSize = sizeof MENUITEMINFOW;
						sub_menu_item.fMask = MIIM_STATE | MIIM_STRING | MIIM_ID;
						if (i == 0x0 && lstrcmpW(caption, L"Untitled - Snail Compiler") != 0x0) sub_menu_item.fState = MFS_ENABLED;
						else sub_menu_item.fState = MFS_DISABLED;
						sub_menu_item.wID = j + 1005;
						sub_menu_item.cch = lstrlenW(debug_menu_items_text_content[j]);
						sub_menu_item.dwTypeData = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sub_menu_item.cch * 0x2 + 0x2);
						if (FAILED(StringCchCopyW((STRSAFE_LPWSTR)sub_menu_item.dwTypeData, sub_menu_item.cch + 0x1, (STRSAFE_LPCWSTR)debug_menu_items_text_content[j]))) {
							RtlZeroMemory(sub_menu_item.dwTypeData, sub_menu_item.cch * 0x2);
							HeapFree(GetProcessHeap(), 0x0, (LPVOID)sub_menu_item.dwTypeData);
							ErrorHandler(window_handle, L"StringCchCopyW() failed !!", (LPDWORD)0x0);
							continue;
						}
						InsertMenuItemW(menu_item.hSubMenu, j, TRUE, &sub_menu_item);
					}
				}
				else {
					MENUITEMINFOW sub_menu_item = { 0x0 };
					sub_menu_item.cbSize = sizeof MENUITEMINFOW;
					sub_menu_item.fMask = MIIM_STATE | MIIM_STRING | MIIM_ID;
					sub_menu_item.fState = MFS_ENABLED;
					sub_menu_item.wID = 1007;
					sub_menu_item.cch = lstrlenW(L"Help me ?	Ctrl+H");
					sub_menu_item.dwTypeData = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sub_menu_item.cch * 0x2 + 0x2);
					if (FAILED(StringCchCopyW((STRSAFE_LPWSTR)sub_menu_item.dwTypeData, sub_menu_item.cch + 0x1, (STRSAFE_LPCWSTR)L"Help me ?	Ctrl+H"))) {
						RtlZeroMemory(sub_menu_item.dwTypeData, sub_menu_item.cch * 0x2);
						HeapFree(GetProcessHeap(), 0x0, (LPVOID)sub_menu_item.dwTypeData);
						ErrorHandler(window_handle, L"StringCchCopyW() failed !!", (LPDWORD)0x0);
						continue;
					}
					InsertMenuItemW(menu_item.hSubMenu, 0x0, TRUE, &sub_menu_item);
				}
				if (!InsertMenuItemW(menu_bar, i, TRUE, &menu_item)) {
					ErrorHandler(window_handle, L"InsertMenuItemW() failed !!", (LPDWORD)0x0);
					continue;
				}
			}

			SetMenu(window_handle, menu_bar);
			return 0x0L;
		}
		case WM_COMMAND: {
			switch (LOWORD(word_param)) {
				case 1000: { // New
					if (windows_list.at(index).edit_control_handle) SetWindowTextA(windows_list.at(index).edit_control_handle, (LPCSTR)"");
					else windows_list.at(index).edit_control_handle = CreateWindowExW(0x0, L"EDIT", (LPCWSTR)0x0, WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_AUTOVSCROLL| ES_MULTILINE | WS_VSCROLL | WS_HSCROLL, 0x0, 0x0, 783, 490, window_handle, (HMENU)0x0, image_base, (LPVOID)0x0);
					std::pair<WORD, BOOL> menu_items[0x2] = { std::make_pair(1003, TRUE), std::make_pair(1005, TRUE) };
					EnableMenuItems(GetMenu(windows_list.at(index).main_window_handle), menu_items, 0x2);
					return 0L;
				}
				case 1001: { // New window
					if (windows_list.size() == 0xa) {
						ErrorHandler(window_handle, L"You can't create more than 10 window at time !!", (LPDWORD)0x0);
						return 0x0L;
					}
					HANDLE gui_thread = CreateThread((LPSECURITY_ATTRIBUTES)0x0, 0x0, GUI_ThreadStartingExePoint, (LPVOID)0x0, 0x0, (LPDWORD)0x0);
					if (!gui_thread) ErrorHandler(window_handle, L"CreateThread() failed !!", (LPDWORD)0x0);
					return 0L;
				}
				case 1002: { // Open
					OPENFILENAMEW open_dialog_info = { 0x0 };
					open_dialog_info.lStructSize = sizeof OPENFILENAMEW;
					open_dialog_info.hwndOwner = window_handle;
					open_dialog_info.lpstrTitle = L"Snail Compiler";
					open_dialog_info.lpstrFilter = L"Snail files\0*.SNL\0\0";
					open_dialog_info.lpstrFile = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MAX_PATH * 0x2);
					open_dialog_info.lpstrFile[0] = L'\0';
					open_dialog_info.nMaxFile = MAX_PATH;
					open_dialog_info.lpstrInitialDir = (LPCWSTR)0x0;
					open_dialog_info.nFilterIndex = 1;
					open_dialog_info.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
					if (GetOpenFileNameW(&open_dialog_info)) {
						if (windows_list.at(index).cwf_handle != INVALID_HANDLE_VALUE) {
							CloseHandle(windows_list.at(index).cwf_handle);
							windows_list.at(index).cwf_handle = INVALID_HANDLE_VALUE;
						}
						windows_list.at(index).cwf_handle = CreateFileW(open_dialog_info.lpstrFile, GENERIC_READ | GENERIC_WRITE, 0x0, (LPSECURITY_ATTRIBUTES)0x0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE)0x0);
						if (!windows_list.at(index).cwf_handle) {
							HeapFree(GetProcessHeap(), 0x0, (LPVOID)open_dialog_info.lpstrFile);
							ErrorHandler(window_handle, L"CreateFileW() failed !!", (LPDWORD)0x0);
							return 0x0L;
						}
						RtlZeroMemory(windows_list.at(index).caption, lstrlenW(windows_list.at(index).caption) * 0x2);
						StringCchCopyW((STRSAFE_LPWSTR)windows_list.at(index).caption, lstrlenW(open_dialog_info.lpstrFile + open_dialog_info.nFileOffset) + 0x1, (STRSAFE_LPCWSTR)(open_dialog_info.lpstrFile + open_dialog_info.nFileOffset));
						StringCchCatW((STRSAFE_LPWSTR)windows_list.at(index).caption, lstrlenW(open_dialog_info.lpstrFile + open_dialog_info.nFileOffset) + 0x4, (STRSAFE_LPCWSTR)L" - ");
						StringCchCatW((STRSAFE_LPWSTR)windows_list.at(index).caption, lstrlenW(open_dialog_info.lpstrFile + open_dialog_info.nFileOffset) + 0x3 + lstrlenW(L"Snail Compiler") + 0x1, (STRSAFE_LPCWSTR)L"Snail Compiler");
						SetWindowTextW(window_handle, windows_list.at(index).caption);
						LARGE_INTEGER file_size = { 0x0 };
						GetFileSizeEx(windows_list.at(index).cwf_handle, &file_size);
						LPSTR filedata_buffer = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, file_size.QuadPart + 0x1);
						if (!filedata_buffer) {
							HeapFree(GetProcessHeap(), 0x0, (LPVOID)open_dialog_info.lpstrFile);
							ErrorHandler(window_handle, L"HeapAllloc() failed !!", (LPDWORD)0x0);
							return 0x0L;
						}
						DWORD bytes_read = 0x0;
						if (!ReadFile(windows_list.at(index).cwf_handle, (LPVOID)filedata_buffer, (DWORD)file_size.QuadPart, &bytes_read, (LPOVERLAPPED)0x0) || bytes_read != file_size.QuadPart) {
							HeapFree(GetProcessHeap(), 0x0, (LPVOID)filedata_buffer);
							HeapFree(GetProcessHeap(), 0x0, (LPVOID)open_dialog_info.lpstrFile);
							ErrorHandler(window_handle, L"ReadFile() failed !!", (LPDWORD)0x0);
							return 0x0L;
						}
						filedata_buffer[bytes_read] = '\0';
						if (!windows_list.at(index).edit_control_handle) windows_list.at(index).edit_control_handle = CreateWindowExW(0x0, L"EDIT", (LPCWSTR)0x0, WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | WS_VSCROLL | WS_HSCROLL, 0x0, 0x0, 783, 490, window_handle, (HMENU)0x0, image_base, (LPVOID)0x0);
						SetWindowTextA(windows_list.at(index).edit_control_handle, filedata_buffer);
						std::pair<WORD, BOOL> menu_items[0x2] = { std::make_pair(1003, TRUE), std::make_pair(1005, TRUE) };
						EnableMenuItems(GetMenu(windows_list.at(index).main_window_handle), menu_items, 0x2);
						HeapFree(GetProcessHeap(), 0x0, (LPVOID)filedata_buffer);
						windows_list.at(index).cmp_vector.at(0).clear();
						windows_list.at(index).cmp_vector.at(1).clear();
						windows_list.at(index).cmp_vector.at(2).clear();
					}
					else ErrorHandler(window_handle, L"GetOpneFileNameW() failed", (LPDWORD)0x0);
					HeapFree(GetProcessHeap(), 0x0, (LPVOID)open_dialog_info.lpstrFile);
					return 0L;
				}
				case 1003: { // Save
					DWORD allocation_size = (DWORD)GetWindowTextLengthA(windows_list.at(index).edit_control_handle);
					LPSTR filedata_buffer = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, allocation_size + 0x1);
					GetWindowTextA(windows_list.at(index).edit_control_handle, filedata_buffer, allocation_size + 0x1);
					filedata_buffer[allocation_size] = L'\0';
					if (!windows_list.at(index).cwf_handle || windows_list.at(index).cwf_handle == INVALID_HANDLE_VALUE) {
						OPENFILENAMEW open_dialog_info = { 0x0 };
						open_dialog_info.lStructSize = sizeof OPENFILENAMEW;
						open_dialog_info.hwndOwner = window_handle;
						open_dialog_info.lpstrTitle = L"Snail Compiler";
						open_dialog_info.lpstrFilter = L"Snail files\0*.SNL\0\0";
						open_dialog_info.lpstrFile = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MAX_PATH * 0x2);
						open_dialog_info.lpstrFile[0] = L'\0';
						open_dialog_info.nMaxFile = MAX_PATH;
						open_dialog_info.lpstrInitialDir = (LPCWSTR)0x0;
						open_dialog_info.nFilterIndex = 1;
						open_dialog_info.Flags = OFN_PATHMUSTEXIST;
						if (GetSaveFileNameW(&open_dialog_info)) {
							if (open_dialog_info.nFileExtension == 0x0) StringCchCatW((STRSAFE_LPWSTR)open_dialog_info.lpstrFile, lstrlenW(open_dialog_info.lpstrFile) + 0x5, (STRSAFE_LPCWSTR)L".snl");
							windows_list.at(index).cwf_handle = CreateFileW(open_dialog_info.lpstrFile, GENERIC_WRITE, 0x0, (LPSECURITY_ATTRIBUTES)0x0, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, (HANDLE)0x0);
							if (windows_list.at(index).cwf_handle != INVALID_HANDLE_VALUE) {
								RtlZeroMemory(windows_list.at(index).caption, lstrlenW(windows_list.at(index).caption) * 0x2);
								StringCchCopyW((STRSAFE_LPWSTR)windows_list.at(index).caption, lstrlenW(open_dialog_info.lpstrFile + open_dialog_info.nFileOffset) + 0x1, (STRSAFE_LPCWSTR)(open_dialog_info.lpstrFile + open_dialog_info.nFileOffset));
								StringCchCatW((STRSAFE_LPWSTR)windows_list.at(index).caption, lstrlenW(open_dialog_info.lpstrFile + open_dialog_info.nFileOffset) + 0x4, (STRSAFE_LPCWSTR)L" - ");
								StringCchCatW((STRSAFE_LPWSTR)windows_list.at(index).caption, lstrlenW(open_dialog_info.lpstrFile + open_dialog_info.nFileOffset) + 0x3 + lstrlenW(L"Snail Compiler") + 0x1, (STRSAFE_LPCWSTR)L"Snail Compiler");
								SetWindowTextW(window_handle, windows_list.at(index).caption);
								DWORD bytes_written = 0x0;
								if (!WriteFile(windows_list.at(index).cwf_handle, filedata_buffer, allocation_size, &bytes_written, (LPOVERLAPPED)0x0) || bytes_written != allocation_size) ErrorHandler(window_handle, L"WriteFile() failed !!", (LPDWORD)0x0);
							}
							else ErrorHandler(window_handle, L"CreateFile() failed !!", (LPDWORD)0x0);
						}
						HeapFree(GetProcessHeap(), 0x0, (LPVOID)open_dialog_info.lpstrFile);
					}
					else {
						FILE_NAME_INFO* filename_info = (FILE_NAME_INFO*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof FILE_NAME_INFO + 260);
						filename_info->FileNameLength = 260;
						GetFileInformationByHandleEx(windows_list.at(index).cwf_handle, FILE_INFO_BY_HANDLE_CLASS::FileNameInfo, (LPVOID)filename_info, sizeof FILE_NAME_INFO + 260);
						CloseHandle(windows_list.at(index).cwf_handle);
						windows_list.at(index).cwf_handle = INVALID_HANDLE_VALUE;
						DeleteFileW(filename_info->FileName);
						windows_list.at(index).cwf_handle = CreateFileW(filename_info->FileName, GENERIC_READ | GENERIC_WRITE, 0x0, (LPSECURITY_ATTRIBUTES)0x0, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, (HANDLE)0x0);
						DWORD bytes_written = 0x0;
						if (!WriteFile(windows_list.at(index).cwf_handle, filedata_buffer, allocation_size, &bytes_written, (LPOVERLAPPED)0x0) || bytes_written != allocation_size) ErrorHandler(window_handle, L"WriteFile() failed !!", (LPDWORD)0x0);
					}
					HeapFree(GetProcessHeap(), 0x0, (LPVOID)filedata_buffer);
					return 0L;
				}
				case 1004: { // Exit
					SendMessageW(window_handle, WM_CLOSE, word_param, long_param);
					return 0L;
				}
				case 1005: { // Compile
					std::pair<UINT, UINT> screen_coordinates = std::make_pair(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
					windows_list.at(index).cmp_window_handle = CreateWindowExW(0x0, L"WC_SubWnd", windows_list.at(index).caption, WS_CAPTION | WS_POPUPWINDOW | WS_VSCROLL | WS_HSCROLL, screen_coordinates.first/2 - 380, screen_coordinates.second/2 - 255, 760, 510, window_handle, (HMENU)0x0, image_base, (LPVOID)&index);
					ShowWindow(windows_list.at(index).cmp_window_handle, SW_SHOWNORMAL);
					UpdateWindow(windows_list.at(index).cmp_window_handle);
					return 0L;
				}
				case 1006: { // generate a c++ pseudo code
					return 0L;
				}
				case 1007: { // Help me ?
					return 0L;
				}
				default: return 0L;
			}
		}
		case WM_SYSCOMMAND: {
			if (word_param == SC_MAXIMIZE) return 0x0;
			DefWindowProcA(window_handle, window_message, word_param, long_param);
			return 0x0;
		}
		case WM_CLOSE: {
			DestroyWindow(window_handle);
			return S_OK;
		}
		case WM_DESTROY: {
			windows_list.at(index).cmp_vector.at(0).clear();
			windows_list.at(index).cmp_vector.at(1).clear();
			windows_list.at(index).cmp_vector.at(2).clear();
			if (windows_list.at(index).cwf_handle != INVALID_HANDLE_VALUE) {
				CloseHandle(windows_list.at(index).cwf_handle);
				windows_list.at(index).cwf_handle = INVALID_HANDLE_VALUE;
			}
			EnterCriticalSection(&critical_section);
			windows_list = RemoveItemFromVector(windows_list, window_handle);
			LeaveCriticalSection(&critical_section);
			PostQuitMessage(EXIT_SUCCESS);
			return S_OK;
		}
		default: return DefWindowProcW(window_handle, window_message, word_param, long_param);
	}
}
LRESULT __stdcall Custom_BTN_Proc(HWND window_handle, UINT window_message, WPARAM word_param, LPARAM long_param) {
	if (window_message == WM_ERASEBKGND) {
		if (long_param == (LPARAM)0x0) return CallWindowProcW((WNDPROC)DefaultMessageHandler, window_handle, window_message, word_param, long_param);;
		RECT client_area = { 0x0 };
		GetClientRect(window_handle, &client_area);
		FillRect((HDC)word_param, &client_area, CreateSolidBrush(RGB(0x00, 0x00, 0x00)));
		DWORD text_len = GetWindowTextLengthW(window_handle);
		LPWSTR text_content = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, text_len * 0x2 + 0x2);
		GetWindowTextW(window_handle, text_content, text_len + 0x1);
		text_content[text_len] = L'\0';
		SetTextColor((HDC)word_param, 0xffffff);
		SetBkMode((HDC)word_param, TRANSPARENT);
		TextOutW((HDC)word_param, (INT)long_param, 4, text_content, text_len);
		HeapFree(GetProcessHeap(), 0x0, (LPVOID)text_content);
		return 0x1L;
	}
	return CallWindowProcW((WNDPROC)DefaultMessageHandler, window_handle, window_message, word_param, long_param);
}
LRESULT __stdcall CompileWndProc(HWND window_handle, UINT window_message, WPARAM word_param, LPARAM long_param) {
	static HWND owner_window_handle = (HWND)0x0;
	static HWND buttons[0x3] = { (HWND)0x0 };
	static std::vector<HWND> comp_messages = std::vector<HWND>();
	static std::vector<SNAIL_VAR> declared_identifiers = std::vector<SNAIL_VAR>();
	ULONGLONG index = 0x0;
	owner_window_handle = GetWindow(window_handle, GW_OWNER);
	GetItemIndex(owner_window_handle, &index);
	static LONG errors_count[] = { -1, -1, -1 };
	switch (window_message) {
		case WM_CREATE: {
			SetScrollRange(window_handle, SB_BOTH, 0x0, 0x0, TRUE);
			const wchar_t* text_contents[] = { L"Analyse lexicale", L"Analyse syntaxique", L"Analyse semantique" };
			for (UINT i = 0x0; i < 0x3; i++) {
				*(buttons + i) = CreateWindowExW(0x0, L"BUTTON", text_contents[i], WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON | BS_CENTER, 0 + 240 * i, 0x0, 225, 25, window_handle, (HMENU)(1008 + i), image_base, (LPVOID)0x0);
				DefaultMessageHandler = SetWindowLongPtrW(*(buttons + i), GWL_WNDPROC, (LONG)Custom_BTN_Proc);
			}
			return 0x0L;
		}
		case WM_COMMAND: {
			LPSTR src_code = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, GetWindowTextLengthA(windows_list.at(index).edit_control_handle) + 0x1);
			GetWindowTextA(windows_list.at(index).edit_control_handle, src_code, GetWindowTextLengthA(windows_list.at(index).edit_control_handle) + 0x1);
			SCROLLINFO scroll_info = { 0x0 };
			scroll_info.cbSize = sizeof SCROLLINFO;
			scroll_info.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
			switch (LOWORD(word_param)) {
				case 1008: {
					for (UINT i = 0x0; i < comp_messages.size(); i++) DestroyWindow(comp_messages.at(i));
					comp_messages.clear();
					SendMessageW((HWND)long_param, WM_ERASEBKGND, (WPARAM)GetWindowDC((HWND)long_param), (LPARAM)60);
					windows_list.at(index).current_view = 0x0;
					windows_list.at(index).cmp_vector.at(0x0).clear();
					errors_count[0x0] = AnalyseLexicale(src_code, &windows_list.at(index).cmp_vector.at(0x0));
					RECT update_rect = { 0x0 };
					GetClientRect(window_handle, &update_rect);
					update_rect.left = 15;
					update_rect.top = 30;
					scroll_info.nPos = scroll_info.nMin = 0x0;
					scroll_info.nMax = windows_list.at(index).cmp_vector.at(0x0).size() - 1;
					scroll_info.nPage = (update_rect.bottom - update_rect.top + 1) / 25;
					SetScrollInfo(window_handle, SB_VERT, &scroll_info, TRUE);
					for (UINT i = 0x0; i < min(scroll_info.nMax + 1, scroll_info.nPos + scroll_info.nPage); i++) {
						HWND static_control_handle = CreateWindowExA(0x0, "STATIC", windows_list.at(index).cmp_vector.at(0x0).at(i).first, WS_CHILD | WS_VISIBLE | SS_LEFT, 10, 30 + 25 * i, 1480, 25, window_handle, (HMENU)0x0, image_base, (LPVOID)0x0);
						SetWindowLongPtrA(static_control_handle, GWL_USERDATA, (LONG)windows_list.at(index).cmp_vector.at(0x0).at(i).second);
						comp_messages.push_back(static_control_handle);
					}
				}break;
				case 1009: {
					if (errors_count[0x0] == -1) return (LRESULT)MessageBoxW(window_handle, L"l'analyse syntaxique ne peut pas etre faire avant l'analyse lexicale", L"Snail Compiler", MB_OK | MB_ICONERROR);
					else if (errors_count[0x0] > 0) return (LRESULT)MessageBoxW(window_handle, L"You still have lexical errors, please check your source code and try to fix them", L"Snail Compiler", MB_OK | MB_ICONERROR);
					for (UINT i = 0x0; i < comp_messages.size(); i++) DestroyWindow(comp_messages.at(i));
					comp_messages.clear();
					SendMessageW((HWND)long_param, WM_ERASEBKGND, (WPARAM)GetWindowDC((HWND)long_param), (LPARAM)49);
					windows_list.at(index).current_view = 0x1;
					windows_list.at(index).cmp_vector.at(0x1).clear();
					for (UINT i = 0x0; i < declared_identifiers.size(); i++) HeapFree(GetProcessHeap(), 0x0, (LPVOID)declared_identifiers.at(i).name);
					declared_identifiers.clear();
					errors_count[0x1] = AnalyseSyntaxique(src_code, &windows_list.at(index).cmp_vector.at(0x1), &declared_identifiers);
					RECT update_rect = { 0x0 };
					GetClientRect(window_handle, &update_rect);
					update_rect.left = 15;
					update_rect.top = 30;
					scroll_info.nPos = scroll_info.nMin = 0x0;
					scroll_info.nMax = windows_list.at(index).cmp_vector.at(0x1).size() - 1;
					scroll_info.nPage = (update_rect.bottom - update_rect.top + 1) / 25;
					SetScrollInfo(window_handle, SB_VERT, &scroll_info, TRUE);
					for (UINT i = 0x0; i < min(scroll_info.nMax + 1, scroll_info.nPos + scroll_info.nPage); i++) {
						HWND static_control_handle = CreateWindowExA(0x0, "STATIC", windows_list.at(index).cmp_vector.at(0x1).at(i).first, WS_CHILD | WS_VISIBLE | SS_LEFT, 10, 30 + 25 * i, 1480, 25, window_handle, (HMENU)0x0, image_base, (LPVOID)0x0);
						SetWindowLongPtrA(static_control_handle, GWL_USERDATA, (LONG)windows_list.at(index).cmp_vector.at(0x1).at(i).second);
						comp_messages.push_back(static_control_handle);
					}
				}break;
				case 1010: {
					if (errors_count[0x1] == -1) return (LRESULT)MessageBoxW(window_handle, L"l'analyse semantique ne peut pas etre faire avant l'analyse syntaxique", L"Snail Compiler", MB_OK | MB_ICONERROR);
					else if (errors_count[0x1] > 0) return (LRESULT)MessageBoxW(window_handle, L"You still have syntactic errors, please check your source code and try to fix them", L"Snail Compiler", MB_OK | MB_ICONERROR);
					for (UINT i = 0x0; i < comp_messages.size(); i++) DestroyWindow(comp_messages.at(i));
					comp_messages.clear();
					SendMessageW((HWND)long_param, WM_ERASEBKGND, (WPARAM)GetWindowDC((HWND)long_param), (LPARAM)47);
					windows_list.at(index).current_view = 0x2;
					windows_list.at(index).cmp_vector.at(0x2).clear();
					if (AnalyseSemantique(src_code, &windows_list.at(index).cmp_vector.at(0x2), declared_identifiers) == 0x0) {
						std::pair<WORD, BOOL> menu_item = std::make_pair(1006, TRUE);
						EnableMenuItems(GetMenu(windows_list.at(index).main_window_handle), &menu_item, 0x1);
					}
					RECT update_rect = { 0x0 };
					GetClientRect(window_handle, &update_rect);
					update_rect.left = 15;
					update_rect.top = 30;
					scroll_info.nPos = scroll_info.nMin = 0x0;
					scroll_info.nMax = windows_list.at(index).cmp_vector.at(0x2).size() - 1;
					scroll_info.nPage = (update_rect.bottom - update_rect.top + 1) / 25;
					SetScrollInfo(window_handle, SB_VERT, &scroll_info, TRUE);
					for (UINT i = 0x0; i < min(scroll_info.nMax + 1, scroll_info.nPos + scroll_info.nPage); i++) {
						HWND static_control_handle = CreateWindowExA(0x0, "STATIC", windows_list.at(index).cmp_vector.at(0x2).at(i).first, WS_CHILD | WS_VISIBLE | SS_LEFT, 10, 30 + 25 * i, 1480, 25, window_handle, (HMENU)0x0, image_base, (LPVOID)0x0);
						SetWindowLongPtrA(static_control_handle, GWL_USERDATA, (LONG)windows_list.at(index).cmp_vector.at(0x2).at(i).second);
						comp_messages.push_back(static_control_handle);
					}
				}break;
			}
			HeapFree(GetProcessHeap(), 0x0, (LPVOID)src_code);
			return 0x0L;
		}
		case WM_CTLCOLORSTATIC: {
			HWND window_handle = (HWND)long_param;
			HDC device_context_handle = (HDC)word_param;
			LOGFONTW font_infos = { 0x0 };
			lstrcpyW(font_infos.lfFaceName, L"Consolas");
			font_infos.lfWeight = FW_BOLD;
			font_infos.lfHeight = 15;
			HFONT font_handle = CreateFontIndirectW(&font_infos);
			SelectObject(device_context_handle, font_handle);
			DWORD text_len = GetWindowTextLengthA(window_handle);
			LPSTR window_text = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, text_len + 1);
			GetWindowTextA(window_handle, window_text, text_len + 1);
			SetBkMode(device_context_handle, TRANSPARENT);
			if ((BYTE)GetWindowLongPtrA(window_handle, GWL_USERDATA) == (BYTE)0x0) SetTextColor(device_context_handle, 0x0000d0);
			TextOutA(device_context_handle, 0x0, 0x5, window_text, text_len);
			HeapFree(GetProcessHeap(), 0x0, window_text);
			DeleteObject(font_handle);
			return (LRESULT)CreateSolidBrush(0xffffff);
		}
		case WM_MOUSEWHEEL: {
			short wheel_rotation = HIWORD(word_param);
			if (wheel_rotation > 0) SendMessageW(window_handle, WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0x0), (LPARAM)0x0);
			else SendMessageW(window_handle, WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0x0), (LPARAM)0x0);
			return 0x0;
		}
		case WM_VSCROLL: {
			SCROLLINFO scroll_info = { 0x0 };
			scroll_info.cbSize = sizeof SCROLLINFO;
			scroll_info.fMask = SIF_ALL;
			DWORD current_scrolling_pos = 0x0;
			switch (LOWORD(word_param)) {
				case SB_LINEDOWN: {
					GetScrollInfo(window_handle, SB_VERT, &scroll_info);
					current_scrolling_pos = scroll_info.nPos;
					if (scroll_info.nPos == (scroll_info.nMax - scroll_info.nPage + 1)) break;
					else {
						scroll_info.nPos += 0x1;
						SetScrollInfo(window_handle, SB_VERT, &scroll_info, TRUE);
					}
				}break;
				case SB_LINEUP: {
					GetScrollInfo(window_handle, SB_VERT, &scroll_info);
					current_scrolling_pos = scroll_info.nPos;
					if (scroll_info.nPos == 0x0) break;
					else {
						scroll_info.nPos -= 0x1;
						SetScrollInfo(window_handle, SB_VERT, &scroll_info, TRUE);
					}
				}break;
				case SB_THUMBTRACK: {
					GetScrollInfo(window_handle, SB_VERT, &scroll_info);
					current_scrolling_pos = scroll_info.nPos;
					scroll_info.nPos += (scroll_info.nTrackPos - scroll_info.nPos);
					SetScrollInfo(window_handle, SB_VERT, &scroll_info, TRUE);
				}break;
				default: return DefWindowProcW(window_handle, window_message, word_param, long_param);
			}
			if (scroll_info.nPos != current_scrolling_pos) {
				for (UINT i = 0x0; i < comp_messages.size(); i++) DestroyWindow(comp_messages.at(i));
				comp_messages.clear();
				for (UINT i = scroll_info.nPos; i < min(scroll_info.nMax + 1, scroll_info.nPos + scroll_info.nPage); i++) {
					HWND static_control_handle = CreateWindowExA(0x0, "STATIC", windows_list.at(index).cmp_vector.at(windows_list.at(index).current_view).at(i).first, WS_CHILD | WS_VISIBLE | SS_LEFT, 10, 30 + 25 * (i - scroll_info.nPos), 1480, 25, window_handle, (HMENU)0x0, image_base, (LPVOID)0x0);
					SetWindowLongPtrA(static_control_handle, GWL_USERDATA, (LONG)windows_list.at(index).cmp_vector.at(windows_list.at(index).current_view).at(i).second);
					comp_messages.push_back(static_control_handle);
				}
			}
			return 0x0L;
		}
		case WM_CLOSE: {
			windows_list.at(index).cmp_vector.at(0).clear();
			windows_list.at(index).cmp_vector.at(1).clear();
			windows_list.at(index).cmp_vector.at(2).clear();
			DestroyWindow(window_handle);
			return 0x0L;
		}
		default: return DefWindowProcW(window_handle, window_message, word_param, long_param);
	}
}

HRESULT __stdcall Init(INT show_flag) {
	if (FAILED(FillAndRegisterWndClass(L"WC_SnailCompiler", MAKEINTRESOURCEW(0x10f), IDC_ARROW, RGB(0xff, 0xff, 0xff), MainWndProc))) return ErrorHandler(GetDesktopWindow(), L"FillAndRegisterWndClass() failed !!", (LPDWORD)0x0);
	if (FAILED(FillAndRegisterWndClass(L"WC_SubWnd", MAKEINTRESOURCEW(0x10f), IDC_ARROW, RGB(0xff, 0xff, 0xff), CompileWndProc))) return ErrorHandler(GetDesktopWindow(), L"FillAndRegisterWndClass() failed !!", (LPDWORD)0x0);
	ACCEL accels[0x8];
	WORD v_keys[0x8] = { 0x4e, 0x4e, 0x4f, 0x53, 0x58, 0x42, 0x74, 0x48 };
	for (UINT i = 0x0; i < 0x8; i++) {
		accels[i].fVirt = FVIRTKEY | FCONTROL;
		if (i == 0x1) accels[i].fVirt |= FSHIFT;
		accels[i].key = v_keys[i];
		accels[i].cmd = 1000 + i;
	}
	accel_handle = CreateAcceleratorTableA(accels, 0x8);
	std::pair<UINT, UINT> screen_coordinates = std::make_pair(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
	WINDOWINFOW window_info = { 0x0 };
	window_info.caption = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lstrlenW(init_caption) * 0x2 + 0x2);
	if (!window_info.caption) MessageBox(0, L"null", 0, 0);
	StringCchCopyW((STRSAFE_LPWSTR)window_info.caption, lstrlenW(init_caption) + 0x1, (STRSAFE_LPCWSTR)init_caption);
	window_info.cwf_handle = INVALID_HANDLE_VALUE;
	window_info.current_view = 0x0;
	window_info.cmp_vector = std::vector<std::vector<std::pair<LPCSTR, BYTE> > > (3);
	window_info.main_window_handle = CreateWindowExW(0x0, L"WC_SnailCompiler", window_info.caption, WS_OVERLAPPED | WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX, screen_coordinates.first/2 - 400, screen_coordinates.second/2 - 275, 800, 550, GetDesktopWindow(), (HMENU)0x0, image_base, (LPVOID)0x0);
	if (!window_info.main_window_handle) {
		UnregisterClassW(L"WC_SnailCompiler", image_base);
		return ErrorHandler(GetDesktopWindow(), L"CreateWidnowExW() failed !!", (LPDWORD)0x0);
	}
	windows_list.push_back(window_info);
	ShowWindow(window_info.main_window_handle, show_flag ? show_flag : SW_SHOWNORMAL);
	UpdateWindow(window_info.main_window_handle);
	return S_OK;
}
HRESULT __stdcall FillAndRegisterWndClass(LPCWSTR class_name, LPCWSTR icon_name, LPCWSTR cursor_name, COLORREF back_color, WNDPROC MessageHandler) {
	WNDCLASSW wnd_class = { 0x0 };
	wnd_class.lpfnWndProc = MessageHandler;
	wnd_class.hInstance = image_base;
	wnd_class.hCursor = LoadCursorW((HINSTANCE)image_base, cursor_name);
	wnd_class.hIcon = LoadIconW((HINSTANCE)image_base, icon_name);
	wnd_class.lpszMenuName = (LPCWSTR)0x0;
	wnd_class.lpszClassName = class_name;
	wnd_class.hbrBackground = CreateSolidBrush(back_color);
	return RegisterClassW(&wnd_class) ? S_OK : E_FAIL;
}
HRESULT __stdcall ErrorHandler(HWND owner_window, LPCWSTR error_message, LPDWORD error_number) {
	MessageBoxW(owner_window, error_message, L"Error !", MB_OK | MB_ICONERROR);
	*error_number = GetLastError();
	return E_FAIL;
}
BOOL __stdcall GetItemIndex(HWND window_handle, ULONGLONG* index) {
	if (window_handle == (HWND)0x0) {
		*index = (ULONGLONG)-1;
		return FALSE;
	}
	for (ULONGLONG i = 0x0; i < windows_list.size(); i++) {
		if (windows_list.at(i).main_window_handle == window_handle) {
			*index = i;
			return TRUE;
		}
	}
	*index = (ULONGLONG)-1;
	return FALSE;
}
BOOL __stdcall EnableMenuItems(HMENU menu_handle, std::pair<WORD, BOOL>* menu_items, SIZE_T items_count) {
	if (menu_handle == (HMENU)0x0) return FALSE;
	if (menu_items == (std::pair<WORD, BOOL>*)0x0) return TRUE;
	MENUITEMINFOW menu_item_info = { 0x0 };
	menu_item_info.cbSize = sizeof MENUITEMINFOW;
	menu_item_info.fMask = MIIM_STATE;
	for (UINT i = 0x0; i < items_count; i++) {
		menu_item_info.fState = menu_items[i].second == TRUE ? MFS_ENABLED : MFS_DISABLED;
		SetMenuItemInfoW(menu_handle, menu_items[i].first, FALSE, &menu_item_info);
	}
	return TRUE;
}
std::vector<WINDOWINFOW> __stdcall RemoveItemFromVector(std::vector<WINDOWINFOW> list, HWND requested_window) {
	std::vector<WINDOWINFOW> new_list = std::vector<WINDOWINFOW>();
	for (UINT i = 0x0; i < list.size(); i++) if (list.at(i).main_window_handle != requested_window) new_list.push_back(list.at(i));
	return new_list;
}
DWORD __stdcall GetFilenameFromCaption(LPWSTR _caption, LPWSTR* filename) {
	if (lstrlenW(_caption) == 0x0) return 0x0;
	DWORD filename_size;
	for (filename_size = 0x0; *(_caption + filename_size) != ' '; filename_size++);
	*filename = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, filename_size * 0x2 + 0x2);
	if ((*filename) == (LPWSTR)0x0) return 0x0;
	StringCchCopyW(*(STRSAFE_LPWSTR*)filename, filename_size + 0x1, (STRSAFE_LPCWSTR)_caption);
	return filename_size;
}
DWORD __stdcall GetFileExtension(LPWSTR file_name, LPWSTR* file_extension) {
	if (lstrlenW(file_name) == 0x0) {
		*file_extension = (LPWSTR)0x0;
		return 0x0;
	}
	UINT i;
	for (i = 0x0; i < lstrlenW(file_name) && *(file_name + i) != '.'; i++);
	*file_extension = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (lstrlenW(file_name) - (i + 0x1)) + 0x1);
	StringCchCopyW(*(STRSAFE_LPWSTR*)file_extension, (lstrlenW(file_name) - (i + 0x1)) + 0x1, (STRSAFE_LPCWSTR)(file_name + i + 0x1));
	return (lstrlenW(file_name) - i) + 0x1;
}

DWORD __stdcall GUI_ThreadStartingExePoint(LPVOID param) {
	EnterCriticalSection(&critical_section);
	UNREFERENCED_PARAMETER(param);
	std::pair<UINT, UINT> screen_coordinates = std::make_pair(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
	WINDOWINFOW window_info = { 0x0 };
	window_info.caption = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lstrlenW(L"Untitled - Snail Compiler") * 0x2 + 0x2);
	StringCchCopyW((STRSAFE_LPWSTR)window_info.caption, lstrlenW(L"Untitled - Snail Compiler") + 0x1, (STRSAFE_LPCWSTR)L"Untitled - Snail Compiler");
	window_info.cwf_handle = INVALID_HANDLE_VALUE;
	window_info.current_view = 0x0;
	window_info.cmp_vector = std::vector<std::vector<std::pair<LPCSTR, BYTE> > >(3);
	window_info.main_window_handle = CreateWindowExW(0x0, L"WC_SnailCompiler", window_info.caption, WS_OVERLAPPED | WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX, screen_coordinates.first / 2 - 400, screen_coordinates.second / 2 - 275, 800, 550, GetDesktopWindow(), (HMENU)0x0, image_base, (LPVOID)0x0);
	ShowWindow(window_info.main_window_handle, SW_SHOWDEFAULT);
	UpdateWindow(window_info.main_window_handle);
	windows_list.push_back(window_info);
	LeaveCriticalSection(&critical_section);
	MSG window_message = { 0x0 };
	while (GetMessageW(&window_message, NULL, 0x0, 0x0)) {
		if (!TranslateAcceleratorW(window_info.main_window_handle, accel_handle, &window_message)) {
			TranslateMessage(&window_message);
			DispatchMessageW(&window_message);
		}
	}
	HeapFree(GetProcessHeap(), 0x0, window_info.caption);
	ExitThread(window_message.wParam);
}

DWORD __stdcall ReadLine(LPSTR char_sequence, LPSTR* line_buffer, LPDWORD line_len, LPDWORD skip) {
	DWORD i, m, j, k;
	*line_len = 0x0;
	for (i = 0x0; i <= UINT_MAX && (char_sequence[i] == ' ' || char_sequence[i] == '\t'); i++);
	while (*(char_sequence + i) != '\r' && Is_Comment(char_sequence + i, &k)) for (i += k; *(char_sequence + i) == ' ' || *(char_sequence + i) == '\t'; i++);
	if (skip) *skip = i;
	for (m = 0x0; m <= UINT_MAX && char_sequence[i + m] != '\0' && char_sequence[i + m] != '\r'; m++);
	UINT next_postion = (i + m) + 0x2; // skip \r\n   
	for (j = (i + m) - 0x1; j >= i && (char_sequence[j] == ' ' || char_sequence[j] == '\t'); j--);
	*line_len = j - i + 0x1;
	if (*line_len == 0x0) return next_postion; // dont't allocate a buffer of zero bytes
	*line_buffer = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, *line_len + 0x1);
	StringCchCopyA(*(STRSAFE_LPSTR*)line_buffer, *line_len + 0x1, (STRSAFE_LPCSTR)char_sequence + i);
	return next_postion;
}
ULONG __stdcall AnalyseSyntaxique(LPSTR snl_code, std::vector<std::pair<LPCSTR, BYTE> >* vector, std::vector<SNAIL_VAR>* declared_vars) {
	DWORD code_len = lstrlenA(snl_code);
	if (code_len == 0x0) {
		vector->push_back(std::make_pair("no instructions !!", (BYTE)0x1));
		return 0x0;
	}
	ULONG errors_count = 0x0;
	UINT i = 0x0;
	UINT line_number = 0x1;
	BOOL is_ifBlock_exist = FALSE;
	BOOL last_ifBlock_line = 0x0;
	BOOL last_elseBlock_line = 0x0;
	char ln_str[20];
	_itoa_s(line_number, ln_str, 10);
	BOOL is_snl_begin_exist = FALSE;
	BOOL is_snl_end_exist = FALSE;
	while (i <= code_len) {
		LPSTR line_buffer = (LPSTR)0x0;
		DWORD line_len = 0x0;
		DWORD skip = 0x0;
		DWORD offset = ReadLine(snl_code + i, &line_buffer, &line_len, &skip);
		if (line_buffer && line_len > 0x0) {
			if (strncmp(line_buffer, "Snl_Begin", lstrlenA("Snl_Begin")) == 0x0) {
				is_snl_begin_exist = TRUE;
				WORD spaces_count;
				for (spaces_count = 0x0; *(line_buffer + 0x9 + spaces_count) == ' ' || *(line_buffer + 0x9 + spaces_count) == '\t'; spaces_count++);
				DWORD symbol_size = 0x0;
				while (Is_Comment(line_buffer + 0x9 + spaces_count, &symbol_size) && *(line_buffer + 0x9 + spaces_count) != '\0') for (spaces_count += symbol_size; *(line_buffer + 0x9 + spaces_count) == ' ' || *(line_buffer + 0x9 + spaces_count) == '\t'; spaces_count++);
				if (*(line_buffer + 0x9 + spaces_count) != '\0') {
					LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : unexpected < ") + (line_len - 0x9 - spaces_count) + lstrlenA(" > line ( ") + lstrlenA(ln_str) + 0x9);
					StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + 0x7, (STRSAFE_LPCSTR)" : unexpected < ");
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - 0x9 - spaces_count) + 0x7, (STRSAFE_LPCSTR)(line_buffer + 0x9 + spaces_count));
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - 0x9 - spaces_count) + lstrlenA(" > line ( ") + 0x7, (STRSAFE_LPCSTR)" > line ( ");
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - 0x9 - spaces_count) + lstrlenA(" > line ( ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - 0x9 - spaces_count) + lstrlenA(" > line ( ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
					vector->push_back(std::make_pair(error_message, (BYTE)0x0));
					errors_count++;
				}
				else {
					if (line_number == 0x1) vector->push_back(std::make_pair("[ + ] Snl_Begin: program start instruction", (BYTE)0x1));
					else {
						LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lstrlenA("Snl_Begin : unexpected < Snl_Begin > < must be at the beginning > ( line ") + lstrlenA(ln_str) + 0x9);
						StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("Snl_Begin : unexpected < Snl_Begin > < must be at the beginning > ( line ") + 0x7, (STRSAFE_LPCSTR)"Snl_Begin : unexpected < Snl_Begin > < must be at the beginning > ( line ");
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("Snl_Begin : unexpected < Snl_Begin > < must be at the beginning > ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("Snl_Begin : unexpected < Snl_Begin > < must be at the beginning > ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
						vector->push_back(std::make_pair(error_message, (BYTE)0x0));
						errors_count++;
					}
				}
			}
			else if (strncmp(line_buffer, "Snl_Int", lstrlenA("Snl_Int")) == 0x0) {
				WORD operand_offset;
				for (operand_offset = 0x0; *(line_buffer + 0x7 + operand_offset) == ' ' || *(line_buffer + 0x7 + operand_offset) == '\t'; operand_offset++);
				DWORD comment_size = 0x0;
				while (Is_Comment(line_buffer + 0x7 + operand_offset, &comment_size) && *(line_buffer + 0x7 + operand_offset) != '\0') for (operand_offset += comment_size; *(line_buffer + 0x7 + operand_offset) == ' ' || *(line_buffer + 0x7 + operand_offset) == '\t'; operand_offset++);
				if ((*(line_buffer + 0x7 + operand_offset) == '%' && *(line_buffer + 0x8 + operand_offset) == '.') || *(line_buffer + 0x7 + operand_offset) == '\0') {
					LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : expected id after < Snl_Int > ( line ") + lstrlenA(ln_str) + 0x9);
					StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected id after < Snl_Int > ( line ") + 0x7, (STRSAFE_LPCSTR)" : expected id after < Snl_Int > ( line ");
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected id after < Snl_Int > ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected id after < Snl_Int > ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
					vector->push_back(std::make_pair(error_message, (BYTE)0x0));
					errors_count++;
				}
				else if (*(line_buffer + 0x7 + operand_offset) == ',') {
					LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : unexpected , after < Snl_Inl > ( line ") + lstrlenA(ln_str) + 0x9);
					StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected , after < Snl_Inl > ( line ") + 0x1, (STRSAFE_LPCSTR)" : unexpected , after < Snl_Inl > ( line ");
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected , after < Snl_Inl > ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected , after < Snl_Inl > ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
					vector->push_back(std::make_pair(error_message, (BYTE)0x0));
					errors_count++;
				}
				else {
					if (operand_offset == 0x0) {
						LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : expected space after < Snl_Int > ( line ") + lstrlenA(ln_str) + 0x9);
						StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
						StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
						StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected space after < Snl_Int > ( line ") + 0x1, (STRSAFE_LPCSTR)" : expected space after < Snl_Int > ( line ");
						StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected space after < Snl_Int > ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
						StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected space after < Snl_Int > ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
						vector->push_back(std::make_pair(error_message, (BYTE)0x0));
						errors_count++;
					}
					else {
						DWORD id_size = 0;
						WORD error_type = NO_ERROR;
						DWORD ids_count = 0x0;
						do {
							while (Is_Comment(line_buffer + 0x7 + operand_offset, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x7 + operand_offset) == ' ' || *(line_buffer + 0x7 + operand_offset) == '\t'; operand_offset++);
							if (!Is_ID(line_buffer + 0x7 + operand_offset, (LPDWORD)&id_size)) {
								error_type = ERROR_INVALID_ID;
								break;
							}
							else {
								SNAIL_VAR new_id = SNAIL_VAR();
								new_id.line = line_number;
								new_id.line_position = ids_count;
								new_id.name = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, id_size + 0x1);
								StringCchCopyA((STRSAFE_LPSTR)new_id.name, id_size + 0x1, (STRSAFE_LPCSTR)(line_buffer + 0x7 + operand_offset));
								new_id.type = VAR_TYPE_INTEGER;
								declared_vars->push_back(new_id);
								ids_count++;
							}
							operand_offset += id_size;
							if (*(line_buffer + 0x7 + operand_offset) == '\0') break;
							for (; *(line_buffer + 0x7 + operand_offset) == ' ' || *(line_buffer + 0x7 + operand_offset) == '\t'; operand_offset++);
							while (Is_Comment(line_buffer + 0x7 + operand_offset, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x7 + operand_offset) == ' ' || *(line_buffer + 0x7 + operand_offset) == '\t'; operand_offset++);
							if (*(line_buffer + 0x7 + operand_offset) != ',') {
								if (*(line_buffer + 0x7 + operand_offset) != '%' || *(line_buffer + 0x8 + operand_offset) != '.') error_type = ERROR_INVALID_SEPARATOR;
								break;
							}
							for (operand_offset++; *(line_buffer + 0x7 + operand_offset) == ' ' || *(line_buffer + 0x7 + operand_offset) == '\t'; operand_offset++);
							while (Is_Comment(line_buffer + 0x7 + operand_offset, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x7 + operand_offset) == ' ' || *(line_buffer + 0x7 + operand_offset) == '\t'; operand_offset++);
							if (*(line_buffer + 0x7 + operand_offset) == ',') {
								error_type = ERROR_EXPECTED_ID;
								break;
							}
							else if ((*(line_buffer + 0x7 + operand_offset) == '%' && *(line_buffer + 0x8 + operand_offset) == '.') || *(line_buffer + 0x7 + operand_offset) == '\0') {
								error_type = ERROR_UNEXPECTED_SEPARATOR;
								break;
							}
						} while (!(*(line_buffer + 0x7 + operand_offset) == '%' && *(line_buffer + 0x8 + operand_offset) == '.') && *(line_buffer + 0x7 + operand_offset) != '\0');
						switch (error_type) {
						case NO_ERROR: {
							while (Is_Comment(line_buffer + 0x7 + operand_offset, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x7 + operand_offset) == ' ' || *(line_buffer + 0x7 + operand_offset) == '\t'; operand_offset++);
							if (*(line_buffer + 0x7 + operand_offset) != '%' || *(line_buffer + 0x8 + operand_offset) != '.') {
								LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : expected %. ( line ") + lstrlenA(ln_str) + 0x9);
								StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected %. ( line ") + 0x7, (STRSAFE_LPCSTR)" : expected %. ( line ");
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected %. ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected %. ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
								vector->push_back(std::make_pair(error_message, (BYTE)0x0));
								errors_count++;
							}
							else {
								for (operand_offset+=0x2; *(line_buffer + 0x7 + operand_offset) == ' ' || *(line_buffer + 0x7 + operand_offset) == '\t'; operand_offset++);
								while (Is_Comment(line_buffer + 0x7 + operand_offset, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x7 + operand_offset) == ' ' || *(line_buffer + 0x7 + operand_offset) == '\t'; operand_offset++);
								if (operand_offset + 0x7 != line_len) {
									LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : unexpected < ") + (line_len - 0x7 - operand_offset) + lstrlenA(" > line ( ") + lstrlenA(ln_str) + 0x9);
									StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + 0x7, (STRSAFE_LPCSTR)" : unexpected < ");
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - 0x7 - operand_offset) + 0x7, (STRSAFE_LPCSTR)(line_buffer + 0x7 + operand_offset));
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - 0x7 - operand_offset) + lstrlenA(" > line ( ") + 0x7, (STRSAFE_LPCSTR)" > line ( ");
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - 0x7 - operand_offset) + lstrlenA(" > line ( ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - 0x7 - operand_offset) + lstrlenA(" > line ( ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
									vector->push_back(std::make_pair(error_message, (BYTE)0x0));
									errors_count++;
								}
								else {
									LPSTR message = (LPSTR)0x0;
									if (ids_count > 1) {
										char idc_str[20];
										_itoa_s(ids_count, idc_str, 10);
										message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : la declaration de ") + lstrlenA(idc_str) + lstrlenA(" variables enti貥s") + 0x7);
										StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ + ] ");
										StringCchCatA((STRSAFE_LPSTR)message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
										StringCchCatA((STRSAFE_LPSTR)message, line_len + lstrlenA(" : la declaration de ") + 0x7, (STRSAFE_LPCSTR)" : la declaration de ");
										StringCchCatA((STRSAFE_LPSTR)message, line_len + lstrlenA(" : la declaration de ") + lstrlenA(idc_str) + 0x7, (STRSAFE_LPCSTR)idc_str);
										StringCchCatA((STRSAFE_LPSTR)message, line_len + lstrlenA(" : la declaration de ") + lstrlenA(idc_str) + lstrlenA(" variables enti貥s") + 0x7, (STRSAFE_LPCSTR)" variables enti貥s");
									}
									else {
										message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : la declaration d'une seule variable enti貥") + 0x7);
										StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ + ] ");
										StringCchCatA((STRSAFE_LPSTR)message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
										StringCchCatA((STRSAFE_LPSTR)message, line_len + lstrlenA(" : la declaration d'une seule variable enti貥") + 0x7, (STRSAFE_LPCSTR)" : la declaration d'une seule variable enti貥");
									}
									vector->push_back(std::make_pair(message, (BYTE)0x1));
								}
							}
						}break;
						case ERROR_INVALID_ID: {
							LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : invalid id ( line ") + lstrlenA(ln_str) + 0x9);
							StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : invalid id ( line ") + 0x7, (STRSAFE_LPCSTR)" : invalid id ( line ");
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : invalid id ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : invalid id ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
							vector->push_back(std::make_pair(error_message, (BYTE)0x0));
							errors_count++;
						}break;
						case ERROR_INVALID_SEPARATOR: {
							LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : invalid or missing separtor ( line ") + lstrlenA(ln_str) + 0x9);
							StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : invalid or missing separtor ( line ") + 0x7, (STRSAFE_LPCSTR)" : invalid or missing separtor ( line ");
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : invalid or missing separtor ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : invalid or missing separtor ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
							vector->push_back(std::make_pair(error_message, (BYTE)0x0));
							errors_count++;
						}break;
						case ERROR_UNEXPECTED_SEPARATOR: {
							LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : unexpected , after the last id ( line ") + lstrlenA(ln_str) + 0x3);
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected , after the last id ( line ") + 0x7, (STRSAFE_LPCSTR)" : unexpected , after the last id ( line ");
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected , after the last id ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected , after the last id ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
							vector->push_back(std::make_pair(error_message, (BYTE)0x0));
							errors_count++;
						}break;
						case ERROR_EXPECTED_ID: {
							LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : expected id < ,{IDENT}, > ( line ") + lstrlenA(ln_str) + 0x9);
							StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected id < ,{IDENT}, > ( line ") + 0x7, (STRSAFE_LPCSTR)" : expected id < ,{IDENT}, > ( line ");
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected id < ,{IDENT}, > ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected id < ,{IDENT}, > ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
							vector->push_back(std::make_pair(error_message, (BYTE)0x0));
							errors_count++;
						}break;
						}
					}
				}
			}
			else if (strncmp(line_buffer, "Snl_Real", lstrlenA("Snl_Real")) == 0x0) {
				WORD operand_offset;
				DWORD comment_size = 0x0;
				for (operand_offset = 0x0; *(line_buffer + 0x8 + operand_offset) == ' ' || *(line_buffer + 0x8 + operand_offset) == '\t'; operand_offset++);
				while (Is_Comment(line_buffer + 0x8 + operand_offset, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x8 + operand_offset) == ' ' || *(line_buffer + 0x8 + operand_offset) == '\t'; operand_offset++);
				if ((*(line_buffer + 0x8 + operand_offset) == '%' && *(line_buffer + 0x9 + operand_offset) == '.') || *(line_buffer + 0x8 + operand_offset) == '\0') {
					LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : expected id after < Snl_Real > ( line ") + lstrlenA(ln_str) + 0x9);
					StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected id after < Snl_Real > ( line ") + 0x7, (STRSAFE_LPCSTR)" : expected id after < Snl_Real > ( line ");
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected id after < Snl_Real > ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected id after < Snl_Real > ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
					vector->push_back(std::make_pair(error_message, (BYTE)0x0));
					errors_count++;
				}
				else if (*(line_buffer + 0x8 + operand_offset) == ',') {
					LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : unexpected , after < Snl_Real > ( line ") + lstrlenA(ln_str) + 0x9);
					StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected , after < Snl_Real > ( line ") + 0x1, (STRSAFE_LPCSTR)" : unexpected , after < Snl_Real > ( line ");
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected , after < Snl_Real > ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected , after < Snl_Real > ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
					vector->push_back(std::make_pair(error_message, (BYTE)0x0));
					errors_count++;
				}
				else {
					if (operand_offset == 0x0) {
						LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : expected space after < Snl_Real > ( line ") + lstrlenA(ln_str) + 0x9);
						StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
						StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
						StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected space after < Snl_Real > ( line ") + 0x1, (STRSAFE_LPCSTR)" : expected space after < Snl_Real > ( line ");
						StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected space after < Snl_Real > ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
						StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected space after < Snl_Real > ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
						vector->push_back(std::make_pair(error_message, (BYTE)0x0));
						errors_count++;
					}
					else {
						signed id_size = 0;
						WORD error_type = NO_ERROR;
						DWORD ids_count = 0x0;
						do {
							while (Is_Comment(line_buffer + 0x8 + operand_offset, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x8 + operand_offset) == ' ' || *(line_buffer + 0x8 + operand_offset) == '\t'; operand_offset++);
							if (!Is_ID(line_buffer + 0x8 + operand_offset, (LPDWORD)&id_size)) {
								error_type = ERROR_INVALID_ID;
								break;
							}
							else {
								SNAIL_VAR new_id = SNAIL_VAR();
								new_id.line = line_number;
								new_id.line_position = ids_count;
								new_id.name = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, id_size + 0x1);
								StringCchCopyA((STRSAFE_LPSTR)new_id.name, id_size + 0x1, (STRSAFE_LPCSTR)(line_buffer + 0x8 + operand_offset));
								new_id.type = VAR_TYPE_REAL;
								declared_vars->push_back(new_id);
								ids_count++;
							}
							operand_offset += id_size;
							if (*(line_buffer + 0x8 + operand_offset) == '\0') break;
							for (; *(line_buffer + 0x8 + operand_offset) == ' ' || *(line_buffer + 0x8 + operand_offset) == '\t'; operand_offset++);
							while (Is_Comment(line_buffer + 0x8 + operand_offset, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x8 + operand_offset) == ' ' || *(line_buffer + 0x8 + operand_offset) == '\t'; operand_offset++);
							if (*(line_buffer + 0x8 + operand_offset) != ',') {
								if (*(line_buffer + 0x8 + operand_offset) != '%' || *(line_buffer + 0x9 + operand_offset) != '.') error_type = ERROR_INVALID_SEPARATOR;
								break;
							}
							for (operand_offset++; *(line_buffer + 0x8 + operand_offset) == ' ' || *(line_buffer + 0x8 + operand_offset) == '\t'; operand_offset++);
							while (Is_Comment(line_buffer + 0x8 + operand_offset, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x8 + operand_offset) == ' ' || *(line_buffer + 0x8 + operand_offset) == '\t'; operand_offset++);
							if (*(line_buffer + 0x8 + operand_offset) == ',') {
								error_type = ERROR_EXPECTED_ID;
								break;
							}
							else if ((*(line_buffer + 0x8 + operand_offset) == '%' && *(line_buffer + 0x9 + operand_offset) == '.') || *(line_buffer + 0x8 + operand_offset) == '\0') {
								error_type = ERROR_UNEXPECTED_SEPARATOR;
								break;
							}
						} while (!(*(line_buffer + 0x8 + operand_offset) == '%' && *(line_buffer + 0x9 + operand_offset) == '.') && *(line_buffer + 0x8 + operand_offset) != '\0');
						switch (error_type) {
						case NO_ERROR: {
							while (Is_Comment(line_buffer + 0x8 + operand_offset, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x8 + operand_offset) == ' ' || *(line_buffer + 0x8 + operand_offset) == '\t'; operand_offset++);
							if (*(line_buffer + 0x8 + operand_offset) != '%' || *(line_buffer + 0x9 + operand_offset) != '.') {
								LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : expected %. ( line ") + lstrlenA(ln_str) + 0x9);
								StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected %. ( line ") + 0x7, (STRSAFE_LPCSTR)" : expected %. ( line ");
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected %. ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected %. ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
								vector->push_back(std::make_pair(error_message, (BYTE)0x0));
								errors_count++;
							}
							else {
								for (operand_offset+=2; *(line_buffer + 0x8 + operand_offset) == ' ' || *(line_buffer + 0x8 + operand_offset) == '\t'; operand_offset++);
								while (Is_Comment(line_buffer + 0x8 + operand_offset, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x8 + operand_offset) == ' ' || *(line_buffer + 0x8 + operand_offset) == '\t'; operand_offset++);
								if (operand_offset + 0x8 != line_len) {
									WORD spaces_count;
									for (spaces_count = 0x0; *(line_buffer + 0xa + operand_offset + spaces_count) == ' ' || *(line_buffer + 0xa + operand_offset + spaces_count) == '\t'; spaces_count++);
									LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : unexpected < ") + (line_len - 0x8 - operand_offset - spaces_count) + lstrlenA(" > line ( ") + lstrlenA(ln_str) + 0x9);
									StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + 0x7, (STRSAFE_LPCSTR)" : unexpected < ");
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - 0x8 - operand_offset - spaces_count) + 0x7, (STRSAFE_LPCSTR)(line_buffer + 0x8 + operand_offset + spaces_count));
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - 0x8 - operand_offset - spaces_count) + lstrlenA(" > line ( ") + 0x7, (STRSAFE_LPCSTR)" > line ( ");
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - 0x8 - operand_offset - spaces_count) + lstrlenA(" > line ( ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - 0x8 - operand_offset - spaces_count) + lstrlenA(" > line ( ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
									vector->push_back(std::make_pair(error_message, (BYTE)0x0));
									errors_count++;
								}
								else {
									LPSTR message = (LPSTR)0x0;
									if (ids_count > 1) {
										char idc_str[20];
										_itoa_s(ids_count, idc_str, 10);
										message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : la declaration de ") + lstrlenA(idc_str) + lstrlenA(" variables r饬es") + 0x7);
										StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ + ] ");
										StringCchCatA((STRSAFE_LPSTR)message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
										StringCchCatA((STRSAFE_LPSTR)message, line_len + lstrlenA(" : la declaration de ") + 0x7, (STRSAFE_LPCSTR)" : la declaration de ");
										StringCchCatA((STRSAFE_LPSTR)message, line_len + lstrlenA(" : la declaration de ") + lstrlenA(idc_str) + 0x7, (STRSAFE_LPCSTR)idc_str);
										StringCchCatA((STRSAFE_LPSTR)message, line_len + lstrlenA(" : la declaration de ") + lstrlenA(idc_str) + lstrlenA(" variables r饬es") + 0x7, (STRSAFE_LPCSTR)" variables r饬es");
									}
									else {
										message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : la declaration d'une seule variable r饬e") + 0x7);
										StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ + ] ");
										StringCchCatA((STRSAFE_LPSTR)message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
										StringCchCatA((STRSAFE_LPSTR)message, line_len + lstrlenA(" : la declaration d'une seule variable r饬e") + 0x7, (STRSAFE_LPCSTR)" : la declaration d'une seule variable r饬e");
									}
									vector->push_back(std::make_pair(message, (BYTE)0x1));
								}
							}
						}break;
						case ERROR_INVALID_ID: {
							LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : invalid id ( line ") + lstrlenA(ln_str) + 0x9);
							StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : invalid id ( line ") + 0x7, (STRSAFE_LPCSTR)" : invalid id ( line ");
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : invalid id ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : invalid id ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
							vector->push_back(std::make_pair(error_message, (BYTE)0x0));
							errors_count++;
						}break;
						case ERROR_INVALID_SEPARATOR: {
							LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : invalid or missing separtor ( line ") + lstrlenA(ln_str) + 0x9);
							StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : invalid or missing separtor ( line ") + 0x7, (STRSAFE_LPCSTR)" : invalid or missing separtor ( line ");
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : invalid or missing separtor ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : invalid or missing separtor ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
							vector->push_back(std::make_pair(error_message, (BYTE)0x0));
							errors_count++;
						}break;
						case ERROR_UNEXPECTED_SEPARATOR: {
							LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : unexpected , after the last id ( line ") + lstrlenA(ln_str) + 0x3);
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected , after the last id ( line ") + 0x7, (STRSAFE_LPCSTR)" : unexpected , after the last id ( line ");
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected , after the last id ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected , after the last id ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
							vector->push_back(std::make_pair(error_message, (BYTE)0x0));
							errors_count++;
						}break;
						case ERROR_EXPECTED_ID: {
							LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : expected id < ,{IDENT}, > ( line ") + lstrlenA(ln_str) + 0x9);
							StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected id < ,{IDENT}, > ( line ") + 0x7, (STRSAFE_LPCSTR)" : expected id < ,{IDENT}, > ( line ");
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected id < ,{IDENT}, > ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected id < ,{IDENT}, > ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
							vector->push_back(std::make_pair(error_message, (BYTE)0x0));
							errors_count++;
						}break;
						}
					}
				}
			}
			else if (strncmp(line_buffer, "Snl_St", lstrlenA("Snl_St")) == 0x0) {
				WORD operand_offset;
				DWORD comment_size = 0x0;
				for (operand_offset = 0x0; *(line_buffer + 0x6 + operand_offset) == ' ' || *(line_buffer + 0x6 + operand_offset) == '\t'; operand_offset++);
				while (Is_Comment(line_buffer + 0x6 + operand_offset, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x6 + operand_offset) == ' ' || *(line_buffer + 0x6 + operand_offset) == '\t'; operand_offset++);
				if ((*(line_buffer + 0x6 + operand_offset) == '%' && *(line_buffer + 0x7 + operand_offset) == '.') || *(line_buffer + 0x6 + operand_offset) == '\0') {
					LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : expected id after < Snl_St > ( line ") + lstrlenA(ln_str) + 0x9);
					StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected id after < Snl_St > ( line ") + 0x7, (STRSAFE_LPCSTR)" : expected id after < Snl_St > ( line ");
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected id after < Snl_St > ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected id after < Snl_St > ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
					vector->push_back(std::make_pair(error_message, (BYTE)0x0));
					errors_count++;
				}
				else if (*(line_buffer + 0x6 + operand_offset) == ',') {
					LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : unexpected , after < Snl_St > ( line ") + lstrlenA(ln_str) + 0x9);
					StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected , after < Snl_St > ( line ") + 0x1, (STRSAFE_LPCSTR)" : unexpected , after < Snl_St > ( line ");
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected , after < Snl_St > ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected , after < Snl_St > ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
					vector->push_back(std::make_pair(error_message, (BYTE)0x0));
					errors_count++;
				}
				else {
					if (operand_offset == 0x0) {
						LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : expected space after < Snl_St > ( line ") + lstrlenA(ln_str) + 0x9);
						StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
						StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
						StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected space after < Snl_St > ( line ") + 0x1, (STRSAFE_LPCSTR)" : expected space after < Snl_St > ( line ");
						StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected space after < Snl_St > ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
						StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected space after < Snl_St > ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
						vector->push_back(std::make_pair(error_message, (BYTE)0x0));
						errors_count++;
					}
					else {
						DWORD id_size = 0;
						WORD error_type = NO_ERROR;
						DWORD ids_count = 0x0;
						do {
							while (Is_Comment(line_buffer + 0x6 + operand_offset, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x6 + operand_offset) == ' ' || *(line_buffer + 0x6 + operand_offset) == '\t'; operand_offset++);
							if (!Is_ID(line_buffer + 0x6 + operand_offset, &id_size)) {
								error_type = ERROR_INVALID_ID;
								break;
							}
							else {
								SNAIL_VAR new_id = SNAIL_VAR();
								new_id.line = line_number;
								new_id.line_position = ids_count;
								new_id.name = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, id_size + 0x1);
								StringCchCopyA((STRSAFE_LPSTR)new_id.name, id_size + 0x1, (STRSAFE_LPCSTR)(line_buffer + 0x6 + operand_offset));
								new_id.type = VAR_TYPE_STRING;
								declared_vars->push_back(new_id);
								ids_count++;
							}
							operand_offset += id_size;
							if (*(line_buffer + 0x6 + operand_offset) == '\0') break;
							for (; *(line_buffer + 0x6 + operand_offset) == ' ' || *(line_buffer + 0x6 + operand_offset) == '\t'; operand_offset++);
							while (Is_Comment(line_buffer + 0x6 + operand_offset, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x6 + operand_offset) == ' ' || *(line_buffer + 0x6 + operand_offset) == '\t'; operand_offset++);
							if (*(line_buffer + 0x6 + operand_offset) != ',') {
								if (*(line_buffer + 0x6 + operand_offset) != '%' || *(line_buffer + 0x7 + operand_offset) != '.') error_type = ERROR_INVALID_SEPARATOR;
								break;
							}
							for (operand_offset++; *(line_buffer + 0x6 + operand_offset) == ' ' || *(line_buffer + 0x6 + operand_offset) == '\t'; operand_offset++);
							while (Is_Comment(line_buffer + 0x6 + operand_offset, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x6 + operand_offset) == ' ' || *(line_buffer + 0x6 + operand_offset) == '\t'; operand_offset++);
							if (*(line_buffer + 0x6 + operand_offset) == ',') {
								error_type = ERROR_EXPECTED_ID;
								break;
							}
							else if ((*(line_buffer + 0x6 + operand_offset) == '%' && *(line_buffer + 0x7 + operand_offset) == '.') || *(line_buffer + 0x6 + operand_offset) == '\0') {
								error_type = ERROR_UNEXPECTED_SEPARATOR;
								break;
							}
						} while (!(*(line_buffer + 0x6 + operand_offset) == '%' && *(line_buffer + 0x7 + operand_offset) == '.') && *(line_buffer + 0x6 + operand_offset) != '\0');
						switch (error_type) {
						case NO_ERROR: {
							while (Is_Comment(line_buffer + 0x6 + operand_offset, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x6 + operand_offset) == ' ' || *(line_buffer + 0x6 + operand_offset) == '\t'; operand_offset++);
							if (*(line_buffer + 0x6 + operand_offset) != '%' || *(line_buffer + 0x7 + operand_offset) != '.') {
								LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : expected %. ( line ") + lstrlenA(ln_str) + 0x9);
								StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected %. ( line ") + 0x7, (STRSAFE_LPCSTR)" : expected %. ( line ");
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected %. ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected %. ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
								vector->push_back(std::make_pair(error_message, (BYTE)0x0));
								errors_count++;
							}
							else {
								for (operand_offset+=0x2; *(line_buffer + 0x6 + operand_offset) == ' ' || *(line_buffer + 0x6 + operand_offset) == '\t'; operand_offset++);
								while (Is_Comment(line_buffer + 0x6 + operand_offset, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x6 + operand_offset) == ' ' || *(line_buffer + 0x6 + operand_offset) == '\t'; operand_offset++);
								if (operand_offset + 0x6 != line_len) {
									LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : unexpected < ") + (line_len - 0x6 - operand_offset) + lstrlenA(" > line ( ") + lstrlenA(ln_str) + 0x9);
									StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + 0x7, (STRSAFE_LPCSTR)" : unexpected < ");
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - 0x6 - operand_offset) + 0x7, (STRSAFE_LPCSTR)(line_buffer + 0x6 + operand_offset));
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - 0x6 - operand_offset) + lstrlenA(" > line ( ") + 0x7, (STRSAFE_LPCSTR)" > line ( ");
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - 0x6 - operand_offset) + lstrlenA(" > line ( ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - 0x6 - operand_offset) + lstrlenA(" > line ( ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
									vector->push_back(std::make_pair(error_message, (BYTE)0x0));
									errors_count++;
								}
								else {
									LPSTR message = (LPSTR)0x0;
									if (ids_count > 1) {
										char idc_str[20];
										_itoa_s(ids_count, idc_str, 10);
										message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : la declaration de ") + lstrlenA(idc_str) + lstrlenA(" chaine de characteres") + 0x7);
										StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ + ] ");
										StringCchCatA((STRSAFE_LPSTR)message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
										StringCchCatA((STRSAFE_LPSTR)message, line_len + lstrlenA(" : la declaration de ") + 0x7, (STRSAFE_LPCSTR)" : la declaration de ");
										StringCchCatA((STRSAFE_LPSTR)message, line_len + lstrlenA(" : la declaration de ") + lstrlenA(idc_str) + 0x7, (STRSAFE_LPCSTR)idc_str);
										StringCchCatA((STRSAFE_LPSTR)message, line_len + lstrlenA(" : la declaration de ") + lstrlenA(idc_str) + lstrlenA(" chaines de characteres") + 0x7, (STRSAFE_LPCSTR)" chaines de characteres");
									}
									else {
										message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : la declaration d'une seule chaine de characteres") + 0x7);
										StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ + ] ");
										StringCchCatA((STRSAFE_LPSTR)message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
										StringCchCatA((STRSAFE_LPSTR)message, line_len + lstrlenA(" : la declaration d'une seule chaine de characteres") + 0x7, (STRSAFE_LPCSTR)" : la declaration d'une seule chaine de characteres");
									}
									vector->push_back(std::make_pair(message, (BYTE)0x1));
								}
							}
						}break;
						case ERROR_INVALID_ID: {
							LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : invalid id ( line ") + lstrlenA(ln_str) + 0x9);
							StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : invalid id ( line ") + 0x7, (STRSAFE_LPCSTR)" : invalid id ( line ");
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : invalid id ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : invalid id ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
							vector->push_back(std::make_pair(error_message, (BYTE)0x0));
							errors_count++;
						}break;
						case ERROR_INVALID_SEPARATOR: {
							LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : invalid or missing separtor ( line ") + lstrlenA(ln_str) + 0x9);
							StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : invalid or missing separtor ( line ") + 0x7, (STRSAFE_LPCSTR)" : invalid or missing separtor ( line ");
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : invalid or missing separtor ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : invalid or missing separtor ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
							vector->push_back(std::make_pair(error_message, (BYTE)0x0));
							errors_count++;
						}break;
						case ERROR_UNEXPECTED_SEPARATOR: {
							LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : unexpected , after the last id ( line ") + lstrlenA(ln_str) + 0x3);
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected , after the last id ( line ") + 0x7, (STRSAFE_LPCSTR)" : unexpected , after the last id ( line ");
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected , after the last id ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected , after the last id ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
							vector->push_back(std::make_pair(error_message, (BYTE)0x0));
							errors_count++;
						}break;
						case ERROR_EXPECTED_ID: {
							LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : expected id < ,{IDENT}, > ( line ") + lstrlenA(ln_str) + 0x9);
							StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected id < ,{IDENT}, > ( line ") + 0x7, (STRSAFE_LPCSTR)" : expected id < ,{IDENT}, > ( line ");
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected id < ,{IDENT}, > ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected id < ,{IDENT}, > ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
							vector->push_back(std::make_pair(error_message, (BYTE)0x0));
							errors_count++;
						}break;
						}
					}
				}
			}
			else if (strncmp(line_buffer, "Snl_Put", lstrlenA("Snl_Put")) == 0x0) {
				WORD operand_offset;
				DWORD comment_size = 0x0;
				for (operand_offset = 0x0; *(line_buffer + 0x7 + operand_offset) == ' ' || *(line_buffer + 0x7 + operand_offset) == '\t'; operand_offset++);
				while (Is_Comment(line_buffer + 0x7 + operand_offset, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x7 + operand_offset) == ' ' || *(line_buffer + 0x7 + operand_offset) == '\t'; operand_offset++);
				if ((*(line_buffer + 0x7 + operand_offset) == '%' && *(line_buffer + 0x8 + operand_offset) == '.') || *(line_buffer + 0x7 + operand_offset) == '\0') {
					LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : expected operand ( line ") + lstrlenA(ln_str) + 0x9);
					StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected operand ( line ") + 0x7, (STRSAFE_LPCSTR)" : expected operand ( line ");
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected operand ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected operand ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
					vector->push_back(std::make_pair(error_message, (BYTE)0x0));
					errors_count++;
				}
				else {
					DWORD sym_size = 0x0;
					if (Is_ID(line_buffer + 0x7 + operand_offset, &sym_size)) {
						for (; sym_size < line_len && (line_buffer[0x7 + operand_offset + sym_size] == ' ' || line_buffer[0x7 + operand_offset + sym_size] == '\t'); sym_size++);
						while (Is_Comment(line_buffer + 0x7 + operand_offset + sym_size, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x7 + operand_offset + sym_size) == ' ' || *(line_buffer + 0x7 + operand_offset + sym_size) == '\t'; operand_offset++);
						if (operand_offset == 0x0) {
							LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : expected space after < Snl_Put > ( line ") + lstrlenA(ln_str) + 0x9);
							StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected space after < Snl_Put > ( line ") + 0x7, (STRSAFE_LPCSTR)" : expected space after < Snl_Put > ( line ");
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected space after < Snl_Put > ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected space after < Snl_Put > ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
							vector->push_back(std::make_pair(error_message, (BYTE)0x0));
							errors_count++;
						}
						else {
							if (*(line_buffer + 0x7 + operand_offset + sym_size) == '%' && *(line_buffer + 0x8 + operand_offset + sym_size) == '.') {
								for (operand_offset += 0x2; *(line_buffer + 0x7 + operand_offset + sym_size) == ' ' || *(line_buffer + 0x7 + operand_offset + sym_size) == '\t'; operand_offset++);
								while (Is_Comment(line_buffer + 0x7 + operand_offset + sym_size, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x7 + operand_offset + sym_size) == ' ' || *(line_buffer + 0x7 + operand_offset + sym_size) == '\t'; operand_offset++);
								if ((sym_size + 0x7 + operand_offset) != line_len) {
									LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : unexpected < ") + (line_len - sym_size - operand_offset - 0x7) + lstrlenA(" > line ( ") + lstrlenA(ln_str) + 0x9);
									StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + 0x7, (STRSAFE_LPCSTR)" : unexpected < ");
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - sym_size - operand_offset - 0x7) + 0x7, (STRSAFE_LPCSTR)(line_buffer + 0x7 + operand_offset + sym_size));
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - sym_size - operand_offset - 0x7) + lstrlenA(" > line ( ") + 0x7, (STRSAFE_LPCSTR)" > line ( ");
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - sym_size - operand_offset - 0x7) + lstrlenA(" > line ( ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - sym_size - operand_offset - 0x7) + lstrlenA(" > line ( ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
									vector->push_back(std::make_pair(error_message, (BYTE)0x0));
									errors_count++;
								}
								else {
									LPSTR message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : affichage de la valeur d'un identificateur") + 0x7);
									StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ + ] ");
									StringCchCatA((STRSAFE_LPSTR)message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
									StringCchCatA((STRSAFE_LPSTR)message, line_len + lstrlenA(" : affichage de la valeur d'un identificateur") + 0x7, (STRSAFE_LPCSTR)" : affichage de la valeur d'un identificateur");
									vector->push_back(std::make_pair(message, (BYTE)0x1));
								}
							}
							else {
								if ((sym_size + 0x7 + operand_offset) != line_len) {
									LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : unexpected second operand ( line ") + lstrlenA(ln_str) + 0x9);
									StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected second operand ( line ") + 0x7, (STRSAFE_LPCSTR)" : unexpected second operand ( line ");
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected second operand ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected second operand ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
									vector->push_back(std::make_pair(error_message, (BYTE)0x0));
									errors_count++;
								}
								else {
									LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : expected %. ( line ") + lstrlenA(ln_str) + 0x9);
									StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected %. ( line ") + 0x7, (STRSAFE_LPCSTR)" : expected %. ( line ");
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected %. ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected %. ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
									vector->push_back(std::make_pair(error_message, (BYTE)0x0));
									errors_count++;
								}
							}
						}
					}
					else if (Is_String(line_buffer + 0x7 + operand_offset, &sym_size)) {
						for (; sym_size < line_len && (line_buffer[0x7 + operand_offset + sym_size] == ' ' || line_buffer[0x7 + operand_offset + sym_size] == '\t'); sym_size++);
						while (Is_Comment(line_buffer + 0x7 + operand_offset + sym_size, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x7 + operand_offset + sym_size) == ' ' || *(line_buffer + 0x7 + operand_offset + sym_size) == '\t'; operand_offset++);
						if (*(line_buffer + 0x7 + operand_offset + sym_size) == '%' && *(line_buffer + 0x8 + operand_offset + sym_size) == '.') {
							for (operand_offset += 0x2; *(line_buffer + 0x7 + operand_offset + sym_size) == ' ' || *(line_buffer + 0x7 + operand_offset + sym_size) == '\t'; operand_offset++);
							while (Is_Comment(line_buffer + 0x7 + operand_offset + sym_size, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x7 + operand_offset + sym_size) == ' ' || *(line_buffer + 0x7 + operand_offset + sym_size) == '\t'; operand_offset++);
							if ((sym_size + 0x7 + operand_offset) != line_len) {
								LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : unexpected < ") + (line_len - sym_size - operand_offset - 0x7) + lstrlenA(" > line ( ") + lstrlenA(ln_str) + 0x9);
								StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + 0x7, (STRSAFE_LPCSTR)" : unexpected < ");
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - sym_size - operand_offset - 0x7) + 0x7, (STRSAFE_LPCSTR)(line_buffer + 0x7 + operand_offset + sym_size));
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - sym_size - operand_offset - 0x7) + lstrlenA(" > line ( ") + 0x7, (STRSAFE_LPCSTR)" > line ( ");
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - sym_size - operand_offset - 0x7) + lstrlenA(" > line ( ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - sym_size - operand_offset - 0x7) + lstrlenA(" > line ( ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
								vector->push_back(std::make_pair(error_message, (BYTE)0x0));
								errors_count++;
							}
							else {
								LPSTR message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : affichage d'une chaine de characteres") + 0x7);
								StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ + ] ");
								StringCchCatA((STRSAFE_LPSTR)message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
								StringCchCatA((STRSAFE_LPSTR)message, line_len + lstrlenA(" : affichage d'une chaine de characteres") + 0x7, (STRSAFE_LPCSTR)" : affichage d'une chaine de characteres");
								vector->push_back(std::make_pair(message, (BYTE)0x1));
							}
						}
						else {
							if ((sym_size + 0x7 + operand_offset) != line_len) {
								LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : unexpected second operand ( line ") + lstrlenA(ln_str) + 0x9);
								StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected second operand ( line ") + 0x7, (STRSAFE_LPCSTR)" : unexpected second operand ( line ");
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected second operand ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected second operand ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
								vector->push_back(std::make_pair(error_message, (BYTE)0x0));
								errors_count++;
							}
							else {
								LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : expected %. ( line ") + lstrlenA(ln_str) + 0x9);
								StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected %. ( line ") + 0x7, (STRSAFE_LPCSTR)" : expected %. ( line ");
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected %. ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected %. ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
								vector->push_back(std::make_pair(error_message, (BYTE)0x0));
								errors_count++;
							}
						}
					}
					else {
						LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : invalid operand ( line ") + lstrlenA(ln_str) + 0x9);
						StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
						StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
						StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : invalid operand ( line ") + 0x7, (STRSAFE_LPCSTR)" : invalid operand ( line ");
						StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : invalid operand ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
						StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : invalid operand ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
						vector->push_back(std::make_pair(error_message, (BYTE)0x0));
						errors_count++;
					}
				}
			}
			else if (strncmp(line_buffer, "Snl_End", lstrlenA("Snl_End")) == 0x0) {
				is_snl_end_exist = TRUE;
				WORD spaces_count;
				for (spaces_count = 0x0; *(line_buffer + 0x7 + spaces_count) == ' ' || *(line_buffer + 0x7 + spaces_count) == '\t'; spaces_count++);
				DWORD symbol_size = 0x0;
				while (Is_Comment(line_buffer + 0x7 + spaces_count, &symbol_size) && *(line_buffer + 0x7 + spaces_count) != '\0') for (spaces_count += symbol_size; *(line_buffer + 0x7 + spaces_count) == ' ' || *(line_buffer + 0x7 + spaces_count) == '\t'; spaces_count++);
				if (*(line_buffer + 0x7 + spaces_count) != '\0') {
					LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : unexpected < ") + (line_len - 0x7 - spaces_count) + lstrlenA(" > line ( ") + lstrlenA(ln_str) + 0x9);
					StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + 0x7, (STRSAFE_LPCSTR)" : unexpected < ");
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - 0x7 - spaces_count) + 0x7, (STRSAFE_LPCSTR)(line_buffer + 0x7 + spaces_count));
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - 0x7 - spaces_count) + lstrlenA(" > line ( ") + 0x7, (STRSAFE_LPCSTR)" > line ( ");
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - 0x7 - spaces_count) + lstrlenA(" > line ( ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - 0x7 - spaces_count) + lstrlenA(" > line ( ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
					vector->push_back(std::make_pair(error_message, (BYTE)0x0));
					errors_count++;
				}
				else {
					BOOL is_valid_position = TRUE;
					for (UINT j = i + spaces_count + 0x7 + skip; j < code_len; j++) if (*(snl_code + j) != '\r' && *(snl_code + j) != '\n') {
						is_valid_position = FALSE;
						break;
					}
					if (is_valid_position) vector->push_back(std::make_pair("[ + ] Snl_End: program end instruction", (BYTE)0x1));
					else {
						LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lstrlenA("Snl_End : unexpected < Snl_End > < must be at the end > ( line ") + lstrlenA(ln_str) + 0x9);
						StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("Snl_End : unexpected < Snl_End > < must be at the end > ( line ") + 0x7, (STRSAFE_LPCSTR)"Snl_End : unexpected < Snl_End > < must be at the end > ( line ");
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("Snl_End : unexpected < Snl_End > < must be at the end > ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("Snl_End : unexpected < Snl_End > < must be at the end > ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
						vector->push_back(std::make_pair(error_message, (BYTE)0x0));
						errors_count++;
					}
				}
			}
			else if (strncmp(line_buffer, "Set", lstrlenA("Set")) == 0x0) {
				WORD operand_offset;
				DWORD comment_size = 0x0;
				for (operand_offset = 0x0; *(line_buffer + 0x3 + operand_offset) == ' ' || *(line_buffer + 0x3 + operand_offset) == '\t'; operand_offset++);
				while (Is_Comment(line_buffer + 0x3 + operand_offset, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x3 + operand_offset) == ' ' || *(line_buffer + 0x3 + operand_offset) == '\t'; operand_offset++);
				if ((*(line_buffer + 0x3 + operand_offset) == '%' && *(line_buffer + 0x4 + operand_offset) == '.') || *(line_buffer + 0x3 + operand_offset) == '\0') {
					LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : expected first operand ( line ") + lstrlenA(ln_str) + 0x9);
					StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected first operand ( line ") + 0x7, (STRSAFE_LPCSTR)" : expected first operand ( line ");
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected first operand ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected first operand ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
					vector->push_back(std::make_pair(error_message, (BYTE)0x0));
					errors_count++;
				}
				else {
					DWORD symbol_size = 0x0;
					if (!Is_ID(line_buffer + 0x3 + operand_offset, &symbol_size)) {
						LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : invalid first operand < must be an id > ( line ") + lstrlenA(ln_str) + 0x9);
						StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
						StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
						StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : invalid first operand < must be an id > ( line ") + 0x7, (STRSAFE_LPCSTR)" : invalid first operand < must be an id > ( line ");
						StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : invalid first operand < must be an id > ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
						StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : invalid first operand < must be an id > ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
						vector->push_back(std::make_pair(error_message, (BYTE)0x0));
						errors_count++;
					}
					else {
						if (operand_offset == 0x0) {
							LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : expected space after < Set > ( line ") + lstrlenA(ln_str) + 0x9);
							StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected space after < Set > ( line ") + 0x7, (STRSAFE_LPCSTR)" : expected space after < Set > ( line ");
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected space after < Set > ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected space after < Set > ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
							vector->push_back(std::make_pair(error_message, (BYTE)0x0));
							errors_count++;
						}
						else {
							DWORD temp = (operand_offset += symbol_size);
							for (; *(line_buffer + 0x3 + operand_offset) == ' ' || *(line_buffer + 0x3 + operand_offset) == '\t'; operand_offset++);
							while (Is_Comment(line_buffer + 0x3 + operand_offset, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x3 + operand_offset) == ' ' || *(line_buffer + 0x3 + operand_offset) == '\t'; operand_offset++);
							if ((*(line_buffer + 0x3 + operand_offset) == '%' && *(line_buffer + 0x4 + operand_offset) == '.') || *(line_buffer + 0x3 + operand_offset) == '\0') {
								LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : expected second operand ( line ") + lstrlenA(ln_str) + 0x9);
								StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected second operand ( line ") + 0x7, (STRSAFE_LPCSTR)" : expected second operand ( line ");
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected second operand ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected second operand ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
								vector->push_back(std::make_pair(error_message, (BYTE)0x0));
								errors_count++;
							}
							else if (operand_offset == temp) {
								LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : expected space after the first operand ( line ") + lstrlenA(ln_str) + 0x9);
								StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected space after the first operand ( line ") + 0x7, (STRSAFE_LPCSTR)" : expected space after the first operand ( line ");
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected space after the first operand ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected space after the first operand ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
								vector->push_back(std::make_pair(error_message, (BYTE)0x0));
								errors_count++;
							}
							else {
								if (!Is_String(line_buffer + 0x3 + operand_offset, &symbol_size) && !Is_FloatNumber(line_buffer + 0x3 + operand_offset, &symbol_size) && !Is_IntegerNumber(line_buffer + 0x3 + operand_offset, &symbol_size)) {
									LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : invalid second operand < must be a constant value > ( line ") + lstrlenA(ln_str) + 0x9);
									StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : invalid second operand < must be a constant value > ( line ") + 0x7, (STRSAFE_LPCSTR)" : invalid second operand < must be a constant value > ( line ");
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : invalid second operand < must be a constant value > ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : invalid second operand < must be a constant value > ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
									vector->push_back(std::make_pair(error_message, (BYTE)0x0));
									errors_count++;
								}
								else {
									for (operand_offset += symbol_size; *(line_buffer + 0x3 + operand_offset) == ' ' || *(line_buffer + 0x3 + operand_offset) == '\t'; operand_offset++);
									while (Is_Comment(line_buffer + 0x3 + operand_offset, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x3 + operand_offset) == ' ' || *(line_buffer + 0x3 + operand_offset) == '\t'; operand_offset++);
									if (*(line_buffer + 0x3 + operand_offset) == '%' && *(line_buffer + 0x4 + operand_offset) == '.') {
										for (operand_offset += 0x2; *(line_buffer + 0x3 + operand_offset) == ' ' || *(line_buffer + 0x3 + operand_offset) == '\t'; operand_offset++);
										while (Is_Comment(line_buffer + 0x3 + operand_offset, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x3 + operand_offset) == ' ' || *(line_buffer + 0x3 + operand_offset) == '\t'; operand_offset++);
										if ((operand_offset + 0x3) != line_len) {
											LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : unexpected < ") + (line_len - operand_offset - 0x3) + lstrlenA(" > line ( ") + lstrlenA(ln_str) + 0x9);
											StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
											StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
											StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + 0x7, (STRSAFE_LPCSTR)" : unexpected < ");
											StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - operand_offset - 0x3) + 0x7, (STRSAFE_LPCSTR)(line_buffer + 0x3 + operand_offset));
											StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - operand_offset - 0x3) + lstrlenA(" > line ( ") + 0x7, (STRSAFE_LPCSTR)" > line ( ");
											StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - operand_offset - 0x3) + lstrlenA(" > line ( ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
											StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - operand_offset - 0x3) + lstrlenA(" > line ( ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
											vector->push_back(std::make_pair(error_message, (BYTE)0x0));
											errors_count++;
										}
										else {
											LPSTR message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : affectation en utilisant < Set >") + 0x7);
											StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ + ] ");
											StringCchCatA((STRSAFE_LPSTR)message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
											StringCchCatA((STRSAFE_LPSTR)message, line_len + lstrlenA(" : affectation en utilisant < Set >") + 0x7, (STRSAFE_LPCSTR)" : affectation en utilisant < Set >");
											vector->push_back(std::make_pair(message, (BYTE)0x1));
										}
									}
									else {
										if ((operand_offset + 0x3) != line_len) {
											LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : unexpected third operand ( line ") + lstrlenA(ln_str) + 0x9);
											StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
											StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
											StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected third operand ( line ") + 0x7, (STRSAFE_LPCSTR)" : unexpected third operand ( line ");
											StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected third operand ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
											StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected third operand ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
											vector->push_back(std::make_pair(error_message, (BYTE)0x0));
											errors_count++;
										}
										else {
											LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : expected %. ( line ") + lstrlenA(ln_str) + 0x9);
											StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
											StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
											StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected %. ( line ") + 0x7, (STRSAFE_LPCSTR)" : expected %. ( line ");
											StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected %. ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
											StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected %. ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
											vector->push_back(std::make_pair(error_message, (BYTE)0x0));
											errors_count++;
										}
									}
								}
							}
						}
					}
				}
			}
			else if (strncmp(line_buffer, "Get", lstrlenA("Get")) == 0x0) {
				WORD operand_offset;
				DWORD comment_size = 0x0;
				for (operand_offset = 0x0; *(line_buffer + 0x3 + operand_offset) == ' ' || *(line_buffer + 0x3 + operand_offset) == '\t'; operand_offset++);
				while (Is_Comment(line_buffer + 0x3 + operand_offset, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x3 + operand_offset) == ' ' || *(line_buffer + 0x3 + operand_offset) == '\t'; operand_offset++);
				if ((*(line_buffer + 0x3 + operand_offset) == '%' && *(line_buffer + 0x4 + operand_offset) == '.') || *(line_buffer + 0x3 + operand_offset) == '\0') {
					LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : expected first operand ( line ") + lstrlenA(ln_str) + 0x9);
					StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected first operand ( line ") + 0x7, (STRSAFE_LPCSTR)" : expected first operand ( line ");
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected first operand ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected first operand ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
					vector->push_back(std::make_pair(error_message, (BYTE)0x0));
					errors_count++;
				}
				else {
					if (operand_offset == 0x0) {
						LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : expected space after < Get > ( line ") + lstrlenA(ln_str) + 0x9);
						StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
						StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
						StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected space after < Get > ( line ") + 0x7, (STRSAFE_LPCSTR)" : expected space after < Get > ( line ");
						StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected space after < Get > ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
						StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected space after < Get > ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
						vector->push_back(std::make_pair(error_message, (BYTE)0x0));
						errors_count++;
					}
					else {
						DWORD symbol_size = 0x0;
						if (!Is_ID(line_buffer + 0x3 + operand_offset, &symbol_size)) {
							LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : invalid first operand < must be an id > ( line ") + lstrlenA(ln_str) + 0x9);
							StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : invalid first operand < must be an id > ( line ") + 0x7, (STRSAFE_LPCSTR)" : invalid first operand < must be an id > ( line ");
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : invalid first operand < must be an id > ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : invalid first operand < must be an id > ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
							vector->push_back(std::make_pair(error_message, (BYTE)0x0));
							errors_count++;
						}
						else {
							DWORD temp = (operand_offset += symbol_size);
							for (; *(line_buffer + 0x3 + operand_offset) == ' ' || *(line_buffer + 0x3 + operand_offset) == '\t'; operand_offset++);
							while (Is_Comment(line_buffer + 0x3 + operand_offset, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x3 + operand_offset) == ' ' || *(line_buffer + 0x3 + operand_offset) == '\t'; operand_offset++);
							if ((*(line_buffer + 0x3 + operand_offset) == '%' && *(line_buffer + 0x4 + operand_offset) == '.') || *(line_buffer + 0x3 + operand_offset) == '\0') {
								LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : expected < from > ( line ") + lstrlenA(ln_str) + 0x9);
								StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected < from > ( line ") + 0x7, (STRSAFE_LPCSTR)" : expected < from > ( line ");
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected < from > ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected < from > ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
								vector->push_back(std::make_pair(error_message, (BYTE)0x0));
								errors_count++;
							}
							else if (operand_offset == temp) {
								LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : expected space after the first opearnd ( line ") + lstrlenA(ln_str) + 0x9);
								StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected space after the first opearnd ( line ") + 0x7, (STRSAFE_LPCSTR)" : expected space after the first opearnd ( line ");
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected space after the first opearnd ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected space after the first opearnd ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
								vector->push_back(std::make_pair(error_message, (BYTE)0x0));
								errors_count++;
							}
							else {
								if (strncmp(line_buffer + 0x3 + operand_offset, "from", 0x4) != 0x0) {
									LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : expected < from > after the first operand ( line ") + lstrlenA(ln_str) + 0x9);
									StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected < from > after the first operand ( line ") + 0x7, (STRSAFE_LPCSTR)" : expected < from > after the first operand ( line ");
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected < from > after the first operand ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected < from > after the first operand ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
									vector->push_back(std::make_pair(error_message, (BYTE)0x0));
									errors_count++;
								}
								else {
									temp = (operand_offset += 0x4);
									for (; *(line_buffer + 0x3 + operand_offset) == ' ' || *(line_buffer + 0x3 + operand_offset) == '\t'; operand_offset++);
									while (Is_Comment(line_buffer + 0x3 + operand_offset, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x3 + operand_offset) == ' ' || *(line_buffer + 0x3 + operand_offset) == '\t'; operand_offset++);
									if ((*(line_buffer + 0x3 + operand_offset) == '%' && *(line_buffer + 0x4 + operand_offset) == '.') || *(line_buffer + 0x3 + operand_offset) == '\0') {
										LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : expected second operand after < from > ( line ") + lstrlenA(ln_str) + 0x9);
										StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
										StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
										StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected second operand after < from > ( line ") + 0x7, (STRSAFE_LPCSTR)" : expected second operand after < from > ( line ");
										StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected second operand after < from > ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
										StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected second operand after < from > ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
										vector->push_back(std::make_pair(error_message, (BYTE)0x0));
										errors_count++;
									}
									else if (operand_offset == temp) {
										LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : expected space after < from > ( line ") + lstrlenA(ln_str) + 0x9);
										StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
										StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
										StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected space after < from > ( line ") + 0x7, (STRSAFE_LPCSTR)" : expected space after < from > ( line ");
										StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected space after < from > ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
										StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected space after < from > ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
										vector->push_back(std::make_pair(error_message, (BYTE)0x0));
										errors_count++;
									}
									else {
										if (!Is_ID(line_buffer + 0x3 + operand_offset, &symbol_size)) {
											LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : invalid second operand < must be an id > ( line ") + lstrlenA(ln_str) + 0x9);
											StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
											StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
											StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : invalid second operand < must be an id > ( line ") + 0x7, (STRSAFE_LPCSTR)" : invalid second operand < must be an id > ( line ");
											StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : invalid second operand < must be an id > ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
											StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : invalid second operand < must be an id > ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
											vector->push_back(std::make_pair(error_message, (BYTE)0x0));
											errors_count++;
										}
										else {
											for (operand_offset += symbol_size; *(line_buffer + 0x3 + operand_offset) == ' ' || *(line_buffer + 0x3 + operand_offset) == '\t'; operand_offset++);
											while (Is_Comment(line_buffer + 0x3 + operand_offset, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x3 + operand_offset) == ' ' || *(line_buffer + 0x3 + operand_offset) == '\t'; operand_offset++);
											if (*(line_buffer + 0x3 + operand_offset) == '%' && *(line_buffer + 0x4 + operand_offset) == '.') {
												for (operand_offset += 0x2; *(line_buffer + 0x3 + operand_offset) == ' ' || *(line_buffer + 0x3 + operand_offset) == '\t'; operand_offset++);
												while (Is_Comment(line_buffer + 0x3 + operand_offset, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x3 + operand_offset) == ' ' || *(line_buffer + 0x3 + operand_offset) == '\t'; operand_offset++);
												if ((operand_offset + 0x3) != line_len) {
													LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : unexpected < ") + (line_len - operand_offset - 0x3) + lstrlenA(" > line ( ") + lstrlenA(ln_str) + 0x9);
													StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
													StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
													StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + 0x7, (STRSAFE_LPCSTR)" : unexpected < ");
													StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - operand_offset - 0x3) + 0x7, (STRSAFE_LPCSTR)(line_buffer + 0x3 + operand_offset));
													StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - operand_offset - 0x3) + lstrlenA(" > line ( ") + 0x7, (STRSAFE_LPCSTR)" > line ( ");
													StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - operand_offset - 0x3) + lstrlenA(" > line ( ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
													StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - operand_offset - 0x3) + lstrlenA(" > line ( ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
													vector->push_back(std::make_pair(error_message, (BYTE)0x0));
													errors_count++;
												}
												else {
													LPSTR message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : affectation en utilisant < Get / from>") + 0x7);
													StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ + ] ");
													StringCchCatA((STRSAFE_LPSTR)message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
													StringCchCatA((STRSAFE_LPSTR)message, line_len + lstrlenA(" : affectation en utilisant < Get / from >") + 0x7, (STRSAFE_LPCSTR)" : affectation en utilisant < Get / from >");
													vector->push_back(std::make_pair(message, (BYTE)0x1));
												}
											}
											else {
												if ((operand_offset + 0x3) != line_len) {
													LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : unexpected third operand ( line ") + lstrlenA(ln_str) + 0x9);
													StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
													StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
													StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected third operand ( line ") + 0x7, (STRSAFE_LPCSTR)" : unexpected third operand ( line ");
													StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected third operand ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
													StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected third operand ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
													vector->push_back(std::make_pair(error_message, (BYTE)0x0));
													errors_count++;
												}
												else {
													LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : expected %. ( line ") + lstrlenA(ln_str) + 0x9);
													StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
													StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
													StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected %. ( line ") + 0x7, (STRSAFE_LPCSTR)" : expected %. ( line ");
													StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected %. ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
													StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected %. ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
													vector->push_back(std::make_pair(error_message, (BYTE)0x0));
													errors_count++;
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
			else if (strncmp(line_buffer, "If", lstrlenA("If")) == 0x0) {
				last_ifBlock_line = line_number;
				WORD line_iterator;
				DWORD comment_size = 0x0;
				for (line_iterator = 0x0; *(line_buffer + 0x2 + line_iterator) == ' ' || *(line_buffer + 0x2 + line_iterator) == '\t'; line_iterator++);
				while (Is_Comment(line_buffer + 0x2 + line_iterator, &comment_size)) for (line_iterator += comment_size; *(line_buffer + 0x2 + line_iterator) == ' ' || *(line_buffer + 0x2 + line_iterator) == '\t'; line_iterator++);
				if (*(line_buffer + 0x2 + line_iterator) != '[') {
					LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : expected < [ > ( line ") + lstrlenA(ln_str) + 0x9);
					StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected < [ > ( line ") + 0x7, (STRSAFE_LPCSTR)" : expected < [ > ( line ");
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected < [ > ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected < [ > ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
					vector->push_back(std::make_pair(error_message, (BYTE)0x0));
					errors_count++;
				}
				else {
					for (line_iterator += 0x1; *(line_buffer + 0x2 + line_iterator) == ' ' || *(line_buffer + 0x2 + line_iterator) == '\t'; line_iterator++);
					while (Is_Comment(line_buffer + 0x2 + line_iterator, &comment_size)) for (line_iterator += comment_size; *(line_buffer + 0x2 + line_iterator) == ' ' || *(line_buffer + 0x2 + line_iterator) == '\t'; line_iterator++);
					DWORD symbol_size = 0x0;
					if (!Is_ID(line_buffer + 0x2 + line_iterator, &symbol_size)) {
						LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : expected identifier after < [ > ( line ") + lstrlenA(ln_str) + 0x9);
						StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
						StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
						StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected identifier after < [ > ( line ") + 0x7, (STRSAFE_LPCSTR)" : expected identifier after < [ > ( line ");
						StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected identifier after < [ > ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
						StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected identifier after < [ > ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
						vector->push_back(std::make_pair(error_message, (BYTE)0x0));
						errors_count++;
					}
					else {
						for (line_iterator += symbol_size; *(line_buffer + 0x2 + line_iterator) == ' ' || *(line_buffer + 0x2 + line_iterator) == '\t'; line_iterator++);
						while (Is_Comment(line_buffer + 0x2 + line_iterator, &comment_size)) for (line_iterator += comment_size; *(line_buffer + 0x2 + line_iterator) == ' ' || *(line_buffer + 0x2 + line_iterator) == '\t'; line_iterator++);
						if (!Is_CmpOpeartor(line_buffer + 0x2 + line_iterator, &symbol_size)) {
							LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : expected comparaison operator ( line ") + lstrlenA(ln_str) + 0x9);
							StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected comparaison operator ( line ") + 0x7, (STRSAFE_LPCSTR)" : expected comparaison operator ( line ");
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected comparaison operator ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
							StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected comparaison operator ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
							vector->push_back(std::make_pair(error_message, (BYTE)0x0));
							errors_count++;
						}
						else {
							for (line_iterator += symbol_size; *(line_buffer + 0x2 + line_iterator) == ' ' || *(line_buffer + 0x2 + line_iterator) == '\t'; line_iterator++);
							while (Is_Comment(line_buffer + 0x2 + line_iterator, &comment_size)) for (line_iterator += comment_size; *(line_buffer + 0x2 + line_iterator) == ' ' || *(line_buffer + 0x2 + line_iterator) == '\t'; line_iterator++);
							if (!Is_String(line_buffer + 0x2 + line_iterator, &symbol_size) && !Is_ID(line_buffer + 0x2 + line_iterator, &symbol_size) && !Is_FloatNumber(line_buffer + 0x2 + line_iterator, &symbol_size) && !Is_IntegerNumber(line_buffer + 0x2 + line_iterator, &symbol_size)) {
								LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : expected either an id, a number or a string ( line ") + lstrlenA(ln_str) + 0x9);
								StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
								StringCchCopyA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected either an id, a number or a string ( line ") + 0x7, (STRSAFE_LPCSTR)" : expected either an id, a number or a string ( line ");
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected either an id, a number or a string ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
								StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected either an id, a number or a string ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
								vector->push_back(std::make_pair(error_message, (BYTE)0x0));
								errors_count++;
							}
							else {
								for (line_iterator += symbol_size; *(line_buffer + 0x2 + line_iterator) == ' ' || *(line_buffer + 0x2 + line_iterator) == '\t'; line_iterator++);
								while (Is_Comment(line_buffer + 0x2 + line_iterator, &comment_size)) for (line_iterator += comment_size; *(line_buffer + 0x2 + line_iterator) == ' ' || *(line_buffer + 0x2 + line_iterator) == '\t'; line_iterator++);
								if (*(line_buffer + 0x2 + line_iterator) != ']') {
									LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : expected < ] > ( line ") + lstrlenA(ln_str) + 0x9);
									StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected < ] > ( line ") + 0x7, (STRSAFE_LPCSTR)" : expected < ] > ( line ");
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected < ] > ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
									StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected < ] > ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
									vector->push_back(std::make_pair(error_message, (BYTE)0x0));
									errors_count++;
								}
								else {
									for (line_iterator += 0x1; *(line_buffer + 0x2 + line_iterator) == ' ' || *(line_buffer + 0x2 + line_iterator) == '\t'; line_iterator++);
									while (Is_Comment(line_buffer + 0x2 + line_iterator, &comment_size)) for (line_iterator += comment_size; *(line_buffer + 0x2 + line_iterator) == ' ' || *(line_buffer + 0x2 + line_iterator) == '\t'; line_iterator++);
									if (*(line_buffer + 0x2 + line_iterator) != '\0') {
										WORD spaces_count;
										for (spaces_count = 0x0; *(line_buffer + 0x3 + line_iterator + spaces_count) == ' ' || *(line_buffer + 0x3 + line_iterator + spaces_count) == '\t'; spaces_count++);
										LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : unexpected < ") + (line_len - line_iterator - spaces_count - 0x3) + lstrlenA(" > line ( ") + lstrlenA(ln_str) + 0x9);
										StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
										StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
										StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + 0x7, (STRSAFE_LPCSTR)" : unexpected < ");
										StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - line_iterator - spaces_count - 0x3) + 0x7, (STRSAFE_LPCSTR)(line_buffer + 0x3 + spaces_count + line_iterator));
										StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - line_iterator - spaces_count - 0x3) + lstrlenA(" > line ( ") + 0x7, (STRSAFE_LPCSTR)" > line ( ");
										StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - line_iterator - spaces_count - 0x3) + lstrlenA(" > line ( ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
										StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - line_iterator - spaces_count - 0x3) + lstrlenA(" > line ( ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
										vector->push_back(std::make_pair(error_message, (BYTE)0x0));
										errors_count++;
									}
									else {
										BOOL is_valid_inst = FALSE;
										if ((last_elseBlock_line != 0x0 && line_number > last_elseBlock_line + 0x1) || last_elseBlock_line == 0x0) {
											LPSTR message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : if block") + 0x7);
											StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ + ] ");
											StringCchCatA((STRSAFE_LPSTR)message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
											StringCchCatA((STRSAFE_LPSTR)message, line_len + lstrlenA(" : if block") + 0x7, (STRSAFE_LPCSTR)" : if block");
											vector->push_back(std::make_pair(message, (BYTE)0x1));
										}
										else {
											LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : expected an instruction before < If block > ( line ") + lstrlenA(ln_str) + 0x9);
											StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
											StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
											StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected an instruction before < If block > ( line ") + 0x7, (STRSAFE_LPCSTR)" : expected an instruction before < If block > ( line ");
											StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected an instruction before < If block > ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
											StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : expected an instruction before < If block > ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
											vector->push_back(std::make_pair(error_message, (BYTE)0x0));
											errors_count++;
										}
									}
								}
							}
						}
					}
				}
			}
			else if (strncmp(line_buffer, "Else", lstrlenA("Else")) == 0x0) {
				last_elseBlock_line = line_number;
				WORD spaces_count;
				for (spaces_count = 0x0; *(line_buffer + 0x4 + spaces_count) == ' ' || *(line_buffer + 0x4 + spaces_count) == '\t'; spaces_count++);
				DWORD symbol_size = 0x0;
				while (Is_Comment(line_buffer + 0x4 + spaces_count, &symbol_size) && *(line_buffer + 0x4 + spaces_count) != '\0') for (spaces_count += symbol_size; *(line_buffer + 0x4 + spaces_count) == ' ' || *(line_buffer + 0x4 + spaces_count) == '\t'; spaces_count++);
				if (*(line_buffer + 0x4 + spaces_count) != '\0') {
					LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : unexpected < ") + (line_len - 0x4 - spaces_count) + lstrlenA(" > line ( ") + lstrlenA(ln_str) + 0x9);
					StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + 0x7, (STRSAFE_LPCSTR)" : unexpected < ");
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - 0x4 - spaces_count) + 0x7, (STRSAFE_LPCSTR)(line_buffer + 0x4 + spaces_count));
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - 0x4 - spaces_count) + lstrlenA(" > line ( ") + 0x7, (STRSAFE_LPCSTR)" > line ( ");
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - 0x4 - spaces_count) + lstrlenA(" > line ( ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
					StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : unexpected < ") + (line_len - 0x4 - spaces_count) + lstrlenA(" > line ( ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
					vector->push_back(std::make_pair(error_message, (BYTE)0x0));
					errors_count++;
				}
				else {
					if (last_ifBlock_line != 0x0) {
						if(line_number == last_ifBlock_line + 0x2) vector->push_back(std::make_pair("[ + ] Else: Else block", (BYTE)0x1));
						else if (line_number == last_ifBlock_line + 0x1) {
							LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lstrlenA("Else : expected an instruction before < Else > ( line ") + lstrlenA(ln_str) + 0x9);
							StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
							StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("Else : expected an instruction before < Else > ( line ") + 0x7, (STRSAFE_LPCSTR)"Else : expected an instruction before < Else > ( line ");
							StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("Else : expected an instruction before < Else > ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
							StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("Else : expected an instruction before < Else > ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
							vector->push_back(std::make_pair(error_message, (BYTE)0x0));
							errors_count++;
						}
						else {
							LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lstrlenA("Else : expected a valid < If block > before < Else > ( line ") + lstrlenA(ln_str) + 0x9);
							StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
							StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("Else : expected a valid < If block > before < Else > ( line ") + 0x7, (STRSAFE_LPCSTR)"Else : expected a valid < If block > before < Else > ( line ");
							StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("Else : expected a valid < If block > before < Else > ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
							StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("Else : expected a valid < If block > before < Else > ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
							vector->push_back(std::make_pair(error_message, (BYTE)0x0));
							errors_count++;
						}
					}
					else {
						LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lstrlenA("Else : expected a valid < If block > before < Else > ( line ") + lstrlenA(ln_str) + 0x9);
						StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("Else : expected a valid < If block > before < Else > ( line ") + 0x7, (STRSAFE_LPCSTR)"Else : expected a valid < If block > before < Else > ( line ");
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("Else : expected a valid < If block > before < Else > ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("Else : expected a valid < If block > before < Else > ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
						vector->push_back(std::make_pair(error_message, (BYTE)0x0));
						errors_count++;
					}
				}
			}
			else {
				LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, line_len + lstrlenA(" : Unknown instruction ( line ") + lstrlenA(ln_str) + 0x9);
				StringCchCopyA((STRSAFE_LPSTR)error_message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
				StringCchCatA((STRSAFE_LPSTR)error_message, line_len + 0x7, (STRSAFE_LPCSTR)line_buffer);
				StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : Unknown instruction ( line ") + 0x7, (STRSAFE_LPCSTR)" : Unknown instruction ( line ");
				StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : Unknown instruction ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
				StringCchCatA((STRSAFE_LPSTR)error_message, line_len + lstrlenA(" : Unknown instruction ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
				vector->push_back(std::make_pair(error_message, (BYTE)0x0));
				errors_count++;
			}
			HeapFree(GetProcessHeap(), 0x0, line_buffer);
		}
		line_number++;
		_itoa_s(line_number, ln_str, 10);
		i += offset;
	}
	if (!is_snl_begin_exist) {
		vector->push_back(std::make_pair("[ - ] <! Syntactic error !> : expected < Snl_Begin > at the beginning", (BYTE)0x0));
		errors_count++;
	}
	if (!is_snl_end_exist) {
		vector->push_back(std::make_pair("[ - ] <! Syntactic error !> : expected < Snl_End > at the end", (BYTE)0x0));
		errors_count++;
	}
	return errors_count;
}
ULONG __stdcall AnalyseLexicale(LPSTR snl_code, std::vector<std::pair<LPCSTR, BYTE> >* vector) {
	DWORD code_len = lstrlenA(snl_code);
	if (code_len == 0x0) {
		vector->push_back(std::make_pair("no instructions !!", (BYTE)0x1));
		return 0x0;
	}
	ULONG errors_count = 0x0;
	UINT offset = 0x0;
	INT error_offset = -1;
	UINT line_number = 0x1;
	char ln_str[20];
	_itoa_s(line_number, ln_str, 10);
	for (ULONGLONG i = 0x0; i < (ULONGLONG)(code_len + 1); i += offset) {
		if (snl_code[i] == ' ' || snl_code[i] == '\t' || snl_code[i] == '\n' || snl_code[i] == '\r') {
			if (snl_code[i] == '\r') {
				if (error_offset != -1) {
					LPSTR message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9);
					StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + 0x7, (STRSAFE_LPCSTR)(snl_code + error_offset));
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + 0x7, (STRSAFE_LPCSTR)" : erreur lexicale ( line ");
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
					vector->push_back(std::make_pair(message, (BYTE)0x0));
					error_offset = -1;
				}
				line_number++;
				_itoa_s(line_number, ln_str, 10);
			}
			offset = 0x1;
		}
		else if (snl_code[i] == '\0') {
			if (error_offset != -1) {
				LPSTR message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9);
				StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
				StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + 0x7, (STRSAFE_LPCSTR)(snl_code + error_offset));
				StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + 0x7, (STRSAFE_LPCSTR)" : erreur lexicale ( line ");
				StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
				StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
				vector->push_back(std::make_pair(message, (BYTE)0x0));
				error_offset = -1;
			}
		}
		else {
			if (strncmp(snl_code + i, "Snl_Begin", lstrlenA("Snl_Begin")) == 0x0) {
				if (error_offset != -1) {
					LPSTR message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9);
					StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + 0x7, (STRSAFE_LPCSTR)(snl_code + error_offset));
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + 0x7, (STRSAFE_LPCSTR)" : erreur lexicale ( line ");
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
					vector->push_back(std::make_pair(message, (BYTE)0x0));
					error_offset = -1;
				}
				vector->push_back(std::make_pair("[ + ] Snl_Begin : mot cl頤e debut de programme", (BYTE)0x1));
				offset = lstrlenA("Snl_Begin");
			}
			else if (strncmp(snl_code + i, "Snl_Int", lstrlenA("Snl_Int")) == 0x0) {
				if (error_offset != -1) {
					LPSTR message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9);
					StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + 0x7, (STRSAFE_LPCSTR)(snl_code + error_offset));
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + 0x7, (STRSAFE_LPCSTR)" : erreur lexicale ( line ");
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
					vector->push_back(std::make_pair(message, (BYTE)0x0));
					error_offset = -1;
				}
				vector->push_back(std::make_pair("[ + ] Snl_Int : mot cl頤e declaration de type entier", (BYTE)0x1));
				offset = lstrlenA("Snl_Int");
			}
			else if (strncmp(snl_code + i, "Snl_Real", lstrlenA("Snl_Real")) == 0x0) {
				if (error_offset != -1) {
					LPSTR message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9);
					StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + 0x7, (STRSAFE_LPCSTR)(snl_code + error_offset));
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + 0x7, (STRSAFE_LPCSTR)" : erreur lexicale ( line ");
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
					vector->push_back(std::make_pair(message, (BYTE)0x0));
					error_offset = -1;
				}
				vector->push_back(std::make_pair("[ + ] Snl_Real : mot cl頤e declaration de type reel", (BYTE)0x1));
				offset = lstrlenA("Snl_Real");
			}
			else if (strncmp(snl_code + i, "If", lstrlenA("If")) == 0x0) {
				if (error_offset != -1) {
					LPSTR message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9);
					StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + 0x7, (STRSAFE_LPCSTR)(snl_code + error_offset));
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + 0x7, (STRSAFE_LPCSTR)" : erreur lexicale ( line ");
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
					vector->push_back(std::make_pair(message, (BYTE)0x0));
					error_offset = -1;
				}
				vector->push_back(std::make_pair("[ + ] If : mot cl頰our conditionnel", (BYTE)0x1));
				offset = lstrlenA("If");
			}
			else if (strncmp(snl_code + i, "Else", lstrlenA("Else")) == 0x0) {
				if (error_offset != -1) {
					LPSTR message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9);
					StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + 0x7, (STRSAFE_LPCSTR)(snl_code + error_offset));
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + 0x7, (STRSAFE_LPCSTR)" : erreur lexicale ( line ");
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
					vector->push_back(std::make_pair(message, (BYTE)0x0));
					error_offset = -1;
				}
				vector->push_back(std::make_pair("[ + ] Else : mot cl頰our conditionnel", (BYTE)0x1));
				offset = lstrlenA("Else");
			}
			else if (strncmp(snl_code + i, "Snl_End", lstrlenA("Snl_End")) == 0x0) {
				if (error_offset != -1) {
					LPSTR message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9);
					StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + 0x7, (STRSAFE_LPCSTR)(snl_code + error_offset));
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + 0x7, (STRSAFE_LPCSTR)" : erreur lexicale ( line ");
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
					vector->push_back(std::make_pair(message, (BYTE)0x0));
					error_offset = -1;
				}
				vector->push_back(std::make_pair("[ + ] Snl_End : mot cl頤e fin de programme", (BYTE)0x1));
				offset = lstrlenA("Snl_End");
			}
			else if (strncmp(snl_code + i, "Set", lstrlenA("Set")) == 0x0) {
				if (error_offset != -1) {
					LPSTR message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9);
					StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + 0x7, (STRSAFE_LPCSTR)(snl_code + error_offset));
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + 0x7, (STRSAFE_LPCSTR)" : erreur lexicale ( line ");
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
					vector->push_back(std::make_pair(message, (BYTE)0x0));
					error_offset = -1;
				}
				vector->push_back(std::make_pair("[ + ] Set : mot cl頰our l'affectation d'une valeur", (BYTE)0x1));
				offset = lstrlenA("Set");
			}
			else if (strncmp(snl_code + i, "Get", lstrlenA("Get")) == 0x0) {
				if (error_offset != -1) {
					LPSTR message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9);
					StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + 0x7, (STRSAFE_LPCSTR)(snl_code + error_offset));
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + 0x7, (STRSAFE_LPCSTR)" : erreur lexicale ( line ");
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
					vector->push_back(std::make_pair(message, (BYTE)0x0));
					error_offset = -1;
				}
				vector->push_back(std::make_pair("[ + ] Get : mot cl頰our l'affectation d'une valeur", (BYTE)0x1));
				offset = lstrlenA("Get");
			}
			else if (strncmp(snl_code + i, "from", lstrlenA("from")) == 0x0) {
				if (error_offset != -1) {
					LPSTR message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9);
					StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + 0x7, (STRSAFE_LPCSTR)(snl_code + error_offset));
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + 0x7, (STRSAFE_LPCSTR)" : erreur lexicale ( line ");
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
					vector->push_back(std::make_pair(message, (BYTE)0x0));
					error_offset = -1;
				}
				vector->push_back(std::make_pair("[ + ] from : mot cl頰our l'affectation d'une valeur", (BYTE)0x1));
				offset = lstrlenA("from");
			}
			else if (strncmp(snl_code + i, "%.", lstrlenA("%.")) == 0x0) {
				if (error_offset != -1) {
					LPSTR message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9);
					StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + 0x7, (STRSAFE_LPCSTR)(snl_code + error_offset));
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + 0x7, (STRSAFE_LPCSTR)" : erreur lexicale ( line ");
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
					vector->push_back(std::make_pair(message, (BYTE)0x0));
					error_offset = -1;
				}
				vector->push_back(std::make_pair("[ + ] %. : fin d'instruction", (BYTE)0x1));
				offset = lstrlenA("%.");
			}
			else if (strncmp(snl_code + i, "Snl_Put", lstrlenA("Snl_Put")) == 0x0) {
				if (error_offset != -1) {
					LPSTR message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9);
					StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + 0x7, (STRSAFE_LPCSTR)(snl_code + error_offset));
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + 0x7, (STRSAFE_LPCSTR)" : erreur lexicale ( line ");
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
					vector->push_back(std::make_pair(message, (BYTE)0x0));
					error_offset = -1;
				}
				vector->push_back(std::make_pair("[ + ] Snl_put : mot cl頰our l'affichage", (BYTE)0x1));
				offset = lstrlenA("Snl_Put");
			}
			else if (strncmp(snl_code + i, "Snl_St", lstrlenA("Snl_St")) == 0x0) {
				if (error_offset != -1) {
					LPSTR message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9);
					StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + 0x7, (STRSAFE_LPCSTR)(snl_code + error_offset));
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + 0x7, (STRSAFE_LPCSTR)" : erreur lexicale ( line ");
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
					vector->push_back(std::make_pair(message, (BYTE)0x0));
					error_offset = -1;
				}
				vector->push_back(std::make_pair("[ + ] Snl_St : mot cl頤e declaration de type string", (BYTE)0x1));
				offset = lstrlenA("Snl_St");
			}
			else if (strncmp(snl_code + i, "[", lstrlenA("[")) == 0x0) {
				if (error_offset != -1) {
					LPSTR message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9);
					StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + 0x7, (STRSAFE_LPCSTR)(snl_code + error_offset));
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + 0x7, (STRSAFE_LPCSTR)" : erreur lexicale ( line ");
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
					vector->push_back(std::make_pair(message, (BYTE)0x0));
					error_offset = -1;
				}
				vector->push_back(std::make_pair("[ + ] [ : debut de condition", (BYTE)0x1));
				offset = 0x1;
			}
			else if (strncmp(snl_code + i, "]", lstrlenA("]")) == 0x0) {
				if (error_offset != -1) {
					LPSTR message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9);
					StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + 0x7, (STRSAFE_LPCSTR)(snl_code + error_offset));
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + 0x7, (STRSAFE_LPCSTR)" : erreur lexicale ( line ");
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
					vector->push_back(std::make_pair(message, (BYTE)0x0));
					error_offset = -1;
				}
				vector->push_back(std::make_pair("[ + ] ] : fin de condition", (BYTE)0x1));
				offset = 0x1;
			}
			else if (strncmp(snl_code + i, ",", lstrlenA(",")) == 0x0) {
				if (error_offset != -1) {
					LPSTR message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9);
					StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + 0x7, (STRSAFE_LPCSTR)(snl_code + error_offset));
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + 0x7, (STRSAFE_LPCSTR)" : erreur lexicale ( line ");
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
					vector->push_back(std::make_pair(message, (BYTE)0x0));
					error_offset = -1;
				}
				vector->push_back(std::make_pair("[ + ] , : separateur", (BYTE)0x1));
				offset = 0x1;
			}
			else if (strncmp(snl_code + i, "(", lstrlenA("(")) == 0x0) {
				if (error_offset != -1) {
					LPSTR message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9);
					StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + 0x7, (STRSAFE_LPCSTR)(snl_code + error_offset));
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + 0x7, (STRSAFE_LPCSTR)" : erreur lexicale ( line ");
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
					vector->push_back(std::make_pair(message, (BYTE)0x0));
					error_offset = -1;
				}
				vector->push_back(std::make_pair("[ + ] ( : parenthese ouvrante", (BYTE)0x1));
				offset = 0x1;
			}
			else if (strncmp(snl_code + i, ")", lstrlenA(")")) == 0x0) {
				if (error_offset != -1) {
					LPSTR message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9);
					StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + 0x7, (STRSAFE_LPCSTR)(snl_code + error_offset));
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + 0x7, (STRSAFE_LPCSTR)" : erreur lexicale ( line ");
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
					StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
					vector->push_back(std::make_pair(message, (BYTE)0x0));
					error_offset = -1;
				}
				vector->push_back(std::make_pair("[ + ] ) : parenthese fermante", (BYTE)0x1));
				offset = 0x1;
			}
			else {
				DWORD symbol_size = 0x0;
				if (Is_Comment(snl_code + i, &symbol_size)) {
					if (error_offset != -1) {
						LPSTR message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9);
						StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
						StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + 0x7, (STRSAFE_LPCSTR)(snl_code + error_offset));
						StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + 0x7, (STRSAFE_LPCSTR)" : erreur lexicale ( line ");
						StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
						StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
						vector->push_back(std::make_pair(message, (BYTE)0x0));
						error_offset = -1;
					}
					LPSTR message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, symbol_size + lstrlenA(" : un commentaire") + 0x7);
					StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ + ] ");
					StringCchCatA((STRSAFE_LPSTR)message, symbol_size + 0x7, (STRSAFE_LPCSTR)(snl_code + i));
					StringCchCatA((STRSAFE_LPSTR)message, symbol_size + lstrlenA(" : un commentaire") + 0x7, (STRSAFE_LPCSTR)" : un commentaire");
					vector->push_back(std::make_pair(message, (BYTE)0x1));
				}
				else if (Is_String(snl_code + i, &symbol_size)) {
					if (error_offset != -1) {
						LPSTR message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9);
						StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
						StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + 0x7, (STRSAFE_LPCSTR)(snl_code + error_offset));
						StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + 0x7, (STRSAFE_LPCSTR)" : erreur lexicale ( line ");
						StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
						StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
						vector->push_back(std::make_pair(message, (BYTE)0x0));
						error_offset = -1;
					}
					LPSTR message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, symbol_size + lstrlenA(" : une chaine de characteres") + 0x7);
					StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ + ] ");
					StringCchCatA((STRSAFE_LPSTR)message, symbol_size + 0x7, (STRSAFE_LPCSTR)(snl_code + i));
					StringCchCatA((STRSAFE_LPSTR)message, symbol_size + lstrlenA(" : une chaine de characteres") + 0x7, (STRSAFE_LPCSTR)" : une chaine de characteres");
					vector->push_back(std::make_pair(message, (BYTE)0x1));
				}
				else if (Is_ID(snl_code + i, &symbol_size)) {
					if (error_offset != -1) {
						LPSTR message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9);
						StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
						StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + 0x7, (STRSAFE_LPCSTR)(snl_code + error_offset));
						StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + 0x7, (STRSAFE_LPCSTR)" : erreur lexicale ( line ");
						StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
						StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
						vector->push_back(std::make_pair(message, (BYTE)0x0));
						error_offset = -1;
					}
					LPSTR message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, symbol_size + lstrlenA(" : un identificateur") + 0x7);
					StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ + ] ");
					StringCchCatA((STRSAFE_LPSTR)message, symbol_size + 0x7, (STRSAFE_LPCSTR)(snl_code + i));
					StringCchCatA((STRSAFE_LPSTR)message, symbol_size + lstrlenA(" : un identificateur") + 0x7, (STRSAFE_LPCSTR)" : un identificateur");
					vector->push_back(std::make_pair(message, (BYTE)0x1));
				}
				else if (Is_CmpOpeartor(snl_code + i, &symbol_size)) {
					if (error_offset != -1) {
						LPSTR message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9);
						StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
						StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + 0x7, (STRSAFE_LPCSTR)(snl_code + error_offset));
						StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + 0x7, (STRSAFE_LPCSTR)" : erreur lexicale ( line ");
						StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
						StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
						vector->push_back(std::make_pair(message, (BYTE)0x0));
						error_offset = -1;
					}
					LPSTR message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, symbol_size + lstrlenA(" : un operateur de comparaison") + 0x7);
					StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ + ] ");
					StringCchCatA((STRSAFE_LPSTR)message, symbol_size + 0x7, (STRSAFE_LPCSTR)(snl_code + i));
					StringCchCatA((STRSAFE_LPSTR)message, symbol_size + lstrlenA(" : un operateur de comparaison") + 0x7, (STRSAFE_LPCSTR)" : un operateur de comparaison");
					vector->push_back(std::make_pair(message, (BYTE)0x1));
				}
				else if (Is_ArtOperator(snl_code + i)) {
					if (error_offset != -1) {
						LPSTR message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9);
						StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
						StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + 0x7, (STRSAFE_LPCSTR)(snl_code + error_offset));
						StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + 0x7, (STRSAFE_LPCSTR)" : erreur lexicale ( line ");
						StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
						StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
						vector->push_back(std::make_pair(message, (BYTE)0x0));
						error_offset = -1;
					}
					symbol_size = 0x1;
					LPSTR message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, symbol_size + lstrlenA(" : un operateur arithmetique") + 0x7);
					StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ + ] ");
					StringCchCatA((STRSAFE_LPSTR)message, symbol_size + 0x7, (STRSAFE_LPCSTR)(snl_code + i));
					StringCchCatA((STRSAFE_LPSTR)message, symbol_size + lstrlenA(" : un operateur arithmetique") + 0x7, (STRSAFE_LPCSTR)" : un operateur arithmetique");
					vector->push_back(std::make_pair(message, (BYTE)0x1));
				}
				else if (Is_FloatNumber(snl_code + i, &symbol_size)) {
					if (error_offset != -1) {
						LPSTR message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9);
						StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
						StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + 0x7, (STRSAFE_LPCSTR)(snl_code + error_offset));
						StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + 0x7, (STRSAFE_LPCSTR)" : erreur lexicale ( line ");
						StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
						StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
						vector->push_back(std::make_pair(message, (BYTE)0x0));
						error_offset = -1;
					}
					LPSTR message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, symbol_size + lstrlenA(" : nombre r饬") + 0x7);
					StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ + ] ");
					StringCchCatA((STRSAFE_LPSTR)message, symbol_size + 0x7, (STRSAFE_LPCSTR)(snl_code + i));
					StringCchCatA((STRSAFE_LPSTR)message, symbol_size + lstrlenA(" : nombre r饬") + 0x7, (STRSAFE_LPCSTR)" : nombre r饬");
					vector->push_back(std::make_pair(message, (BYTE)0x1));
				}
				else if (Is_IntegerNumber(snl_code + i, &symbol_size)) {
					if (error_offset != -1) {
						LPSTR message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9);
						StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ - ] ");
						StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + 0x7, (STRSAFE_LPCSTR)(snl_code + error_offset));
						StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + 0x7, (STRSAFE_LPCSTR)" : erreur lexicale ( line ");
						StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x7, (STRSAFE_LPCSTR)ln_str);
						StringCchCatA((STRSAFE_LPSTR)message, i - error_offset + lstrlenA(" : erreur lexicale ( line ") + lstrlenA(ln_str) + 0x9, (STRSAFE_LPCSTR)" )");
						vector->push_back(std::make_pair(message, (BYTE)0x0));
						error_offset = -1;
					}
					LPSTR message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, symbol_size + lstrlenA(" : nombre enteir") + 0x7);
					StringCchCopyA((STRSAFE_LPSTR)message, 0x7, (STRSAFE_LPCSTR)"[ + ] ");
					StringCchCatA((STRSAFE_LPSTR)message, symbol_size + 0x7, (STRSAFE_LPCSTR)(snl_code + i));
					StringCchCatA((STRSAFE_LPSTR)message, symbol_size + lstrlenA(" : nombre entier") + 0x7, (STRSAFE_LPCSTR)" : nombre entier");
					vector->push_back(std::make_pair(message, (BYTE)0x1));
				}
				else {
					if (error_offset == -1) {
						error_offset = i;
						errors_count++;
					}
					symbol_size = 0x1;
				}
				offset = symbol_size;
			}
		}
	}
	return errors_count;
}
ULONG __stdcall  AnalyseSemantique(LPSTR snl_code, std::vector<std::pair<LPCSTR, BYTE> >* vector, std::vector<SNAIL_VAR> declared_vars) {
	DWORD code_len = lstrlenA(snl_code);
	if (code_len == 0x0) {
		vector->push_back(std::make_pair("no instructions !!", (BYTE)0x1));
		return 0x0;
	}
	ULONG errors_count = 0x0;
	UINT i = 0x0;
	UINT line_number = 0x1;
	char ln_str[20];
	_itoa_s(line_number, ln_str, 10);
	while (i <= code_len) {
		LPSTR line_buffer = (LPSTR)0x0;
		DWORD line_len = 0x0;
		DWORD offset = ReadLine(snl_code + i, &line_buffer, &line_len, (LPDWORD)0x0);
		if (line_buffer && line_len > 0) {
			if(strncmp(line_buffer, "Set", 0x3) == 0x0) {
				WORD operand_offset;
				DWORD comment_size = 0x0;
				for (operand_offset = 0x0; *(line_buffer + 0x3 + operand_offset) == ' ' || *(line_buffer + 0x3 + operand_offset) == '\t'; operand_offset++);
				while (Is_Comment(line_buffer + 0x3 + operand_offset, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x3 + operand_offset) == ' ' || *(line_buffer + 0x3 + operand_offset) == '\t'; operand_offset++);
				DWORD id_size = 0x0;
				Is_ID(line_buffer + 0x3 + operand_offset, &id_size);
				BOOL is_declared = FALSE;
				WORD variable_type = VAR_TYPE_UNKNOWN;
				UINT index;
				for (index = 0x0; index < declared_vars.size(); index++) {
					if (line_number > declared_vars.at(index).line && lstrlenA(declared_vars.at(index).name) == id_size && strncmp(declared_vars.at(index).name, line_buffer + 0x3 + operand_offset, id_size) == 0x0) {
						is_declared = TRUE;
						variable_type = declared_vars.at(index).type;
						break;
					}
				}
				if (!is_declared) {
					LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,lstrlenA("[ - ] Undeclared variable { ") + id_size + lstrlenA(" } ( line ") + lstrlenA(ln_str) + 0x3);
					StringCchCopyA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] Undeclared variable { ") + 0x1, (STRSAFE_LPCSTR)"[ - ] Undeclared variable { ");
					StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] Undeclared variable { ") + id_size + 0x1, (STRSAFE_LPCSTR)(line_buffer + 0x3 + operand_offset));
					StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] Undeclared variable { ") + id_size + lstrlenA(" } ( line ( ") + 0x1, (STRSAFE_LPCSTR)" } ( line ");
					StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] Undeclared variable { ") + id_size + lstrlenA(" } ( line ( ") + lstrlenA(ln_str) + 0x1, (STRSAFE_LPCSTR)ln_str);
					StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] Undeclared variable { ") + id_size + lstrlenA(" } ( line ( ") + lstrlenA(ln_str) + 0x3, (STRSAFE_LPCSTR)" )");
					vector->push_back(std::make_pair(error_message, (BYTE)0x0));
					errors_count++;
				}
				else {
					for (operand_offset += id_size; *(line_buffer + 0x3 + operand_offset) == ' ' || *(line_buffer + 0x3 + operand_offset) == '\t'; operand_offset++);
					while (Is_Comment(line_buffer + 0x3 + operand_offset, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x3 + operand_offset) == ' ' || *(line_buffer + 0x3 + operand_offset) == '\t'; operand_offset++);
					DWORD operand_size = 0x0;
					if (variable_type == VAR_TYPE_STRING && !Is_String(line_buffer + 0x3 + operand_offset, &operand_size)) {
						if (!Is_FloatNumber(line_buffer + 0x3 + operand_offset, &operand_size)) Is_IntegerNumber(line_buffer + 0x3 + operand_offset, &operand_size);
						LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lstrlenA("[ - ] type mismatch : { ") + operand_size + lstrlenA(" } is not a valid Snt_St value ( line ") + lstrlenA(ln_str) + 0x3);
						StringCchCopyA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] type mismatch : { ") + 0x1, (STRSAFE_LPCSTR)"[ - ] type mismatch : { ");
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] type mismatch : { ") + operand_size + 0x1, (STRSAFE_LPCSTR)(line_buffer + 0x3 + operand_offset));
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] type mismatch : { ") + operand_size + lstrlenA(" } is not a valid Snt_St value ( line ( ") + 0x1, (STRSAFE_LPCSTR)" } is not a valid Snt_St value ( line ");
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] type mismatch : { ") + operand_size + lstrlenA(" } is not a valid Snt_St value ( line ( ") + lstrlenA(ln_str) + 0x1, (STRSAFE_LPCSTR)ln_str);
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] type mismatch : { ") + operand_size + lstrlenA(" } is not a valid Snt_St value ( line ( ") + lstrlenA(ln_str) + 0x3, (STRSAFE_LPCSTR)" )");
						vector->push_back(std::make_pair(error_message, (BYTE)0x0));
						errors_count++;
					}
					else if (variable_type == VAR_TYPE_REAL && !Is_FloatNumber(line_buffer + 0x3 + operand_offset, &operand_size)) {
						if (!Is_String(line_buffer + 0x3 + operand_offset, &operand_size)) Is_IntegerNumber(line_buffer + 0x3 + operand_offset, &operand_size);
						LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lstrlenA("[ - ] type mismatch : { ") + operand_size + lstrlenA(" } is not a valid Snl_Real value ( line ") + lstrlenA(ln_str) + 0x3);
						StringCchCopyA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] type mismatch : { ") + 0x1, (STRSAFE_LPCSTR)"[ - ] type mismatch : { ");
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] type mismatch : { ") + operand_size + 0x1, (STRSAFE_LPCSTR)(line_buffer + 0x3 + operand_offset));
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] type mismatch : { ") + operand_size + lstrlenA(" } is not a valid Snl_Real value ( line ( ") + 0x1, (STRSAFE_LPCSTR)" } is not a valid Snl_Real value ( line ");
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] type mismatch : { ") + operand_size + lstrlenA(" } is not a valid Snl_Real value ( line ( ") + lstrlenA(ln_str) + 0x1, (STRSAFE_LPCSTR)ln_str);
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] type mismatch : { ") + operand_size + lstrlenA(" } is not a valid Snl_Real value ( line ( ") + lstrlenA(ln_str) + 0x3, (STRSAFE_LPCSTR)" )");
						vector->push_back(std::make_pair(error_message, (BYTE)0x0));
						errors_count++;
					}
					else if (variable_type == VAR_TYPE_INTEGER) {
						BOOL is_valid = TRUE;
						if (!Is_FloatNumber(line_buffer + 0x3 + operand_offset, &operand_size)) {
							if (!Is_IntegerNumber(line_buffer + 0x3 + operand_offset, &operand_size)) {
								Is_String(line_buffer + 0x3 + operand_offset, &operand_size);
								is_valid = FALSE;
							}
						}
						else is_valid = FALSE;
						if (!is_valid) {
							LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lstrlenA("[ - ] type mismatch : { ") + operand_size + lstrlenA(" } is not a valid Snl_Int value ( line ") + lstrlenA(ln_str) + 0x3);
							StringCchCopyA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] type mismatch : { ") + 0x1, (STRSAFE_LPCSTR)"[ - ] type mismatch : { ");
							StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] type mismatch : { ") + operand_size + 0x1, (STRSAFE_LPCSTR)(line_buffer + 0x3 + operand_offset));
							StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] type mismatch : { ") + operand_size + lstrlenA(" } is not a valid Snl_Int value ( line ( ") + 0x1, (STRSAFE_LPCSTR)" } is not a valid Snl_Int value ( line ");
							StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] type mismatch : { ") + operand_size + lstrlenA(" } is not a valid Snl_Int value ( line ( ") + lstrlenA(ln_str) + 0x1, (STRSAFE_LPCSTR)ln_str);
							StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] type mismatch : { ") + operand_size + lstrlenA(" } is not a valid Snl_Int value ( line ( ") + lstrlenA(ln_str) + 0x3, (STRSAFE_LPCSTR)" )");
							vector->push_back(std::make_pair(error_message, (BYTE)0x0));
							errors_count++;
						}
						else {
							declared_vars.at(index).value = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, operand_size + 0x1);
							StringCchCopyA((STRSAFE_LPSTR)declared_vars.at(index).value, operand_size + 0x1, (STRSAFE_LPCSTR)(line_buffer + 0x3 + operand_offset));
						}
					}
					else {
						declared_vars.at(index).value = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, operand_size + 0x1);
						StringCchCopyA((STRSAFE_LPSTR)declared_vars.at(index).value, operand_size + 0x1, (STRSAFE_LPCSTR)(line_buffer + 0x3 + operand_offset));
					}
				}
			}
			else if (strncmp(line_buffer, "Get", 0x3) == 0x0) {
				WORD first_operand_offset;
				DWORD comment_size = 0x0;
				for (first_operand_offset = 0x0; *(line_buffer + 0x3 + first_operand_offset) == ' ' || *(line_buffer + 0x3 + first_operand_offset) == '\t'; first_operand_offset++);
				while (Is_Comment(line_buffer + 0x3 + first_operand_offset, &comment_size)) for (first_operand_offset += comment_size; *(line_buffer + 0x3 + first_operand_offset) == ' ' || *(line_buffer + 0x3 + first_operand_offset) == '\t'; first_operand_offset++);
				DWORD first_id_size = 0x0;
				Is_ID(line_buffer + 0x3 + first_operand_offset, &first_id_size);
				BOOL is_declared = FALSE;
				std::pair<WORD, WORD> var_type = std::make_pair(VAR_TYPE_UNKNOWN, VAR_TYPE_UNKNOWN);
				UINT index_1;
				for (index_1 = 0x0; index_1 < declared_vars.size(); index_1++) {
					if (line_number > declared_vars.at(index_1).line && lstrlenA(declared_vars.at(index_1).name) == first_id_size && strncmp(declared_vars.at(index_1).name, line_buffer + 0x3 + first_operand_offset, first_id_size) == 0x0) {
						is_declared = TRUE;
						var_type.first = declared_vars.at(index_1).type;
						break;
					}
				}
				if (!is_declared) {
					LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lstrlenA("[ - ] Undeclared variable { ") + first_id_size + lstrlenA(" } ( line ") + lstrlenA(ln_str) + 0x3);
					StringCchCopyA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] Undeclared variable { ") + 0x1, (STRSAFE_LPCSTR)"[ - ] Undeclared variable { ");
					StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] Undeclared variable { ") + first_id_size + 0x1, (STRSAFE_LPCSTR)(line_buffer + 0x3 + first_operand_offset));
					StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] Undeclared variable { ") + first_id_size + lstrlenA(" } ( line ( ") + 0x1, (STRSAFE_LPCSTR)" } ( line ");
					StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] Undeclared variable { ") + first_id_size + lstrlenA(" } ( line ( ") + lstrlenA(ln_str) + 0x1, (STRSAFE_LPCSTR)ln_str);
					StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] Undeclared variable { ") + first_id_size + lstrlenA(" } ( line ( ") + lstrlenA(ln_str) + 0x3, (STRSAFE_LPCSTR)" )");
					vector->push_back(std::make_pair(error_message, (BYTE)0x0));
					errors_count++;
				}
				else {
					is_declared = FALSE;
					WORD second_operand_offset;
					for (second_operand_offset = (first_operand_offset + first_id_size); *(line_buffer + 0x3 + second_operand_offset) == ' ' || *(line_buffer + 0x3 + second_operand_offset) == '\t'; second_operand_offset++);
					while (Is_Comment(line_buffer + 0x3 + second_operand_offset, &comment_size)) for (second_operand_offset += comment_size; *(line_buffer + 0x3 + second_operand_offset) == ' ' || *(line_buffer + 0x3 + second_operand_offset) == '\t'; second_operand_offset++);
					for (second_operand_offset += 0x4; *(line_buffer + 0x3 + second_operand_offset) == ' ' || *(line_buffer + 0x3 + second_operand_offset) == '\t'; second_operand_offset++);
					while (Is_Comment(line_buffer + 0x3 + second_operand_offset, &comment_size)) for (second_operand_offset += comment_size; *(line_buffer + 0x3 + second_operand_offset) == ' ' || *(line_buffer + 0x3 + second_operand_offset) == '\t'; second_operand_offset++);
					DWORD second_id_size = 0x0;
					Is_ID(line_buffer + 0x3 + second_operand_offset, &second_id_size);
					UINT index_2;
					for (index_2 = 0x0; index_2 < declared_vars.size(); index_2++) {
						if (line_number > declared_vars.at(index_2).line && lstrlenA(declared_vars.at(index_2).name) == second_id_size && strncmp(declared_vars.at(index_2).name, line_buffer + 0x3 + second_operand_offset, second_id_size) == 0x0) {
							is_declared = TRUE;
							var_type.second = declared_vars.at(index_2).type;
							break;
						}
					}
					if (!is_declared) {
						LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lstrlenA("[ - ] Undeclared variable { ") + second_id_size + lstrlenA(" } ( line ") + lstrlenA(ln_str) + 0x3);
						StringCchCopyA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] Undeclared variable { ") + 0x1, (STRSAFE_LPCSTR)"[ - ] Undeclared variable { ");
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] Undeclared variable { ") + second_id_size + 0x1, (STRSAFE_LPCSTR)(line_buffer + 0x3 + second_operand_offset));
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] Undeclared variable { ") + second_id_size + lstrlenA(" } ( line ( ") + 0x1, (STRSAFE_LPCSTR)" } ( line ");
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] Undeclared variable { ") + second_id_size + lstrlenA(" } ( line ( ") + lstrlenA(ln_str) + 0x1, (STRSAFE_LPCSTR)ln_str);
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] Undeclared variable { ") + second_id_size + lstrlenA(" } ( line ( ") + lstrlenA(ln_str) + 0x3, (STRSAFE_LPCSTR)" )");
						vector->push_back(std::make_pair(error_message, (BYTE)0x0));
						errors_count++;
					}
					else {
						LPSTR s_var_type = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 0x9);
						switch (var_type.first) {
							case VAR_TYPE_INTEGER: StringCchCopyA((STRSAFE_LPSTR)s_var_type, 0x9, (STRSAFE_LPCSTR)"Snl_Int"); break;
							case VAR_TYPE_REAL: StringCchCopyA((STRSAFE_LPSTR)s_var_type, 0x9, (STRSAFE_LPCSTR)"Snl_Real"); break;
							case VAR_TYPE_STRING: StringCchCopyA((STRSAFE_LPSTR)s_var_type, 0x9, (STRSAFE_LPCSTR)"Snl_St"); break;
						}
						if (var_type.first != var_type.second) {
							LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lstrlenA("[ - ] type mismatch : { ") + second_id_size + lstrlenA(" } is not a valid ") + lstrlenA(s_var_type) + lstrlenA(" variable ( line ") + lstrlenA(ln_str) + 0x3);
							StringCchCopyA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] type mismatch : { ") + 0x1, (STRSAFE_LPCSTR)"[ - ] type mismatch : { ");
							StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] type mismatch : { ") + second_id_size + 0x1, (STRSAFE_LPCSTR)(line_buffer + 0x3 + second_operand_offset));
							StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] type mismatch : { ") + second_id_size + lstrlenA(" } is not a valid ") + 0x1, (STRSAFE_LPCSTR)" } is not a valid ");
							StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] type mismatch : { ") + second_id_size + lstrlenA(" } is not a valid ") + lstrlenA(s_var_type) + 0x1, (STRSAFE_LPCSTR)s_var_type);
							StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] type mismatch : { ") + second_id_size + lstrlenA(" } is not a valid ") + lstrlenA(s_var_type) + lstrlenA(" variable ( line ") + 0x1, (STRSAFE_LPCSTR)" variable ( line ");
							StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] type mismatch : { ") + second_id_size + lstrlenA(" } is not a valid ") + lstrlenA(s_var_type) + lstrlenA(" variable ( line ") + lstrlenA(ln_str) + 0x1, (STRSAFE_LPCSTR)ln_str);
							StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] type mismatch : { ") + second_id_size + lstrlenA(" } is not a valid ") + lstrlenA(s_var_type) + lstrlenA(" variable ( line ") + lstrlenA(ln_str) + 0x3, (STRSAFE_LPCSTR)" )");
							vector->push_back(std::make_pair(error_message, (BYTE)0x0));
							errors_count++;
						}
						else if(declared_vars.at(index_2).value == (LPSTR)0x0) {
							LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lstrlenA("[ - ] { ") + second_id_size + lstrlenA(" } is not assigned any value ") + lstrlenA(" ( line ") + lstrlenA(ln_str) + 0x3);
							StringCchCopyA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + 0x1, (STRSAFE_LPCSTR)"[ - ] { ");
							StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + second_id_size + 0x1, (STRSAFE_LPCSTR)(line_buffer + 0x3 + second_operand_offset));
							StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + second_id_size + lstrlenA(" } is not assigned any value") + 0x1, (STRSAFE_LPCSTR)" } is not assigned any value");
							StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + second_id_size + lstrlenA(" } is not assigned any value") + lstrlenA(" ( line ") + 0x1, (STRSAFE_LPCSTR)" ( line ");
							StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + second_id_size + lstrlenA(" } is not assigned any value") + lstrlenA(" ( line ") + lstrlenA(ln_str) + 0x1, (STRSAFE_LPCSTR)ln_str);
							StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + second_id_size + lstrlenA(" } is not assigned any value") + lstrlenA(" ( line ") + lstrlenA(ln_str) + 0x3, (STRSAFE_LPCSTR)" )");
							vector->push_back(std::make_pair(error_message, (BYTE)0x0));
							errors_count++;
						}
						else {
							declared_vars.at(index_1).value = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lstrlenA(declared_vars.at(index_2).value) + 0x1);
							StringCchCopyA((STRSAFE_LPSTR)declared_vars.at(index_1).value, lstrlenA(declared_vars.at(index_2).value) + 0x1, (STRSAFE_LPCSTR)declared_vars.at(index_2).value);
						}
					}
				}
			}
			else if (strncmp(line_buffer, "If", 0x2) == 0x0) {
				WORD var_offset;
				DWORD comment_size = 0x0;
				for (var_offset = 0x0; *(line_buffer + 0x2 + var_offset) == ' ' || *(line_buffer + 0x2 + var_offset) == '\t'; var_offset++);
				while (Is_Comment(line_buffer + 0x2 + var_offset, &comment_size)) for (var_offset += comment_size; *(line_buffer + 0x2 + var_offset) == ' ' || *(line_buffer + 0x2 + var_offset) == '\t'; var_offset++);
				for (++var_offset; *(line_buffer + 0x2 + var_offset) == ' ' || *(line_buffer + 0x2 + var_offset) == '\t'; var_offset++);
				while (Is_Comment(line_buffer + 0x2 + var_offset, &comment_size)) for (var_offset += comment_size; *(line_buffer + 0x2 + var_offset) == ' ' || *(line_buffer + 0x2 + var_offset) == '\t'; var_offset++);
				DWORD symbol_size = 0x0;
				Is_ID(line_buffer + 0x2 + var_offset, &symbol_size);
				BOOL is_declared = FALSE;
				std::pair<WORD, WORD> var_type = std::make_pair(VAR_TYPE_UNKNOWN, VAR_TYPE_UNKNOWN);
				UINT index;
				for (index = 0x0; index < declared_vars.size(); index++) if (line_number > declared_vars.at(index).line && lstrlenA(declared_vars.at(index).name) == symbol_size && strncmp(declared_vars.at(index).name, line_buffer + 0x2 + var_offset, symbol_size) == 0x0) {
					is_declared = TRUE;
					var_type.first = declared_vars.at(index).type;
					break;
				}
				if (!is_declared) {
					LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lstrlenA("[ - ] Undeclared variable { ") + symbol_size + lstrlenA(" } ( line ") + lstrlenA(ln_str) + 0x3);
					StringCchCopyA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] Undeclared variable { ") + 0x1, (STRSAFE_LPCSTR)"[ - ] Undeclared variable { ");
					StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] Undeclared variable { ") + symbol_size + 0x1, (STRSAFE_LPCSTR)(line_buffer + 0x2 + var_offset));
					StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] Undeclared variable { ") + symbol_size + lstrlenA(" } ( line ( ") + 0x1, (STRSAFE_LPCSTR)" } ( line ");
					StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] Undeclared variable { ") + symbol_size + lstrlenA(" } ( line ( ") + lstrlenA(ln_str) + 0x1, (STRSAFE_LPCSTR)ln_str);
					StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] Undeclared variable { ") + symbol_size + lstrlenA(" } ( line ( ") + lstrlenA(ln_str) + 0x3, (STRSAFE_LPCSTR)" )");
					vector->push_back(std::make_pair(error_message, (BYTE)0x0));
					errors_count++;
				}
				else {
					if (declared_vars.at(index).value == (LPSTR)0x0) {
						LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lstrlenA("[ - ] { ") + symbol_size + lstrlenA(" } is not assigned any value ") + lstrlenA(" ( line ") + lstrlenA(ln_str) + 0x3);
						StringCchCopyA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + 0x1, (STRSAFE_LPCSTR)"[ - ] { ");
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + symbol_size + 0x1, (STRSAFE_LPCSTR)(line_buffer + 0x2 + var_offset));
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + symbol_size + lstrlenA(" } is not assigned any value") + 0x1, (STRSAFE_LPCSTR)" } is not assigned any value");
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + symbol_size + lstrlenA(" } is not assigned any value") + lstrlenA(" ( line ") + 0x1, (STRSAFE_LPCSTR)" ( line ");
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + symbol_size + lstrlenA(" } is not assigned any value") + lstrlenA(" ( line ") + lstrlenA(ln_str) + 0x1, (STRSAFE_LPCSTR)ln_str);
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + symbol_size + lstrlenA(" } is not assigned any value") + lstrlenA(" ( line ") + lstrlenA(ln_str) + 0x3, (STRSAFE_LPCSTR)" )");
						vector->push_back(std::make_pair(error_message, (BYTE)0x0));
						errors_count++;
					}
					else {
						for (var_offset += symbol_size; *(line_buffer + 0x2 + var_offset) == ' ' || *(line_buffer + 0x2 + var_offset) == '\t'; var_offset++);
						while (Is_Comment(line_buffer + 0x2 + var_offset, &comment_size)) for (var_offset += comment_size; *(line_buffer + 0x2 + var_offset) == ' ' || *(line_buffer + 0x2 + var_offset) == '\t'; var_offset++);
						DWORD operator_size = 0x0;
						BOOL _continue = TRUE;
						Is_CmpOpeartor(line_buffer + 0x2 + var_offset, &operator_size);
						if (var_type.first == VAR_TYPE_STRING) {
							if (!(operator_size == 0x2 && (strncmp(line_buffer + 0x2 + var_offset, "==", 0x2) == 0x0 || strncmp(line_buffer + 0x2 + var_offset, "!=", 0x2) == 0x0))) {
								LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lstrlenA("[ - ] invalid compraison operator : { ") + operator_size + lstrlenA(" } is not valid for strings ( line ") + lstrlenA(ln_str) + 0x3);
								StringCchCopyA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] invalid compraison operator : { ") + 0x1, (STRSAFE_LPCSTR)"[ - ] invalid compraison operator : { ");
								StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] invalid compraison operator : { ") + operator_size + 0x1, (STRSAFE_LPCSTR)(line_buffer + 0x2 + var_offset));
								StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] invalid compraison operator : { ") + operator_size + lstrlenA(" } is not valid for strings ( line ( ") + 0x1, (STRSAFE_LPCSTR)" } is not valid for strings ( line ");
								StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] invalid compraison operator : { ") + operator_size + lstrlenA(" } is not valid for strings ( line ( ") + lstrlenA(ln_str) + 0x1, (STRSAFE_LPCSTR)ln_str);
								StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] invalid compraison operator : { ") + operator_size + lstrlenA(" } is not valid for strings ( line ( ") + lstrlenA(ln_str) + 0x3, (STRSAFE_LPCSTR)" )");
								vector->push_back(std::make_pair(error_message, (BYTE)0x0));
								errors_count++;
								_continue = FALSE;
							}

						}
						if (_continue) {
							for (var_offset += operator_size; *(line_buffer + 0x2 + var_offset) == ' ' || *(line_buffer + 0x2 + var_offset) == '\t'; var_offset++);
							while (Is_Comment(line_buffer + 0x2 + var_offset, &comment_size)) for (var_offset += comment_size; *(line_buffer + 0x2 + var_offset) == ' ' || *(line_buffer + 0x2 + var_offset) == '\t'; var_offset++);
							_continue = TRUE;
							if (Is_ID(line_buffer + 0x2 + var_offset, &symbol_size)) {
								is_declared = FALSE;
								for (index = 0x0; index < declared_vars.size(); index++) if (lstrlenA(declared_vars.at(index).name) == symbol_size && strncmp(declared_vars.at(index).name, line_buffer + 0x2 + var_offset, symbol_size) == 0x0) {
									is_declared = TRUE;
									var_type.second = declared_vars.at(index).type;
									break;
								}
								if (!is_declared) {
									LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lstrlenA("[ - ] Undeclared variable { ") + symbol_size + lstrlenA(" } ( line ") + lstrlenA(ln_str) + 0x3);
									StringCchCopyA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] Undeclared variable { ") + 0x1, (STRSAFE_LPCSTR)"[ - ] Undeclared variable { ");
									StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] Undeclared variable { ") + symbol_size + 0x1, (STRSAFE_LPCSTR)(line_buffer + 0x2 + var_offset));
									StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] Undeclared variable { ") + symbol_size + lstrlenA(" } ( line ( ") + 0x1, (STRSAFE_LPCSTR)" } ( line ");
									StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] Undeclared variable { ") + symbol_size + lstrlenA(" } ( line ( ") + lstrlenA(ln_str) + 0x1, (STRSAFE_LPCSTR)ln_str);
									StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] Undeclared variable { ") + symbol_size + lstrlenA(" } ( line ( ") + lstrlenA(ln_str) + 0x3, (STRSAFE_LPCSTR)" )");
									vector->push_back(std::make_pair(error_message, (BYTE)0x0));
									errors_count++;
									_continue = FALSE;
								}
								else if (declared_vars.at(index).value == (LPSTR)0x0) {
									LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lstrlenA("[ - ] { ") + symbol_size + lstrlenA(" } is not assigned any value ") + lstrlenA(" ( line ") + lstrlenA(ln_str) + 0x3);
									StringCchCopyA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + 0x1, (STRSAFE_LPCSTR)"[ - ] { ");
									StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + symbol_size + 0x1, (STRSAFE_LPCSTR)(line_buffer + 0x2 + var_offset));
									StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + symbol_size + lstrlenA(" } is not assigned any value") + 0x1, (STRSAFE_LPCSTR)" } is not assigned any value");
									StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + symbol_size + lstrlenA(" } is not assigned any value") + lstrlenA(" ( line ") + 0x1, (STRSAFE_LPCSTR)" ( line ");
									StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + symbol_size + lstrlenA(" } is not assigned any value") + lstrlenA(" ( line ") + lstrlenA(ln_str) + 0x1, (STRSAFE_LPCSTR)ln_str);
									StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + symbol_size + lstrlenA(" } is not assigned any value") + lstrlenA(" ( line ") + lstrlenA(ln_str) + 0x3, (STRSAFE_LPCSTR)" )");
									vector->push_back(std::make_pair(error_message, (BYTE)0x0));
									errors_count++;
								}
							}
							else if (Is_String(line_buffer + 0x2 + var_offset, &symbol_size)) var_type.second = VAR_TYPE_STRING;
							else if (Is_FloatNumber(line_buffer + 0x2 + var_offset, &symbol_size)) var_type.second = VAR_TYPE_REAL;
							else if (Is_IntegerNumber(line_buffer + 0x2 + var_offset, &symbol_size)) var_type.second = VAR_TYPE_INTEGER;
							if (_continue) {
								if (var_type.first != var_type.second) {
									LPSTR s_var_type = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 0x9);
									switch (var_type.first) {
									case VAR_TYPE_INTEGER: StringCchCopyA((STRSAFE_LPSTR)s_var_type, 0x9, (STRSAFE_LPCSTR)"Snl_Int"); break;
									case VAR_TYPE_REAL: StringCchCopyA((STRSAFE_LPSTR)s_var_type, 0x9, (STRSAFE_LPCSTR)"Snl_Real"); break;
									case VAR_TYPE_STRING: StringCchCopyA((STRSAFE_LPSTR)s_var_type, 0x9, (STRSAFE_LPCSTR)"Snl_St"); break;
									}
									LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lstrlenA("[ - ] type mismatch : { ") + symbol_size + lstrlenA(" } is not a valid ") + lstrlenA(s_var_type) + lstrlenA(" variable/value ( line ") + lstrlenA(ln_str) + 0x3);
									StringCchCopyA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] type mismatch : { ") + 0x1, (STRSAFE_LPCSTR)"[ - ] type mismatch : { ");
									StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] type mismatch : { ") + symbol_size + 0x1, (STRSAFE_LPCSTR)(line_buffer + 0x2 + var_offset));
									StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] type mismatch : { ") + symbol_size + lstrlenA(" } is not a valid ") + 0x1, (STRSAFE_LPCSTR)" } is not a valid ");
									StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] type mismatch : { ") + symbol_size + lstrlenA(" } is not a valid ") + lstrlenA(s_var_type) + 0x1, (STRSAFE_LPCSTR)s_var_type);
									StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] type mismatch : { ") + symbol_size + lstrlenA(" } is not a valid ") + lstrlenA(s_var_type) + lstrlenA(" variable/value ( line ") + 0x1, (STRSAFE_LPCSTR)" variable/value ( line ");
									StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] type mismatch : { ") + symbol_size + lstrlenA(" } is not a valid ") + lstrlenA(s_var_type) + lstrlenA(" variable/value ( line ") + lstrlenA(ln_str) + 0x1, (STRSAFE_LPCSTR)ln_str);
									StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] type mismatch : { ") + symbol_size + lstrlenA(" } is not a valid ") + lstrlenA(s_var_type) + lstrlenA(" variable/value ( line ") + lstrlenA(ln_str) + 0x3, (STRSAFE_LPCSTR)" )");
									vector->push_back(std::make_pair(error_message, (BYTE)0x0));
									errors_count++;
								}
							}
						}
					}
				}
			}
			else if (strncmp(line_buffer, "Snl_Put", 0x7) == 0x0) {
				WORD operand_offset;
				DWORD comment_size = 0x0;
				for (operand_offset = 0x0; *(line_buffer + 0x7 + operand_offset) == ' ' || *(line_buffer + 0x7 + operand_offset) == '\t'; operand_offset++);
				while (Is_Comment(line_buffer + 0x7 + operand_offset, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x7 + operand_offset) == ' ' || *(line_buffer + 0x7 + operand_offset) == '\t'; operand_offset++);
				DWORD symbol_size = 0x0;
				BOOL is_id = FALSE;
				if (Is_ID(line_buffer + operand_offset + 0x7, &symbol_size)) {
					is_id = TRUE;
					BOOL is_declared = FALSE;
					UINT index;
					for (index = 0x0; index < declared_vars.size(); index++) if (line_number > declared_vars.at(index).line && lstrlenA(declared_vars.at(index).name) == symbol_size && strncmp(declared_vars.at(index).name, line_buffer + 0x7 + operand_offset, symbol_size) == 0x0) {
						is_declared = TRUE;
						break;
					}
					if (!is_declared) {
						LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lstrlenA("[ - ] Undeclared variable { ") + symbol_size + lstrlenA(" } ( line ") + lstrlenA(ln_str) + 0x3);
						StringCchCopyA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] Undeclared variable { ") + 0x1, (STRSAFE_LPCSTR)"[ - ] Undeclared variable { ");
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] Undeclared variable { ") + symbol_size + 0x1, (STRSAFE_LPCSTR)(line_buffer + 0x7 + operand_offset));
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] Undeclared variable { ") + symbol_size + lstrlenA(" } ( line ( ") + 0x1, (STRSAFE_LPCSTR)" } ( line ");
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] Undeclared variable { ") + symbol_size + lstrlenA(" } ( line ( ") + lstrlenA(ln_str) + 0x1, (STRSAFE_LPCSTR)ln_str);
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] Undeclared variable { ") + symbol_size + lstrlenA(" } ( line ( ") + lstrlenA(ln_str) + 0x3, (STRSAFE_LPCSTR)" )");
						vector->push_back(std::make_pair(error_message, (BYTE)0x0));
						errors_count++;
					}
					else if (declared_vars.at(index).value == (LPSTR)0x0) {
						LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lstrlenA("[ - ] { ") + symbol_size + lstrlenA(" } is not assigned any value ") + lstrlenA(" ( line ") + lstrlenA(ln_str) + 0x3);
						StringCchCopyA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + 0x1, (STRSAFE_LPCSTR)"[ - ] { ");
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + symbol_size + 0x1, (STRSAFE_LPCSTR)(line_buffer + 0x7 + operand_offset));
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + symbol_size + lstrlenA(" } is not assigned any value") + 0x1, (STRSAFE_LPCSTR)" } is not assigned any value");
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + symbol_size + lstrlenA(" } is not assigned any value") + lstrlenA(" ( line ") + 0x1, (STRSAFE_LPCSTR)" ( line ");
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + symbol_size + lstrlenA(" } is not assigned any value") + lstrlenA(" ( line ") + lstrlenA(ln_str) + 0x1, (STRSAFE_LPCSTR)ln_str);
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + symbol_size + lstrlenA(" } is not assigned any value") + lstrlenA(" ( line ") + lstrlenA(ln_str) + 0x3, (STRSAFE_LPCSTR)" )");
						vector->push_back(std::make_pair(error_message, (BYTE)0x0));
						errors_count++;
					}
				}
			}
			else if(strncmp(line_buffer, "Snl_Int", 0x7) == 0x0) {
				WORD operand_offset;
				DWORD comment_size = 0x0;
				for (operand_offset = 0x0; *(line_buffer + 0x7 + operand_offset) == ' ' || *(line_buffer + 0x7 + operand_offset) == '\t'; operand_offset++);
				while (Is_Comment(line_buffer + 0x7 + operand_offset, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x7 + operand_offset) == ' ' || *(line_buffer + 0x7 + operand_offset) == '\t'; operand_offset++);
				DWORD id_size = 0x0;
				DWORD id_index = 0x0;
				do {
					while (Is_Comment(line_buffer + 0x7 + operand_offset, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x7 + operand_offset) == ' ' || *(line_buffer + 0x7 + operand_offset) == '\t'; operand_offset++);
					Is_ID(line_buffer + 0x7 + operand_offset, (LPDWORD)&id_size);
					BOOL ipd = FALSE;
					UINT i;
					for (i = 0x0; i < declared_vars.size(); i++) {
						if (lstrlenA(declared_vars.at(i).name) == id_size && strncmp(declared_vars.at(i).name, line_buffer + 0x7 + operand_offset, id_size) == 0x0 && (declared_vars.at(i).line < line_number || (declared_vars.at(i).line == line_number && declared_vars.at(i).line_position < id_index))) {
							ipd = TRUE;
							break;
						}
					}
					if (ipd) {
						char line_str[20];
						_itoa_s(declared_vars.at(i).line, line_str, 10);
						LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lstrlenA("[ - ] { ") + id_size + lstrlenA(" } is previously declared in line < ") + lstrlenA(line_str) + lstrlenA(" > ( line ") + lstrlenA(ln_str) + 0x3);
						StringCchCopyA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + 0x1, (STRSAFE_LPCSTR)"[ - ] { ");
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + id_size + 0x1, (STRSAFE_LPCSTR)(line_buffer + 0x7 + operand_offset));
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + id_size + lstrlenA(" } is previously declared in line < ") + 0x1, (STRSAFE_LPCSTR)" } is previously declared in line < ");
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + id_size + lstrlenA(" } is previously declared in line < ") + lstrlenA(line_str) + 0x1, (STRSAFE_LPCSTR)line_str);
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + id_size + lstrlenA(" } is previously declared in line < ") + lstrlenA(line_str) + lstrlenA(" > ( line ") + 0x1, (STRSAFE_LPCSTR)" > ( line ");
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + id_size + lstrlenA(" } is previously declared in line < ") + lstrlenA(line_str) + lstrlenA(" > ( line ") + lstrlenA(ln_str) +  0x1, (STRSAFE_LPCSTR)ln_str);
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + id_size + lstrlenA(" } is previously declared in line < ") + lstrlenA(line_str) + lstrlenA(" > ( line ") + lstrlenA(ln_str) + 0x3, (STRSAFE_LPCSTR)" )");
						vector->push_back(std::make_pair(error_message, (BYTE)0x0));
						errors_count++;
					}
					operand_offset += id_size;
					if (*(line_buffer + 0x7 + operand_offset) == '\0') break;
					for (; *(line_buffer + 0x7 + operand_offset) == ' ' || *(line_buffer + 0x7 + operand_offset) == '\t'; operand_offset++);
					while (Is_Comment(line_buffer + 0x7 + operand_offset, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x7 + operand_offset) == ' ' || *(line_buffer + 0x7 + operand_offset) == '\t'; operand_offset++);
					for (operand_offset++; *(line_buffer + 0x7 + operand_offset) == ' ' || *(line_buffer + 0x7 + operand_offset) == '\t'; operand_offset++);
					while (Is_Comment(line_buffer + 0x7 + operand_offset, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x7 + operand_offset) == ' ' || *(line_buffer + 0x7 + operand_offset) == '\t'; operand_offset++);
					id_index++;
				} while (!(*(line_buffer + 0x7 + operand_offset) == '%' && *(line_buffer + 0x8 + operand_offset) == '.') && *(line_buffer + 0x7 + operand_offset) != '\0');
			}
			else if (strncmp(line_buffer, "Snl_Real", 0x8) == 0x0) {
				WORD operand_offset;
				DWORD comment_size = 0x0;
				for (operand_offset = 0x0; *(line_buffer + 0x8 + operand_offset) == ' ' || *(line_buffer + 0x8 + operand_offset) == '\t'; operand_offset++);
				while (Is_Comment(line_buffer + 0x8 + operand_offset, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x8 + operand_offset) == ' ' || *(line_buffer + 0x8 + operand_offset) == '\t'; operand_offset++);
				DWORD id_size = 0x0;
				DWORD id_index = 0x0;
				do {
					while (Is_Comment(line_buffer + 0x8 + operand_offset, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x8 + operand_offset) == ' ' || *(line_buffer + 0x8 + operand_offset) == '\t'; operand_offset++);
					Is_ID(line_buffer + 0x8 + operand_offset, (LPDWORD)&id_size);
					BOOL ipd = FALSE;
					UINT i;
					for (i = 0x0; i < declared_vars.size(); i++) {
						if (lstrlenA(declared_vars.at(i).name) == id_size && strncmp(declared_vars.at(i).name, line_buffer + 0x8 + operand_offset, id_size) == 0x0 && (declared_vars.at(i).line < line_number || (declared_vars.at(i).line == line_number && declared_vars.at(i).line_position < id_index))) {
							ipd = TRUE;
							break;
						}
					}
					if (ipd) {
						char line_str[20];
						_itoa_s(declared_vars.at(i).line, line_str, 10);
						LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lstrlenA("[ - ] { ") + id_size + lstrlenA(" } is previously declared in line < ") + lstrlenA(line_str) + lstrlenA(" > ( line ") + lstrlenA(ln_str) + 0x3);
						StringCchCopyA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + 0x1, (STRSAFE_LPCSTR)"[ - ] { ");
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + id_size + 0x1, (STRSAFE_LPCSTR)(line_buffer + 0x8 + operand_offset));
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + id_size + lstrlenA(" } is previously declared in line < ") + 0x1, (STRSAFE_LPCSTR)" } is previously declared in line < ");
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + id_size + lstrlenA(" } is previously declared in line < ") + lstrlenA(line_str) + 0x1, (STRSAFE_LPCSTR)line_str);
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + id_size + lstrlenA(" } is previously declared in line < ") + lstrlenA(line_str) + lstrlenA(" > ( line ") + 0x1, (STRSAFE_LPCSTR)" > ( line ");
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + id_size + lstrlenA(" } is previously declared in line < ") + lstrlenA(line_str) + lstrlenA(" > ( line ") + lstrlenA(ln_str) + 0x1, (STRSAFE_LPCSTR)ln_str);
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + id_size + lstrlenA(" } is previously declared in line < ") + lstrlenA(line_str) + lstrlenA(" > ( line ") + lstrlenA(ln_str) + 0x3, (STRSAFE_LPCSTR)" )");
						vector->push_back(std::make_pair(error_message, (BYTE)0x0));
						errors_count++;
					}
					operand_offset += id_size;
					if (*(line_buffer + 0x8 + operand_offset) == '\0') break;
					for (; *(line_buffer + 0x8 + operand_offset) == ' ' || *(line_buffer + 0x8 + operand_offset) == '\t'; operand_offset++);
					while (Is_Comment(line_buffer + 0x8 + operand_offset, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x8 + operand_offset) == ' ' || *(line_buffer + 0x8 + operand_offset) == '\t'; operand_offset++);
					for (operand_offset++; *(line_buffer + 0x8 + operand_offset) == ' ' || *(line_buffer + 0x8 + operand_offset) == '\t'; operand_offset++);
					while (Is_Comment(line_buffer + 0x8 + operand_offset, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x8 + operand_offset) == ' ' || *(line_buffer + 0x8 + operand_offset) == '\t'; operand_offset++);
					id_index++;
				} while (!(*(line_buffer + 0x8 + operand_offset) == '%' && *(line_buffer + 0x9 + operand_offset) == '.') && *(line_buffer + 0x8 + operand_offset) != '\0');
			}
			else if (strncmp(line_buffer, "Snl_St", 0x6) == 0x0) {
				WORD operand_offset;
				DWORD comment_size = 0x0;
				for (operand_offset = 0x0; *(line_buffer + 0x6 + operand_offset) == ' ' || *(line_buffer + 0x6 + operand_offset) == '\t'; operand_offset++);
				while (Is_Comment(line_buffer + 0x6 + operand_offset, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x6 + operand_offset) == ' ' || *(line_buffer + 0x6 + operand_offset) == '\t'; operand_offset++);
				DWORD id_size = 0x0;
				DWORD id_index = 0x0;
				do {
					while (Is_Comment(line_buffer + 0x6 + operand_offset, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x6 + operand_offset) == ' ' || *(line_buffer + 0x6 + operand_offset) == '\t'; operand_offset++);
					Is_ID(line_buffer + 0x6 + operand_offset, (LPDWORD)&id_size);
					BOOL ipd = FALSE;
					UINT i;
					for (i = 0x0; i < declared_vars.size(); i++) {
						if (lstrlenA(declared_vars.at(i).name) == id_size && strncmp(declared_vars.at(i).name, line_buffer + 0x6 + operand_offset, id_size) == 0x0 && (declared_vars.at(i).line < line_number || (declared_vars.at(i).line == line_number && declared_vars.at(i).line_position < id_index))) {
							ipd = TRUE;
							break;
						}
					}
					if (ipd) {
						char line_str[20];
						_itoa_s(declared_vars.at(i).line, line_str, 10);
						LPSTR error_message = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lstrlenA("[ - ] { ") + id_size + lstrlenA(" } is previously declared in line < ") + lstrlenA(line_str) + lstrlenA(" > ( line ") + lstrlenA(ln_str) + 0x3);
						StringCchCopyA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + 0x1, (STRSAFE_LPCSTR)"[ - ] { ");
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + id_size + 0x1, (STRSAFE_LPCSTR)(line_buffer + 0x6 + operand_offset));
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + id_size + lstrlenA(" } is previously declared in line < ") + 0x1, (STRSAFE_LPCSTR)" } is previously declared in line < ");
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + id_size + lstrlenA(" } is previously declared in line < ") + lstrlenA(line_str) + 0x1, (STRSAFE_LPCSTR)line_str);
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + id_size + lstrlenA(" } is previously declared in line < ") + lstrlenA(line_str) + lstrlenA(" > ( line ") + 0x1, (STRSAFE_LPCSTR)" > ( line ");
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + id_size + lstrlenA(" } is previously declared in line < ") + lstrlenA(line_str) + lstrlenA(" > ( line ") + lstrlenA(ln_str) + 0x1, (STRSAFE_LPCSTR)ln_str);
						StringCchCatA((STRSAFE_LPSTR)error_message, lstrlenA("[ - ] { ") + id_size + lstrlenA(" } is previously declared in line < ") + lstrlenA(line_str) + lstrlenA(" > ( line ") + lstrlenA(ln_str) + 0x3, (STRSAFE_LPCSTR)" )");
						vector->push_back(std::make_pair(error_message, (BYTE)0x0));
						errors_count++;
					}
					operand_offset += id_size;
					if (*(line_buffer + 0x6 + operand_offset) == '\0') break;
					for (; *(line_buffer + 0x6 + operand_offset) == ' ' || *(line_buffer + 0x6 + operand_offset) == '\t'; operand_offset++);
					while (Is_Comment(line_buffer + 0x6 + operand_offset, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x6 + operand_offset) == ' ' || *(line_buffer + 0x6 + operand_offset) == '\t'; operand_offset++);
					for (operand_offset++; *(line_buffer + 0x6 + operand_offset) == ' ' || *(line_buffer + 0x6 + operand_offset) == '\t'; operand_offset++);
					while (Is_Comment(line_buffer + 0x6 + operand_offset, &comment_size)) for (operand_offset += comment_size; *(line_buffer + 0x6 + operand_offset) == ' ' || *(line_buffer + 0x6 + operand_offset) == '\t'; operand_offset++);
					id_index++;
				} while (!(*(line_buffer + 0x6 + operand_offset) == '%' && *(line_buffer + 0x7 + operand_offset) == '.') && *(line_buffer + 0x6 + operand_offset) != '\0');
			}
			HeapFree(GetProcessHeap(), 0x0, line_buffer);
		}
		line_number++;
		_itoa_s(line_number, ln_str, 10);
		i += offset;
	}
	if (errors_count == 0x0) vector->push_back(std::make_pair("[ + ] no errors ", (BYTE)0x1));
	return errors_count;
}

BOOL __stdcall Is_Comment(LPSTR char_sequence, LPDWORD symbol_size) {
	if (char_sequence[0] != '%') return FALSE;
	UINT i;
	for (i = 0x1; i < UINT_MAX && char_sequence[i] != '%' && char_sequence[i] != '\0' && char_sequence[i] != '\r' && char_sequence[i] != '.'; i++);
	if (char_sequence[i] != '%') {
		*symbol_size = 0x0;
		return FALSE;
	}
	if (symbol_size) *symbol_size = i + 1;
	return TRUE;
}
BOOL __stdcall Is_String(LPSTR char_sequence, LPDWORD symbol_size) {
	if (char_sequence[0] != '"') return FALSE;
	UINT i;
	for (i = 0x1; i < UINT_MAX && char_sequence[i] != '"' && char_sequence[i] != '\0' && char_sequence[i] != '\r'; i++);
	if (char_sequence[i] != '"') {
		*symbol_size = 0x0;
		return FALSE;
	}
	if (symbol_size) *symbol_size = i + 1;
	return TRUE;
}
BOOL __stdcall Is_ID(LPSTR char_sequence, LPDWORD symbol_size) {
	UINT i;
	BOOL is_valid = FALSE;
	for (int _i = 65; _i <= 90; _i++) if (char_sequence[0] == (char)_i || char_sequence[0] == (char)(_i + 32)) {
		is_valid = TRUE;
		break;
	}
	if (!is_valid) return FALSE;
	for (i = 0x1; i < UINT_MAX && char_sequence[i] != '\0' && char_sequence[i] != '\r'; i++) {
		is_valid = FALSE;
		for (int j = 48; j <= 57; j++) if (char_sequence[i] == (char)j) is_valid = TRUE;
		for (int j = 65; j <= 90; j++) if (char_sequence[i] == (char)j || char_sequence[i] == (char)(j + 32)) is_valid = TRUE;
		if (char_sequence[i] == '_') is_valid = TRUE;
		if (!is_valid) break;
	}
	if (char_sequence[i - 1] == '_') if (symbol_size) *symbol_size = i - 1;
	else if (symbol_size) *symbol_size = i;
	if ((*symbol_size == lstrlenA("Snl_Begin") && strncmp(char_sequence, "Snl_Begin", *symbol_size) == 0x0) ||
		(*symbol_size == lstrlenA("Snl_Int") && strncmp(char_sequence, "Snl_Int", *symbol_size) == 0x0) ||
		(*symbol_size == lstrlenA("Snl_Real") && strncmp(char_sequence, "Snl_Real", *symbol_size) == 0x0) ||
		(*symbol_size == lstrlenA("Snl_Put") && strncmp(char_sequence, "Snl_Put", *symbol_size) == 0x0) ||
		(*symbol_size == lstrlenA("Snl_End") && strncmp(char_sequence, "Snl_End", *symbol_size) == 0x0) ||
		(*symbol_size == lstrlenA("Set") && strncmp(char_sequence, "Set", *symbol_size) == 0x0) ||
		(*symbol_size == lstrlenA("Get") && strncmp(char_sequence, "Get", *symbol_size) == 0x0) ||
		(*symbol_size == lstrlenA("from") && strncmp(char_sequence, "from", *symbol_size) == 0x0) ||
		(*symbol_size == lstrlenA("If") && strncmp(char_sequence, "If", *symbol_size) == 0x0) ||
		(*symbol_size == lstrlenA("Else") && strncmp(char_sequence, "Else", *symbol_size) == 0x0) ||
		(*symbol_size == lstrlenA("Snl_St") && strncmp(char_sequence, "Snl_St", *symbol_size) == 0x0)) {
		*symbol_size = 0x0;
		return FALSE;
	}
	if (symbol_size) *symbol_size = i;
	return TRUE;
}
BOOL __stdcall Is_IntegerNumber(LPSTR char_sequence, LPDWORD symbol_size) {
	UINT i;
	for (i = 0; i < UINT_MAX && char_sequence[i] != '\0' && char_sequence[i] != '\r'; i++) if (char_sequence[i] != '0' && char_sequence[i] != '1' && char_sequence[i] != '2' && char_sequence[i] != '3' && char_sequence[i] != '4' && char_sequence[i] != '5' && char_sequence[i] != '6' && char_sequence[i] != '7' && char_sequence[i] != '8' && char_sequence[i] != '9') break;
	if (i == 0x0) {
		*symbol_size = 0x0;
		return FALSE;
	}
	if (symbol_size) *symbol_size = i;
	return TRUE;
}
BOOL __stdcall Is_FloatNumber(LPSTR char_sequence, LPDWORD symbol_size) {
	if (char_sequence[0] == '.') {
		*symbol_size = 0x0;
		return FALSE;
	}
	UINT i, j;
	for (i = 0; i < UINT_MAX && char_sequence[i] != '\0' && char_sequence[i] != '\r'; i++) if (char_sequence[i] != '0' && char_sequence[i] != '1' && char_sequence[i] != '2' && char_sequence[i] != '3' && char_sequence[i] != '4' && char_sequence[i] != '5' && char_sequence[i] != '6' && char_sequence[i] != '7' && char_sequence[i] != '8' && char_sequence[i] != '9') break;
	if (char_sequence[i] != '.') {
		*symbol_size = 0x0;
		return FALSE;
	}
	for (j = i + 1; j < UINT_MAX && char_sequence[i] != '\0' && char_sequence[i] != '\r'; j++) if (char_sequence[j] != '0' && char_sequence[j] != '1' && char_sequence[j] != '2' && char_sequence[j] != '3' && char_sequence[j] != '4' && char_sequence[j] != '5' && char_sequence[j] != '6' && char_sequence[j] != '7' && char_sequence[j] != '8' && char_sequence[j] != '9') break;
	if (char_sequence[j - 1] == '.') {
		*symbol_size = 0x0;
		return FALSE;
	}
	if (symbol_size) *symbol_size = j;
	return TRUE;
}
BOOL __stdcall Is_CmpOpeartor(LPSTR char_sequence, LPDWORD symbol_size) {
	if (char_sequence[0] != '>' && char_sequence[0] != '<' && char_sequence[0] != '=' && char_sequence[0] != '!') return FALSE;
	if (char_sequence[0] == '=' || char_sequence[0] == '!') {
		if (char_sequence[1] == '=') {
			if (symbol_size) *symbol_size = 0x2;
			return TRUE;
		}
		*symbol_size = 0x0;
		return FALSE;
	}
	if (char_sequence[1] == '=') {
		if (symbol_size) *symbol_size = 0x2;
		return TRUE;
	}
	if (symbol_size) *symbol_size = 0x1;
	return TRUE;
}
BOOL __stdcall Is_ArtOperator(LPSTR char_sequence) {
	if (char_sequence[0] != '+' && char_sequence[0] != '-' && char_sequence[0] != '*' && char_sequence[0] != '/' && char_sequence[0] != '=') return FALSE;
	return TRUE;
}