/* SECU-3  - An open source, free engine control unit
   Copyright (C) 2007 Alexey A. Shabelnikov. Ukraine, Gorlovka

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   contacts:
              http://secu-3.org
              email: shabelnikov@secu-3.org
*/

#include "stdafx.h"
#include "Resources\resource.h"
#include "KnockChannelTabDlg.h"

#include <math.h>
#include "Application\DLLLinkedFunctions.h"
#include "common\FastDelegate.h"
#include "common\MathHelpers.h"
#include "HiSCCtrl\sources\ChartCtrl.h"
#include "HiSCCtrl\sources\ChartPointsSerie.h"
#include "HiSCCtrl\sources\ChartLineSerie.h"
#include "ParamDesk\KnockPageDlg.h"
#include "ui-core\OScopeCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;
using namespace fastdelegate;

#define TIMER_ID 0

#define K_SIG_MIN 0.0f
#define K_SIG_MAX 5.0f
#define LEVEL_SLIDER_POS_NUM 100

const UINT CKnockChannelTabDlg::IDD = IDD_KNOCK_CHANNEL;

CKnockChannelTabDlg::CKnockChannelTabDlg(CWnd* pParent /*=NULL*/)
: Super(CKnockChannelTabDlg::IDD, pParent)
, mp_RTChart(NULL)
, mp_knock_parameters_dlg(new CKnockPageDlg())
, mp_OScopeCtrl(new COScopeCtrl())
, m_all_enabled(true)
, m_pPointSerie(NULL)
, m_pLineSerie(NULL)
, m_pLineSerieLevel(NULL)
, m_copy_to_attenuator_table_button_state(true)
, m_clear_function_button_state(true)
{
 //na  
}

void CKnockChannelTabDlg::DoDataExchange(CDataExchange* pDX)
{
 Super::DoDataExchange(pDX);
 DDX_Control(pDX, IDC_KNOCK_CHANNEL_SAVE_PARAM_BUTTON, m_param_save_button);  
 DDX_Control(pDX, IDC_KNOCK_CHANNEL_COPY_TO_ATTENUATOR_TABLE, m_copy_to_attenuator_table_button);
 DDX_Control(pDX, IDC_KNOCK_CHANNEL_CLEAR_FUNCTION, m_clear_function_button);
 DDX_Control(pDX, IDC_KNOCK_CHANNEL_DESIRED_LEVEL_SLIDER, m_level_slider);
}

LPCTSTR CKnockChannelTabDlg::GetDialogID(void) const
{
 return (LPCTSTR)IDD; 
}

BEGIN_MESSAGE_MAP(CKnockChannelTabDlg, Super)
 ON_WM_DESTROY()
 ON_BN_CLICKED(IDC_KNOCK_CHANNEL_SAVE_PARAM_BUTTON, OnSaveParameters)
 ON_UPDATE_COMMAND_UI(IDC_KNOCK_CHANNEL_SAVE_PARAM_BUTTON, OnUpdateControls)
 ON_BN_CLICKED(IDC_KNOCK_CHANNEL_COPY_TO_ATTENUATOR_TABLE, OnCopyToAttenuatorTable)
 ON_UPDATE_COMMAND_UI(IDC_KNOCK_CHANNEL_COPY_TO_ATTENUATOR_TABLE, OnUpdateCopyToAttenuatorTable)
 ON_BN_CLICKED(IDC_KNOCK_CHANNEL_CLEAR_FUNCTION, OnClearFunction)
 ON_UPDATE_COMMAND_UI(IDC_KNOCK_CHANNEL_CLEAR_FUNCTION, OnUpdateClearFunction)
 ON_WM_TIMER()
 ON_WM_VSCROLL()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CKnockChannelTabDlg message handlers

BOOL CKnockChannelTabDlg::OnInitDialog() 
{
 Super::OnInitDialog();

 CRect rect;
 GetDlgItem(IDC_KNOCK_CHANNEL_PARAMETERS_FRAME)->GetWindowRect(rect);
 ScreenToClient(rect);

 //������� ������ � ����������� ��
 mp_knock_parameters_dlg->Create(CKnockPageDlg::IDD,this);
 mp_knock_parameters_dlg->SetWindowPos(NULL,rect.TopLeft().x,rect.TopLeft().y,0,0,SWP_NOZORDER|SWP_NOSIZE);
 mp_knock_parameters_dlg->ShowWindow(SW_SHOWNORMAL);

 SetTimer(TIMER_ID,250,NULL);

 //�������������� ����������� �������
 _InitializeRPMKnockSignalControl();

 //�������������� �����������
 _InitializeOscilloscopeControl();

 return TRUE;  // return TRUE unless you set the focus to a control
               // EXCEPTION: OCX Property Pages should return FALSE
}

