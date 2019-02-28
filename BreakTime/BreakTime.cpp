#include "pch.h"
#include <iostream>
#include <WinSock2.h>
#include <ws2bth.h>
#include <string>
#include <initguid.h>

#pragma comment(lib, "Ws2_32.lib")

//DEFINE_GUID(g_guidServiceClass, 

void printGUID(GUID *guid)
{
	std::cout << "\t\tGUID ------------------------\n";
	std::cout << "\t\tguid.Data1: " << guid->Data1 << '\n';
	std::cout << "\t\tguid.Data2: " << guid->Data2 << '\n';
	std::cout << "\t\tguid.Data3: " << guid->Data3 << '\n';
	std::cout << "\t\tguid.Data4: " << guid->Data4 << '\n';
}

void printSAB(SOCKADDR_BTH *sab)
{
	std::cout << "\tSOCKADDR_BTH ------------------------------\n";
	std::cout << "\tsab.addressFamily: " << sab->addressFamily << '\n';
	std::cout << "\tsab.btAddr: " << sab->btAddr << '\n';
	std::cout << "\tsab.serviceClassId: \n";
	printGUID(&(sab->serviceClassId));
	std::cout << "\tsab.port: " << sab->port << '\n';
	std::cout << '\n';
}

void printBDI(BTH_DEVICE_INFO *bdi)
{
	std::cout << "\tBTH_DEVICE_INFO ----------------------\n";
	std::cout << "\tflags: " << (bdi->flags ? bdi->flags : 0)<< '\n';
	std::cout << "\taddress: " << bdi->address << '\n';
	std::cout << "\tclassOfDevice: " << bdi->classOfDevice << '\n';
	std::cout << "\tname: " << bdi->name << '\n';
}

void printSA(SOCKET_ADDRESS &sa)
{
	std::cout << "\t\tSOCKET_ADDRESS -----------------------\n";
	std::cout << "\t\tlpSockaddr: " << sa.lpSockaddr << '\n';
	std::cout << "\t\t\tsockaddr --------------------------\n";
	std::cout << "\t\t\tsa_family: " << (sa.lpSockaddr)->sa_family << '\n';
	std::cout << "\t\t\tsa_data: " << (sa.lpSockaddr)->sa_data << '\n';
	std::cout << "\t\tiSockaddrLength: " << sa.iSockaddrLength << '\n';
}

void printCSAI(CSADDR_INFO &csai)
{
	std::cout << "\tCSADDR_INFO ------------------------\n";
	std::cout << "\tLocalAddr: \n";
	printSA(csai.LocalAddr);
	printSAB((SOCKADDR_BTH*)csai.LocalAddr.lpSockaddr);
	std::cout << "\tRemoteAddr: \n";
	printSA(csai.RemoteAddr);
	printSAB((SOCKADDR_BTH*)csai.RemoteAddr.lpSockaddr);
	std::cout << "\tiSocketType: " << csai.iSocketType << '\n';
	std::cout << "\tiProtocol: " << csai.iProtocol << '\n';
}

void printQS(WSAQUERYSET &qs)
{
	std::cout << "WSAQUERYSET --------------------------\n";
	std::cout << "dwSize: " << qs.dwSize << '\n';
	std::cout << "lpszServiceInstanceName: " << qs.lpszServiceInstanceName << " : ";
	if(qs.lpszServiceInstanceName) std::cout << *(qs.lpszServiceInstanceName);
	std::cout << '\n';
	std::cout << "lpServiceClassId: " << qs.lpServiceClassId << '\n';
	if(qs.lpServiceClassId) printGUID(qs.lpServiceClassId);
	std::cout << "lpVersion: " << qs.lpVersion << '\n';
	std::cout << "lpszComment: " << qs.lpszComment << '\n';
	std::cout << "dwNameSpace: " << qs.dwNameSpace << '\n';
	std::cout << "lpNSProviderId: " << qs.lpNSProviderId << '\n';
	std::cout << "lpszContext: " << qs.lpszContext << '\n';
	std::cout << "dwNumberOfProtocols: " << qs.dwNumberOfProtocols << '\n';
	std::cout << "lpafpProtocols: " << qs.lpafpProtocols << '\n';
	std::cout << "lpszQueryString: " << qs.lpszQueryString << '\n';
	std::cout << "dwNumberofCsAddrs: " << qs.dwNumberOfCsAddrs << '\n';
	for(int i=0; i<qs.dwNumberOfCsAddrs; ++i) printCSAI(*(qs.lpcsaBuffer));
	std::cout << "dwOutputFlags: " << qs.dwOutputFlags << '\n';
	std::cout << "lpBlob: " << qs.lpBlob << '\n';
	if(qs.lpBlob && qs.lpBlob->pBlobData) printBDI((BTH_DEVICE_INFO *)qs.lpBlob->pBlobData);
	std::cout << '\n';
}

