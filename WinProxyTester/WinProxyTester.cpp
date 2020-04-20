// WinProxyTester.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <Winhttp.h>

#define NONE 0x0
#define READ 0x1
#define TEST 0x2
#define WRITE 0x4

BOOL ParseCmdArgs(char* argv[], int argc, std::wstring &URL, DWORD &dwAccessType, LPCWSTR &pszProxyW, LPCWSTR &pszProxyBypassW, DWORD &dwFlags, BYTE &AppFlags);
BOOL ParseCmdArgs(char* argv[], int argc, std::wstring &URL, DWORD &dwAccessType, LPCWSTR& pszProxyW, LPCWSTR& pszProxyBypassW, DWORD &dwFlags, BYTE &AppFlags)
{
	std::string Temp = std::string(argv[1]);
	URL = std::wstring(Temp.begin(), Temp.end());

	for (size_t i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-r") == 0)
		{
			AppFlags |= READ;
		}
		else if (strcmp(argv[i], "-d") == 0)
		{
			dwAccessType = WINHTTP_ACCESS_TYPE_NO_PROXY;
			AppFlags |= TEST;
		}
		else if (strcmp(argv[i], "-h") == 0)
		{
			dwAccessType = WINHTTP_ACCESS_TYPE_DEFAULT_PROXY;
			AppFlags |= TEST;
		}
		else if (strcmp(argv[i], "-i") == 0)
		{
			dwAccessType = WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY;
			AppFlags |= TEST;
		}
		else if (strcmp(argv[i], "-n") == 0)
		{
			dwAccessType = WINHTTP_ACCESS_TYPE_NAMED_PROXY;
			AppFlags |= TEST;
			if (argc > 3)
			{
				pszProxyW = (LPCWSTR)argv[3];
			}
			else
			{
				printf("Please enter a proxy when using -n flag\n");
				exit(1);
			}
		}
		else if (strcmp(argv[i], "-w") == 0)
		{
			AppFlags |= WRITE;
		}
	}
	return TRUE;
}
int main(int argc, char *argv[])
{
	BYTE AppFlags = 0;
	DWORD dwAccessType;
	LPCWSTR pszProxyW = NULL;
	LPCWSTR pszProxyBypassW = NULL;
	DWORD dwFlags;
    DWORD dwSize = 0;
    DWORD dwDownloaded = 0;
    LPSTR pszOutBuffer = 0;
    BOOL bResults = FALSE;
    HINTERNET 	hSession = NULL,
				hConnect = NULL,
				hRequest = NULL;
	WINHTTP_CURRENT_USER_IE_PROXY_CONFIG pINETProxyConfig;
	WINHTTP_PROXY_INFO pHTTPProxyInfo;
	std::wstring URL;

	if (argc < 2)
	{
		printf("USAGE\n");
		printf("\tWinProxyTester.exe [URL]\n");
		printf("\t\t-d Go direct\n");
		printf("\t\t-i Use WININET\n");
		printf("\t\t-h Use WINHTTP\n");
		printf("\t\t-n [proxy] Use specified proxy\n");
		printf("\t\t-r Read proxy settings for current user\n");
		printf("\t\t-w Write output of HTTP Query\n");
		exit(1);
	}
	
	ParseCmdArgs(argv, argc, URL, dwAccessType, pszProxyW, pszProxyBypassW, dwFlags, AppFlags);
	if ((AppFlags & READ) == READ)
	{
		if (WinHttpGetIEProxyConfigForCurrentUser(&pINETProxyConfig))
		{
			wprintf(L"WININET Settings:\n");
			wprintf(L"Access Type: %d, Auto-Config URL: %s, Proxy String: %s, Bypass-List: %s\n", pINETProxyConfig.fAutoDetect, pINETProxyConfig.lpszAutoConfigUrl, pINETProxyConfig.lpszProxy, pINETProxyConfig.lpszProxyBypass);
		}
		if (WinHttpGetDefaultProxyConfiguration(&pHTTPProxyInfo))
		{
			wprintf(L"WinHTTP Setting:\n");
			wprintf(L"Access Type: %d, Proxy String: %s, Bypass-List: %s\n", pHTTPProxyInfo.dwAccessType, pHTTPProxyInfo.lpszProxy, pHTTPProxyInfo.lpszProxyBypass);
		}
		else
		{
			printf("Failed with %d\n", GetLastError());
			exit(1);
		}
	}
	if ((AppFlags & TEST) == TEST)
	{
		hSession = WinHttpOpen(L"WinHTTP Example/1.0",
			dwAccessType,
			pszProxyW,
			pszProxyBypassW, 0);

		if (hSession)
			hConnect = WinHttpConnect(hSession, URL.c_str(),
				INTERNET_DEFAULT_HTTPS_PORT, 0);
		if (hConnect)
			hRequest = WinHttpOpenRequest(hConnect, L"GET", NULL,
				NULL, WINHTTP_NO_REFERER,
				WINHTTP_DEFAULT_ACCEPT_TYPES,
				WINHTTP_FLAG_SECURE);
		if (hRequest)
			bResults = WinHttpSendRequest(hRequest,
				WINHTTP_NO_ADDITIONAL_HEADERS, 0,
				WINHTTP_NO_REQUEST_DATA, 0,
				0, 0);

		if (bResults)
			bResults = WinHttpReceiveResponse(hRequest, NULL);

		if (bResults && (AppFlags & WRITE) == WRITE)
		{
			do
			{
				dwSize = 0;
				if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
					printf("Error %u in WinHttpQueryDataAvailable.\n",
						GetLastError());
				pszOutBuffer = new char[dwSize + 1];
				if (!pszOutBuffer)
				{
					printf("Out of memory\n");
					dwSize = 0;
				}
				else
				{
					ZeroMemory(pszOutBuffer, dwSize + 1);
					if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer,
						dwSize, &dwDownloaded))
						printf("Error %u in WinHttpReadData.\n", GetLastError());
					else
						printf("%s", pszOutBuffer);
					delete[] pszOutBuffer;
				}
			} while (dwSize > 0);
		}

		if (!bResults)
			printf("Error %d has occurred.\n", GetLastError());
		else
			printf("Successfully reached %s\n", argv[1]); //@TODO Add a conversion for the WINERROR codes
	}
	if (hRequest) WinHttpCloseHandle(hRequest);
	if (hConnect) WinHttpCloseHandle(hConnect);
	if (hSession) WinHttpCloseHandle(hSession);
	return 0;
	
	
}