void CKnockChannelTabDlg::OnDestroy() 
{
 Super::OnDestroy();
 delete mp_RTChart;
 KillTimer(TIMER_ID);
}

void CKnockChannelTabDlg::OnSaveParameters(void)
{
 if (m_OnSaveParameters)
  m_OnSaveParameters();
}

void CKnockChannelTabDlg::EnableAll(bool i_enable)
{
 m_all_enabled = i_enable; //remember state
 UpdateDialogControls(this,TRUE);
}

void CKnockChannelTabDlg::OnUpdateControls(CCmdUI* pCmdUI)
{
 pCmdUI->Enable(m_all_enabled);
}

void CKnockChannelTabDlg::OnUpdateCopyToAttenuatorTable(CCmdUI* pCmdUI)
{
 pCmdUI->Enable(/*m_all_enabled && */m_copy_to_attenuator_table_button_state);
}

void CKnockChannelTabDlg::OnUpdateClearFunction(CCmdUI* pCmdUI)
{
 pCmdUI->Enable(/*m_all_enabled && */m_clear_function_button_state);
}

void CKnockChannelTabDlg::OnTimer(UINT nIDEvent)
{
 //dirty hack
 UpdateDialogControls(this,TRUE);
 Super::OnTimer(nIDEvent);
}

void CKnockChannelTabDlg::AppendPoint(float value)
{
 mp_OScopeCtrl->AppendPoint(value);
}

void CKnockChannelTabDlg::SetRPMKnockSignal(const std::vector<float> &i_values)
{
 _ASSERTE(i_values.size()==RPM_KNOCK_SIGNAL_POINTS);
 mp_RTChart->EnableRefresh(false);
 for(size_t i = 0; i < i_values.size(); i++)
 {
  m_pPointSerie->SetYPointValue(i, i_values[i]);
  m_pLineSerie->SetYPointValue(i, i_values[i]);
 }
 mp_RTChart->EnableRefresh(true);
}

void CKnockChannelTabDlg::_InitializeRPMKnockSignalControl(void)
{
 //�������������� ����������� �������
 mp_RTChart = new CChartCtrl();
 CRect rect;
 GetDlgItem(IDC_KNOCK_CHANNEL_REALTIME_CHART_HOLDER)->GetWindowRect(rect);
 ScreenToClient(rect);
 mp_RTChart->Create(this,rect,IDC_KNOCK_CHANNEL_REALTIME_CHART);

 m_pPointSerie = dynamic_cast<CChartPointsSerie*>(mp_RTChart->AddSerie(CChartSerie::stPointsSerie));
 m_pLineSerie = dynamic_cast<CChartLineSerie*>(mp_RTChart->AddSerie(CChartSerie::stLineSerie));
 m_pLineSerieLevel = dynamic_cast<CChartLineSerie*>(mp_RTChart->AddSerie(CChartSerie::stLineSerie));

 m_pLineSerie->SetColor(RGB(80,80,200));
 m_pLineSerieLevel->SetColor(RGB(50,200,0));

 int rpm = 200;
 int rpm_step = 60;

 //������ ����� ����� ��������� ������ �������
 m_pLineSerieLevel->AddPoint(rpm, 0);

 // Sets the min and max values of the bottom and left axis.
 mp_RTChart->GetBottomAxis()->SetMinMax(rpm, rpm + (rpm_step * RPM_KNOCK_SIGNAL_POINTS));
 mp_RTChart->GetLeftAxis()->SetMinMax(K_SIG_MIN, K_SIG_MAX); 

 for (size_t i = 0; i < RPM_KNOCK_SIGNAL_POINTS; i++)
 {	  
  m_pPointSerie->AddPoint(rpm, 0.0);
  m_pLineSerie->AddPoint(rpm, 0.0);
  rpm+=rpm_step;
 }

 //������ ����� ����� ��������� ������ �������
 m_pLineSerieLevel->AddPoint(rpm, 0);

 //������������� �������� � ��������� ����������� �������� ������
 m_level_slider.SetRange(0, LEVEL_SLIDER_POS_NUM);
 SetDesiredLevel(K_SIG_MIN);
}

