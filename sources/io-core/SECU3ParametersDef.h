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

#ifndef _SECU3_PARAMETERS_DEF_H_
#define _SECU3_PARAMETERS_DEF_H_

#include "SECU3Types.h"

#pragma pack( push, enter_SECU3ParametersDef )
#pragma pack(1) //<--SECU3

namespace SECU3IO {

using namespace SECU3Types;

//��������� ��������� �������
typedef struct
{
 _uchar tmp_use;                        //������� ������������ ����-��
 _uchar carb_invers;                    //�������� ��������� �� �����������
 _uchar idl_regul;                      //������������ �������� ������� �� �������������� ���
 _uchar fn_benzin;                      //����� ������ ������������� ������������ ��� �������
 _uchar fn_gas;                         //����� ������ ������������� ������������ ��� ����
 _uint  map_lower_pressure;             //������ ������� ��� �� ��� �������(���)
 _uint  ephh_lot;                       //������ ����� ���� (���-1)
 _uint  ephh_hit;                       //������� ����� ���� (���-1)
 _uint  starter_off;                    //����� ���������� �������� (���-1)
 _int   map_upper_pressure;             //������� ������� ��� �� ��� ������� (���)
 _uint  smap_abandon;                   //������� �������� � �������� ����� �� �������  (���-1) 
 _int   max_angle;                      //����������� ������������� ���
 _int   min_angle;                      //����������� ������������ ���
 _int   angle_corr;                     //�����-��������� ���    
 _uint  idling_rpm;                     //�������� ������� �� ��� ����������� �������������� ���   
 _int   ifac1;                          //������������ �-���������� �������� ��, ��� ������������� �
 _int   ifac2;                          //������������� ������ ��������������.
 _int   MINEFR;                         //���� ������������������ ���������� (�������)
 _int   vent_on;                        //����������� ��������� �����������
 _int   vent_off;                       //����������� ���������� �����������  

 _int  map_adc_factor;
 _long map_adc_correction;
 _int  ubat_adc_factor;
 _long ubat_adc_correction;
 _int  temp_adc_factor;
 _long temp_adc_correction;
  
 _uchar ckps_edge_type;                
 _uchar ckps_cogs_btdc;
 _uchar ckps_ignit_cogs;

 _int angle_dec_spead;
 _int angle_inc_spead;  
 _int idlreg_min_angle;
 _int idlreg_max_angle;
 _uint map_curve_offset;
 _uint map_curve_gradient;

 _int epm_on_threshold;

 _uint  ephh_lot_g;                  //������ ����� ���� (���)
 _uint  ephh_hit_g;                  //������� ����� ���� (���)
 _uchar shutoff_delay;               //�������� ���������� �������

 _uint  uart_divisor;                //�������� ��� ��������������� �������� UART-a
 _uchar uart_period_t_ms;            //������ ������� ������� � �������� �����������

 _uchar ckps_engine_cyl;             //���-�� ��������� ��������� 

 //--knock 
 _uchar knock_use_knock_channel;     //������� ������������� ������ ���������
 _uchar knock_bpf_frequency;         //����������� ������� ���������� �������
 _int   knock_k_wnd_begin_angle;     //������ �������������� ���� (�������)
 _int   knock_k_wnd_end_angle;       //����� �������������� ���� (�������)
 _uchar knock_int_time_const;        //���������� ������� �������������� (���)

 _int knock_retard_step;             //��� �������� ��� ��� ��������� 
 _int knock_advance_step;            //��� �������������� ��� 
 _int knock_max_retard;              //������������ �������� ���
 _uint knock_threshold;              //����� ��������� - ����������
 _uchar knock_recovery_delay;        //�������� �������������� ��� � ������� ������ ���������

 //--/knock

 _uchar vent_pwm;                    //control ventilator by using pwm

 //��� ����������������� ����� ���������� ��� ���������� �������� �������������
 //����� ������ �������� � ����� ������� ��������. ��� ���������� ����� ������
 //� ���������, ���������� ����������� ��� �����.
 _uchar reserved[9];

 _uint crc;                          //����������� ����� ������ ���� ��������� (��� �������� ������������ ������ ����� ���������� �� EEPROM)  
}params;

} //SECU3IO

#pragma pack( pop, enter_SECU3ParametersDef )

#endif //_SECU3_PARAMETERS_DEF_H_
