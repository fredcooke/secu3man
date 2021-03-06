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

#pragma once

#include <string>
#include <vector>
#include "common/unicodesupport.h"
#include "ISettingsData.h"

class CAppSettingsController;

//������ � ���������/��������� ������ 
class CAppSettingsModel : public ISettingsData 
{
  friend CAppSettingsController;

 public:
  std::vector<DWORD> m_AllowableBaudRates;
  std::vector<_TSTRING> m_AllowablePorts;

  //<UIstring, SepSymbol>
  std::vector<std::pair<_TSTRING, char> > m_AllowaleCSVSepSymbols;

  //<<UIString, INIString>, ID>
  std::vector<std::pair<std::pair<_TSTRING, _TSTRING>, int> > m_AllowableLanguages;

  //<<UIString, INIString>, ID>
  std::vector<std::pair<std::pair<_TSTRING, _TSTRING>, int> > m_AllowablePlatforms;

  CAppSettingsModel();
  virtual ~CAppSettingsModel();

  ////////////////////////////////////////////////////
  //���������� ������ ��� INI-�����. INI-���� ��������� � �������� �� ��������
  //�������� ���������.
  CString GetINIFileFullName(void) const;

  //������ �������� �� INI-�����
  bool ReadSettings(void);
    
  //������ �������� � INI-����
  bool WriteSettings(void);

  //��� ������ � �������� �����
  const CString m_Name_Options_Section;
  const CString m_Name_PortName;
  const CString m_Name_BaudRateApplication;
  const CString m_Name_BaudRateBootloader;
  const CString m_Name_LogFilesFolder;
  const CString m_Name_UseAppFolder;
  const CString m_Name_CSVSepSymbol;
  const CString m_Name_MIDeskUpdatePeriod;
  const CString m_Name_InterfaceLang;
  const CString m_Name_ECUPlatformType;

  //������� ���� ������ ���
  const CString m_Name_WndSettings_Section;
  const CString m_Name_StrtMapWnd_X;
  const CString m_Name_StrtMapWnd_Y;
  const CString m_Name_IdleMapWnd_X;
  const CString m_Name_IdleMapWnd_Y;
  const CString m_Name_WorkMapWnd_X;
  const CString m_Name_WorkMapWnd_Y;
  const CString m_Name_TempMapWnd_X;
  const CString m_Name_TempMapWnd_Y;
  const CString m_Name_AttenMapWnd_X;
  const CString m_Name_AttenMapWnd_Y;
  const CString m_Name_MainFrmWnd_X;
  const CString m_Name_MainFrmWnd_Y;
  const CString m_Name_CoilRegulMapWnd_X;
  const CString m_Name_CoilRegulMapWnd_Y;

  //fixture settings
  const CString m_Name_Fixtures_Section;
  const CString m_Name_Tachometer_Max;
  const CString m_Name_Pressure_Max;

  //ISettingsData
  virtual const _TSTRING& GetPortName(void) const;   
  virtual DWORD GetBaudRateApplication(void) const;
  virtual DWORD GetBaudRateBootloader(void) const;
  virtual const CString& GetLogFilesFolder(void) const;
  virtual bool  GetUseAppFolder(void) const;
  virtual char  GetCSVSepSymbol(void) const;
  virtual int   GetMIDeskUpdatePeriod(void) const;
  //���������� ������ ���� � �������� �� �������� ���� �������� ����������
  //(������� ������� ��� ������� �� ������ ����� ������������ ����� ������)
  virtual CString GetAppDirectory(void) const;
  //������� ����
  virtual void SetWndSettings(const WndSettings& i_wndSettings);
  virtual void GetWndSettings(WndSettings& o_wndSettings) const;
  //���� ���������� � ��� ���������
  virtual EInterLang GetInterfaceLanguage(void) const;
  virtual EECUPlatform GetECUPlatformType(void) const;
  //fixtures's customization
  virtual int GetTachometerMax(void) const;
  virtual int GetPressureMax(void) const;
  ////////////////////////////////////////////////////

 private:
  //������ ������� �������� � INI-�����
  _TSTRING m_optPortName;      
  DWORD m_optBaudRateApplication;
  DWORD m_optBaudRateBootloader;
  CString m_optLogFilesFolder;
  bool  m_optUseAppFolder;
  char  m_optCSVSepSymbol;
  int   m_optMIDeskUpdatePeriod;
  EInterLang m_optInterLang;
  EECUPlatform m_optECUPlatformType;
  int m_optTachometerMax;
  int m_optPressureMax;

  int m_optStrtMapWnd_X;
  int m_optStrtMapWnd_Y;
  int m_optIdleMapWnd_X;
  int m_optIdleMapWnd_Y;
  int m_optWorkMapWnd_X;
  int m_optWorkMapWnd_Y;
  int m_optTempMapWnd_X;
  int m_optTempMapWnd_Y;
  int m_optAttenMapWnd_X;
  int m_optAttenMapWnd_Y;
  int m_optMainFrmWnd_X;
  int m_optMainFrmWnd_Y;
  int m_optCoilRegulMapWnd_X;
  int m_optCoilRegulMapWnd_Y;

  //��������� ��������� �������� �������� �� ������������ ���������
  bool CheckAllowableBaudRate(DWORD baud);
  bool CheckAllowableCSVSepSymbol(char i_symbol);
  bool CheckAllowableLanguage(const _TSTRING& i_string, EInterLang& o_language_id);
  bool CheckAllowablePlatform(const _TSTRING& i_string, EECUPlatform& o_platform_id);

  //���������� �� ������� ���� �������� ����������
  TCHAR m_current_directory[MAX_PATH+1]; 
};
