#include "Stdafx.h"
#include "Net/HTTPQueue.h"
#include "Sync/MutexUsage.h"
#include "Sync/Thread.h"
#include "Text/URLString.h"

Net::HTTPQueue::HTTPQueue(Net::SocketFactory *sockf, Net::SSLEngine *ssl)
{
	this->sockf = sockf;
	this->ssl = ssl;
	NEW_CLASS(this->statusMap, Data::StringUTF8Map<DomainStatus*>());
	NEW_CLASS(this->statusMut, Sync::Mutex());
	NEW_CLASS(this->statusEvt, Sync::Event(true, (const UTF8Char*)"Net.HTTPQueue.statusEvt"));
}

Net::HTTPQueue::~HTTPQueue()
{
	Data::ArrayList<DomainStatus*> *statusList;
	DomainStatus *status;
	UOSInt i;

	Sync::MutexUsage mutUsage(this->statusMut);
	statusList = this->statusMap->GetValues();
	i = statusList->GetCount();
	while (i-- > 0)
	{
		status = statusList->GetItem(i);
		if (status->req1)
		{
			status->req1->Close();
		}
		if (status->req2)
		{
			status->req2->Close();
		}
	}
	mutUsage.EndUse();
	while (this->statusMap->GetCount() > 0)
	{
		Sync::Thread::Sleep(10);
	}
	DEL_CLASS(this->statusMap);
	DEL_CLASS(this->statusMut);
	DEL_CLASS(this->statusEvt);
}

Net::HTTPClient *Net::HTTPQueue::MakeRequest(const UTF8Char *url, const Char *method, Bool noShutdown)
{
	UTF8Char sbuff[512];
	Text::URLString::GetURLDomain(sbuff, url, 0);
	Bool found = false;;
	DomainStatus *status;
	Net::HTTPClient *cli;
	while (true)
	{
		Sync::MutexUsage mutUsage(this->statusMut);
		status = this->statusMap->Get(sbuff);
		if (status)
		{
			if (status->req1 == 0)
			{
				cli = Net::HTTPClient::CreateConnect(this->sockf, this->ssl, url, method, noShutdown);
				status->req1 = cli;
				found = true;
			}
			else if (status->req2 == 0)
			{
				cli = Net::HTTPClient::CreateConnect(this->sockf, this->ssl, url, method, noShutdown);
				status->req2 = cli;
				found = true;
			}
		}
		else
		{
			status = MemAlloc(DomainStatus, 1);
			status->req1 = 0;
			status->req2 = 0;
			cli = Net::HTTPClient::CreateConnect(this->sockf, this->ssl, url, method, noShutdown);
			status->req1 = cli;
			this->statusMap->Put(sbuff, status);
			found = true;
		}
		mutUsage.EndUse();
		if (found)
			break;
		this->statusEvt->Wait(1000);
	}
	return cli;
}

void Net::HTTPQueue::EndRequest(Net::HTTPClient *cli)
{
	UTF8Char sbuff[512];
	DomainStatus *status;
	Text::URLString::GetURLDomain(sbuff, cli->GetURL(), 0);

	Sync::MutexUsage mutUsage(this->statusMut);
	status = this->statusMap->Get(sbuff);
	if (status)
	{
		if (status->req1 == cli)
		{
			status->req1 = 0;
		}
		else if (status->req2 == cli)
		{
			status->req2 = 0;
		}
		DEL_CLASS(cli);
		if (status->req1 == 0 && status->req2 == 0)
		{
			MemFree(status);
			this->statusMap->Remove(sbuff);
		}
		this->statusEvt->Set();
	}
	else
	{
		DEL_CLASS(cli);
	}
	mutUsage.EndUse();
}
