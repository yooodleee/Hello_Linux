#include <Windows.h>
#include <stdio.h>
#include <string.h>

#define BUF_SIZE 10

int main(int argc, char* argv[]) {
	HANDLE hRead, hWrite;

	if (argc != 2) {
		printf("Usage: %s string\n", argv[0]);
		return 1;
	}

	SECURITY_ATTRIBUTES saAttr;

	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	/* generate a pipe */
	if (!CreatePipe(&hRead, &hWrite, &saAttr, 0)) {
		fprintf(stderr, "CreatePipe failed\n");
		return 1;
	}

	if (!SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT)) {
		fprintf(stderr, "SetHandleInformation failed\n");
		return 1;
	}

	/* set STARTUPINFO */
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizzeof(si);

	si.hStdInput = hRead;
	si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
	si.dwFlags = STARTF_USESTDHANDLES;

	ZeroMemory(&pi, sizeof(pi));

	if (!CreatProcess(
		NULL,
		"cmd.exe /C type CON",
		NULL,
		NULL,
		TRUE,		/* handle inherit */
		0,
		NULL,
		NULL,
		&si,
		&pi
	)) {
		fprintf(stderr, "CreateProcess failed\n");
		return 1;
	}

	CloseHandle(hRead);

	DWORD bytesWritten;
	WriteFile(hWrite, argv[1], strlen(argv[1]), &bytesWritten, NULL);

	CloseHandle(hWrite);

	WaitForSingleObject(pi.hProcess, INFINITE);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	return 0;
}