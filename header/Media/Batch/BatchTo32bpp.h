#ifndef _SM_MEDIA_BATCH_BATCHTO32BPP
#define _SM_MEDIA_BATCH_BATCHTO32BPP
#include "Media/Batch/BatchHandler.h"

namespace Media
{
	namespace Batch
	{
		class BatchTo32bpp : public BatchHandler
		{
		private:
			Media::Batch::BatchHandler *hdlr;

		public:
			BatchTo32bpp(Media::Batch::BatchHandler *hdlr);
			virtual ~BatchTo32bpp();
			
			void SetHandler(Media::Batch::BatchHandler *hdlr);
		private:
			virtual void ImageOutput(Media::ImageList *imgList, const UTF8Char *fileId, const UTF8Char *subId);
		};
	};
};
#endif
