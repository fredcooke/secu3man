 /****************************************************************
 *       SECU-3  - An open source, free engine control unit
 *    Designed by Alexey A. Shabelnikov. Ukraine, Gorlovka 2007.
 *       Microprocessors systems - design & programming.
 *    contacts:
 *              http://secu-3.narod.ru
 *              ICQ: 405-791-931
 ****************************************************************/

#include "stdafx.h"
#include "resource.h"
#include "ISECU3Man.h"
#include "MainTabManager.h"
#include "ChildView.h"
#include "MainFrameManager.h"
#include "AppSettingsManager.h"
#include "MainTabController.h"
#include "ui-core/TabController.h"

#include "ParamMonTabController.h"
#include "FirmwareTabController.h"
#include "CheckEngineTabController.h"
#include "KnockChannelTabController.h"
#include "LogPlayerTabController.h"

#include "ParamMonTabDlg.h"
#include "FirmwareTabDlg.h"
#include "CheckEngineTabDlg.h"
#include "KnockChannelTabDlg.h"
#include "LogPlayerTabDlg.h"

#define MAIN_TAB_CTRL_BITMAPS_COLOR_MASK RGB(192,192,192)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMainTabManager::CMainTabManager()
: m_pImgList(NULL)
, m_pParent(NULL)
, mp_MainTabController(new CMainTabController())
, mp_tab_control(new CTabController())
, m_pParamMonTabController(NULL)
, m_pFirmwareTabController(NULL)
, m_pCheckEngineTabController(NULL)
, m_pKnockChannelTabController(NULL)
, m_pLogPlayerTabController(NULL)
, m_pParamMonTabDlg(NULL)
, m_pFirmwareTabDlg(NULL)
, m_pCheckEngineTabDlg(NULL)
, m_pKnockChannelTabDlg(NULL)
, m_pLogPlayerTabDlg(NULL)
{
 //na
}

CMainTabManager::~CMainTabManager()
{
 if (m_pImgList)
 {
  delete m_pImgList;
  m_pImgList = NULL;
 }

 delete m_pParamMonTabController;
 delete m_pFirmwareTabController;
 delete m_pCheckEngineTabController;
 delete m_pKnockChannelTabController;
 delete m_pLogPlayerTabController;

 delete m_pParamMonTabDlg;
 delete m_pFirmwareTabDlg;
 delete m_pCheckEngineTabDlg;
 delete m_pKnockChannelTabDlg;
 delete m_pLogPlayerTabDlg;
}

bool CMainTabManager::Init(CChildView* i_pChildView)
{
 ASSERT(i_pChildView);
 m_pParent = i_pChildView;
 m_pImgList = new CImageList(); 
 m_pImgList->Create(16, 16, ILC_COLOR24|ILC_MASK, 0, 0);
 CBitmap bitmap;
 bitmap.LoadBitmap((LPCTSTR)IDB_MAIN_TAB_CTRL_BITMAPS);
 m_pImgList->Add(&bitmap, MAIN_TAB_CTRL_BITMAPS_COLOR_MASK);

 CRect rect(0,0,300,200);

 mp_tab_control->SetStyle(WS_VISIBLE | WS_CHILD  | TCS_BOTTOM | TCS_OWNERDRAWFIXED);
 mp_tab_control->SetResourceModule(::GetModuleHandle(NULL));
 VERIFY(mp_tab_control->Create(i_pChildView, rect, IDC_MAIN_TAB_CTRL, false));
 mp_tab_control->SetImageList(m_pImgList);

 mp_tab_control->SetEventListener(mp_MainTabController.get()); //���������� ����� ��������� ��������� �� View
 mp_MainTabController->SetTabController(mp_tab_control.get()); //set View
  
 CCommunicationManager* p_comm = ISECU3Man::GetSECU3Man()->GetCommunicationManager();
 CStatusBarManager*     p_sbar = ISECU3Man::GetSECU3Man()->GetMainFrameManager()->GetStatusBarManager();
 ISettingsData*         p_settings = ISECU3Man::GetSECU3Man()->GetAppSettingsManager()->GetSettings();

 //������� ����������� ������� � ��������� Tab control ���������
 m_pParamMonTabDlg = new CParamMonTabDlg(); //view
 m_pParamMonTabController = new CParamMonTabController(m_pParamMonTabDlg, p_comm, p_sbar, p_settings);

 mp_MainTabController->AddTabController(m_pParamMonTabController);
 mp_tab_control->AddPage(MLL::LoadString(IDS_TAB_PARAMETERS_AND_MONOTOR),m_pParamMonTabDlg,0);

 m_pFirmwareTabDlg = new CFirmwareTabDlg(); //view
 m_pFirmwareTabController = new CFirmwareTabController(m_pFirmwareTabDlg, p_comm, p_sbar);

 mp_MainTabController->AddTabController(m_pFirmwareTabController);
 mp_tab_control->AddPage(MLL::LoadString(IDS_TAB_FIRMWARE_DATA),m_pFirmwareTabDlg,1);	

 m_pCheckEngineTabDlg = new CCheckEngineTabDlg(); //view
 m_pCheckEngineTabController = new CCheckEngineTabController(m_pCheckEngineTabDlg, p_comm, p_sbar);

 mp_MainTabController->AddTabController(m_pCheckEngineTabController);
 mp_tab_control->AddPage(MLL::LoadString(IDS_TAB_CHECK_ENGINE),m_pCheckEngineTabDlg,2);

 m_pKnockChannelTabDlg = new CKnockChannelTabDlg(); //view
 m_pKnockChannelTabController = new CKnockChannelTabController(m_pKnockChannelTabDlg, p_comm, p_sbar);

 mp_MainTabController->AddTabController(m_pKnockChannelTabController);
 mp_tab_control->AddPage(MLL::LoadString(IDS_TAB_KNOCK_SETTINGS),m_pKnockChannelTabDlg,3);

 m_pLogPlayerTabDlg = new CLogPlayerTabDlg(); //view
 m_pLogPlayerTabController = new CLogPlayerTabController(m_pLogPlayerTabDlg, p_comm, p_sbar, p_settings);

 mp_MainTabController->AddTabController(m_pLogPlayerTabController);
 mp_tab_control->AddPage(MLL::LoadString(IDS_TAB_LOG_PLAYER),m_pLogPlayerTabDlg,4);

 mp_tab_control->SetCurSel(0);
 return true;
}

void CMainTabManager::OnAfterCreate(void)
{
 CRect rect;
 m_pParent->GetClientRect(rect);
 rect.right-=0;
 rect.bottom-=0; 
 mp_tab_control->MoveWindow(rect);
}

bool CMainTabManager::OnClose(void)
{  
 std::vector<ITabController*>& list = mp_MainTabController->GetControllersList();
 for (size_t i = 0; i < list.size(); i++)
 {
  if (list[i]->OnClose()==false)
   return false;
 }    
 return true;
}
