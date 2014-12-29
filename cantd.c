/*----------------------------------------------------------------------------
    台达模块
    CAN 台达  规约通信服务
    Designer: 张飞雄
    E-mail:   tenudy@139.com
    Files:    tscan.h cantd.c
    Cteated:  2014-9-16
    System:   Freescale TES-B
    Copyright(c) Newclear 2011
------------------------------------------------------------------------------
    说明
        1. 基于 CAN Protocol between 10KW EV Charger and CSU
    ------------------------------
    正常
------------------------------------------------------------------------------
    版本历史
    ------------------------------
    || 2014-9-16  建立文件

----------------------------------------------------------------------------*/
#define Can_Private
#include "can.h"
/*----------------------------------------------------------------------------
    Declaration
----------------------------------------------------------------------------*/

static void CtRecIsr(void);
static void CtRecSlaveStatus(void);
/*----------------------------------------------------------------------------
    TSCAN
----------------------------------------------------------------------------*/
/*
    Predefine
*/
#define Tscan_Predefine
#include "tscan.h"
/*
    Setting
*/

    // Channel
#define TSCAN_CH                    1
    // Baud initialize, TSCAN_BAUD_250000bps ..., other = by TSCanInitBaud()
#define TSCAN_INIT_Baud             TSCAN_BAUD_250000bps
    // Clock select, 0 = Oscillator clock, 1 = Bus clock
#define TSCAN_INIT_ClkSel           1
    // Identifier Filter, 0 = Two 32bits, 1 = Four 16bits, 2 = Eight 8bits, 3 = Close
#define TSCAN_INIT_Idam             0
    // Identifier Acceptance & Mask, 0 = invaild, 1 = vaild
#define TSCAN_INIT_Idar0            0x00000000
#define TSCAN_INIT_Idmr0            0x00000000
#define TSCAN_INIT_Idar1            0x00000000
#define TSCAN_INIT_Idmr1            0x00000000
    // Buffer
#define TSCAN_SEND_Max              14
#define TSCAN_REC_Max               14
    // Transport protocol
#define TSCAN_TP_En                 0
#define TSCAN_TP_SEND_ByteMax       9
#define TSCAN_TP_REC_ByteMax        9
#define TSCAN_TP_BAM_SEND_ByteMax   9
#define TSCAN_TP_BAM_REC_ByteMax    9
    // Receive process
#define TSCAN_REC_ProcMax           1
    #if TS_TSCAN_PagedRAM_En == 1
#pragma DATA_SEG __RPAGE_SEG PAGED_RAM
    #endif
TSCanRecProcDataDefine =
{    
    { CanJ1939ToEid(0x1A, 0x00, 0x10, 0x00), CanJ1939ToEid(0x1f, 0xff, 0xff, 0xff) + 1, TSNopFn,CtRecIsr  }, // 绝缘模块通讯协议
    { CanJ1939ToEid(0x09, 0x41, 0x55, 0x00), CanJ1939ToEid(0x1f, 0xff, 0xff, 0x00) + 1, CtRecSlaveStatus, TSNopFn }, // All SAE 1939-21 frame    
};
    #if TS_TSCAN_PagedRAM_En == 1
#pragma DATA_SEG DEFAULT
    #endif
    // Function
#define TSCAN_Ds_En                 1
#define TSCAN_RecProcSetJ1939Ps_En  0
/*
    Private
*/
#define Tscan_Private
#include "tscan.h"
/*----------------------------------------------------------------------------
    Define
----------------------------------------------------------------------------*/
#define CT_Debug                1
#define Ct_Ctrl_On              0x00
#define Ct_Ctrl_Off             0x01



#define Ct_Pm_Flag_Set          0x01
#define Ct_Pm_Flag_Reset        0x00


#define Cr_Pm_Num               (McData.Para.Pm.Num + 1)        
   
