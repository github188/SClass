#include "Stdafx.h"
#include "IO/FileStream.h"
#include "IO/Path.h"
#include "IO/StmData/MemoryData.h"
#include "Manage/HiResClock.h"
#include "Net/HTTPClient.h"
#include "Net/HTTPOSClient.h"
#include "Net/MIME.h"
#include "SSWR/AVIRead/AVIRHTTPClientForm.h"
#include "SSWR/AVIRead/AVIRUserAgentSelForm.h"
#include "Sync/MutexUsage.h"
#include "Sync/Thread.h"
#include "Text/IMIMEObj.h"
#include "Text/JSONBuilder.h"
#include "Text/MyString.h"
#include "Text/MyStringFloat.h"
#include "Text/URLString.h"
#include "Text/TextBinEnc/Base64Enc.h"
#include "Text/TextEnc/FormEncoding.h"
#include "UI/FileDialog.h"
#include "UI/MessageDialog.h"

void __stdcall SSWR::AVIRead::AVIRHTTPClientForm::OnUserAgentClicked(void *userObj)
{
	SSWR::AVIRead::AVIRHTTPClientForm *me = (SSWR::AVIRead::AVIRHTTPClientForm*)userObj;
	SSWR::AVIRead::AVIRUserAgentSelForm *frm;
	NEW_CLASS(frm, SSWR::AVIRead::AVIRUserAgentSelForm(0, me->ui, me->core, me->userAgent));
	if (frm->ShowDialog(me))
	{
		SDEL_TEXT(me->userAgent);
		me->userAgent = Text::StrCopyNew(frm->GetUserAgent());
		me->lblUserAgent->SetText(me->userAgent);
	}
	DEL_CLASS(frm);
}