//������������� ������������
void CKnockChannelTabDlg::_InitializeOscilloscopeControl(void)
{
 CRect rect;
 GetDlgItem(IDC_KNOCK_CHANNEL_SIGNAL_OSCILLOSCOPE_HOLDER)->GetWindowRect(rect);
 ScreenToClient(rect);

 // create the control
 mp_OScopeCtrl->Create(WS_VISIBLE | WS_CHILD, rect, this); 

 // customize the control
 mp_OScopeCtrl->SetRange(K_SIG_MIN, K_SIG_MAX, 1);
 mp_OScopeCtrl->SetYUnits(MLL::LoadString(IDS_KC_OSCILLOSCOPE_V_UNIT));
 mp_OScopeCtrl->SetXUnits(MLL::LoadString(IDS_KC_OSCILLOSCOPE_H_UNIT));
 mp_OScopeCtrl->SetBackgroundColor(RGB(0, 0, 64));
 mp_OScopeCtrl->SetGridColor(RGB(192, 192, 255));
 mp_OScopeCtrl->SetPlotColor(RGB(255, 255, 255));
}

void CKnockChannelTabDlg::setOnSaveParameters(EventHandler OnFunction) 
{
 m_OnSaveParameters = OnFunction;
}

void CKnockChannelTabDlg::setOnCopyToAttenuatorTable(EventHandler OnFunction)
{
 m_OnCopyToAttenuatorTable = OnFunction;
}

void CKnockChannelTabDlg::setOnClearFunction(EventHandler OnFunction)
{
 m_OnClearFunction = OnFunction;
}

void CKnockChannelTabDlg::OnCopyToAttenuatorTable()
{
 if (m_OnCopyToAttenuatorTable)
  m_OnCopyToAttenuatorTable();
}

void CKnockChannelTabDlg::EnableCopyToAttenuatorTableButton(bool i_enable)
{
 m_copy_to_attenuator_table_button_state = i_enable;
}

void CKnockChannelTabDlg::EnableClearFunctionButton(bool i_enable)
{
 m_clear_function_button_state = i_enable;
}

void CKnockChannelTabDlg::OnClearFunction()
{
 if (m_OnClearFunction)
  m_OnClearFunction();
}

static float SliderToLevel(int i_pos)
{
 float value = ((float)i_pos) / (((float)LEVEL_SLIDER_POS_NUM) / (K_SIG_MAX - K_SIG_MIN));
 return K_SIG_MAX - value;
}

void CKnockChannelTabDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
#define _SET_LEVEL(pos) \
    m_pLineSerieLevel->SetYPointValue(0, SliderToLevel((pos))); \
    m_pLineSerieLevel->SetYPointValue(1, SliderToLevel((pos)));

 switch(nSBCode)
 {
  case TB_ENDTRACK:
   break;

  case TB_BOTTOM:
  case TB_TOP:
   _SET_LEVEL(m_level_slider.GetPos());
   break;

  case TB_PAGEDOWN:
  case TB_PAGEUP:
   _SET_LEVEL(m_level_slider.GetPos());
   break;

  case TB_LINEDOWN:
  case TB_LINEUP:
   _SET_LEVEL(m_level_slider.GetPos());
   break;

  case TB_THUMBPOSITION:
  case TB_THUMBTRACK:
   _SET_LEVEL(nPos);
   break;
 }
 #undef _SET_LEVEL
}

void CKnockChannelTabDlg::SetDesiredLevel(float i_level)
{
 ASSERT(i_level <= K_SIG_MAX);
 ASSERT(i_level >= K_SIG_MIN);
 float factor = (((float)LEVEL_SLIDER_POS_NUM) / (K_SIG_MAX - K_SIG_MIN));
 m_level_slider.SetPos(MathHelpers::Round((K_SIG_MAX - i_level) * factor));
 m_pLineSerieLevel->SetYPointValue(0, i_level);
 m_pLineSerieLevel->SetYPointValue(1, i_level);
}

float CKnockChannelTabDlg::GetDesiredLevel(void)
{
 return SliderToLevel(m_level_slider.GetPos());
}
