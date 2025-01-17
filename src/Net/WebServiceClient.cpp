#include "Stdafx.h"
#include "MyMemory.h"
#include "IO/MemoryStream.h"
#include "Net/HTTPClient.h"
#include "Net/WebServiceClient.h"
#include "Text/MyString.h"
#include "Text/StringBuilderUTF8.h"
#include "Text/XML.h"
#include "Text/XMLDOM.h"
#include "Text/UTF8Writer.h"
#include "Text/TextEnc/FormEncoding.h"

Net::WebServiceClient::WebServiceClient(Net::SocketFactory *sockf, const UTF8Char *serviceAddr, const UTF8Char *serviceName, const UTF8Char *targetNS)
{
	this->sockf = sockf;
	this->serviceAddr = Text::StrCopyNew(serviceAddr);
	this->serviceName = Text::StrCopyNew(serviceName);
	this->targetNS = Text::StrCopyNew(targetNS);
	this->soapAction = 0;
	NEW_CLASS(this->paramList, Data::ArrayList<ParamInfo*>());
	this->responseVal = 0;
}

Net::WebServiceClient::~WebServiceClient()
{
	Text::StrDelNew(this->serviceAddr);
	Text::StrDelNew(this->serviceName);
	Text::StrDelNew(this->targetNS);
	OSInt i;
	ParamInfo *param;
	i = this->paramList->GetCount();
	while (i-- > 0)
	{
		param = this->paramList->GetItem(i);
		Text::StrDelNew(param->name);
		Text::StrDelNew(param->val);
		MemFree(param);
	}
	SDEL_TEXT(this->soapAction);
	DEL_CLASS(this->paramList);
	SDEL_TEXT(this->responseVal);
}

void Net::WebServiceClient::AddParam(const UTF8Char *paramName, const UTF8Char *paramVal)
{
	ParamInfo *param;
	param = MemAlloc(ParamInfo, 1);
	param->name = Text::StrCopyNew(paramName);
	param->val = Text::StrCopyNew(paramVal);
	this->paramList->Add(param);
}

void Net::WebServiceClient::SetSOAPAction(const UTF8Char *soapAction)
{
	SDEL_TEXT(this->soapAction);
	if (soapAction)
	{
		this->soapAction = Text::StrCopyNew(soapAction);
	}
}

