#include "Stdafx.h"
#include "MyMemory.h"
#include "Core/Core.h"
#include "Exporter/GUIJPGExporter.h"
#include "IO/ConsoleWriter.h"
#include "IO/FileStream.h"
#include "Media/ImageList.h"
#include "Media/StaticImage.h"

Int32 MyMain(Core::IProgControl *progCtrl)
{
	const UTF8Char *fileName = (const UTF8Char*)"export.jpg";
	IO::FileStream *fs;
	Exporter::GUIJPGExporter exporter;
	Media::StaticImage *simg;
	Media::ImageList *imgList;

	Media::ColorProfile color(Media::ColorProfile::CPT_SRGB);
	NEW_CLASS(simg, Media::StaticImage(320, 240, 0, 32, Media::PF_B8G8R8A8, 320 * 240 * 4, &color, Media::ColorProfile::YUVT_BT601, Media::AT_NO_ALPHA, Media::YCOFST_C_CENTER_LEFT));
	NEW_CLASS(imgList, Media::ImageList((const UTF8Char*)"Temp"));
	imgList->AddImage(simg, 0);
	NEW_CLASS(fs, IO::FileStream(fileName, IO::FileStream::FILE_MODE_CREATE, IO::FileStream::FILE_SHARE_DENY_NONE, IO::FileStream::BT_NORMAL));
	exporter.ExportFile(fs, fileName, imgList, 0);
	DEL_CLASS(fs);
	DEL_CLASS(imgList);
	return 0;
}