/*
    Period
*/
#define Ct_RequstDelay          (0)
#define Ct_RequstPeriod         (TS_TIME_1S/50)
#define Ct_RefreshPeriod        (TS_TIME_1S/2)
/*----------------------------------------------------------------------------
    MaCto
----------------------------------------------------------------------------*/
/*
    Register
*/
#define CbRegRxId               (TSCanData.Rec.Isr.Id&CanJ1939ToEid(0x03,0xff,0xff,0xff))
#define CbRegRxDlc              TSCanData.Rec.Isr.Dlc
#define CbRegRxData0            TSCanData.Rec.Isr.Data[0]
#define CbRegRxData1            TSCanData.Rec.Isr.Data[1]
#define CbRegRxData2            TSCanData.Rec.Isr.Data[2]
#define CbRegRxData3            TSCanData.Rec.Isr.Data[3]
#define CbRegRxData4            TSCanData.Rec.Isr.Data[4]
#define CbRegRxData5            TSCanData.Rec.Isr.Data[5]
#define CbRegRxData6            TSCanData.Rec.Isr.Data[6]
#define CbRegRxData7            TSCanData.Rec.Isr.Data[7]
#define CbRegTxId               TSCanData.Send.Id
#define CbRegTxDlc              TSCanData.Send.Dlc
#define CbRegTxData0            TSCanData.Send.Data[0]
#define CbRegTxData1            TSCanData.Send.Data[1]
#define CbRegTxData2            TSCanData.Send.Data[2]
#define CbRegTxData3            TSCanData.Send.Data[3]
#define CbRegTxData4            TSCanData.Send.Data[4]
#define CbRegTxData5            TSCanData.Send.Data[5]
#define CbRegTxData6            TSCanData.Send.Data[6]
#define CbRegTxData7            TSCanData.Send.Data[7]
/*----------------------------------------------------------------------------
    Variable
----------------------------------------------------------------------------*/
struct CtDataS CtData;

