#ifndef _SM_MEDIA_BATCH_BATCHSAVEJPEGSIZE
#define _SM_MEDIA_BATCH_BATCHSAVEJPEGSIZE
#include "Exporter/GUIJPGExporter.h"
#include "Media/Batch/BatchSaver.h"
#include "Sync/Mutex.h"
namespace Media
{
	namespace Batch
	{
		class BatchSaveJPEGSize : public BatchSaver
		{
		private:
			Exporter::GUIJPGExporter *exporter;
			Sync::Mutex *mut;
			Int32 sizePercent;

		public:
			BatchSaveJPEGSize(Int32 sizePercent);
			virtual ~BatchSaveJPEGSize();
			
			virtual void ImageOutput(Media::ImageList *imgList, const UTF8Char *fileId, const UTF8Char *targetId);
		};
	}
}
#endif