void __stdcall SSWR::AVIRead::AVIRHTTPClientForm::OnRequestClicked(void *userObj)
{
	SSWR::AVIRead::AVIRHTTPClientForm *me = (SSWR::AVIRead::AVIRHTTPClientForm*)userObj;
	UTF8Char sbuff[512];
	UTF8Char *sptr;
	Text::StringBuilderUTF8 sb;
	Text::StringBuilderUTF8 sbTmp;
	me->txtURL->GetText(&sb);
	if (!sb.StartsWith((const UTF8Char*)"http://") && !sb.StartsWith((const UTF8Char*)"https://"))
	{
		UI::MessageDialog::ShowDialog((const UTF8Char *)"Please enter valid http URL", (const UTF8Char *)"Request", me);
		return;
	}

	sbTmp.ClearStr();
	if (me->txtUserName->GetText(&sbTmp) && sbTmp.GetCharCnt() > 0)
	{
		me->reqUserName = Text::StrCopyNew(sbTmp.ToString());
	}
	sbTmp.ClearStr();
	if (me->txtPassword->GetText(&sbTmp) && sbTmp.GetCharCnt() > 0)
	{
		me->reqPassword = Text::StrCopyNew(sbTmp.ToString());
	}


	me->noShutdown = me->chkNoShutdown->IsChecked();
	me->reqMeth = (const Char*)me->cboMethod->GetSelectedItem();
	me->reqOSClient = me->chkOSClient->IsChecked();
	if (Text::StrEqualsICase(me->reqMeth, "GET") || Text::StrEqualsICase(me->reqMeth, "DELETE"))
	{
		UOSInt i = 0;
		UOSInt j = me->params->GetCount();
		if (j > 0)
		{
			if (sb.IndexOf('?') != INVALID_INDEX)
			{
				sb.AppendChar('&', 1);
			}
			else
			{
				sb.AppendChar('?', 1);
			}
			SSWR::AVIRead::AVIRHTTPClientForm::ParamValue *param;
			Text::StringBuilderUTF8 sb2;
			while (i < j)
			{
				param = me->params->GetItem(i);
				if (sb2.GetCharCnt() > 0)
				{
					sb2.AppendChar('&', 1);
				}
				Text::TextEnc::FormEncoding::FormEncode(sbuff, param->name);
				sb2.Append(sbuff);
				sb2.AppendChar('=', 1);
				Text::TextEnc::FormEncoding::FormEncode(sbuff, param->value);
				sb2.Append(sbuff);
				i++;
			}
			sb.Append(sb2.ToString());
		}
		me->reqBody = 0;
		me->reqBodyLen = 0;
		me->reqBodyType = 0;
	}
	else if (me->fileList->GetCount() == 1 && me->cboPostFormat->GetSelectedIndex() == 2)
	{
		const UTF8Char *fileName = me->fileList->GetItem(0);
		UTF8Char sbuff[32];
		IO::FileStream *fs;
		NEW_CLASS(fs, IO::FileStream(fileName, IO::FileStream::FILE_MODE_READONLY, IO::FileStream::FILE_SHARE_DENY_NONE, IO::FileStream::BT_NORMAL));
		me->reqBodyLen = (UOSInt)fs->GetLength();
		me->reqBody = MemAlloc(UInt8, me->reqBodyLen);
		fs->Read((UInt8*)me->reqBody, me->reqBodyLen);
		DEL_CLASS(fs);
		if (IO::Path::GetFileExt(sbuff, fileName))
		{
			me->reqBodyType = Text::StrCopyNew(Net::MIME::GetMIMEFromExt(sbuff));
		}
		else
		{
			me->reqBodyType = 0;
		}
	}
	else if (me->fileList->GetCount() > 0)
	{
		Text::StringBuilderUTF8 sbBoundary;
		Text::StringBuilderUTF8 sb2;
		Data::DateTime dt;
		dt.SetCurrTimeUTC();
		sbBoundary.Append((const UTF8Char*)"---------------------------Boundary");
		sbBoundary.AppendI64(dt.ToTicks());
		sb2.Append((const UTF8Char*)"multipart/form-data; boundary=");
		sb2.Append(sbBoundary.ToString());
		me->reqBodyType = Text::StrCopyNew(sb2.ToString());
		IO::MemoryStream mstm((const UTF8Char*)"SSWR.AVIRead.AVIRHTTPClientForm.OnRequestClicked.mstm");
		UOSInt i = 0;
		UOSInt j = me->params->GetCount();
		UOSInt k;
		SSWR::AVIRead::AVIRHTTPClientForm::ParamValue *param;
		const UTF8Char *csptr;
		while (i < j)
		{
			param = me->params->GetItem(i);
			mstm.Write((const UInt8*)"--", 2);
			mstm.Write(sbBoundary.ToString(), sbBoundary.GetCharCnt());
			csptr = (const UTF8Char*)"\r\nContent-Disposition: form-data; name=\"";
			mstm.Write(csptr, Text::StrCharCnt(csptr));
			sptr = Text::TextEnc::FormEncoding::FormEncode(sbuff, param->name);
			mstm.Write(sbuff, (UOSInt)(sptr - sbuff));
			mstm.Write((const UInt8*)"\"\r\n\r\n", 5);
			mstm.Write(param->value, Text::StrCharCnt(param->value));
			mstm.Write((const UInt8*)"\r\n", 2);

			i++;
		}

		sb2.ClearStr();
		me->txtFileFormName->GetText(&sb2);
		UInt8 fileBuff[4096];
		i = 0;
		j = me->fileList->GetCount();
		while (i < j)
		{
			csptr = me->fileList->GetItem(i);
			IO::FileStream *fs;
			UInt64 fileLength;
			UInt64 ofst;
			const UTF8Char *csptr2;
			NEW_CLASS(fs, IO::FileStream(csptr, IO::FileStream::FILE_MODE_READONLY, IO::FileStream::FILE_SHARE_DENY_NONE, IO::FileStream::BT_NORMAL));
			fileLength = fs->GetLength();
			if (fileLength > 0 && fileLength < 104857600)
			{
				mstm.Write((const UInt8*)"--", 2);
				mstm.Write(sbBoundary.ToString(), sbBoundary.GetCharCnt());
				csptr2 = (const UTF8Char*)"\r\nContent-Disposition: form-data; ";
				mstm.Write(csptr2, Text::StrCharCnt(csptr2));
				if (sb.GetCharCnt() > 0)
				{
					mstm.Write((const UInt8*)"name=\"", 6);
					sptr = Text::TextEnc::FormEncoding::FormEncode(sbuff, sb2.ToString());
					mstm.Write(sbuff, (UOSInt)(sptr - sbuff));
					mstm.Write((const UInt8*)"\"; ", 3);
				}
				k = Text::StrLastIndexOf(csptr, IO::Path::PATH_SEPERATOR);
				mstm.Write((const UInt8*)"filename=\"", 10);
				sptr = Text::TextEnc::FormEncoding::FormEncode(sbuff, &csptr[k + 1]);
				mstm.Write(sbuff, (UOSInt)(sptr - sbuff));
				mstm.Write((const UInt8*)"\"\r\n", 3);

				IO::Path::GetFileExt(sbuff, &csptr[k]);
				csptr2 = Net::MIME::GetMIMEFromExt(sbuff);
				mstm.Write((const UInt8*)"Content-Type: ", 14);
				mstm.Write(csptr2, Text::StrCharCnt(csptr2));
				mstm.Write((const UInt8*)"\r\n\r\n", 4);

				ofst = 0;
				while (ofst < fileLength)
				{
					k = fs->Read(fileBuff, 4096);
					if (k <= 0)
					{
						break;
					}
					mstm.Write(fileBuff, k);
					ofst += k;
				}
				mstm.Write((const UInt8*)"\r\n", 2);
			}
			DEL_CLASS(fs);
			i++;
		}
		mstm.Write((const UInt8*)"--", 2);
		mstm.Write(sbBoundary.ToString(), sbBoundary.GetCharCnt());
		mstm.Write((const UInt8*)"--", 2);

		UOSInt buffSize;
		UInt8 *reqBuff = mstm.GetBuff(&buffSize);

		me->reqBody = MemAlloc(UInt8, buffSize);
		me->reqBodyLen = buffSize;
		MemCopyNO((UInt8*)me->reqBody, reqBuff, buffSize);
	}
	else if (me->cboPostFormat->GetSelectedIndex() == 1)
	{
		UOSInt i = 0;
		UOSInt j = me->params->GetCount();
		SSWR::AVIRead::AVIRHTTPClientForm::ParamValue *param;
		Text::StringBuilderUTF8 sb2;
		Text::JSONBuilder *json;
		NEW_CLASS(json, Text::JSONBuilder(&sb2, Text::JSONBuilder::OT_OBJECT));
		while (i < j)
		{
			param = me->params->GetItem(i);
			json->ObjectAddStrUTF8(param->name, param->value);
			i++;
		}
		DEL_CLASS(json);
		me->reqBody = Text::StrCopyNew(sb2.ToString());
		me->reqBodyLen = sb2.GetCharCnt();
		me->reqBodyType = Text::StrCopyNew((const UTF8Char*)"application/json");
	}
	else
	{
		UOSInt i = 0;
		UOSInt j = me->params->GetCount();
		SSWR::AVIRead::AVIRHTTPClientForm::ParamValue *param;
		Text::StringBuilderUTF8 sb2;
		while (i < j)
		{
			param = me->params->GetItem(i);
			if (sb2.GetCharCnt() > 0)
			{
				sb2.AppendChar('&', 1);
			}
			Text::TextEnc::FormEncoding::FormEncode(sbuff, param->name);
			sb2.Append(sbuff);
			sb2.AppendChar('=', 1);
			Text::TextEnc::FormEncoding::FormEncode(sbuff, param->value);
			sb2.Append(sbuff);
			i++;
		}
		me->reqBody = Text::StrCopyNew(sb2.ToString());
		me->reqBodyLen = sb2.GetCharCnt();
		me->reqBodyType = Text::StrCopyNew((const UTF8Char*)"application/x-www-form-urlencoded");
	}
	me->reqURL = Text::StrCopyNew(sb.ToString());
	me->threadEvt->Set();
	while (me->threadRunning && me->reqURL && !me->respChanged)
	{
		Sync::Thread::Sleep(1);
	}
}

void __stdcall SSWR::AVIRead::AVIRHTTPClientForm::OnSaveClicked(void *userObj)
{
	SSWR::AVIRead::AVIRHTTPClientForm *me = (SSWR::AVIRead::AVIRHTTPClientForm*)userObj;
	if (me->respData == 0)
	{
		return;
	}
	UI::FileDialog *dlg;
	NEW_CLASS(dlg, UI::FileDialog(L"SSWR", L"AVIRead", L"HTTPClientSave", true));
	if (dlg->ShowDialog(me))
	{
		IO::FileStream *fs;
		Bool succ = false;
		NEW_CLASS(fs, IO::FileStream(dlg->GetFileName(), IO::FileStream::FILE_MODE_CREATE, IO::FileStream::FILE_SHARE_DENY_NONE, IO::FileStream::BT_NORMAL));
		Sync::MutexUsage mutUsage(me->respMut);
		if (me->respData)
		{
			UOSInt buffSize;
			UOSInt writeSize;
			UInt8 *buff = me->respData->GetBuff(&buffSize);
			writeSize = fs->Write(buff, buffSize);
			succ = (writeSize == buffSize);
		}
		mutUsage.EndUse();
		DEL_CLASS(fs);
		if (!succ)
		{
			UI::MessageDialog::ShowDialog((const UTF8Char*)"Error in storing to file", (const UTF8Char*)"HTTP Client", me);
		}
	}
	DEL_CLASS(dlg);
}

