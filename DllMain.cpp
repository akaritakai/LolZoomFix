#include <windows.h>
#include <tchar.h>
#include <string>
#include <ddraw.h>
#include <Psapi.h>

HMODULE hDDRAW = nullptr;

typedef HRESULT (WINAPI *DirectDrawCreate_t) (GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter);
DirectDrawCreate_t fpDirectDrawCreate = nullptr;
HRESULT WINAPI DirectDrawCreate(GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter)
{
	return fpDirectDrawCreate(lpGUID, lplpDD, pUnkOuter);
}

typedef HRESULT (WINAPI *DirectDrawCreateEx_t) (GUID FAR * lpGuid, LPVOID *lplpDD, REFIID iid, IUnknown FAR *pUnkOuter);
DirectDrawCreateEx_t fpDirectDrawCreateEx = nullptr;
HRESULT WINAPI DirectDrawCreateEx(GUID FAR * lpGuid, LPVOID  *lplpDD, REFIID  iid, IUnknown FAR *pUnkOuter)
{
	return fpDirectDrawCreateEx(lpGuid, lplpDD, iid, pUnkOuter);
}

VOID LoadDDRAW()
{
	TCHAR dllPath[MAX_PATH];
	GetSystemDirectory(dllPath, MAX_PATH);
	_tcscat_s(dllPath, MAX_PATH, _T("\\ddraw.dll"));
	hDDRAW = LoadLibrary(dllPath);

	fpDirectDrawCreate = reinterpret_cast<DirectDrawCreate_t>(GetProcAddress(hDDRAW, "DirectDrawCreate"));
	fpDirectDrawCreateEx = reinterpret_cast<DirectDrawCreateEx_t>(GetProcAddress(hDDRAW, "DirectDrawCreateEx"));
}

VOID ExecuteHack()
{
	LoadDDRAW();

	/* Get the base address */
	MODULEINFO mInfo;
	HMODULE hModule = GetModuleHandle(nullptr);
	GetModuleInformation(GetCurrentProcess(), hModule, &mInfo, sizeof(MODULEINFO));
	LPVOID base = mInfo.lpBaseOfDll;

	/* Set the values to change and where to change them */
	byte camValue[] = { 0x00, 0x40, 0x9c, 0x45 };
	auto addressToWrite = reinterpret_cast<DWORD>(base) + 0x0127D1FC;

	/* Do the memory change */
	DWORD oldProtection;
	VirtualProtect(reinterpret_cast<LPVOID>(addressToWrite), 4, PAGE_EXECUTE_READWRITE, &oldProtection);
	memcpy(reinterpret_cast<LPVOID>(addressToWrite), camValue, 4);
	VirtualProtect(reinterpret_cast<LPVOID>(addressToWrite), 4, oldProtection, nullptr);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
		CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(ExecuteHack), nullptr, 0, nullptr);
	return TRUE;
}
