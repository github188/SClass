#include "Stdafx.h"
#include "MyMemory.h"
#include "Core/Core.h"
#include "IO/Path.h"
#include "IO/StmData/FileData.h"
#include "Manage/ExceptionRecorder.h"
#if 1
#include "Media/Decoder/FFMPEGDecoder.h"
#define USE_FFMPEG
#endif
#if 0 && defined(WIN32) && defined(CPU_X86_64)
//#include "Media/Decoder/IMSDKDecoder.h"
#define USE_IMSDK
#endif
#if 0 && !defined(WIN32)
//#include "Media/Decoder/VPXDecoder.h"
#define USE_VPX
#endif
#include "SSWR/AVIRead/AVIRCoreWin.h"
#include "SSWR/AVIRead/AVIRBaseForm.h"

Int32 MyAdd(Int32 a, Int32 b)
{
	Int32 tmp;
	while (b != 0)
	{
		tmp = a & b;
		a = a ^ b;
		b = tmp << 1;
	}
	return a;
}

Int32 MyMain(Core::IProgControl *progCtrl)
{
	UI::GUICore *ui;
	SSWR::AVIRead::AVIRBaseForm *frm;
	SSWR::AVIRead::AVIRCore *core;
	Manage::ExceptionRecorder *exHdlr;
	UTF8Char sbuff[512];
	IO::StmData::FileData *fd;
	UOSInt argc;
	UOSInt i;
	UTF8Char **argv;

//	MemSetBreakPoint(0x014746E8);
	MemSetLogFile((const UTF8Char*)"Memory.log");
#ifdef USE_FFMPEG
	Media::Decoder::FFMPEGDecoder::Enable();
#endif
#ifdef USE_IMSDK
	Media::Decoder::IMSDKDecoder::Enable();
#endif
#ifdef USE_VPX
	Media::Decoder::VPXDecoder::Enable();
#endif

	IO::Path::GetProcessFileName(sbuff);
	IO::Path::ReplaceExt(sbuff, (const UTF8Char*)"log");
	NEW_CLASS(exHdlr, Manage::ExceptionRecorder(sbuff, Manage::ExceptionRecorder::EA_CLOSE));
	ui = Core::IProgControl::CreateGUICore(progCtrl);
	NEW_CLASS(core, SSWR::AVIRead::AVIRCoreWin(ui));
	NEW_CLASS(frm, SSWR::AVIRead::AVIRBaseForm(0, ui, core));
	frm->SetExitOnClose(true);
	frm->Show();
	argv = progCtrl->GetCommandLines(progCtrl, &argc);
	if (argc > 1)
	{
		core->BeginLoad();
		i = 1;
		while (i < argc)
		{
			NEW_CLASS(fd, IO::StmData::FileData(argv[i], false));
			core->LoadData(fd, 0);
			DEL_CLASS(fd);

			i++;
		}
		core->EndLoad();
	}
	ui->Run();

	DEL_CLASS(core);
	DEL_CLASS(ui);
	DEL_CLASS(exHdlr);
	return 0;
}