void __stdcall SSWR::AVIRead::AVIRHTTPClientForm::OnViewClicked(void *userObj)
{
	SSWR::AVIRead::AVIRHTTPClientForm *me = (SSWR::AVIRead::AVIRHTTPClientForm*)userObj;
	Sync::MutexUsage mutUsage(me->respMut);
	if (me->respData)
	{
		UOSInt buffSize;
		UInt8 *buff = me->respData->GetBuff(&buffSize);
		Text::IMIMEObj *mimeObj;
		IO::StmData::MemoryData *md;
		NEW_CLASS(md, IO::StmData::MemoryData(buff, buffSize));
		mimeObj = Text::IMIMEObj::ParseFromData(md, me->respContType);
		DEL_CLASS(md);
		if (mimeObj)
		{
			me->core->OpenObject(mimeObj);
		}
	}
	mutUsage.EndUse();
}

void __stdcall SSWR::AVIRead::AVIRHTTPClientForm::OnDataStrClicked(void *userObj)
{
	SSWR::AVIRead::AVIRHTTPClientForm *me = (SSWR::AVIRead::AVIRHTTPClientForm*)userObj;
	Text::StringBuilderUTF8 sb;
	SSWR::AVIRead::AVIRHTTPClientForm::ParamValue *param;
	UTF8Char sbuff[512];
	me->txtDataStr->GetText(&sb);
	me->ClearParams();
	me->lvReqData->ClearItems();
	if (sb.GetCharCnt() > 0)
	{
		UTF8Char *sptr = sb.ToString();
		UOSInt spInd;
		UOSInt eqInd;
		UOSInt i;
		while (true)
		{
			spInd = Text::StrIndexOf(sptr, '&');
			if (spInd != INVALID_INDEX)
			{
				sptr[spInd] = 0;
			}
			eqInd = Text::StrIndexOf(sptr, '=');
			param = MemAlloc(SSWR::AVIRead::AVIRHTTPClientForm::ParamValue, 1);
			if (eqInd != INVALID_INDEX)
			{
				sptr[eqInd] = 0;
				Text::TextEnc::FormEncoding::FormDecode(sbuff, sptr);
				param->name = Text::StrCopyNew(sbuff);
				Text::TextEnc::FormEncoding::FormDecode(sbuff, &sptr[eqInd + 1]);
				param->value = Text::StrCopyNew(sbuff);
			}
			else
			{
				Text::TextEnc::FormEncoding::FormDecode(sbuff, sptr);
				param->name = Text::StrCopyNew(sbuff);
				param->value = Text::StrCopyNew((const UTF8Char*)"");
			}
			me->params->Add(param);
			i = me->lvReqData->AddItem(param->name, param);
			me->lvReqData->SetSubItem(i, 1, param->value);

			if (spInd < 0)
			{
				break;
			}
			sptr = &sptr[spInd + 1];
		}

	}
}

void __stdcall SSWR::AVIRead::AVIRHTTPClientForm::OnFileSelectClicked(void *userObj)
{
	SSWR::AVIRead::AVIRHTTPClientForm *me = (SSWR::AVIRead::AVIRHTTPClientForm*)userObj;
	UI::FileDialog *dlg;
	NEW_CLASS(dlg, UI::FileDialog(L"SSWR", L"AVIRead", L"HTTPClientSelect", false));
	dlg->SetAllowMultiSel(true);
	if (dlg->ShowDialog(me->GetHandle()))
	{
		me->ClearFiles();

		UOSInt i = 0;
		UOSInt j = dlg->GetFileNameCount();
		const UTF8Char *fileName;
		while (i < j)
		{
			fileName = dlg->GetFileNames(i);
			me->fileList->Add(Text::StrCopyNew(fileName));
			i++;
		}
		Text::StringBuilderUTF8 sb;
		sb.AppendUOSInt(j);
		if (j > 1)
		{
			sb.Append((const UTF8Char*)" files selected");
		}
		else
		{
			sb.Append((const UTF8Char*)" files selected");
		}
		me->lblFileStatus->SetText(sb.ToString());
	}
	DEL_CLASS(dlg);
}

void __stdcall SSWR::AVIRead::AVIRHTTPClientForm::OnFileClearClicked(void *userObj)
{
	SSWR::AVIRead::AVIRHTTPClientForm *me = (SSWR::AVIRead::AVIRHTTPClientForm*)userObj;
	me->ClearFiles();
	me->lblFileStatus->SetText((const UTF8Char*)"No files selected");
}

