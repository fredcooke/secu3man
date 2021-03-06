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
#include "ParamMonTabController.h"

#include <map>
#include <algorithm>
#include "Application\CommunicationManager.h"
#include "common\FastDelegate.h"
#include "io-core\ufcodes.h"
#include "MainFrame\StatusBarManager.h"
#include "MIDesk\MIDeskDlg.h"
#include "MIDesk\RSDeskDlg.h"
#include "ParamDesk\ParamDeskDlg.h"
#include "Settings\ISettingsData.h"
#include "TabDialogs\ParamMonTabDlg.h"

using namespace fastdelegate;
using namespace std; 
using namespace SECU3IO;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define EHKEY _T("ParamMonCntr")

CParamMonTabController::CParamMonTabController(CParamMonTabDlg* i_view, CCommunicationManager* i_comm, CStatusBarManager* i_sbar, ISettingsData* ip_settings)
: m_view(i_view)
, m_comm(i_comm)
, m_sbar(i_sbar)
, mp_settings(ip_settings)
, m_operation_state(-1)
, m_packet_processing_state(PPS_READ_MONITOR_DATA)
, m_parameters_changed(false)
, m_lastSel(0)
{ 
 m_view->mp_ParamDeskDlg->SetOnTabActivate(MakeDelegate(this,&CParamMonTabController::OnParamDeskTabActivate));
 m_view->mp_ParamDeskDlg->SetOnChangeInTab(MakeDelegate(this,&CParamMonTabController::OnParamDeskChangeInTab));
 m_view->setOnRawSensorsCheck(MakeDelegate(this,&CParamMonTabController::OnRawSensorsCheckBox));
 m_view->mp_ParamDeskDlg->SetOnSaveButton(MakeDelegate(this,&CParamMonTabController::OnPDSaveButton));
}

CParamMonTabController::~CParamMonTabController()
{
 //na  
}

//���������� ��������� ���������!
void CParamMonTabController::OnSettingsChanged(void)
{
 //�������� ����������� ��� ������� ��������� ���������������� ����������
 m_comm->SwitchOn(CCommunicationManager::OP_ACTIVATE_APPLICATION, true); 
 m_view->mp_MIDeskDlg->SetUpdatePeriod(mp_settings->GetMIDeskUpdatePeriod());
 //��������� ��������� ��������
 m_view->mp_MIDeskDlg->SetTachometerMax(mp_settings->GetTachometerMax());
 m_view->mp_MIDeskDlg->SetPressureMax(mp_settings->GetPressureMax());
 m_view->Invalidate();
}

//from ParamDesk
void CParamMonTabController::OnParamDeskTabActivate(void)
{
 //����� ��������� ����� ������� ���������� ��������� ��� ��� ��������� �� SECU
 StartReadingNecessaryParameters();
}

//from ParamDesk
void CParamMonTabController::OnParamDeskChangeInTab(void)
{
 m_parameters_changed = true;
}

//from MainTabController
void CParamMonTabController::OnActivate(void)
{
 m_view->mp_MIDeskDlg->SetTachometerMax(mp_settings->GetTachometerMax());
 m_view->mp_MIDeskDlg->SetPressureMax(mp_settings->GetPressureMax());

 //�������� ����� ��������� ������� �� ������ ����������	
 bool result = m_view->mp_ParamDeskDlg->SetCurSel(m_lastSel);
 m_view->mp_MIDeskDlg->SetUpdatePeriod(mp_settings->GetMIDeskUpdatePeriod());

 //////////////////////////////////////////////////////////////////
 //���������� ���������� � ������ ������ �� SECU-3
 m_comm->m_pAppAdapter->AddEventHandler(this,EHKEY); 
 m_comm->setOnSettingsChanged(MakeDelegate(this,&CParamMonTabController::OnSettingsChanged)); 
 //////////////////////////////////////////////////////////////////
 
 //�������� ����������� ��� ������� ��������� ���������������� ����������
 m_comm->SwitchOn(CCommunicationManager::OP_ACTIVATE_APPLICATION);

 //��������� ������ �� �������� ����� �������������� ������� ������� ������ � SECU-3
 m_pd_changes_timer.SetTimer(this,&CParamMonTabController::OnParamDeskChangesTimer,500);

 //���������� ��������� ��������� ��� ���������� ����������, ��� ��� OnConnection ���������� ������ ����
 //���������� ��� ����������� ������������� (����� ����������� ����������������� �����������)
 bool online_status = m_comm->m_pControlApp->GetOnlineStatus();
 OnConnection(online_status);
}

