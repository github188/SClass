#ifndef _SM_MEDIA_RESIZER_LANCZOSRESIZERH8_8
#define _SM_MEDIA_RESIZER_LANCZOSRESIZERH8_8
#include "Media/IImgResizer.h"
#include "Sync/Event.h"
#include "Sync/Mutex.h"
#include "Sync/ParallelTask.h"

namespace Media
{
	namespace Resizer
	{
		class LanczosResizerH8_8 : public Media::IImgResizer
		{
		private:
			typedef struct
			{
				LanczosResizerH8_8 *me;
				Int32 funcType; // 3 = h filter, 5 = v filter, 7 = expand, 9 = collapse, 11 = copying, 13 = h filter
				UInt8 *inPt;
				UInt8 *outPt;
				UOSInt width;
				UOSInt height;
				UOSInt tap;
				OSInt *index;
				Int64 *weight;
				OSInt sstep;
				OSInt dstep;
			} TaskParam;

			typedef struct
			{
				UOSInt length;
				Int64 *weight;
				OSInt *index;
				UOSInt tap;
			} LRHPARAMETER;

		private:
			UOSInt hnTap;
			UOSInt vnTap;
			UOSInt nThread;
			Sync::Mutex *mut;
			TaskParam *params;
			Sync::ParallelTask *ptask;

			Double hsSize;
			Double hsOfst;
			UOSInt hdSize;
			OSInt *hIndex;
			Int64 *hWeight;
			UOSInt hTap;

			Double vsSize;
			Double vsOfst;
			UOSInt vdSize;
			OSInt vsStep;
			OSInt *vIndex;
			Int64 *vWeight;
			UOSInt vTap;

			UOSInt buffW;
			UOSInt buffH;
			UInt8 *buffPtr;

			Double hTime;
			Double vTime;

			void setup_interpolation_parameter_v(UOSInt nTap, Double source_length, UOSInt source_max_pos, UOSInt result_length, LRHPARAMETER *out, OSInt indexSep, Double offsetCorr);
			void setup_decimation_parameter_v(UOSInt nTap, Double source_length, UOSInt source_max_pos, UOSInt result_length, LRHPARAMETER *out, OSInt indexSep, Double offsetCorr);
			void setup_interpolation_parameter_h(UOSInt nTap, Double source_length, UOSInt source_max_pos, UOSInt result_length, LRHPARAMETER *out, OSInt indexSep, Double offsetCorr);
			void setup_decimation_parameter_h(UOSInt nTap, Double source_length, UOSInt source_max_pos, UOSInt result_length, LRHPARAMETER *out, OSInt indexSep, Double offsetCorr);

			void mt_horizontal_filter(UInt8 *inPt, UInt8 *outPt, UOSInt width, UOSInt height, UOSInt tap, OSInt *index, Int64 *weight, OSInt sstep, OSInt dstep);
			void mt_horizontal_filter8(UInt8 *inPt, UInt8 *outPt, UOSInt width, UOSInt height, UOSInt tap, OSInt *index, Int64 *weight, OSInt sstep, OSInt dstep);
			void mt_vertical_filter(UInt8 *inPt, UInt8 *outPt, UOSInt width, UOSInt height, UOSInt tap, OSInt *index, Int64 *weight, OSInt sstep, OSInt dstep);
			void mt_expand(UInt8 *inPt, UInt8 *outPt, UOSInt width, UOSInt height, OSInt sstep, OSInt dstep);
			void mt_collapse(UInt8 *inPt, UInt8 *outPt, UOSInt width, UOSInt height, OSInt sstep, OSInt dstep);
			void mt_copy(UInt8 *inPt, UInt8 *outPt, UOSInt width, UOSInt height, OSInt sstep, OSInt dstep);

			static void __stdcall DoTask(void *obj);
			void DestoryHori();
			void DestoryVert();
		public:
			LanczosResizerH8_8(UOSInt hnTap, UOSInt vnTap, Media::AlphaType srcAlphaType);
			virtual ~LanczosResizerH8_8();

			virtual void Resize(UInt8 *src, OSInt sbpl, Double swidth, Double sheight, Double xOfst, Double yOfst, UInt8 *dest, OSInt dbpl, UOSInt dwidth, UOSInt dheight);
			virtual Bool Resize(Media::StaticImage *srcImg, Media::StaticImage *destImg);

			virtual Bool IsSupported(Media::FrameInfo *srcInfo);
			virtual Media::StaticImage *ProcessToNewPartial(Media::StaticImage *srcImage, Double srcX1, Double srcY1, Double srcX2, Double srcY2);

			Double GetHAvgTime();
			Double GetVAvgTime();
		};
	}
}
#endif
