#ifndef _SM_SSWR_ORGANMGR_ORGANPARENTITEM
#define _SM_SSWR_ORGANMGR_ORGANPARENTITEM

#include "SSWR/OrganMgr/OrganGroupItem.h"

namespace SSWR
{
	namespace OrganMgr
	{
		class OrganParentItem : public OrganGroupItem
		{
		public:
			OrganParentItem();
			virtual ~OrganParentItem();

			virtual ItemType GetItemType();
			virtual UTF8Char *GetItemName(UTF8Char *buff);
			virtual UTF8Char *GetEngName(UTF8Char *buff);
			virtual OrganGroupItem *Clone();
		};
	}
}
#endif
