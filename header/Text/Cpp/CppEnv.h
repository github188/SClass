#ifndef _SM_TEXT_CPP_CPPENV
#define _SM_TEXT_CPP_CPPENV
#include "Data/ArrayListStrUTF8.h"
#include "IO/ConfigFile.h"
#include "Text/VSProject.h"
#include "Text/Cpp/CppCodeParser.h"

namespace Text
{
	namespace Cpp
	{
		class CppEnv
		{
		private:
			Data::ArrayListStrUTF8 *includePaths;
			Text::VSProject::VisualStudioVersion vsv;
			Text::CodeProject::ProjectType pt;
			const UTF8Char *baseFile;
			
			Bool InitVSEnv(Text::VSProject::VisualStudioVersion vsv);
		public:
			CppEnv(Text::VSProject::VisualStudioVersion vsv);
			CppEnv(Text::CodeProject *proj, IO::ConfigFile *cfg);
			~CppEnv();

			void AddIncludePath(const UTF8Char *includePath);
			UTF8Char *GetIncludeFilePath(UTF8Char *buff, const UTF8Char *includeFile, const UTF8Char *sourceFile);

			void InitEnvStatus(Text::Cpp::CppParseStatus *status);

			static CppEnv *LoadVSEnv(Text::VSProject::VisualStudioVersion vsv);
			static CppEnv *LoadVSEnv();
			static UTF8Char *GetVCInstallDir(UTF8Char *sbuff, Text::VSProject::VisualStudioVersion vsv);
			static UTF8Char *GetWindowsSdkDir(UTF8Char *sbuff);
			static Bool IsCompilerExist(Text::VSProject::VisualStudioVersion vsv);
//			static CompilerType GetSystemCompiler(WChar *includePath);
		};
	};
};
#endif