/*----------------------------------------------------------------------------
    Declaration
----------------------------------------------------------------------------*/
static void CtEnInit(void);
static void CtRec(void);
static void CtRefresh(void);
static void CtRecStatusRefresh(void);
static void CtProcessRequstInit(void);
static void CtProcessRequstMaster(void);
static void CtSendSnapStep(INT8U snapstep);
static void CtSendRequstID(void);
static void CtSendAssigID(INT8U pmaddr);
static void CtSendStart(INT8U pmaddr);
static void CtSendSkip(INT8U pmaddr);
static void CtSendPing(INT8U pmaddr);
static void CtSendCtrlCmd(void);
static void CtSendAdjust(void);
static void CtNop(void);
/*----------------------------------------------------------------------------
    Initialize
----------------------------------------------------------------------------*/
void CtEn(void)
{
    CtData.IsrRun = TSNopFn;
    CtData.RecRun = TSNopFn;
    if(TSCanData.Sta.Bits.Curnt == TSCAN_STA_Ds) TSCanInit(CtEnInit);

}
void CtDs(void)
{
    TSCanDs();
    //TSTaskDelete(TS_ID_Ct);
}
static void CtEnInit(void)
{
    INT8U i;

    TSCanRecEn(); 
    for(i = 0; i < CT_PM_Size; i++)
    {
        CtData.Pm[i].AssignId = i + 1;
        CtData.Pm[i].Status  = Ct_Pm_Sta_Offline;
        CtData.Pm[i].FlagRefresh = Ct_Pm_Flag_Reset;
    } 
    CtData.Sta.Bits.RecRun = 0;
    CtData.Para.Cmd = Ct_Ctrl_Off;    
    CtData.IsrRun = CtProcessRequstInit;
    CtData.Sta.Bits.IsrRun = 1;
    //McData.Run.Sdn.Bits.PmTout = 1;   
}
static void CtNop(void)
{
    TSNop();
}
/*----------------------------------------------------------------------------
    Refresh
----------------------------------------------------------------------------*/
#define Ct_StaRefresh_Cnt       (TS_TIME_1S / Ct_RefreshPeriod)
#define Ct_Pm_Tout_Cnt          ((TS_TIME_1S * 10) /Ct_RefreshPeriod)
static void CtRefresh(void)
{
    static INT16U stacnt = 0;
    static INT16U pm_toutcnt = 0;
    INT8U i;
    CtSendCtrlCmd();
    CtSendAdjust();

    for(i = 0; i < (Cr_Pm_Num) ;i++)
    {
        if(CtData.Pm[i].Status  != Ct_Pm_Sta_Offline)
        {
            CtSendPing(CtData.Pm[i].AssignId);
        }
    }
    
    if((++stacnt) > Ct_StaRefresh_Cnt)
    {
        stacnt = 0;
        CtRecStatusRefresh();
    }
    if((++pm_toutcnt) > Ct_Pm_Tout_Cnt)
    {
        pm_toutcnt = 0;
        for(i = 0; i < Cr_Pm_Num ;i++)
        {
            
            if(CtData.Pm[i].FlagRefresh == Ct_Pm_Flag_Set)
            {
                CtData.Pm[i].FlagRefresh = Ct_Pm_Flag_Reset;
            }
            else 
            {
                if(CtData.Pm[i].Status == Ct_Pm_Sta_Online)
                {
                    if(CtData.ModuleCnt > 0)
                    {
                        CtData.ModuleCnt--;
                        if(CtData.ModuleCnt == 0)
                        {
                            McData.Run.Sdn.Bits.PmTout = 1;
                        }
                    }
                    CtData.Pm[i].Status = Ct_Pm_Sta_Offline;
                    CtData.Pm[i].OutVol = 0;
                    CtData.Pm[i].OutCur = 0;
                }      
                //McData.Run.Sdn.Bits.PmTout = 1;
                CtData.Sta.Bits.IsrRun = 1;
            }
        }
    }   
}
static void CtRecStatusRefresh(void)
{
    INT8U i;
    INT16U a;
    INT32U e;
    struct McDataRunStS *z;
  
    e = 0;
    a = 0;
    if(CtData.ModuleCnt <= Cr_Pm_Num)
    {
        for(i = 0; i < Cr_Pm_Num; i++)
        {
            z = & McData.Run.St[i];
            if(CtData.Pm[i].Status  == Ct_Pm_Sta_Online)
            {
                e += CtData.Pm[i].OutVol;
                a += CtData.Pm[i].OutCur;
            }
            z -> Vol = CtData.Pm[i].OutVol; 
            z -> Cur = CtData.Pm[i].OutCur;  
            z -> ToutFlag = CtData.Pm[i].Status;
        }
        McData.Run.Ypm.Rec.Vol = (INT16U)(e / CtData.ModuleCnt);
        McData.Run.Ypm.Rec.Cur = a;
    }  
}
/*
    ISR run
*/
static void CtRecIsr(void)
{
    if(CtData.Sta.Bits.IsrRun)
    {
        CtData.Sta.Bits.IsrRun = 0;
        (* CtData.IsrRun)();
    }
}
/*
    Rec run
*/
static void CtRec(void)
{
    if(CtData.Sta.Bits.RecRun)
    {
        CtData.Sta.Bits.RecRun = 0;
        (* CtData.RecRun)();
        TSTaskIeDelete(TS_IEID_CtRec);
    }  
}
/*
    Requst
*/
static void CtProcessRequstInit(void)
{  
    TSTaskDelete(TS_ID_CtRequst);
    TSTaskCreate(TS_ID_CtRequst,CtProcessRequstMaster,Ct_RequstDelay,Ct_RequstPeriod);
}
static void CtProcessRequstMaster(void)
{
    static INT8U step = 0;
    static INT8U cnt  = 0;
    INT8U i;
   
    switch(step)
    {
        case 0:
        {
            if((CtData.ModuleCnt > Cr_Pm_Num))  
            {
                step = 0;
                break;               
            }
            for(i = 0; i < Cr_Pm_Num; i++)//有模块发送请求，分配空闲ID
            {
                if(CtData.Pm[i].Status == Ct_Pm_Sta_Offline)
                {
                    cnt = i;
                    CtData.Pm[cnt].Status = Ct_Pm_Sta_Requsted;
                    break;
                }    
            }
            step++;           
        }
        case 1:
        {
            CtSendSnapStep(step++);
            break;
        }
        case 2:
        {
            CtSendSnapStep(step++);
            break;
        }
        case 3:
        {
            CtSendSnapStep(step++);
            break;
        }
        case 4:
        {
            CtSendSnapStep(step++);
            break;
        }
        case 5:
        {
            CtSendRequstID();
            step++;
            break;
        }
        case 6:
        {
            CtSendAssigID(CtData.Pm[cnt].AssignId);
            step++;
            break;
        }
        case 7:
        {
            CtSendSkip(CtData.Pm[cnt].AssignId);
            step++;
            break;
        }
        case 8:
        {
            CtSendStart(CtData.Pm[cnt].AssignId);
            step++;
            break;
        }
        case 9:
        {
            CtSendPing(CtData.Pm[cnt].AssignId);           
            step = 0;
            if((++CtData.ModuleCnt) < Cr_Pm_Num)
            {
                CtData.Sta.Bits.IsrRun = 1;            
            }
            McData.Run.Sdn.Bits.PmTout = 0;
            TSTaskDelete(TS_ID_CtRequst);
            TSTaskDelete(TS_ID_CtRefresh);
            TSTaskCreate(TS_ID_CtRefresh,CtRefresh,0,Ct_RefreshPeriod);
            break;
        }
        default:
        {           
            break;
        }
    }
}
static void CtSendSnapStep(INT8U snapstep)
{ 
    INT8U a;
    
    a = 0x20 + snapstep - 1;
    TSCanData.Send.Id = CanJ1939ToEid(0x1A, 0x10, a , 0x00);
    TSCanData.Send.Dlc = 0;
    TSCanSend();
}
static void CtSendRequstID(void)
{
    TSCanData.Send.Id = CanJ1939ToEid(0x1A, 0x10, 0x30, 0x00);
    TSCanData.Send.Dlc = 0;
    TSCanSend();
}
static void CtSendAssigID(INT8U pmaddr)
{
    TSCanData.Send.Id = CanJ1939ToEid(0x1B, 0x50, 0x32, pmaddr);
    TSCanData.Send.Dlc = 0;
    TSCanSend();
}
static void CtSendSkip(INT8U pmaddr)
{
    TSCanData.Send.Id = CanJ1939ToEid(0x19, 0x50, 0x22, pmaddr);
    TSCanData.Send.Dlc = 0;
    TSCanSend();
}
static void CtSendStart(INT8U pmaddr)
{
    TSCanData.Send.Id = CanJ1939ToEid(0x01, 0x50, 0x31, pmaddr);
    TSCanData.Send.Dlc = 0;
    TSCanSend();
}
static void CtSendPing(INT8U pmaddr)
{
    TSCanData.Send.Id = CanJ1939ToEid(0x1D, 0x50, 0x40, pmaddr);
    
    TSCanData.Send.Dlc = 2;
    TSCanData.Send.Data[0] = 0x29;
    TSCanData.Send.Data[1] = 0x80;
    TSCanSend();
}
static INT16U CtRecPmVolConver(INT16U vol, INT8U mul);
static INT16U CtRecPmCurConver(INT16U data,INT8U mul);
static void CtRecSlaveStatus(void)
{
    INT8U e;
    INT16U v ;
    INT16U c ; 
    INT8U slave;
   
    slave = TSCanEidToJ1939Sa(TSCanData.Rec.Id) - 1;
    
    if(slave < Cr_Pm_Num)
    {
        
        if(CtData.Pm[slave].Status != Ct_Pm_Sta_Offline)
        {
             CtData.Pm[slave].FlagRefresh = Ct_Pm_Flag_Set;
             CtData.Pm[slave].Status = Ct_Pm_Sta_Online;
             // Vol
             e = (TSCanData.Rec.Data[0] & 0xc0) >> 6;
             v = ((TSCanData.Rec.Data[0] & 0x1f) << 8 )+ TSCanData.Rec.Data[1];
             CtData.Pm[slave].OutVol = CtRecPmVolConver(v,e);
             //Cur
             e = (TSCanData.Rec.Data[2] & 0xe0) >> 5;
             c = ((TSCanData.Rec.Data[2] & 0x0f) << 8) + TSCanData.Rec.Data[3];
             CtData.Pm[slave].OutCur = CtRecPmCurConver(c,e);   
             //Sta
             //CtData.Pm[slave].Sta.Bits.SubSta =  TSCanData.Rec.Data[7] ;
             //CtData.Pm[slave].Sta.Bits.MainSta = TSCanData.Rec.Data[6] & 0x0f ;                  
        }         
    }
}

