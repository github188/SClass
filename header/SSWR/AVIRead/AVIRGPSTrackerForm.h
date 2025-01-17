#ifndef _SM_SSWR_AVIREAD_AVIRGPSTRACKERFORM
#define _SM_SSWR_AVIREAD_AVIRGPSTRACKERFORM
#include "IO/GPSNMEA.h"
#include "SSWR/AVIRead/AVIRCore.h"
#include "SSWR/AVIRead/IMapNavigator.h"
#include "UI/GUIButton.h"
#include "UI/GUICheckBox.h"
#include "UI/GUIForm.h"
#include "UI/GUIGroupBox.h"
#include "UI/GUIHSplitter.h"
#include "UI/GUILabel.h"
#include "UI/GUIListBox.h"
#include "UI/GUITabControl.h"
#include "UI/GUITabPage.h"
#include "UI/GUITextBox.h"

namespace SSWR
{
	namespace AVIRead
	{
		class AVIRGPSTrackerForm : public UI::GUIForm
		{
			class DisplayOffButton : public UI::GUIButton
			{
			private:
				AVIRGPSTrackerForm *frm;
			public:
				DisplayOffButton(UI::GUICore *ui, UI::GUIClientControl *parent, const UTF8Char *txt, AVIRGPSTrackerForm *frm);
				virtual ~DisplayOffButton();

				virtual void OnFocusLost();
			};
		private:
			SSWR::AVIRead::AVIRCore *core;
			Map::ILocationService *locSvc;
			Bool relLocSvc;

			Map::GPSTrack::GPSRecord recCurr;
			Map::GPSTrack *gpsTrk;
			Sync::Mutex *recMut;
			Bool recUpdated;
			SSWR::AVIRead::IMapNavigator *mapNavi;
			Math::GeographicCoordinateSystem *wgs84;
			Double lastLat;
			Double lastLon;
			Double dist;
			Data::DateTime *lastUpdateTime;
			Bool dispOffClk;
			Int64 dispOffTime;
			Bool dispIsOff;

			UI::GUITabControl *tcMain;

			UI::GUITabPage *tpLocation;
			UI::GUILabel *lblGPSTime;
			UI::GUITextBox *txtGPSTime;
			UI::GUILabel *lblLatitude;
			UI::GUITextBox *txtLatitude;
			UI::GUILabel *lblLongitude;
			UI::GUITextBox *txtLongitude;
			UI::GUILabel *lblAltitude;
			UI::GUITextBox *txtAltitude;
			UI::GUILabel *lblSpeed;
			UI::GUITextBox *txtSpeed;
			UI::GUILabel *lblHeading;
			UI::GUITextBox *txtHeading;
			UI::GUILabel *lblGPSValid;
			UI::GUITextBox *txtGPSValid;
			UI::GUILabel *lblNSateUsed;
			UI::GUITextBox *txtNSateUsed;
			UI::GUILabel *lblNSateView;
			UI::GUITextBox *txtNSateView;
			UI::GUICheckBox *chkAutoPan;
			UI::GUICheckBox *chkNoSleep;
			UI::GUICheckBox *chkTopMost;
			UI::GUIButton *btnDispOff;
			UI::GUILabel *lblDistance;
			UI::GUITextBox *txtDistance;

			UI::GUITabPage *tpAlert;
			UI::GUITabControl *tcAlert;
			UI::GUITabPage *tpAlertAdd;
			UI::GUIListBox *lbAlertLyr;
			UI::GUIHSplitter *hspAlertAdd;
			UI::GUIListBox *lbAlertAdd;
			UI::GUITabPage *tpAlertView;
			UI::GUIPanel *pnlAlertView;
			UI::GUIListBox *lbAlert;

			UI::GUITabPage *tpMTK;
			UI::GUIGroupBox *grpMTKFirmware;
			UI::GUILabel *lblMTKRelease;
			UI::GUITextBox *txtMTKRelease;
			UI::GUILabel *lblMTKBuildID;
			UI::GUITextBox *txtMTKBuildID;
			UI::GUILabel *lblMTKProdMode;
			UI::GUITextBox *txtMTKProdMode;
			UI::GUILabel *lblMTKSDKVer;
			UI::GUITextBox *txtMTKSDKVer;
			UI::GUIButton *btnMTKFirmware;
			UI::GUIButton *btnMTKLogDownload;
			UI::GUIButton *btnMTKLogDelete;
			UI::GUIButton *btnMTKFactoryReset;
			UI::GUIButton *btnMTKTest;

			static void __stdcall OnGPSUpdate(void *userObj, Map::GPSTrack::GPSRecord *record);
			static void __stdcall OnTimerTick(void *userObj);
			static void __stdcall OnMTKFirmwareClicked(void *userObj);
			static void __stdcall OnMTKLogDownloadClicked(void *userObj);
			static void __stdcall OnMTKLogDeleteClicked(void *userObj);
			static void __stdcall OnMTKTestClicked(void *userObj);
			static void __stdcall OnMTKFactoryResetClicked(void *userObj);
			static void __stdcall OnDispOffClicked(void *userObj);
			static void __stdcall OnTopMostChg(void *userObj, Bool newState);
		public:
			AVIRGPSTrackerForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core, Map::ILocationService *locSvc, Bool toRelease);
			virtual ~AVIRGPSTrackerForm();

			virtual void OnMonitorChanged();
			virtual void OnFocus();

			void SetGPSTrack(Map::GPSTrack *gpsTrk);
			void SetMapNavigator(SSWR::AVIRead::IMapNavigator *mapNavi);

			void DispOffFocusLost();
		};
	};
};
#endif