Bool Net::WebServiceClient::Request(RequestType rt)
{
	const UTF8Char *csptr;
	UOSInt i;
	UOSInt j;
	ParamInfo *param;
	UInt8 *buff;
	UTF8Char sbuff[512];
	UTF8Char *sptr;

	if (Text::StrCharCnt(this->serviceName) > 64)
		return false;
	if (rt == RT_SOAP11)
	{
		Bool succ;
		Net::HTTPClient *cli;
		IO::MemoryStream *mstm;
		Text::UTF8Writer *writer;
		NEW_CLASS(mstm, IO::MemoryStream((const UTF8Char*)"Net.WebServiceClient.Request.S11"));
		NEW_CLASS(writer, Text::UTF8Writer(mstm));
		writer->Write((const UTF8Char*)"<?xml version=\"1.0\" encoding=\"utf-8\"?>");
		writer->Write((const UTF8Char*)"<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">");
		writer->Write((const UTF8Char*)"<soap:Body>");
		writer->Write((const UTF8Char*)"<");
		csptr = Text::XML::ToNewXMLText(this->serviceName);
		writer->Write(csptr);
		Text::XML::FreeNewText(csptr);
		writer->Write((const UTF8Char*)" xmlns=");
		csptr = Text::XML::ToNewAttrText(this->targetNS);
		writer->Write(csptr);
		Text::XML::FreeNewText(csptr);
		writer->Write((const UTF8Char*)">");

		i = 0;
		j = this->paramList->GetCount();
		while (i < j)
		{
			param = this->paramList->GetItem(i);
			writer->Write((const UTF8Char*)"<");
			csptr = Text::XML::ToNewXMLText(param->name);
			writer->Write(csptr);
			Text::XML::FreeNewText(csptr);
			writer->Write((const UTF8Char*)">");

			csptr = Text::XML::ToNewXMLTextLite(param->val);
			writer->Write(csptr);
			Text::XML::FreeNewText(csptr);
			
			writer->Write((const UTF8Char*)"</");
			csptr = Text::XML::ToNewXMLText(param->name);
			writer->Write(csptr);
			Text::XML::FreeNewText(csptr);
			writer->Write((const UTF8Char*)">");
			i++;
		}
		writer->Write((const UTF8Char*)"</");
		csptr = Text::XML::ToNewXMLText(this->serviceName);
		writer->Write(csptr);
		Text::XML::FreeNewText(csptr);
		writer->Write((const UTF8Char*)">");
		writer->Write((const UTF8Char*)"</soap:Body>");
		writer->Write((const UTF8Char*)"</soap:Envelope>");
		DEL_CLASS(writer);

		cli = Net::HTTPClient::CreateConnect(sockf, this->serviceAddr, "POST", false);
		if (this->soapAction == 0)
		{
			cli->AddHeader((const UTF8Char*)"SOAPAction", (const UTF8Char*)"\"\"");
		}
		else
		{
			sbuff[0] = '\"';
			sptr = Text::TextEnc::FormEncoding::FormEncode(&sbuff[1], this->soapAction);
			*sptr++ = '\"';
			*sptr = 0;
			cli->AddHeader((const UTF8Char*)"SOAPAction", sbuff);
		}
		cli->AddHeader((const UTF8Char*)"Content-Type", (const UTF8Char*)"text/xml; charset=utf-8");
		buff = mstm->GetBuff(&i);
		Text::StrUOSInt(sbuff, i);
		cli->AddHeader((const UTF8Char*)"Content-Length", sbuff);

		cli->Write(buff, i);

		succ = false;
		Int32 status = cli->GetRespStatus();
		if (status == 200)
		{
			UInt64 contLeng = cli->GetContentLength();
			if (contLeng > 0 && contLeng < 1048576)
			{
				buff = MemAlloc(UInt8, contLeng);
				i = 0;
				while (i < contLeng)
				{
					j = cli->Read(&buff[i], (OSInt)contLeng - i);
					if (j == 0)
						break;
					i += j;
				}
				if (i == contLeng)
				{
					Text::EncodingFactory encFact;
					Text::XMLDocument *doc;
					Text::XMLNode *node1;
					Text::XMLNode *node2;
					Text::StringBuilderUTF8 sb;
					NEW_CLASS(doc, Text::XMLDocument());
					if (doc->ParseBuff(&encFact, buff, contLeng))
					{
						i = doc->GetChildCnt();
						while (i-- > 0)
						{
							node1 = doc->GetChild(i);
							if (node1->GetNodeType() == Text::XMLNode::NT_ELEMENT && Text::StrEquals(node1->name, (const UTF8Char*)"soap:Envelope"))
							{
								i = node1->GetChildCnt();
								while (i-- > 0)
								{
									node2 = node1->GetChild(i);
									if (node2->GetNodeType() == Text::XMLNode::NT_ELEMENT && Text::StrEquals(node2->name, (const UTF8Char*)"soap:Body"))
									{
										node1 = node2;
										Text::StrConcat(Text::StrConcat(sbuff, this->serviceName), (const UTF8Char*)"Response");
										i = node1->GetChildCnt();
										while (i-- > 0)
										{
											node2 = node1->GetChild(i);
											if (node2->GetNodeType() == Text::XMLNode::NT_ELEMENT && Text::StrEquals(node2->name, sbuff))
											{
												node1 = node2;
												Text::StrConcat(Text::StrConcat(sbuff, this->serviceName), (const UTF8Char*)"Result");
												i = node1->GetChildCnt();
												while (i-- > 0)
												{
													node2 = node1->GetChild(i);
													if (node2->GetNodeType() == Text::XMLNode::NT_ELEMENT && Text::StrEquals(node2->name, sbuff))
													{
														sb.ClearStr();
														node2->GetInnerText(&sb);
														this->responseVal = Text::StrCopyNew(sb.ToString());
														succ = true;
														break;
													}
												}
												break;
											}
										}

										break;
									}
								}
								break;
							}
						}
					}
					DEL_CLASS(doc);
				}
				MemFree(buff);
			}
		}
		DEL_CLASS(mstm);
		DEL_CLASS(cli);
		return succ;
	}
	else if (rt == RT_SOAP12)
	{
		Bool succ;
		Net::HTTPClient *cli;
		IO::MemoryStream *mstm;
		Text::UTF8Writer *writer;
		NEW_CLASS(mstm, IO::MemoryStream((const UTF8Char*)"Net.WebServiceClient.Request.S11"));
		NEW_CLASS(writer, Text::UTF8Writer(mstm));
		writer->Write((const UTF8Char*)"<?xml version=\"1.0\" encoding=\"utf-8\"?>");
		writer->Write((const UTF8Char*)"<soap12:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap12=\"http://www.w3.org/2003/05/soap-envelope\">");
		writer->Write((const UTF8Char*)"<soap12:Body>");
		writer->Write((const UTF8Char*)"<");
		csptr = Text::XML::ToNewXMLText(this->serviceName);
		writer->Write(csptr);
		Text::XML::FreeNewText(csptr);
		writer->Write((const UTF8Char*)" xmlns=");
		csptr = Text::XML::ToNewAttrText(this->targetNS);
		writer->Write(csptr);
		Text::XML::FreeNewText(csptr);
		writer->Write((const UTF8Char*)">");

		i = 0;
		j = this->paramList->GetCount();
		while (i < j)
		{
			param = this->paramList->GetItem(i);
			writer->Write((const UTF8Char*)"<");
			csptr = Text::XML::ToNewXMLText(param->name);
			writer->Write(csptr);
			Text::XML::FreeNewText(csptr);
			writer->Write((const UTF8Char*)">");

			csptr = Text::XML::ToNewXMLTextLite(param->val);
			writer->Write(csptr);
			Text::XML::FreeNewText(csptr);
			
			writer->Write((const UTF8Char*)"</");
			csptr = Text::XML::ToNewXMLText(param->name);
			writer->Write(csptr);
			Text::XML::FreeNewText(csptr);
			writer->Write((const UTF8Char*)">");
			i++;
		}
		writer->Write((const UTF8Char*)"</");
		csptr = Text::XML::ToNewXMLText(this->serviceName);
		writer->Write(csptr);
		Text::XML::FreeNewText(csptr);
		writer->Write((const UTF8Char*)">");
		writer->Write((const UTF8Char*)"</soap12:Body>");
		writer->Write((const UTF8Char*)"</soap12:Envelope>");
		DEL_CLASS(writer);

		cli = Net::HTTPClient::CreateConnect(sockf, this->serviceAddr, "POST", false);
		cli->AddHeader((const UTF8Char*)"Content-Type", (const UTF8Char*)"application/soap+xml; charset=utf-8");
		buff = mstm->GetBuff(&i);
		Text::StrOSInt(sbuff, i);
		cli->AddHeader((const UTF8Char*)"Content-Length", sbuff);

		cli->Write(buff, i);

		succ = false;
		Int32 status = cli->GetRespStatus();
		if (status == 200)
		{
			UInt64 contLeng = cli->GetContentLength();
			if (contLeng > 0 && contLeng < 1048576)
			{
				buff = MemAlloc(UInt8, contLeng);
				i = 0;
				while (i < contLeng)
				{
					j = cli->Read(&buff[i], (OSInt)contLeng - i);
					if (j == 0)
						break;
					i += j;
				}
				if (i == contLeng)
				{
					Text::EncodingFactory encFact;
					Text::XMLDocument *doc;
					Text::XMLNode *node1;
					Text::XMLNode *node2;
					Text::StringBuilderUTF8 sb;
					NEW_CLASS(doc, Text::XMLDocument());
					if (doc->ParseBuff(&encFact, buff, contLeng))
					{
						i = doc->GetChildCnt();
						while (i-- > 0)
						{
							node1 = doc->GetChild(i);
							if (node1->GetNodeType() == Text::XMLNode::NT_ELEMENT && Text::StrEndsWithICase(node1->name, (const UTF8Char*)":Envelope"))
							{
								i = node1->GetChildCnt();
								while (i-- > 0)
								{
									node2 = node1->GetChild(i);
									if (node2->GetNodeType() == Text::XMLNode::NT_ELEMENT && Text::StrEndsWithICase(node2->name, (const UTF8Char*)":Body"))
									{
										node1 = node2;
										Text::StrConcat(Text::StrConcat(sbuff, this->serviceName), (const UTF8Char*)"Response");
										i = node1->GetChildCnt();
										while (i-- > 0)
										{
											node2 = node1->GetChild(i);
											if (node2->GetNodeType() == Text::XMLNode::NT_ELEMENT && Text::StrEquals(node2->name, sbuff))
											{
												node1 = node2;
												Text::StrConcat(Text::StrConcat(sbuff, this->serviceName), (const UTF8Char*)"Result");
												i = node1->GetChildCnt();
												while (i-- > 0)
												{
													node2 = node1->GetChild(i);
													if (node2->GetNodeType() == Text::XMLNode::NT_ELEMENT && Text::StrEquals(node2->name, sbuff))
													{
														sb.ClearStr();
														node2->GetInnerText(&sb);
														this->responseVal = Text::StrCopyNew(sb.ToString());
														succ = true;
														break;
													}
												}
												break;
											}
										}

										break;
									}
								}
								break;
							}
						}
					}
					DEL_CLASS(doc);
				}
				MemFree(buff);
			}
		}
		DEL_CLASS(mstm);
		DEL_CLASS(cli);
		return succ;
	}
	/////////////////////////////////////////
	return false;
}

const UTF8Char *Net::WebServiceClient::GetResponseVal()
{
	return this->responseVal;
}