static INT16U CtRecPmVolConver(INT16U vol, INT8U mul)
{
   INT16U e,a;
   INT8U  i;
   
   a = 1;
   for(i = (3- mul); i > 0; i--)
   {
       a *= 10;
   }
   e = vol * 10 / a;
   return e;
}
static INT16U CtRecPmCurConver(INT16U cur,INT8U mul)
{
    INT16U e,a;
    INT8U i;
    
    a = 1;
    if(mul <= 3)
    {
        for(i = (3- mul); i > 0; i--)
        {
            a *= 10;
        }
        e = cur * 10 / a;
        return e;
    }
}
/*
    Send
*/
//cmd
static void CtSendCtrlCmd(void)
{
    TSCanData.Send.Id = CanJ1939ToEid(0x09, 0x50, 0x81, 0x00); //广播帧
    TSCanData.Send.Dlc = 1;
    TSCanData.Send.Data[0] = CtData.Para.Cmd;
    TSCanSend();
}
static void CtCtrlSub(INT8U cmd)
{
    CtData.Para.Cmd  = cmd;
}
void CtCtrlSubOn(void)
{
    CtCtrlSub(Ct_Ctrl_On);
}
void CtCtrlSubOff(void)
{
    CtCtrlSub(Ct_Ctrl_Off);
}
//adjust
static void CtSendCmdConverVol(INT16U vol);
static void CtSendCmdConverCur(INT16U cur);
static void CtSendCmdConverPower(void);

