#ifndef _SM_MEDIA_H264PARSER
#define _SM_MEDIA_H264PARSER
#include "Data/ArrayListInt32.h"
#include "IO/BitReaderMSB.h"
#include "Media/FrameInfo.h"

namespace Media
{
	class H264Parser
	{
	public:
		typedef struct
		{
			Bool pic_struct_present_flag;
			Bool nal_hrd_parameters_present_flag;
			Bool vcl_hrd_parameters_present_flag;
			Int32 initial_cpb_removal_delay_length_minus1;
			Int32 cpb_removal_delay_length_minus1;
			Int32 dpb_output_delay_length_minus1;
			Bool frameOnly;
			Bool mbaff;
			Bool separateColourPlane;
			Int32 maxFrameNum_4;
			Int32 frameRateNorm;
			Int32 frameRateDenorm;
		} H264Flags;
	private:
		static Bool ParseHRDParameters(IO::BitReaderMSB *reader, H264Flags *flags);
		static Bool ParseVUIParameters(IO::BitReaderMSB *reader, Media::FrameInfo *info, H264Flags *flags);
	public:
		static Bool GetFrameInfo(const UInt8 *frame, OSInt frameSize, Media::FrameInfo *info, H264Flags *flags); //Only update defined values
		static Bool ParseVari(IO::BitReaderMSB *reader, Int32 *val);
		static Bool ParseSVari(IO::BitReaderMSB *reader, Int32 *val);
		static Bool FindSPS(const UInt8 *frame, OSInt frameSize, const UInt8 **sps, OSInt *spsSize);
		static Bool FindPPS(const UInt8 *frame, OSInt frameSize, const UInt8 **pps, OSInt *ppsSize);
		static Bool FindNALs(const UInt8 *frame, OSInt frameSize, Data::ArrayListInt32 *nalList);
		static UTF8Char *GetFrameType(UTF8Char *sbuff, const UInt8 *frame, OSInt frameSize);
	};
};
#endif