//from MainTabController
void CParamMonTabController::OnDeactivate(void)
{
 m_comm->m_pAppAdapter->RemoveEventHandler(EHKEY);

 //������ �������� ������ ���� �� ��������� � ��������� "���������� � ��������"
 m_pd_changes_timer.KillTimer(); 

 //���������� ����� ��������� ��������� ������� �� ������ ����������
 m_lastSel = m_view->mp_ParamDeskDlg->GetCurSel();

 m_sbar->SetInformationText(_T(""));
}

void CParamMonTabController::StartCollectingInitialData(void) 
{
 m_comm->m_pControlApp->ChangeContext(FNNAME_DAT); //change context!

 m_packet_processing_state = PPS_COLLECT_INITIAL_DATA;
 m_operation_state = 0;
}

//���������� true ����� ������ �������� ���������
//m_operation_state = 0 ��� �������
bool CParamMonTabController::CollectInitialDataFromSECU(const BYTE i_descriptor, const void* i_packet_data)
{
 static std::vector<_TSTRING> fn_names; //Achtung! singleton.
 static std::vector<int> fn_indexes;    //Achtung! singleton.

 const FnNameDat* fn_data = NULL;

 //state machine 
 switch(m_operation_state)
 {
  case 0: //�������� ������� �� SECU-3 ����� ��� �������� ������ � ����������� � ���������� �������������
  {///////////	
   m_sbar->SetInformationText(MLL::LoadString(IDS_PM_READING_CONFIGURATION)); 

   if (i_descriptor!=FNNAME_DAT)
   {
    m_comm->m_pControlApp->ChangeContext(FNNAME_DAT); //!!!		  
   }
   else
   { //���������� ���� ���������� � ���������� �������������
    m_operation_state = 1; 
	 
    fn_data = (FnNameDat*)i_packet_data;
		 
    fn_names.clear();
    fn_indexes.clear();
    fn_names.resize(fn_data->tables_num);
		 
    //������ � SECU �������� � ���� ASCII, �� ���� ������������� � UNICODE
    TCHAR name_string[256];
    OemToChar(fn_data->name,name_string);
    fn_names[fn_data->index] = name_string;

    fn_indexes.push_back(fn_data->index);
   }
  }///////////
  break;	

  case 1: //��������� (���������� ���������) � ��������� ���������� � ���� ���������� �������������
  {///////////
   if (i_descriptor!=FNNAME_DAT)
   {
    m_operation_state = 0;		  
    break; 
   }

   fn_data = (FnNameDat*)i_packet_data;
          
   TCHAR name_string[256];
   OemToChar(fn_data->name,name_string);
   fn_names[fn_data->index] = name_string;

   fn_indexes.push_back(fn_data->index);

   bool all_names_received = true;
   for(int i = 0; i < fn_data->tables_num; i++)
   {
    if (std::find(fn_indexes.begin(),fn_indexes.end(),i)==fn_indexes.end())
    {
     all_names_received = false;
    }
   }

   if (all_names_received) //��� ���������� � ��������� ������������� �������
   {
    m_operation_state = -1;  //������� �� - �������� ���������
    m_view->mp_ParamDeskDlg->SetFunctionsNames(fn_names);
    return true; //�������� ���������
   }
  }/////////
  break;	
 }//switch

 return false; //�� ���������� ������...
}

void CParamMonTabController::StartReadingNecessaryParameters(void) 
{
 BYTE view_descriptor = m_view->mp_ParamDeskDlg->GetCurrentDescriptor();
 m_comm->m_pControlApp->ChangeContext(view_descriptor);  //change context!	  

 m_packet_processing_state = PPS_READ_NECESSARY_PARAMETERS;
 m_operation_state = 0;
}

