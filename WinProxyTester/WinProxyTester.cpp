// WinProxyTester.cpp : This file contains the 'main' function. Program execution begins and ends there.
// # ============================ DISCLAIMER ============================
// #
// # This Sample Code is provided for the purpose of illustration only
// #and is not intended to be used in a production environment.
// # THIS SAMPLE CODE AND ANY RELATED INFORMATION ARE PROVIDED "AS IS" WITHOUT
// # WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
// # TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND / OR FITNESS FOR A PARTICULAR
// # PURPOSE.We grant You a nonexclusive, royalty - free right to use and modify
// # the Sample Code and to reproduce and distribute the object code form of the
// # Sample Code, provided that You agree : (i)to not use Our name, logo, or
// # trademarks to market Your software product in which the Sample Code is
// # embedded; (ii)to include a valid copyright notice on Your software product
// # in which the Sample Code is embedded; and (iii)to indemnify, hold harmless,
// #and defend Usand Our suppliers fromand against any claims or lawsuits,
// # including attorneys fees, that arise or result from the use or
// # distribution of the Sample Code.
// #
// # ============================ DISCLAIMER ============================ 

#include <iostream>
#include <Windows.h>
#include <Winhttp.h>

#define NONE 0x0
#define READ 0x1
#define TRAFFIC 0x2
#define WRITE 0x4
#define DIRECT 0x8
#define HAS_URL 0x10
//@TODO Add goto for cleanups
//@TODO Add pulling WPAD files / Autoconfig scripts
BOOL ParseCmdArgs(char* argv[], int argc, std::wstring &URL, DWORD &dwAccessType, LPCWSTR &pszProxyW, LPCWSTR &pszProxyBypassW, DWORD &dwFlags, BYTE &AppFlags);
BOOL ParseCmdArgs(char* argv[], int argc, std::wstring &URL, DWORD &dwAccessType, LPCWSTR& pszProxyW, LPCWSTR& pszProxyBypassW, DWORD &dwFlags, BYTE &AppFlags)
{
	std::string Temp = std::string(argv[1]);
	URL = std::wstring(Temp.begin(), Temp.end());
	if (URL.length() > 2)
	{
		if (URL.find(L"http") == std::wstring::npos)
		{
			URL = L"http://" + URL;
		}
		AppFlags |= HAS_URL;
	}
	
	for (size_t i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-r") == 0)
			AppFlags |= READ;
		else if (strcmp(argv[i], "-d") == 0)
		{
			dwAccessType = WINHTTP_ACCESS_TYPE_NO_PROXY;
			AppFlags |= DIRECT;
		}
		else if (strcmp(argv[i], "-h") == 0)
		{
			dwAccessType = WINHTTP_ACCESS_TYPE_DEFAULT_PROXY;

		}
		else if (strcmp(argv[i], "-i") == 0)
		{
			dwAccessType = WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY;
		}
		else if (strcmp(argv[i], "-n") == 0)
		{
			dwAccessType = WINHTTP_ACCESS_TYPE_NAMED_PROXY;
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
		else if (strcmp(argv[i], "-s") == 0)
			AppFlags |= TRAFFIC;
		else if (strcmp(argv[i], "-w") == 0)
			AppFlags |= WRITE;
		
	}
	return TRUE;
}


int main(int argc, char *argv[])
{
	BYTE AppFlags = 0;
	DWORD dwAccessType;
	LPCWSTR pszProxyW = NULL;
	LPCWSTR pszProxyBypassW = NULL;
	//WINHTTP_AUTOPROXY_OPTIONS pAutoProxyOptions = { WINHTTP_AUTOPROXY_AUTO_DETECT | WINHTTP_AUTOPROXY_ALLOW_AUTOCONFIG, WINHTTP_AUTO_DETECT_TYPE_DHCP | WINHTTP_AUTO_DETECT_TYPE_DNS_A, NULL, NULL, 0, FALSE };
	WINHTTP_AUTOPROXY_OPTIONS pAutoProxyOptions;
	DWORD dwFlags;
    DWORD dwSize = 0;
    DWORD dwDownloaded = 0;
    LPSTR pszOutBuffer = 0;
    BOOL bResults = FALSE;
    HINTERNET 	hSession = NULL,
				hConnect = NULL,
				hRequest = NULL;
	WINHTTP_CURRENT_USER_IE_PROXY_CONFIG pCUProxyConfig;
	WINHTTP_PROXY_INFO pProxyInfo;
	std::wstring URL;
	URL_COMPONENTS UrlComp;
	ZeroMemory(&UrlComp, sizeof(URL_COMPONENTS));
	UrlComp.dwStructSize = sizeof(UrlComp);
	UrlComp.dwSchemeLength = (DWORD)-1;
	UrlComp.dwHostNameLength = (DWORD)-1;
	UrlComp.dwUrlPathLength = (DWORD)-1;
	UrlComp.dwExtraInfoLength = (DWORD)-1;

	

	if (argc < 2)
	{
		printf("USAGE\n");
		printf("\tWinProxyTester.exe [URL]");
		printf("\tThis will tell you where we would be sending our traffic\n");
		printf("\t\t-s Send HTTP request\n");
		printf("\t\t-d Go direct\n");
		printf("\t\t-i Use WININET\n");
		printf("\t\t-h Use WINHTTP\n");
		printf("\t\t-g Get proxy config file\n");
		printf("\t\t-n [proxy] Use specified proxy\n");
		printf("\t\t-r Read proxy settings for current user\n");
		printf("\t\t-w Write output of HTTP Query\n");
		exit(1);
	}
	
	ParseCmdArgs(argv, argc, URL, dwAccessType, pszProxyW, pszProxyBypassW, dwFlags, AppFlags);
		if (WinHttpGetIEProxyConfigForCurrentUser(&pCUProxyConfig) && WinHttpGetDefaultProxyConfiguration(&pProxyInfo))
		{
			if((AppFlags & READ ) == READ)
			{
				wprintf(L"WININET Settings:\n");
				wprintf(L"WPAD Detection: %d, Auto-Config URL: %s, Proxy String: %s, Bypass-List: %s\n", pCUProxyConfig.fAutoDetect, pCUProxyConfig.lpszAutoConfigUrl, pCUProxyConfig.lpszProxy, pCUProxyConfig.lpszProxyBypass);
				wprintf(L"WinHTTP Setting:\n");
				wprintf(L"Enabled: %d, Proxy String: %s, Bypass-List: %s\n\n", pProxyInfo.dwAccessType, pProxyInfo.lpszProxy, pProxyInfo.lpszProxyBypass);
			}
			
		}
		else
		{
			printf("Failed with %d\n", GetLastError());
			exit(1);
		}
		if ((AppFlags & HAS_URL) == HAS_URL)
		{


			hSession = WinHttpOpen(L"WinHTTP Example/1.0",
				dwAccessType,
				pszProxyW,
				pszProxyBypassW, 0);
			//Reads the current IE proxy settings and applies them to our traffic
			if ((AppFlags & DIRECT) != DIRECT)
			{
				if (pCUProxyConfig.fAutoDetect != 0 || pCUProxyConfig.lpszAutoConfigUrl != NULL && dwAccessType != WINHTTP_ACCESS_TYPE_DEFAULT_PROXY)
				{
					pAutoProxyOptions.dwAutoDetectFlags = 0;
					if (pCUProxyConfig.fAutoDetect == 1)
					{
						pAutoProxyOptions.dwAutoDetectFlags = WINHTTP_AUTO_DETECT_TYPE_DNS_A;
						pAutoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_AUTO_DETECT;
					}
					if (pCUProxyConfig.lpszAutoConfigUrl != NULL)
					{
						pAutoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_ALLOW_AUTOCONFIG;
						pAutoProxyOptions.lpszAutoConfigUrl = pCUProxyConfig.lpszAutoConfigUrl;
					}
					pAutoProxyOptions.lpvReserved = NULL;
					pAutoProxyOptions.dwReserved = 0;
					pAutoProxyOptions.fAutoLogonIfChallenged = FALSE;
					if (!WinHttpGetProxyForUrl(hSession, URL.c_str(), &pAutoProxyOptions, &pProxyInfo))
					{
						printf("GetProxy for URL failed with %d\n", GetLastError());
						exit(1);
					}
					if (pProxyInfo.dwAccessType == 1)
					{
						wprintf(L"Using proxy information: Using proxy: NONE (%d), Proxy String: %s, Bypass-List: %s\n", pProxyInfo.dwAccessType, pProxyInfo.lpszProxy, pProxyInfo.lpszProxyBypass);
					}
					else
					{
						wprintf(L"Using proxy information: Using proxy: TRUE(%d), Proxy String: %s, Bypass-List: %s\n", pProxyInfo.dwAccessType, pProxyInfo.lpszProxy, pProxyInfo.lpszProxyBypass);
					}
				}
				else
				{
					if(dwAccessType == WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY)
						wprintf(L"Using proxy: %s with Bypass-List: %s\n", pCUProxyConfig.lpszProxy, pCUProxyConfig.lpszProxyBypass);
					else
						wprintf(L"Using proxy: %s with Bypass-List: %s\n", pProxyInfo.lpszProxy, pProxyInfo.lpszProxyBypass);
				}
			}
			else
				printf("Going direct\n");

			if ((AppFlags & TRAFFIC) == TRAFFIC)
			{

				if (!WinHttpCrackUrl((LPCWSTR)URL.c_str(), (DWORD)wcslen(URL.c_str()), 0,  &UrlComp))
				{
					printf("WinHttpCrackUrl failed with: %d\n", GetLastError());
					exit(1);
				}
				if (hSession)
					hConnect = WinHttpConnect(hSession, UrlComp.lpszHostName,
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
		}
	return 0;
	
	
}
