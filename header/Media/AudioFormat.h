#ifndef _SM_MEDIA_AUDIOFORMAT
#define _SM_MEDIA_AUDIOFORMAT
#include "Text/StringBuilderUTF8.h"

namespace Media
{
	class AudioFormat
	{
	public:
		typedef enum
		{
			IT_NORMAL,
			IT_BIGENDIAN,
			IT_BIGENDIAN16
		} IntType;
	public:
		UInt32 formatId;
		UInt32 frequency;
		UInt16 bitpersample;
		UInt16 nChannels;
		UInt32 bitRate;
		UInt32 align;
		UInt32 other;
		IntType intType;
		UInt32 extraSize;
		UInt8 *extra;

		AudioFormat();
		~AudioFormat();
		void FromWAVEFORMATEX(UInt8 *fmt);
		void Clear();
		void FromAudioFormat(Media::AudioFormat *fmt);
		void ToString(Text::StringBuilderUTF *sb);

		static const UTF8Char *GetNameFormatId(UInt32 formatId);
		static const UTF8Char *GetNameIntType(IntType intType);
	};
};
#endif