//���������� true ����� ������ �������� ���������
//m_operation_state = 0 ��� �������
bool CParamMonTabController::ReadNecessaryParametersFromSECU(const BYTE i_descriptor, const void* i_packet_data)
{
 m_sbar->SetInformationText(MLL::LoadString(IDS_PM_READING_PARAMS)); 

 switch(m_operation_state)
 {
  case 0:  //������� ������ � ���������� ���� �����������
  {
   BYTE view_descriptor = m_view->mp_ParamDeskDlg->GetCurrentDescriptor();
   if (i_descriptor!=view_descriptor)
   {
    m_comm->m_pControlApp->ChangeContext(view_descriptor);	//!!!	  		  
   }
   else
   {//��� ��� ����!
    m_view->mp_ParamDeskDlg->SetValues(i_descriptor,i_packet_data);
           
    //������� ������������� �������
    m_operation_state = -1; //������� �� - �������� ���������
    m_sbar->SetInformationText(MLL::LoadString(IDS_PM_READY));
    return true; //�������� ���������
   }
  }	
  break;
 }//switch

 return false; //�� ���������� ������...
}

//hurrah!!! ������� ����� �� SECU-3
void CParamMonTabController::OnPacketReceived(const BYTE i_descriptor, SECU3IO::SECU3Packet* ip_packet)
{
 //������ ������: ������ ����� � �������������� �����
 if (i_descriptor == OP_COMP_NC)
 {
  const OPCompNc* p_ndata = (OPCompNc*)ip_packet;
  switch(p_ndata->opcode)
  {
   case OPCODE_EEPROM_PARAM_SAVE:
    m_sbar->SetInformationText(MLL::LoadString(IDS_PM_PARAMS_HAS_BEEN_SAVED));
    return;
  }		
 }

 //��������� ���������� ������� � ����������� �� �������� ������
 switch(m_packet_processing_state)
 {
  case PPS_COLLECT_INITIAL_DATA:  //������������� ������� �� SECU-3	
   if (CollectInitialDataFromSECU(i_descriptor,ip_packet))
    StartReadingNecessaryParameters();
   break;

  case PPS_READ_NECESSARY_PARAMETERS:  //������ ��������� ����������
   if (ReadNecessaryParametersFromSECU(i_descriptor,ip_packet))
   {
    m_packet_processing_state = PPS_BEFORE_READ_MONITOR_DATA;

    //������������ ��������� - ����� ��������� ������
    bool state = m_comm->m_pControlApp->GetOnlineStatus();
    m_view->mp_ParamDeskDlg->Enable(state);
   }
   break;
	
  case PPS_BEFORE_READ_MONITOR_DATA: //� ���� ����� �� �������� ������ ���� ���

   if (!m_view->GetRawSensorsCheckState())
   {
    //--������� ������ ��� ��� �� � ������ ������ ��������--
    if (i_descriptor!=SENSOR_DAT)
    {
     m_comm->m_pControlApp->ChangeContext(SENSOR_DAT); //!!!		  		
    }
    else
    {
     //������������� �������� ��������, ��������� �� � ��������� � �������� �����
     m_view->mp_MIDeskDlg->SetValues((SensorDat*)(ip_packet)); 	
     bool state = m_comm->m_pControlApp->GetOnlineStatus();
     m_view->mp_MIDeskDlg->Enable(state);
     m_packet_processing_state = PPS_READ_MONITOR_DATA;
    }
   }
   else
   {
    //--������� ������ ��� ��� �� � ������ "�����" ��������--
    if (i_descriptor!=ADCRAW_DAT)
    {
     m_comm->m_pControlApp->ChangeContext(ADCRAW_DAT); //!!!		  		
    }
    else
    {
     //������������� �������� ��������, ��������� �� � ��������� � �������� �����
     m_view->mp_RSDeskDlg->SetValues((RawSensDat*)(ip_packet)); 	
     bool state = m_comm->m_pControlApp->GetOnlineStatus();
     m_view->mp_RSDeskDlg->Enable(state);
     m_packet_processing_state = PPS_READ_MONITOR_DATA;
    }
   }
   break;

  case PPS_READ_MONITOR_DATA:  //��������� ������ ��� ��������
   if (!m_view->GetRawSensorsCheckState())
   {
    //--������� ������ ��� ��� �� � ������ ������ ��������--
    if (i_descriptor!=SENSOR_DAT)
    {
     m_comm->m_pControlApp->ChangeContext(SENSOR_DAT); //!!!		  		
    }
    else
    {
     m_view->mp_MIDeskDlg->SetValues((SensorDat*)(ip_packet)); 	
    }
   }
   else
   {
    //--������� ������ ��� ��� �� � ������ "�����" ��������--
    if (i_descriptor!=ADCRAW_DAT)
    {
     m_comm->m_pControlApp->ChangeContext(ADCRAW_DAT); //!!!		  		
    }
    else
    {
     m_view->mp_RSDeskDlg->SetValues((RawSensDat*)(ip_packet)); 	
    }	
   }
   break;	
 }//switch
}