UInt32 __stdcall SSWR::AVIRead::AVIRHTTPClientForm::ProcessThread(void *userObj)
{
	SSWR::AVIRead::AVIRHTTPClientForm *me = (SSWR::AVIRead::AVIRHTTPClientForm*)userObj;
	const UTF8Char *currURL;
	const UTF8Char *currBody;
	UOSInt currBodyLen;
	const UTF8Char *currBodyType;
	const UTF8Char *currUserName;
	const UTF8Char *currPassword;
	const Char *currMeth;
	Bool currOSClient;
	UInt8 buff[4096];
	UTF8Char *pathPtr;
	UTF8Char *sbuff;
	UTF8Char *cookiePtr;
	SSWR::AVIRead::AVIRHTTPClientForm::HTTPCookie *cookie;
	UOSInt i;
	UOSInt j;
	UOSInt len1;
	UOSInt len2;
	me->threadRunning = true;
	sbuff = MemAlloc(UTF8Char, 65536);
	while (!me->threadToStop)
	{
		if (me->reqURL && !me->respChanged)
		{
			currURL = me->reqURL;
			currBody = me->reqBody;
			currBodyLen = me->reqBodyLen;
			currBodyType = me->reqBodyType;
			currMeth = me->reqMeth;
			currUserName = me->reqUserName;
			currPassword = me->reqPassword;
			currOSClient = me->reqOSClient;
			me->reqURL = 0;
			me->reqBody = 0;
			me->reqBodyLen = 0;
			me->reqBodyType = 0;
			me->reqUserName = 0;
			me->reqPassword = 0;
			
			Net::HTTPClient *cli;
			cli = Net::HTTPClient::CreateClient(me->core->GetSocketFactory(), currOSClient?0:me->core->GetSSLEngine(), me->userAgent, me->noShutdown, Text::StrStartsWith(currURL, (const UTF8Char*)"https://"));
//			NEW_CLASS(cli, Net::HTTPOSClient(me->core->GetSocketFactory(), me->userAgent, me->noShutdown));
			if (cli->Connect(currURL, currMeth, &me->respTimeDNS, &me->respTimeConn, false))
			{
				IO::MemoryStream *mstm;
				const UTF8Char *contType = 0;
				NEW_CLASS(mstm, IO::MemoryStream((const UTF8Char*)"SSWR.AVIRead.AVIRHTTPClientForm.respData"));
				cli->AddHeader((const UTF8Char*)"Accept", (const UTF8Char*)"*/*");
				cli->AddHeader((const UTF8Char*)"Accept-Charset", (const UTF8Char*)"*");
				
				cookiePtr = 0;
				pathPtr = Text::URLString::GetURLDomain(buff, currURL, 0) + 1;
				Text::URLString::GetURLPath(pathPtr, currURL);
				len1 = Text::StrCharCnt(buff);
				Sync::MutexUsage mutUsage(me->cookieMut);
				i = 0;
				j = me->cookieList->GetCount();
				while (i < j)
				{
					cookie = me->cookieList->GetItem(i);
					len2 = Text::StrCharCnt(cookie->domain);
					if ((len1 == len2 && Text::StrEquals(buff, cookie->domain)) || (len1 > len2 && buff[len1 - len2 - 1] == '.' && Text::StrEquals(&buff[len1 - len2], cookie->domain)))
					{
						if (cookie->path == 0 || Text::StrStartsWith(pathPtr, cookie->path))
						{
							if (cookiePtr == 0)
							{
								cookiePtr = Text::StrConcat(sbuff, cookie->name);
							}
							else
							{
								cookiePtr = Text::StrConcat(cookiePtr, (const UTF8Char*)"; ");
								cookiePtr = Text::StrConcat(cookiePtr, cookie->name);
							}
							cookiePtr = Text::StrConcat(cookiePtr, (const UTF8Char*)"=");
							cookiePtr = Text::StrConcat(cookiePtr, cookie->value);
						}
					}
					i++;
				}
				mutUsage.EndUse();
				if (cookiePtr)
				{
					cli->AddHeader((const UTF8Char*)"Cookie", sbuff);
				}

				if (!Text::StrEquals(currMeth, "GET") && currBody)
				{
					Text::StrUOSInt(sbuff, currBodyLen);
					cli->AddHeader((const UTF8Char*)"Content-Length", sbuff);
					cli->AddHeader((const UTF8Char*)"Content-Type", (const UTF8Char*)currBodyType);
					cli->Write(currBody, currBodyLen);
				}

				cli->EndRequest(&me->respTimeReq, &me->respTimeResp);
				UInt64 totalRead = 0;
				UOSInt thisRead;
				while ((thisRead = cli->Read(buff, 4096)) > 0)
				{
					mstm->Write(buff, thisRead);
					totalRead += thisRead;
				}
				me->respTimeTotal = cli->GetTotalTime();
				me->respSize = totalRead;
				me->respStatus = cli->GetRespStatus();
				if (me->respStatus == 401 && currUserName != 0 && currPassword != 0)
				{
					DEL_CLASS(cli);
					cli = Net::HTTPClient::CreateClient(me->core->GetSocketFactory(), me->core->GetSSLEngine(), me->userAgent, me->noShutdown, Text::StrStartsWith(currURL, (const UTF8Char*)"https://"));
					if (cli->Connect(currURL, currMeth, &me->respTimeDNS, &me->respTimeConn, false))
					{
						contType = 0;
						mstm->Clear();
						cli->AddHeader((const UTF8Char*)"Accept", (const UTF8Char*)"*/*");
						cli->AddHeader((const UTF8Char*)"Accept-Charset", (const UTF8Char*)"*");
						i = (UOSInt)(Text::StrConcat(Text::StrConcat(Text::StrConcat(buff, currUserName), (const UTF8Char*)":"), currPassword) - buff);
						Text::StringBuilderUTF8 sbAuth;
						sbAuth.Append((const UTF8Char*)"Basic ");
						Text::TextBinEnc::Base64Enc b64Enc;
						b64Enc.EncodeBin(&sbAuth, buff, i);
						cli->AddHeader((const UTF8Char*)"Authorization", sbAuth.ToString());
						
						cookiePtr = 0;
						pathPtr = Text::URLString::GetURLDomain(buff, currURL, 0) + 1;
						Text::URLString::GetURLPath(pathPtr, currURL);
						len1 = Text::StrCharCnt(buff);
						mutUsage.ReplaceMutex(me->cookieMut);
						i = 0;
						j = me->cookieList->GetCount();
						while (i < j)
						{
							cookie = me->cookieList->GetItem(i);
							len2 = Text::StrCharCnt(cookie->domain);
							if ((len1 == len2 && Text::StrEquals(buff, cookie->domain)) || (len1 > len2 && buff[len1 - len2 - 1] == '.' && Text::StrEquals(&buff[len1 - len2], cookie->domain)))
							{
								if (cookie->path == 0 || Text::StrStartsWith(pathPtr, cookie->path))
								{
									if (cookiePtr == 0)
									{
										cookiePtr = Text::StrConcat(sbuff, cookie->name);
									}
									else
									{
										cookiePtr = Text::StrConcat(cookiePtr, (const UTF8Char*)"; ");
										cookiePtr = Text::StrConcat(cookiePtr, cookie->name);
									}
									cookiePtr = Text::StrConcat(cookiePtr, (const UTF8Char*)"=");
									cookiePtr = Text::StrConcat(cookiePtr, cookie->value);
								}
							}
							i++;
						}
						mutUsage.EndUse();
						if (cookiePtr)
						{
							cli->AddHeader((const UTF8Char*)"Cookie", sbuff);
						}

						if (!Text::StrEquals(currMeth, "GET") && currBody)
						{
							Text::StrUOSInt(sbuff, currBodyLen);
							cli->AddHeader((const UTF8Char*)"Content-Length", sbuff);
							cli->AddHeader((const UTF8Char*)"Content-Type", (const UTF8Char*)currBodyType);
							cli->Write(currBody, currBodyLen);
						}

						cli->EndRequest(&me->respTimeReq, &me->respTimeResp);
						totalRead = 0;
						while ((thisRead = cli->Read(buff, 4096)) > 0)
						{
							mstm->Write(buff, thisRead);
							totalRead += thisRead;
						}
						me->respTimeTotal = cli->GetTotalTime();
						me->respSize = totalRead;
						me->respStatus = cli->GetRespStatus();
					}
					else
					{
						me->respTimeDNS = -1;
						me->respTimeConn = -1;
						me->respTimeReq = -1;
						me->respTimeResp = -1;
						me->respTimeTotal = -1;
						me->respSize = 0;
						me->respStatus = 0;
					}
				}
				me->ClearHeaders();
				i = 0;
				j = cli->GetRespHeaderCnt();
				while (i < j)
				{
					cli->GetRespHeader(i, sbuff);
					me->respHeaders->Add(Text::StrCopyNew(sbuff));
					i++;
				}
				Text::StringBuilderUTF8 sb;
				if (cli->GetRespHeader((const UTF8Char*)"Content-Type", &sb))
				{
					contType = Text::StrCopyNew(sb.ToString());
				}
				me->respSvrAddr = *cli->GetSvrAddr();
				Sync::MutexUsage respMutUsage(me->respMut);
				SDEL_TEXT(me->respReqURL)
				SDEL_TEXT(me->respContType);
				SDEL_CLASS(me->respData);
				me->respReqURL = Text::StrCopyNew(currURL);
				me->respContType = contType;
				me->respData = mstm;
				respMutUsage.EndUse();
			}
			else
			{
				me->respTimeDNS = -1;
				me->respTimeConn = -1;
				me->respTimeReq = -1;
				me->respTimeResp = -1;
				me->respTimeTotal = -1;
				me->respSize = 0;
				me->respStatus = 0;
				Sync::MutexUsage mutUsage(me->respMut);
				SDEL_TEXT(me->respReqURL)
				SDEL_CLASS(me->respData);
				SDEL_TEXT(me->respContType);
				me->respReqURL = Text::StrCopyNew(currURL);
				mutUsage.EndUse();
			}

			DEL_CLASS(cli);
			me->respChanged = true;

			Text::StrDelNew(currURL);
			if (currBody)
			{
				MemFree((UInt8*)currBody);
				currBody = 0;
			}
			SDEL_TEXT(currBodyType);
			SDEL_TEXT(currUserName);
			SDEL_TEXT(currPassword);
		}
		else
		{
			me->threadEvt->Wait(1000);
		}
	}
	MemFree(sbuff);
	SDEL_TEXT(me->reqURL);
	SDEL_TEXT(me->reqBody);
	SDEL_TEXT(me->reqBodyType);
	me->threadToStop = false;
	me->threadRunning = false;
	return 0;
}

