#include "Stdafx.h"
#include "SSWR/AVIRead/AVIRGISGroupQueryForm.h"
#include "Text/MyString.h"
#include "Text/MyStringFloat.h"

Bool __stdcall SSWR::AVIRead::AVIRGISGroupQueryForm::OnMouseDown(void *userObj, OSInt x, OSInt y)
{
	SSWR::AVIRead::AVIRGISGroupQueryForm *me = (SSWR::AVIRead::AVIRGISGroupQueryForm*)userObj;
	me->downX = x;
	me->downY = y;
	return false;
}

Bool __stdcall SSWR::AVIRead::AVIRGISGroupQueryForm::OnMouseUp(void *userObj, OSInt x, OSInt y)
{
	SSWR::AVIRead::AVIRGISGroupQueryForm *me = (SSWR::AVIRead::AVIRGISGroupQueryForm*)userObj;
	if (me->downX == x && me->downY == y)
	{
		Double mapX;
		Double mapY;
		Double ptNearX = 0;
		Double ptNearY = 0;
		UOSInt ptNearInd = 0;
		Double ptNearDist = 0;
		Int64 ptNearId = -1;
		UOSInt pgNearInd = 0;
		Int64 pgNearId = -1;
		Double nearX;
		Double nearY;
		void *sess;
		Int64 id;
		UOSInt i;
		UOSInt j;
		UTF8Char sbuff[512];
		Data::ArrayList<Map::IMapDrawLayer*> layers;
		Map::IMapDrawLayer *lyr;
		me->navi->ScnXY2MapXY(x, y, &mapX, &mapY);
		me->env->GetLayersInGroup(me->group, &layers);
		i = layers.GetCount();
		while (i-- > 0)
		{
			lyr = layers.GetItem(i);
			sess = lyr->BeginGetObject();
			nearX = 0;
			nearY = 0;
			id = lyr->GetNearestObjectId(sess, mapX, mapY, &nearX, &nearY);
			if (id != -1)
			{
				Map::DrawLayerType lyrType = lyr->GetLayerType();
				if (lyrType == Map::DRAW_LAYER_POINT3D || lyrType == Map::DRAW_LAYER_POINT || lyrType == Map::DRAW_LAYER_POLYLINE || lyrType == Map::DRAW_LAYER_POLYLINE3D)
				{
					Double dist = (nearX - mapX) * (nearX - mapX) + (nearY - mapY) * (nearY - mapY);
					if (ptNearId == -1 || ptNearDist > dist)
					{
						ptNearId = id;
						ptNearInd = i;
						ptNearDist = dist;
						ptNearX = nearX;
						ptNearY = nearY;
					}
				}
				else if (mapX == nearX && mapY == nearY)
				{
					if (pgNearId == -1)
					{
						pgNearInd = i;
						pgNearId = id;
					}
				}
			}
			lyr->EndGetObject(sess);
		}

		lyr = 0;
		id = -1;
		if (pgNearId != -1)
		{
			id = pgNearId;
			lyr = layers.GetItem(pgNearInd);
		}
		if (ptNearId != -1)
		{
			OSInt nearScnX;
			OSInt nearScnY;
			me->navi->MapXY2ScnXY(ptNearX, ptNearY, &nearScnX, &nearScnY);
			if (nearScnX < x)
			{
				nearScnX = x - nearScnX;
			}
			else
			{
				nearScnX = nearScnX - x;
			}
			if (nearScnY < y)
			{
				nearScnY = y - nearScnY;
			}
			else
			{
				nearScnY = nearScnY - y;
			}
			if (nearScnX < 10 && nearScnY < 10)
			{
				id = ptNearId;
				lyr = layers.GetItem(ptNearInd);
			}
		}

		me->lvInfo->ClearItems();
		if (id == -1)
		{
			me->txtLayer->SetText((const UTF8Char*)"");
			me->navi->SetSelectedVector(0);
		}
		else
		{
			Data::ArrayListInt64 arr;
			void *nameArr;
			me->txtLayer->SetText(lyr->GetName());
			sess = lyr->BeginGetObject();
			lyr->GetObjectIdsMapXY(&arr, &nameArr, mapX, mapY, mapX, mapY, true);
			i = 0;
			j = lyr->GetColumnCnt();
			while (i < j)
			{
				sbuff[0] = 0;
				lyr->GetColumnName(sbuff, i);
				me->lvInfo->AddItem(sbuff, 0);
				lyr->GetString(sbuff, sizeof(sbuff), nameArr, id, i);
				me->lvInfo->SetSubItem(i, 1, sbuff);
				i++;
			}
			me->navi->SetSelectedVector(lyr->GetVectorById(sess, id));
			lyr->ReleaseNameArr(nameArr);
			lyr->EndGetObject(sess);
		}
	}
	return false;
}

SSWR::AVIRead::AVIRGISGroupQueryForm::AVIRGISGroupQueryForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core, IMapNavigator *navi, Map::MapEnv *env, Map::MapEnv::GroupItem *group) : UI::GUIForm(parent, 416, 408, ui)
{
	Text::StringBuilderUTF8 sb;
	this->core = core;
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));
	this->navi = navi;
	this->env = env;
	this->group = group;
	sb.Append((const UTF8Char*)"Group Query - ");
	if (group == 0)
	{
		sb.Append((const UTF8Char*)"ROOT");
	}
	else
	{
		const UTF8Char *name = env->GetGroupName(group);
		if (name)
		{
			sb.Append(name);
		}
		else
		{
			sb.Append((const UTF8Char*)"Unknown");
		}
	}
	SetText(sb.ToString());
	SetFont(0, 8.25, false);

	NEW_CLASS(this->txtLayer, UI::GUITextBox(ui, this, (const UTF8Char*)""));
	this->txtLayer->SetRect(0, 0, 100, 23, false);
	this->txtLayer->SetReadOnly(true);
	this->txtLayer->SetDockType(UI::GUIControl::DOCK_TOP);
	NEW_CLASS(this->lvInfo, UI::GUIListView(ui, this, UI::GUIListView::LVSTYLE_TABLE, 2));
	this->lvInfo->SetDockType(UI::GUIControl::DOCK_FILL);
	this->lvInfo->AddColumn((const UTF8Char*)"Name", 100);
	this->lvInfo->AddColumn((const UTF8Char*)"Value", 300);
	this->lvInfo->SetShowGrid(true);
	this->lvInfo->SetFullRowSelect(true);

	this->navi->HandleMapMouseDown(OnMouseDown, this);
	this->navi->HandleMapMouseUp(OnMouseUp, this);
}

SSWR::AVIRead::AVIRGISGroupQueryForm::~AVIRGISGroupQueryForm()
{
	this->navi->UnhandleMapMouse(this);
}

void SSWR::AVIRead::AVIRGISGroupQueryForm::OnMonitorChanged()
{
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));
}
