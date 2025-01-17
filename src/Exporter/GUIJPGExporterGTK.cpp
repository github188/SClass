#include "Stdafx.h"
#include "Data/ByteTool.h"
#include "Exporter/GUIJPGExporter.h"
#include "IO/MemoryStream.h"
#include "Media/ImageList.h"
#include "Text/MyString.h"
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <wchar.h>

Exporter::GUIJPGExporter::GUIJPGExporter() : Exporter::GUIExporter()
{
}

Exporter::GUIJPGExporter::~GUIJPGExporter()
{
}

Int32 Exporter::GUIJPGExporter::GetName()
{
	return *(Int32*)"GPJP";
}

Bool Exporter::GUIJPGExporter::GetOutputName(UOSInt index, UTF8Char *nameBuff, UTF8Char *fileNameBuff)
{
	if (index == 0)
	{
		Text::StrConcat(nameBuff, (const UTF8Char*)"JPEG file (GDK)");
		Text::StrConcat(fileNameBuff, (const UTF8Char*)"*.jpg");
		return true;
	}
	return false;
}

Bool Exporter::GUIJPGExporter::ExportFile(IO::SeekableStream *stm, const UTF8Char *fileName, IO::ParsedObject *pobj, void *param)
{
	UInt8 *tmpBuff;
	GdkPixbuf *image = (GdkPixbuf*)ToImage(pobj, &tmpBuff);
	if (image == 0)
	{
		return false;
	}
	Char cbuff[32];
	gchar *buff = 0;
	gsize buffSize;

	if (param)
	{
		Text::StrInt32(cbuff, *(Int32*)param);
		gdk_pixbuf_save_to_buffer(image, &buff, &buffSize, "jpeg", 0, "quality", cbuff, (void*)0);
	}
	else
	{
		gdk_pixbuf_save_to_buffer(image, &buff, &buffSize, "jpeg", 0, (void*)0);
	}
	g_object_unref(image);
	if (tmpBuff)
	{
		MemFreeA(tmpBuff);
	}

	if (buff)
	{
		Media::Image *srcImg = 0;
		Media::ImageList *imgList;
		UInt8 *jpgBuff;
		UOSInt jpgSize;
		if (pobj->GetParserType() == IO::ParsedObject::PT_IMAGE_LIST_PARSER)
		{
			imgList = (Media::ImageList*)pobj;
			srcImg = imgList->GetImage(0, 0);
		}
		jpgBuff = (UInt8*)buff;
		jpgSize = buffSize;
		if (srcImg != 0 && srcImg->exif != 0 && jpgBuff[0] == 0xff && jpgBuff[1] == 0xd8)
		{
			UOSInt i;
			UOSInt j;
			Media::EXIFData *exif = srcImg->exif->Clone();
			exif->Remove(254); //NewSubfileType
			exif->Remove(256); //Width
			exif->Remove(257); //Height
			exif->Remove(258); //BitPerSample
			exif->Remove(259); //Compression
			exif->Remove(262); //PhotometricInterpretation
			exif->Remove(273); //StripOffsets
			exif->Remove(277); //SamplePerPixel
			exif->Remove(278); //RowsPerStrip
			exif->Remove(279); //StripByteCounts
			exif->Remove(284); //PlanarConfiguration
			exif->Remove(700); //Photoshop XMP
			exif->Remove(33723); //IPTC/NAA
			exif->Remove(34377); //PhotoshopImageResources
			exif->Remove(34675); //ICC Profile
			i = 2;
			stm->Write(jpgBuff, 2);
			while (true)
			{
				if (i >= jpgSize)
				{
					break;
				}
				if (jpgBuff[i] != 0xff)
				{
					stm->Write(&jpgBuff[i], jpgSize - i);
					break;
				}
				if (jpgBuff[i + 1] == 0xdb)
				{
					const UInt8 *iccBuff = srcImg->info->color->GetRAWICC();
					if (iccBuff)
					{
						UOSInt iccLeng = ReadMUInt32(iccBuff);
						UInt8 iccHdr[18];
						iccHdr[0] = 0xff;
						iccHdr[1] = 0xe2;
						WriteMInt16(&iccHdr[2], iccLeng + 16);
						Text::StrConcat((Char*)&iccHdr[4], "ICC_PROFILE");
						iccHdr[16] = 1;
						iccHdr[17] = 1;
						stm->Write(iccHdr, 18);
						stm->Write(iccBuff, iccLeng);
					}
					/////////////////////////////////////


					UInt32 exifSize;
					UInt32 endOfst;
					UInt32 k;
					UInt32 l;

					UInt8 *exifBuff;
					exif->GetExifBuffSize(&exifSize, &endOfst);
					exifBuff = MemAlloc(UInt8, exifSize + 18);
					exifBuff[0] = 0xff;
					exifBuff[1] = 0xe1;
					WriteMInt16(&exifBuff[2], exifSize + 16);
					WriteInt32(&exifBuff[4], ReadInt32("Exif"));
					WriteInt16(&exifBuff[8], 0);
					WriteInt16(&exifBuff[10], ReadInt16("II"));
					WriteInt16(&exifBuff[12], 42);
					WriteInt32(&exifBuff[14], 8);
					k = 8;
					l = endOfst + 8;
					exif->ToExifBuff(&exifBuff[10], &k, &l);
					stm->Write(exifBuff, exifSize + 18);
					MemFree(exifBuff);

					stm->Write(&jpgBuff[i], jpgSize - i);
					break;
				}
				else if (jpgBuff[i + 1] == 0xe1)
				{
					i += (UOSInt)ReadMUInt16(&jpgBuff[i + 2]) + 2;
				}
				else
				{
					j = (UOSInt)ReadMUInt16(&jpgBuff[i + 2]) + 2;
					stm->Write(&jpgBuff[i], j);
					i += j;
				}
			}
			DEL_CLASS(exif);
		}
		else
		{
			stm->Write(jpgBuff, jpgSize);
		}
		g_free(buff);
		return true;
	}
	else
	{
	}
	return false;
}

UOSInt Exporter::GUIJPGExporter::GetParamCnt()
{
	return 1;
}

void *Exporter::GUIJPGExporter::CreateParam(IO::ParsedObject *pobj)
{
	Int32 *val = MemAlloc(Int32, 1);
	*val = 100;
	return val;
}

void Exporter::GUIJPGExporter::DeleteParam(void *param)
{
	MemFree(param);
}

Bool Exporter::GUIJPGExporter::GetParamInfo(UOSInt index, ParamInfo *info)
{
	if (index == 0)
	{
		info->name = (const UTF8Char*)"Quality";
		info->paramType = IO::FileExporter::PT_INT32;
		info->allowNull = false;
		return true;
	}
	return false;
}

Bool Exporter::GUIJPGExporter::SetParamInt32(void *param, UOSInt index, Int32 val)
{
	if (index == 0)
	{
		if (val >= 0 && val <= 100)
		{
			*(Int32*)param = val;
			return true;
		}
		return false;
	}
	return false;
}

Int32 Exporter::GUIJPGExporter::GetParamInt32(void *param, UOSInt index)
{
	if (index == 0)
	{
		return *(Int32*)param;
	}
	return 0;
}
