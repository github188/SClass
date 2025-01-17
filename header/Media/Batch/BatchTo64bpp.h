#ifndef _SM_MEDIA_BATCH_BATCHTO64BPP
#define _SM_MEDIA_BATCH_BATCHTO64BPP
#include "Media/Batch/BatchHandler.h"

namespace Media
{
	namespace Batch
	{
		class BatchTo64bpp : public BatchHandler
		{
		private:
			Media::Batch::BatchHandler *hdlr;

		public:
			BatchTo64bpp(Media::Batch::BatchHandler *hdlr);
			virtual ~BatchTo64bpp();
			
			void SetHandler(Media::Batch::BatchHandler *hdlr);
		private:
			virtual void ImageOutput(Media::ImageList *imgList, const UTF8Char *fileId, const UTF8Char *subId);
		};
	};
};
#endif
