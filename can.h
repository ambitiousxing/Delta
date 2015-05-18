/*----------------------------------------------------------------------------
    CAN
    CAN 通信
    Designer: 张飞雄
    E-mail:   tenudy@139.com
    Files:    canbms.h canbms.c
    Created:  2010-8-14
    System:   Freescale TES-B
    Copyright(c) Newclear 2010
------------------------------------------------------------------------------
    说明
        1. CAN 通信
    ------------------------------
    || 正常
------------------------------------------------------------------------------
    版本历史
    ------------------------------
    || 2010-7-
    ------------------------------
    || 2010-8-14 建立文件
----------------------------------------------------------------------------*/
#ifndef _CAN_H_
#define _CAN_H_
#include "tesb.h"
/*----------------------------------------------------------------------------
    Public define
        real ++
----------------------------------------------------------------------------*/
/*
    Step
*/
#define CB_STEP_Tout            0xe0
#define CB_STEP_SetIdle         0xe1
#define CB_STEP_Idle            0
#define CB_STEP_Connect         1
#define CB_STEP_Config          2
#define CB_STEP_Charge          3
#define CB_STEP_Cgcmp           4
#define CB_STEP_Bmcmp           5
/*
    Start
*/
#define CB_SUP_Rc               0xe0
#define CB_SUP_Manual           0xe3
/*
    Run
*/
struct CbRecDataS
{
    FLASH INT32U Id;
    FLASH INT8U Dlc;
    void (* FLASH Rec)(void);
    void (* FLASH Isr)(void);
};
/*----------------------------------------------------------------------------
    Public variable
----------------------------------------------------------------------------*/
struct CbDataS
{
    union
    {
        INT8U Byte;
        struct
        {
            INT16U Reced        :1;
            INT16U RecRun       :1;
            INT16U IsrRun       :1;
            INT16U Sup          :1;
            INT16U Tout         :1;
            INT16U Limt         :1;
            INT16U :2;
        }Bits;
    }Sta;
    struct
    {
        INT8U Cnt1s;
        INT8U Cnt10s;
        INT8U Cnt60s;
        INT8U BMS_LIFE;
        INT8U BMS_BCS;
        INT8U BMS_BEM;
        INT8U BMS_BFM;
    }Timer;
    struct CbDataRecRunBufS
    {
        INT32U Id;
        INT8U Data[8];
        INT8U Dlc;
    }RecRunBuf;
    void (* RecIsr)(void);
    void (* RecRun)(void);
    INT8U Step;
    INT8U State;
    INT8U State2;
    INT8U SdnStep;
    INT8U SupStep;
    INT8U ToutCode[4];
    struct
    {
        INT8U Step;
        INT8U State;
        INT8U State2;
    }Last;
    struct
    {
        struct
        {
            INT8U ComVer[3];
            INT8U BattType;
            INT16U BattRateAh;
            INT16U BattRateVol;
            INT8U BattProduct[4];
            INT8U BattId[4];
            INT8U BattDate[3];
            INT8U BattChgTime[3];
            INT8U BattPri;
            INT8U Rev;
            INT8U Vin[17];
        }Brm;
        struct
        {
            INT16U MaxAllowSvol;
            INT16U MaxAllowCur;
            INT16U MaxAllowAh;
            INT16U MaxAllowVol;
            INT8U MaxAllowTemp;
            INT16U Ah;
            INT16U Vol;
            INT8U Soh;
        }Bcp;
        struct
        {
            INT8U Sta;
        }Bro;
        struct
        {
            INT16U Vol;
            INT16U Cur;
            INT8U Mode;
            
        }Bcl;
        struct
        {
          