void __stdcall SSWR::AVIRead::AVIRHTTPClientForm::OnTimerTick(void *userObj)
{
	SSWR::AVIRead::AVIRHTTPClientForm *me = (SSWR::AVIRead::AVIRHTTPClientForm*)userObj;
	const UTF8Char *hdr;
	UTF8Char sbuff[64];
	UOSInt i;
	UOSInt j;
	if (me->respChanged)
	{
		me->txtReqURL->SetText(me->respReqURL);
		Net::SocketUtil::GetAddrName(sbuff, &me->respSvrAddr);
		me->txtSvrIP->SetText(sbuff);
		if (me->respTimeDNS == -1)
		{
			me->txtTimeDNS->SetText((const UTF8Char*)"-1");
		}
		else
		{
			Text::StrDoubleFmt(sbuff, me->respTimeDNS, "0.0000000000");
			me->txtTimeDNS->SetText(sbuff);
		}
		if (me->respTimeConn == -1)
		{
			me->txtTimeConn->SetText((const UTF8Char*)"-1");
		}
		else
		{
			Text::StrDoubleFmt(sbuff, me->respTimeConn - me->respTimeDNS, "0.0000000000");
			me->txtTimeConn->SetText(sbuff);
		}
		if (me->respTimeReq == -1)
		{
			me->txtTimeSendHdr->SetText((const UTF8Char*)"-1");
		}
		else
		{
			Text::StrDoubleFmt(sbuff, me->respTimeReq - me->respTimeConn, "0.0000000000");
			me->txtTimeSendHdr->SetText(sbuff);
		}
		if (me->respTimeResp == -1)
		{
			me->txtTimeResp->SetText((const UTF8Char*)"-1");
		}
		else
		{
			Text::StrDoubleFmt(sbuff, me->respTimeResp - me->respTimeReq, "0.0000000000");
			me->txtTimeResp->SetText(sbuff);
		}
		if (me->respTimeTotal == -1)
		{
			me->txtTimeTotal->SetText((const UTF8Char*)"-1");
		}
		else
		{
			Text::StrDoubleFmt(sbuff, me->respTimeTotal - me->respTimeResp, "0.0000000000");
			me->txtTimeTotal->SetText(sbuff);
		}
		Text::StrUInt64(sbuff, me->respSize);
		me->txtRespSize->SetText(sbuff);
		Text::StrInt32(sbuff, me->respStatus);
		me->txtRespStatus->SetText(sbuff);

		me->lvHeaders->ClearItems();
		i = 0;
		j = me->respHeaders->GetCount();
		while (i < j)
		{
			hdr = me->respHeaders->GetItem(i);
			if (Text::StrStartsWith(hdr, (const UTF8Char*)"Set-Cookie: "))
			{
				SSWR::AVIRead::AVIRHTTPClientForm::HTTPCookie *cookie = me->SetCookie(&hdr[12], me->respReqURL);
				if (cookie)
				{
					UOSInt k = me->lvCookie->AddItem(cookie->domain, cookie);
					if (cookie->path)
					{
						me->lvCookie->SetSubItem(k, 1, cookie->path);
					}
					me->lvCookie->SetSubItem(k, 2, cookie->name);
					me->lvCookie->SetSubItem(k, 3, cookie->value);
				}
			}
			me->lvHeaders->AddItem(hdr, 0);
			i++;
		}
		me->respChanged = false;
		me->tcMain->SetSelectedIndex(1);
	}
}

void SSWR::AVIRead::AVIRHTTPClientForm::ClearHeaders()
{
	UOSInt i;
	i = this->respHeaders->GetCount();
	while (i-- > 0)
	{
		Text::StrDelNew(this->respHeaders->RemoveAt(i));
	}
}

void SSWR::AVIRead::AVIRHTTPClientForm::ClearParams()
{
	UOSInt i;
	SSWR::AVIRead::AVIRHTTPClientForm::ParamValue *param;
	i = this->params->GetCount();
	while (i-- > 0)
	{
		param = this->params->GetItem(i);
		Text::StrDelNew(param->name);
		Text::StrDelNew(param->value);
		MemFree(param);
	}
	this->params->Clear();
}

void SSWR::AVIRead::AVIRHTTPClientForm::ClearCookie()
{
	UOSInt i;
	SSWR::AVIRead::AVIRHTTPClientForm::HTTPCookie *cookie;
	i = this->cookieList->GetCount();
	while (i-- > 0)
	{
		cookie = this->cookieList->GetItem(i);
		Text::StrDelNew(cookie->name);
		Text::StrDelNew(cookie->value);
		Text::StrDelNew(cookie->domain);
		SDEL_TEXT(cookie->path);
		MemFree(cookie);
	}
	this->cookieList->Clear();

}

void SSWR::AVIRead::AVIRHTTPClientForm::ClearFiles()
{
	UOSInt i = this->fileList->GetCount();
	while (i-- > 0)
	{
		Text::StrDelNew(this->fileList->GetItem(i));
	}
	this->fileList->Clear();
}

