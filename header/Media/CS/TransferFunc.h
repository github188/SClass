#ifndef _SM_MEDIA_CS_TRANSFERFUNC
#define _SM_MEDIA_CS_TRANSFERFUNC
#include "Media/CS/TransferParam.h"

namespace Media
{
	namespace CS
	{
		class TransferFunc
		{
		protected:
			TransferParam *param;
		public:
			TransferFunc(TransferType tranType, Double gamma);
			TransferFunc(Media::LUT *lut);
			virtual ~TransferFunc();

			virtual Double ForwardTransfer(Double linearVal) = 0;
			virtual Double InverseTransfer(Double gammaVal) = 0;
			TransferType GetTransferType();
			Double GetTransferGamma();
			const TransferParam *GetTransferParam();

			static TransferFunc *CreateFunc(TransferParam *param);
			static Double GetRefLuminance(TransferParam *param);
			static const UTF8Char *GetTransferFuncName(Media::CS::TransferType ttype);
		};
	};
};
#endif