            INT8U MinTemp;
            INT8U MaxTemp;
            INT8U Soc;
            INT16U Vol;
            INT16U Cur;
            INT16U MaxSvolN;
            INT16U SvolMax;          
            INT16U CmpTime;
        }Bcs;
        struct
        {
            INT16U MaxSvol;
            INT8U MaxSvolBattId;
            INT8U MaxTemp;
            INT8U MaxTempPos;
            INT8U MinTemp;
            INT8U Sta[2]; 
        }Bsm;
        struct
        {
            INT8U Num;
        }Bmv;
        struct
        {
            INT8U Num;
        }Bmt;
        struct
        {
            INT8U Code[4];
            INT8U CrCode[4];
        }Bst;
        struct
        {
            INT8U SocB;
            INT8U SocCurnt;
            INT16U MinSvol;
            INT16U MaxSvol;
            INT8U MinTemp;
            INT8U MaxTemp;
        }Bsd;
        struct
        {
            INT8U Code[4];
        }Bem;
        struct
        {
            INT8U Data;
        }Life;
    }Bm;
    struct
    {
        struct
        {
            INT8U Code[4];
            INT8U CrCode[4];
        }Stop;
    }Cg;
};
extern struct CbDataS CbData;
struct CrDataS
{
    union
    {
        INT16U Word;
        struct
        {
            INT16U Send_CCM_STOP        :1;
            INT16U Send_CCM_COUNT       :1;
            INT16U Send_CCM_ERR         :1;
            INT16U Send_BMS_ID          :1;
            INT16U Send_BMS_STOP        :1;
            INT16U Send_BMS_COUNT       :1;
            INT16U Send_BAT_STATE_VN    :1;
            INT16U Send_BAT_STATE_TN    :1;
            INT16U Sta_BMS_LIFE         :1;
            INT16U Sta_BMS_VBI          :1;
            INT16U Sta_BMS_BCS          :1;
            INT16U Sta_BMS_BCP          :1;
            INT16U Sta_BMS_BEM          :1;
            INT16U Sta_BMS_BFM          :1;
        }Bits;
    }Sta;
    struct
    {
        INT16U MCM_STATE0;
        INT16U MCM_STATE1;
        INT16U MCM_STATE2;
        INT16U MCM_STATE3;
    }Timer;
};
extern struct CrDataS CrData;


#define Ct_Pm_Sta_Offline       0x00
#define Ct_Pm_Sta_Requsted      0x01
#define Ct_Pm_Sta_Online        0x02


#define CT_PM_Size   16
struct CtDataS
{
    void (* IsrRun)(void);
    void (* RecRun)(void);
    union 
    {
        INT8U Byte;
        struct
        {
            INT8U IsrRun        :1;
            INT8U RecRun        :1;
            INT8U SnapAll       :1;
            INT8U :5;
            
        }Bits;
    }Sta;   
    INT8U ModuleCnt;
    struct
    {
        INT8U AssignId;
        INT8U Status;
        INT8U FlagRefresh;
        
        INT16U OutVol;
        INT16U OutCur;
        union
        {
            INT32U All;
            struct
            {
                INT32U SubSta   :8;
                INT32U MainSta  :4;
                INT32U Delta    :20;
            }BitS;
        }Sta;
    }Pm[CT_PM_Size];
    struct
    {
        INT8U Cmd;
        INT16U Vol;
        INT16U Cur;
        INT16U Power;
    }Para;
};
 extern struct CtDataS CtData;
/*----------------------------------------------------------------------------
    Public declaration
----------------------------------------------------------------------------*/
void CanInit(void);
void CnEn(void);
void CnDs(void);
void CnCtrlOn(void);
void CnCtrlOff(void);
void CnSendVol(INT16U vpara);
void CnSendCur(INT16U ipara);
void CbEn(void);
void CbDs(void);
void CbSetConnect(void);
void CbSetIdle(void);
void CrEn(void);
void CrDs(void);
void CrSendImsStart(void);
void CrSendImsStop(void);
void CbSendSdnDword(void);


void CtEn(void);
void CtCtrlSubOn(void);
void CtCtrlSubOff(void);
/*----------------------------------------------------------------------------
    Private
----------------------------------------------------------------------------*/
#ifdef Can_Private
#include "beep.h"
#include "fram.h"
#include "iic.h"
#include "led.h"
#include "mc.h"
/*----------------------------------------------------------------------------
    Private end
----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------
    End
----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------
    By Tenudy
----------------------------------------------------------------------------*/