//��������� ����������� ����������!
void CParamMonTabController::OnConnection(const bool i_online)
{
 int state;
 ASSERT(m_sbar);

 if (i_online) //������� � ������
 {
  state = CStatusBarManager::STATE_ONLINE;
  StartCollectingInitialData();
 }
 else
 {
  state = CStatusBarManager::STATE_OFFLINE;  
 } 
 
 if (i_online==false) //����� �� ����� ������ ��������� ������, � ��������� �� ����� ������ �����, ����� ��������� ������������ 
 {
  m_view->mp_MIDeskDlg->Enable(i_online);
  m_view->mp_RSDeskDlg->Enable(i_online);
  m_view->mp_ParamDeskDlg->Enable(i_online);
 }

 m_sbar->SetConnectionState(state);
}

//�������� ������� � ����������� � SECU ����� ����������� �� ���� ��� ����� ����� �����������
void CParamMonTabController::OnParamDeskChangesTimer(void)
{ 
 if (m_parameters_changed)
 {
  //�������� ������ �� view � ��������� �� �� ��������� ����� 
  BYTE packet_data[1024];
  BYTE view_descriptor = m_view->mp_ParamDeskDlg->GetCurrentDescriptor();
  m_view->mp_ParamDeskDlg->GetValues(view_descriptor,packet_data);

  //������� ���������� ������������� ������ (��� �������� ��������� �����, ������� �� ������ � ����� ����� �� ������������)
  m_comm->m_pControlApp->SendPacket(view_descriptor,packet_data);

  m_parameters_changed = false; //���������� ������� - ���������� �������
 }
}

//������� �� �������� �������������� ����� ����������� (�������/����� ������)
void CParamMonTabController::OnRawSensorsCheckBox(void)
{
 bool state = m_view->GetRawSensorsCheckState();

 if (state)
 {//���������� ����� �������� (������ ������ �������� � ���������� ������ ����� ��������)
  m_view->mp_MIDeskDlg->ShowWindow(SW_HIDE);
  m_view->mp_RSDeskDlg->ShowWindow(SW_SHOW); 
  m_packet_processing_state = PPS_BEFORE_READ_MONITOR_DATA;
 }
 else
 {//���������� ������ �������� (������ ������ ����� �������� � ���������� ����� ��������)
  m_view->mp_MIDeskDlg->ShowWindow(SW_SHOW);
  m_view->mp_RSDeskDlg->ShowWindow(SW_HIDE);  
  m_packet_processing_state = PPS_BEFORE_READ_MONITOR_DATA;
 }  
}

bool CParamMonTabController::OnClose(void)
{
 return true;
}

bool CParamMonTabController::OnAskFullScreen(void)
{
 return true;
}

//������ ������ ���������� ���������� - ���� ������� ������� ����������
//� SECU-3
void CParamMonTabController::OnPDSaveButton()
{
 m_sbar->SetInformationText(MLL::LoadString(IDS_PM_WRITING_PARAMS));
 OPCompNc packet_data;
 packet_data.opcode = OPCODE_EEPROM_PARAM_SAVE;
 m_comm->m_pControlApp->SendPacket(OP_COMP_NC,&packet_data);
}

void CParamMonTabController::OnFullScreen(bool i_what, const CRect& i_rect)
{
 //��� ��������� �������������� ������ �������� ���� ������� ��� ����� ��� ����
 //������ ��� ��������. ��� ���������� �������������� ������ ��� ������� ��� ��������
 //������� � ������� �������.   

 if (i_what) 
  m_view->MoveWindow(i_rect.left, i_rect.top, i_rect.Width(), i_rect.Height()); 

 m_view->MakePDFloating(i_what);
 m_view->EnlargeMonitor(i_what);
}