SSWR::AVIRead::AVIRHTTPClientForm::HTTPCookie *SSWR::AVIRead::AVIRHTTPClientForm::SetCookie(const UTF8Char *cookieStr, const UTF8Char *reqURL)
{
	UTF8Char domain[512];
	UTF8Char path[512];
	UTF8Char *sarr[2];
	UTF8Char *cookieValue;
	UOSInt cnt;
	UOSInt i;
	Bool secure = false;
	Int64 expiryTime = 0;
	Bool valid = true;
	path[0] = 0;
	Text::URLString::GetURLDomain(domain, reqURL, 0);
	Text::StringBuilderUTF8 sb;
	sb.Append(cookieStr);
	cnt = Text::StrSplitTrim(sarr, 2, sb.ToString(), ';');
	cookieValue = sarr[0];
	i = Text::StrIndexOf(cookieValue, '=');
	if (i == INVALID_INDEX)
	{
		return 0;
	}
	while (cnt == 2)
	{
		cnt = Text::StrSplitTrim(sarr, 2, sarr[1], ';');
		if (Text::StrEquals(sarr[0], (const UTF8Char*)"Secure"))
		{
			secure = true;
		}
		else if (Text::StrEquals(sarr[0], (const UTF8Char*)"HttpOnly"))
		{

		}
		else if (Text::StrStartsWith(sarr[0], (const UTF8Char*)"SameSite="))
		{

		}
		else if (Text::StrStartsWith(sarr[0], (const UTF8Char*)"Expires="))
		{
			Data::DateTime dt;
			dt.SetValue(&sarr[0][8]);
			expiryTime = dt.ToTicks();
		}
		else if (Text::StrStartsWith(sarr[0], (const UTF8Char*)"Max-Age="))
		{
			Data::DateTime dt;
			dt.SetCurrTimeUTC();
			dt.AddSecond(Text::StrToOSInt(&sarr[0][8]));
			expiryTime = dt.ToTicks();
		}
		else if (Text::StrStartsWith(sarr[0], (const UTF8Char*)"Domain="))
		{
			if (Text::StrEqualsICase(domain, &sarr[0][7]))
			{

			}
			else
			{
				UOSInt len1 = Text::StrCharCnt(domain);
				UOSInt len2 = Text::StrCharCnt(&sarr[0][7]);
				if (len1 > len2 && len2 > 0 && domain[len1 - len2 - 1] == '.' && Text::StrEquals(&domain[len1 - len2], &sarr[0][7]))
				{
					Text::StrConcat(domain, &sarr[0][7]);
				}
				else
				{
					valid = false;
				}
			}
		}
		else if (Text::StrStartsWith(sarr[0], (const UTF8Char*)"Path="))
		{
			Text::StrConcat(path, &sarr[0][5]);
		}
	}
	if (valid)
	{
		const UTF8Char *cookieName = Text::StrCopyNewC(cookieValue, (UOSInt)i);
		SSWR::AVIRead::AVIRHTTPClientForm::HTTPCookie *cookie;
		Bool eq;
		UOSInt j = this->cookieList->GetCount();
		while (j-- > 0)
		{
			cookie = this->cookieList->GetItem(j);
			eq = Text::StrEquals(cookie->domain, domain) && cookie->secure == secure && Text::StrEquals(cookie->name, cookieName);
			if (cookie->path == 0)
			{
				eq = eq && (path[0] == 0);
			}
			else
			{
				eq = eq && Text::StrEquals(cookie->path, path);
			}
			if (eq)
			{
				Sync::MutexUsage mutUsage(this->cookieMut);
				SDEL_TEXT(cookie->value);
				cookie->value  = Text::StrCopyNew(&cookieValue[i + 1]);
				mutUsage.EndUse();
				return cookie;
			}
		}
		cookie = MemAlloc(SSWR::AVIRead::AVIRHTTPClientForm::HTTPCookie, 1);
		cookie->domain = Text::StrCopyNew(domain);
		if (path[0])
		{
			cookie->path = Text::StrCopyNew(path);
		}
		else
		{
			cookie->path = 0;
		}
		cookie->secure = secure;
		cookie->expireTime = expiryTime;
		cookie->name = Text::StrCopyNewC(cookieValue, (UOSInt)i);
		cookie->value = Text::StrCopyNew(&cookieValue[i + 1]);
		Sync::MutexUsage mutUsage(this->cookieMut);
		this->cookieList->Add(cookie);
		mutUsage.EndUse();
		return cookie;
	}
	else
	{
		return 0;
	}
}

