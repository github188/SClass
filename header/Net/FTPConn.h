#ifndef _SM_NET_FTPCONN
#define _SM_NET_FTPCONN
#include "Data/DateTime.h"
#include "IO/Stream.h"
#include "IO/StreamWriter.h"
#include "Net/TCPClient.h"
#include "Net/SocketFactory.h"

namespace Net
{
	class FTPConn
	{
	private:
		Net::TCPClient *cli;
		IO::StreamWriter *writer;
		Bool threadToStop;
		Bool threadRunning;
		Int32 codePage;
		Bool logged;
		Bool statusChg;
		Int32 lastStatus;
		UTF8Char *msgRet;
		Sync::Event *evt;

		static UInt32 __stdcall FTPThread(void *userObj);
		Int32 WaitForResult();
	public:
		FTPConn(const UTF8Char *host, UInt16 port, Net::SocketFactory *sockf, Int32 codePage);
		~FTPConn();

		Bool IsLogged();

		Bool SendUser(const UTF8Char *userName);
		Bool SendPassword(const UTF8Char *password);
		Bool ChangeDirectory(const UTF8Char *dir);
		Bool MakeDirectory(const UTF8Char *dir);
		Bool RemoveDirectory(const UTF8Char *dir);
		Bool GetFileSize(const UTF8Char *fileName, Int64 *fileSize);
		Bool GetFileModTime(const UTF8Char *fileName, Data::DateTime *modTime);
		Bool ToBinaryType();
		Bool ToASCIIType();
		Bool ToEBCDICType();
		Bool ChangePassiveMode(Int32 *ip, UInt16 *port);
		Bool ChangeActiveMode(Int32 ip, UInt16 port);
		Bool ResumeTransferPos(Int64 pos);
		Bool GetFile(const UTF8Char *fileName);
		Bool RenameFile(const UTF8Char *fromFile, const UTF8Char *toFile);
	};
};
#endif