static void CtSendAdjust(void)
{
    
    if(McData.Para.Ctrl.Bits.Bm == 0)
    {
        CtSendCmdConverCur(McData.Para.ChgPara.Cur*10/Cr_Pm_Num);
        CtSendCmdConverVol(McData.Para.ChgPara.Vol*10);
    }
    else 
    {
        if(McData.Run.Cb.MaxAllowCur < McData.Para.ChgPara.Cur) 
        {
            CtSendCmdConverCur(McData.Run.Cb.MaxAllowCur*10/Cr_Pm_Num);
        }
        else  
        {
            CtSendCmdConverCur(McData.Para.ChgPara.Cur*10/Cr_Pm_Num);
        }
        
        if(McData.Run.Cb.MaxAllowVol < McData.Para.ChgPara.Vol) 
        {
            CtSendCmdConverVol(McData.Run.Cb.MaxAllowVol*10);
        }
        else  
        {
            CtSendCmdConverVol(McData.Para.ChgPara.Vol*10);
        }
    }
    CtSendCmdConverPower();
    
    TSCanData.Send.Id = CanJ1939ToEid(0x09, 0x50, 0x8A, 0x00);//广播帧
    TSCanData.Send.Dlc = 6;
    TSCanData.Send.Data[0] = (INT8U)(CtData.Para.Vol >> 8);
    TSCanData.Send.Data[1] = (INT8U)CtData.Para.Vol;
    TSCanData.Send.Data[2] = (INT8U)(CtData.Para.Cur >> 8);
    TSCanData.Send.Data[3] = (INT8U)CtData.Para.Cur;
    TSCanData.Send.Data[4] = (INT8U)(CtData.Para.Power >> 8);
    TSCanData.Send.Data[5] = (INT8U)CtData.Para.Power;
    TSCanSend();
}
static void CtSendCmdConverVol(INT16U vol)
{
  
    CtData.Para.Vol = (2U << 14) + vol;
}
static void CtSendCmdConverCur(INT16U cur)
{

    CtData.Para.Cur = (2U << 13) + cur;
}
static void CtSendCmdConverPower(void)
{
    CtData.Para.Power = 0x23E8;
}
/*----------------------------------------------------------------------------
    End
----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
    By Tenudy
----------------------------------------------------------------------------*/