SSWR::AVIRead::AVIRHTTPClientForm::AVIRHTTPClientForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core) : UI::GUIForm(parent, 1024, 768, ui)
{
	this->SetFont(0, 8.25, false);
	this->SetText((const UTF8Char*)"HTTP Client");

	this->core = core;
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));
	this->sockf = core->GetSocketFactory();
	this->respChanged = false;
	this->threadRunning = false;
	this->threadToStop = false;
	this->reqURL = 0;
	this->reqBody = 0;
	this->reqBodyLen = 0;
	this->reqBodyType = 0;
	this->reqUserName = 0;
	this->reqPassword = 0;
	NEW_CLASS(this->threadEvt, Sync::Event(true, (const UTF8Char*)"SSWR.AVIRead.AVIRHTTPClientForm.threadEvt"));
	NEW_CLASS(this->respHeaders, Data::ArrayList<const UTF8Char *>());
	NEW_CLASS(this->respMut, Sync::Mutex());
	NEW_CLASS(this->params, Data::ArrayList<SSWR::AVIRead::AVIRHTTPClientForm::ParamValue*>());
	NEW_CLASS(this->cookieList, Data::ArrayList<SSWR::AVIRead::AVIRHTTPClientForm::HTTPCookie*>());
	NEW_CLASS(this->cookieMut, Sync::Mutex());
	NEW_CLASS(this->fileList, Data::ArrayList<const UTF8Char*>());
	this->respContType = 0;
	this->respReqURL = 0;
	this->respData = 0;
	this->userAgent = Text::StrCopyNew((const UTF8Char*)"SSWR/1.0");

	NEW_CLASS(this->tcMain, UI::GUITabControl(ui, this));
	this->tcMain->SetDockType(UI::GUIControl::DOCK_FILL);

	this->tpRequest = this->tcMain->AddTabPage((const UTF8Char*)"Request");
	NEW_CLASS(this->pnlRequest, UI::GUIPanel(ui, this->tpRequest));
	this->pnlRequest->SetRect(0, 0, 100, 220, false);
	this->pnlRequest->SetDockType(UI::GUIControl::DOCK_TOP);
	NEW_CLASS(this->lblURL, UI::GUILabel(ui, this->pnlRequest, (const UTF8Char*)"URL"));
	this->lblURL->SetRect(4, 4, 100, 23, false);
	NEW_CLASS(this->txtURL, UI::GUITextBox(ui, this->pnlRequest, (const UTF8Char*)"http://"));
	this->txtURL->SetRect(104, 4, 400, 23, false);
	NEW_CLASS(this->chkNoShutdown, UI::GUICheckBox(ui, this->pnlRequest, (const UTF8Char*)"No Shutdown", false));
	this->chkNoShutdown->SetRect(504, 4, 100, 23, false);
	NEW_CLASS(this->lblMethod, UI::GUILabel(ui, this->pnlRequest, (const UTF8Char*)"Method"));
	this->lblMethod->SetRect(4, 28, 100, 23, false);
	NEW_CLASS(cboMethod, UI::GUIComboBox(ui, this->pnlRequest, false));
	this->cboMethod->SetRect(104, 28, 100, 23, false);
	this->cboMethod->AddItem((const UTF8Char*)"GET", (void*)"GET");
	this->cboMethod->AddItem((const UTF8Char*)"POST", (void*)"POST");
	this->cboMethod->AddItem((const UTF8Char*)"PUT", (void*)"PUT");
	this->cboMethod->AddItem((const UTF8Char*)"PATCH", (void*)"PATCH");
	this->cboMethod->AddItem((const UTF8Char*)"DELETE", (void*)"DELETE");
	this->cboMethod->SetSelectedIndex(0);
	NEW_CLASS(this->chkOSClient, UI::GUICheckBox(ui, this->pnlRequest, (const UTF8Char*)"OS Client", false));
	this->chkOSClient->SetRect(204, 28, 100, 23, false);
	NEW_CLASS(this->btnUserAgent, UI::GUIButton(ui, this->pnlRequest, (const UTF8Char*)"User Agent"));
	this->btnUserAgent->SetRect(4, 52, 75, 23, false);
	this->btnUserAgent->HandleButtonClick(OnUserAgentClicked, this);
	NEW_CLASS(this->lblUserAgent, UI::GUILabel(ui, this->pnlRequest, this->userAgent));
	this->lblUserAgent->SetRect(104, 52, 400, 23, false);
	NEW_CLASS(this->lblUserName, UI::GUILabel(ui, this->pnlRequest, (const UTF8Char*)"UserName"));
	this->lblUserName->SetRect(4, 76, 100, 23, false);
	NEW_CLASS(this->txtUserName, UI::GUITextBox(ui, this->pnlRequest, (const UTF8Char*)""));
	this->txtUserName->SetRect(104, 76, 150, 23, false);
	NEW_CLASS(this->lblPassword, UI::GUILabel(ui, this->pnlRequest, (const UTF8Char*)"Password"));
	this->lblPassword->SetRect(4, 100, 100, 23, false);
	NEW_CLASS(this->txtPassword, UI::GUITextBox(ui, this->pnlRequest, (const UTF8Char*)""));
	this->txtPassword->SetPasswordChar('*');
	this->txtPassword->SetRect(104, 100, 150, 23, false);
	NEW_CLASS(this->lblFileUpload, UI::GUILabel(ui, this->pnlRequest, (const UTF8Char*)"File Upload"));
	this->lblFileUpload->SetRect(4, 124, 100, 23, false);
	NEW_CLASS(this->txtFileFormName, UI::GUITextBox(ui, this->pnlRequest, (const UTF8Char*)""));
	this->txtFileFormName->SetRect(104, 124, 150, 23, false);
	NEW_CLASS(this->btnFileSelect, UI::GUIButton(ui, this->pnlRequest, (const UTF8Char*)"Select"));
	this->btnFileSelect->SetRect(254, 124, 75, 23, false);
	this->btnFileSelect->HandleButtonClick(OnFileSelectClicked, this);
	NEW_CLASS(this->btnFileClear, UI::GUIButton(ui, this->pnlRequest, (const UTF8Char*)"Clear"));
	this->btnFileClear->SetRect(334, 124, 75, 23, false);
	this->btnFileClear->HandleButtonClick(OnFileClearClicked, this);
	NEW_CLASS(this->lblFileStatus, UI::GUILabel(ui, this->pnlRequest, (const UTF8Char*)"No files selected"));
	this->lblFileStatus->SetRect(414, 124, 200, 23, false);
	NEW_CLASS(this->lblDataStr, UI::GUILabel(ui, this->pnlRequest, (const UTF8Char*)"Data String"));
	this->lblDataStr->SetRect(4, 148, 100, 23, false);
	NEW_CLASS(this->txtDataStr, UI::GUITextBox(ui, this->pnlRequest, (const UTF8Char*)""));
	this->txtDataStr->SetRect(104, 148, 400, 23, false);
	NEW_CLASS(this->btnDataStr, UI::GUIButton(ui, this->pnlRequest, (const UTF8Char*)"Parse"));
	this->btnDataStr->SetRect(504, 148, 75, 23, false);
	this->btnDataStr->HandleButtonClick(OnDataStrClicked, this);
	NEW_CLASS(this->lblPostFormat, UI::GUILabel(ui, this->pnlRequest, (const UTF8Char*)"Post Format"));
	this->lblPostFormat->SetRect(4, 172, 100, 23, false);
	NEW_CLASS(this->cboPostFormat, UI::GUIComboBox(ui, this->pnlRequest, false));
	this->cboPostFormat->SetRect(104, 172, 150, 23, false);
	this->cboPostFormat->AddItem((const UTF8Char*)"application/x-www-form-urlencoded", 0);
	this->cboPostFormat->AddItem((const UTF8Char*)"application/json", 0);
	this->cboPostFormat->AddItem((const UTF8Char*)"RAW", 0);
	this->cboPostFormat->SetSelectedIndex(0);
	NEW_CLASS(this->btnRequest, UI::GUIButton(ui, this->pnlRequest, (const UTF8Char*)"Request"));
	this->btnRequest->SetRect(104, 196, 75, 23, false);
	this->btnRequest->HandleButtonClick(OnRequestClicked, this);
	NEW_CLASS(this->lvReqData, UI::GUIListView(ui, this->tpRequest, UI::GUIListView::LVSTYLE_TABLE, 2));
	this->lvReqData->SetDockType(UI::GUIControl::DOCK_FILL);
	this->lvReqData->AddColumn((const UTF8Char*)"Name", 150);
	this->lvReqData->AddColumn((const UTF8Char*)"Value", 400);

	this->tpResponse = this->tcMain->AddTabPage((const UTF8Char*)"Response");
	NEW_CLASS(this->pnlResponse, UI::GUIPanel(ui, this->tpResponse));
	this->pnlResponse->SetRect(0, 0, 100, 223, false);
	this->pnlResponse->SetDockType(UI::GUIControl::DOCK_TOP);
	NEW_CLASS(this->lblReqURL, UI::GUILabel(ui, this->pnlResponse, (const UTF8Char*)"Req URL"));
	this->lblReqURL->SetRect(4, 4, 100, 23, false);
	NEW_CLASS(this->txtReqURL, UI::GUITextBox(ui, this->pnlResponse, (const UTF8Char*)""));
	this->txtReqURL->SetRect(104, 4, 400, 23, false);
	this->txtReqURL->SetReadOnly(true);
	NEW_CLASS(this->lblSvrIP, UI::GUILabel(ui, this->pnlResponse, (const UTF8Char*)"Server IP"));
	this->lblSvrIP->SetRect(4, 28, 100, 23, false);
	NEW_CLASS(this->txtSvrIP, UI::GUITextBox(ui, this->pnlResponse, (const UTF8Char*)""));
	this->txtSvrIP->SetRect(104, 28, 150, 23, false);
	this->txtSvrIP->SetReadOnly(true);
	NEW_CLASS(this->lblTimeDNS, UI::GUILabel(ui, this->pnlResponse, (const UTF8Char*)"DNS Time"));
	this->lblTimeDNS->SetRect(4, 52, 100, 23, false);
	NEW_CLASS(this->txtTimeDNS, UI::GUITextBox(ui, this->pnlResponse, (const UTF8Char*)""));
	this->txtTimeDNS->SetRect(104, 52, 150, 23, false);
	this->txtTimeDNS->SetReadOnly(true);
	NEW_CLASS(this->lblTimeConn, UI::GUILabel(ui, this->pnlResponse, (const UTF8Char*)"Conn Time"));
	this->lblTimeConn->SetRect(4, 76, 100, 23, false);
	NEW_CLASS(this->txtTimeConn, UI::GUITextBox(ui, this->pnlResponse, (const UTF8Char*)""));
	this->txtTimeConn->SetRect(104, 76, 150, 23, false);
	this->txtTimeConn->SetReadOnly(true);
	NEW_CLASS(this->lblTimeSendHdr, UI::GUILabel(ui, this->pnlResponse, (const UTF8Char*)"Request Time"));
	this->lblTimeSendHdr->SetRect(4, 100, 100, 23, false);
	NEW_CLASS(this->txtTimeSendHdr, UI::GUITextBox(ui, this->pnlResponse, (const UTF8Char*)""));
	this->txtTimeSendHdr->SetRect(104, 100, 150, 23, false);
	this->txtTimeSendHdr->SetReadOnly(true);
	NEW_CLASS(this->lblTimeResp, UI::GUILabel(ui, this->pnlResponse, (const UTF8Char*)"Response Time"));
	this->lblTimeResp->SetRect(4, 124, 100, 23, false);
	NEW_CLASS(this->txtTimeResp, UI::GUITextBox(ui, this->pnlResponse, (const UTF8Char*)""));
	this->txtTimeResp->SetRect(104, 124, 150, 23, false);
	this->txtTimeResp->SetReadOnly(true);
	NEW_CLASS(this->lblTimeTotal, UI::GUILabel(ui, this->pnlResponse, (const UTF8Char*)"Download Time"));
	this->lblTimeTotal->SetRect(4, 148, 100, 23, false);
	NEW_CLASS(this->txtTimeTotal, UI::GUITextBox(ui, this->pnlResponse, (const UTF8Char*)""));
	this->txtTimeTotal->SetRect(104, 148, 150, 23, false);
	this->txtTimeTotal->SetReadOnly(true);
	NEW_CLASS(this->lblRespStatus, UI::GUILabel(ui, this->pnlResponse, (const UTF8Char*)"Status Code"));
	this->lblRespStatus->SetRect(4, 172, 100, 23, false);
	NEW_CLASS(this->txtRespStatus, UI::GUITextBox(ui, this->pnlResponse, (const UTF8Char*)""));
	this->txtRespStatus->SetRect(104, 172, 150, 23, false);
	this->txtRespStatus->SetReadOnly(true);
	NEW_CLASS(this->lblRespSize, UI::GUILabel(ui, this->pnlResponse, (const UTF8Char*)"Download Size"));
	this->lblRespSize->SetRect(4, 196, 100, 23, false);
	NEW_CLASS(this->txtRespSize, UI::GUITextBox(ui, this->pnlResponse, (const UTF8Char*)""));
	this->txtRespSize->SetRect(104, 196, 150, 23, false);
	this->txtRespSize->SetReadOnly(true);
	NEW_CLASS(this->pnlControl, UI::GUIPanel(ui, this->tpResponse));
	this->pnlControl->SetRect(0, 0, 100, 31, false);
	this->pnlControl->SetDockType(UI::GUIControl::DOCK_BOTTOM);
	NEW_CLASS(this->btnSave, UI::GUIButton(ui, this->pnlControl, (const UTF8Char*)"Save"));
	this->btnSave->SetRect(4, 4, 75, 23, false);
	this->btnSave->HandleButtonClick(OnSaveClicked, this);
	NEW_CLASS(this->btnView, UI::GUIButton(ui, this->pnlControl, (const UTF8Char*)"View"));
	this->btnView->SetRect(84, 4, 75, 23, false);
	this->btnView->HandleButtonClick(OnViewClicked, this);
	NEW_CLASS(this->lvHeaders, UI::GUIListView(ui, this->tpResponse, UI::GUIListView::LVSTYLE_TABLE, 1));
	this->lvHeaders->SetDockType(UI::GUIControl::DOCK_FILL);
	this->lvHeaders->SetShowGrid(true);
	this->lvHeaders->SetFullRowSelect(true);
	this->lvHeaders->AddColumn((const UTF8Char*)"Header", 1000);

	this->tpCookie = this->tcMain->AddTabPage((const UTF8Char*)"Cookie");
	NEW_CLASS(this->lvCookie, UI::GUIListView(ui, this->tpCookie, UI::GUIListView::LVSTYLE_TABLE, 4));
	this->lvCookie->SetDockType(UI::GUIControl::DOCK_FILL);
	this->lvCookie->SetShowGrid(true);
	this->lvCookie->SetFullRowSelect(true);
	this->lvCookie->AddColumn((const UTF8Char*)"Domain", 150);
	this->lvCookie->AddColumn((const UTF8Char*)"Path", 150);
	this->lvCookie->AddColumn((const UTF8Char*)"Name", 150);
	this->lvCookie->AddColumn((const UTF8Char*)"Value", 350);

	this->SetDefaultButton(this->btnRequest);
	this->txtURL->Focus();
	Sync::Thread::Create(ProcessThread, this, 0);
	this->AddTimer(100, OnTimerTick, this);
}

SSWR::AVIRead::AVIRHTTPClientForm::~AVIRHTTPClientForm()
{
	this->threadToStop = true;
	this->threadEvt->Set();
	while (this->threadRunning)
	{
		Sync::Thread::Sleep(1);
	}
	DEL_CLASS(this->threadEvt);
	this->ClearHeaders();
	DEL_CLASS(this->respHeaders);
	this->ClearParams();
	DEL_CLASS(this->params);
	this->ClearCookie();
	DEL_CLASS(this->cookieList);
	DEL_CLASS(this->cookieMut);
	this->ClearFiles();
	DEL_CLASS(this->fileList);
	SDEL_TEXT(this->respReqURL);
	SDEL_TEXT(this->respContType);
	SDEL_CLASS(this->respData);
	DEL_CLASS(this->respMut);
	Text::StrDelNew(this->userAgent);
}

void SSWR::AVIRead::AVIRHTTPClientForm::OnMonitorChanged()
{
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));
}