int main()
{
	// WSAStartup
	WSAData wsaData;
	if(WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
	{
		std::cout << "WSAStartup failed\n";
		return 1;
	}

	//WSALookup
	/*
	BTH_QUERY_DEVICE bqd;
	ZeroMemory(&bqd, sizeof(BTH_QUERY_DEVICE));
	bqd.LAP = 0;

	BLOB blob;
	ZeroMemory(&blob, sizeof(BLOB));
	blob.cbSize = sizeof(BTH_QUERY_DEVICE);
	blob.pBlobData = (BYTE *)&bqd;
	*/

	WSAQUERYSET wqs;
	ZeroMemory(&wqs, sizeof(WSAQUERYSET));
	wqs.dwSize = sizeof(WSAQUERYSET);
	//wqs.lpBlob = &blob;
	wqs.dwNameSpace = NS_BTH;
	printQS(wqs);

	HANDLE lookup;
	int flags = LUP_CONTAINERS | LUP_FLUSHCACHE;
	if(WSALookupServiceBegin(&wqs, flags, &lookup))
	{
		std::cout << "WSALookupServiceBegin: " << WSAGetLastError() << '\n';
		return 1;
	}
	printQS(wqs);

	flags = LUP_RETURN_TYPE | LUP_RES_SERVICE | LUP_RETURN_NAME | LUP_RETURN_ADDR | LUP_RETURN_BLOB;

	int bufSize = 0x2000;
	void* buf = malloc(bufSize);

	//SOCKADDR_BTH struct
	SOCKADDR_BTH sab;
	ZeroMemory(&sab, sizeof(SOCKADDR_BTH));
	sab.addressFamily = AF_BTH;
	sab.port = 0;

	while(1)
	{
		ZeroMemory(buf, bufSize);
		LPWSAQUERYSET rqs = (LPWSAQUERYSET)buf;
		rqs->dwSize = sizeof(WSAQUERYSET);
		rqs->dwNameSpace = NS_BTH;
		DWORD size = bufSize;

		if(WSALookupServiceNext(lookup, flags, &size, rqs))
		{
			int err = WSAGetLastError();
			if(err == WSA_E_NO_MORE)
			{
				std::cout << "NO MORE DEVICES\n\n";
				break;
			}
			else
			{
				std::cout << "WSALookupServiceNext: " << WSAGetLastError() << '\n';
				return 1;
			}
		}
		printQS(*rqs);
		
		//sab.btAddr = ((BTH_DEVICE_INFO *)(rqs->lpBlob->pBlobData))->address;
		//CopyMemory(&(sab.btAddr), , sizeof(
		//sab.serviceClassId = *(rqs->lpServiceClassId);
		//CopyMemory(&(sab.serviceClassId), 

		// socket
		SOCKET ConnectSocket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
		if(ConnectSocket == INVALID_SOCKET)
		{
			std::cout << "socket: " << WSAGetLastError() << '\n';
			return 1;
		}

		//connect
		int iResult = -1;
		iResult = connect(ConnectSocket, (sockaddr*)(rqs->lpcsaBuffer->RemoteAddr.lpSockaddr), rqs->lpcsaBuffer->RemoteAddr.iSockaddrLength);
		if(iResult)
			std::cout << "connect: " << WSAGetLastError() << '\n';
		else 
		{
			std::cout << "connect successful!\n";
			break;
		}
	}

	printSAB(&sab);

	if(buf != NULL) free(buf);
	if(lookup != NULL)
	{
		if(WSALookupServiceEnd(lookup))
		{
			std::cout << "WSALookupServiceEnd: " << WSAGetLastError() << '\n';
			return 1;
		}
		lookup = NULL;
	}

	printQS(wqs);


	//int iResult = -1;
	printSAB(&sab);
	//while(iResult)
	//{
	//}
	/*
	if(connect(ConnectSocket, (sockaddr*)&sab, sizeof(SOCKADDR_BTH)))
	{
		std::cout << "connect: " << WSAGetLastError() << '\n';
		return 1;
	}
	*/
	std::cout << "done\n";
}
