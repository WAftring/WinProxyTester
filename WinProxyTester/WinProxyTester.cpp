// WinProxyTester.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <Winhttp.h>

LPCWSTR g_URL;
DWORD g_dwAccessType;
LPCWSTR g_pszProxyW;
LPCWSTR g_pszProxyBypassW;
DWORD g_dwFlags;

BOOL ParseCmdArgs(char* argv[]);
BOOL ParseCmdArgs(char* argv[])
{

}
int main(int argc, char *argv[])
{
	
    DWORD dwSize = 0;
    DWORD dwDownloaded = 0;
    LPSTR pszOutBuffer = 0;
    BOOL bResults = FALSE;
    HINTERNET 	hSession = NULL,
				hConnect = NULL,
				hRequest = NULL;
	if (argc < 2)
	{
		printf("Here we are printing out all of our important info...\n");
		exit(1);
	}


    hSession = WinHttpOpen(L"WinHTTP Example/1.0",
							WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
							WINHTTP_NO_PROXY_NAME,
							WINHTTP_NO_PROXY_BYPASS,0);
							
	if(hSession)
		hConnect = WinHttpConnect(hSession, L"www.microsoft.com",
									INTERNET_DEFAULT_HTTPS_PORT,0);
	if(hConnect)
		hRequest = WinHttpOpenRequest(hConnect, L"GET", NULL,
										NULL, WINHTTP_NO_REFERER,
										WINHTTP_DEFAULT_ACCEPT_TYPES,
										WINHTTP_FLAG_SECURE);
	if(hRequest)
		bResults = WinHttpSendRequest(hRequest,
										WINHTTP_NO_ADDITIONAL_HEADERS, 0,
										WINHTTP_NO_REQUEST_DATA, 0,
										0,0);
										
	if(bResults)
		bResults = WinHttpReceiveResponse(hRequest, NULL);
	
	if(bResults)
	{
		do
		{
			dwSize = 0;
			if(!WinHttpQueryDataAvailable(hRequest, &dwSize))
				printf("Error %u in WinHttpQueryDataAvailable.\n",
				GetLastError());
			pszOutBuffer = new char [dwSize+1];
			if(!pszOutBuffer)
			{
				printf("Out of memory\n");
				dwSize = 0;
			}
			else
			{
				ZeroMemory(pszOutBuffer, dwSize+1);
				if(!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer,
					dwSize, &dwDownloaded))
					printf("Error %u in WinHttpReadData.\n", GetLastError());
				else
					printf("%s", pszOutBuffer);
				delete [] pszOutBuffer;
			}
		} while (dwSize > 0);
	}

	if(!bResults)
		printf("Error %d has occurred.\n", GetLastError());
	
	if(hRequest) WinHttpCloseHandle(hRequest);
	if(hConnect) WinHttpCloseHandle(hConnect);
	if(hSession) WinHttpCloseHandle(hSession);
	
	
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
