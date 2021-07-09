#include <Windows.h>
#include <stdio.h>

#include "NtDirect.h"
#include "HotPatch.h"

// #define NAKED  __declspec(naked)
NtDirect* ntd_NtProtectVirtualMemory = nullptr;

// NOTE: These use __cdecl because we don't want to have to adjust the stack coming back from this.
typedef NTSTATUS(__cdecl* pNtProtectVirtualMemory)(HANDLE ProcessHandle, PVOID* BaseAddress, PSIZE_T NumberOfBytesToProtect, ULONG NewAccessProtection, PULONG OldAccessProtection);


static NTSTATUS NTAPI hk_NtProtectVirtualMemory(HANDLE ProcessHandle, PVOID* BaseAddress, PSIZE_T NumberOfBytesToProtect, ULONG NewAccessProtection, PULONG OldAccessProtection) {
	OutputDebugStringA("In Our Hooked NtProtectVirtualMemory");
	pNtProtectVirtualMemory ofn = reinterpret_cast<pNtProtectVirtualMemory>(ntd_NtProtectVirtualMemory->ptr);
	return ofn(ProcessHandle, BaseAddress, NumberOfBytesToProtect, NewAccessProtection, OldAccessProtection);

}




int main() {
	ntd_NtProtectVirtualMemory = new NtDirect(0, "NtProtectVirtualMemory");
	//ntd_NtProtectVirtualMemory = new NtDirect(0x50,nullptr);
	
	printf("[-] Binding NtProtectVirtualMemory via direct syscall ...\n\n");

	if (!ntd_NtProtectVirtualMemory->ptr) {
		printf("NtDirect Bind Fail!\n");
		exit(-1);
	}

	
	printf("[-] HotPatching NtProtectVirtualMemory... \n\n");
	HotPatch* hp = new HotPatch(GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtProtectVirtualMemory"));
	if (!hp->patch(hk_NtProtectVirtualMemory)) { printf("HotPatch Failed :(\n"); exit(-1); }


	printf("[-] This is purely to do something with it hooked - look at your debug output...\n\n");
	printf("[!] Ok! clean everything up and don't leave anything behind!\n\n");
	delete hp;
	delete ntd_NtProtectVirtualMemory;
	printf("---Unhooked and Cleaned Up! -- SEE YOU!---\n");
	return 0;
}