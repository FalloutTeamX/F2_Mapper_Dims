//---------------------------------------------------------------------------
#ifndef mdiH
#define mdiH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Menus.hpp>
#include <ExtCtrls.hpp>
#include <Buttons.hpp>
#include <ComCtrls.hpp>
#include <ToolWin.hpp>
#include <Dialogs.hpp>
#include <ddraw.h>
#include "pbar.h"
#include <Graphics.hpp>
#include "log.h"
#include "datfile.h"
#include "utilites.h"
#include "pal.h"
#include "lists.h"
#include "frmset.h"
#include "proset.h"
#include "msg.h"
#include "properts.h"
#include "rndobj.h"

const int crHandCursor = 1;
const int crHandTakeCursor = 2;
const int crCrossCursor = 3;
const int crMyPenCursor = 4;
const int crMoveCursor = 8;
//---------------------------------------------------------------------------
class TfrmMDI : public TForm
{
__published:	// IDE-managed Components
        TControlBar *cb;
        TToolBar *tb;
        TSpeedButton *btnOpen;
        TSpeedButton *btnSave;
        TOpenDialog *OpenDialog1;
        TToolBar *tbcurs;
        TSpeedButton *btnselect1;
        TSpeedButton *btnhand;
        TSpeedButton *btnselect2;
        TToolBar *tbVis;
        TSpeedButton *btnRoof;
        TSpeedButton *btnFloor;
        TSpeedButton *btnItems;
        TSpeedButton *btnScenery;
        TSpeedButton *btnCritters;
        TSpeedButton *btnWalls;
        TSpeedButton *btnLightBlock;
        TSpeedButton *btnpen3;
        TSpeedButton *btnselect3;
        TSpeedButton *btnAbout;
        TSpeedButton *btnConfig;
        TSpeedButton *btnMisc;
        TSpeedButton *btnpen2;
        TSpeedButton *btnpen1;
        TSpeedButton *btnSaveAs;
        TSaveDialog *SaveDialog1;
        TSpeedButton *btnScrollBlock;
        TSpeedButton *btnmove;
        TSpeedButton *btnObjBlock;
        TSpeedButton *btnWallBlock;
        TSpeedButton *btnSaiBlock;
        TSpeedButton *btnEGBlock;
        TSpeedButton *btnObjSelfBlock;
        TToolButton *s7;
        TSpeedButton *btnBlockTop;
        TToolButton *s2;
        TToolButton *s3;
        TToolButton *s4;
        TToolButton *s5;
        TToolButton *s6;
        TToolBar *tbPaint;
        TSpeedButton *btnDrawRndObj;
        TToolButton *s1;
        TComboBox *PresetObj;
        TToolButton *s0;
        TSpeedButton *btnDrawSFBlock;
        TSpeedButton *btnDrawSBlock;
        TSpeedButton *btnDrawSAI;
        TSpeedButton *btnDrawWFBlock;
        TSpeedButton *btnDrawWBlock;
        TSpeedButton *btnNone;
        TToolButton *s8;
        TSpeedButton *btnHEX;
        TSpeedButton *btnUndo;

        void __fastcall btnOpenClick(TObject *Sender);
        void __fastcall btnhandClick(TObject *Sender);
        void __fastcall FormDestroy(TObject *Sender);
        void __fastcall btnFloorClick(TObject *Sender);
        void __fastcall btnRoofClick(TObject *Sender);
        void __fastcall btnItemsClick(TObject *Sender);
        void __fastcall btnCrittersClick(TObject *Sender);
        void __fastcall btnSceneryClick(TObject *Sender);
        void __fastcall btnWallsClick(TObject *Sender);
        void __fastcall btnMiscClick(TObject *Sender);
        void __fastcall btnselect1Click(TObject *Sender);
        void __fastcall btnselect2Click(TObject *Sender);
        void __fastcall btnselect3Click(TObject *Sender);
        void __fastcall btnpen3Click(TObject *Sender);
        void __fastcall btnConfigClick(TObject *Sender);
        void __fastcall FormCreate(TObject *Sender);
        void __fastcall btnAboutClick(TObject *Sender);
        void __fastcall btnEGBlockClick(TObject *Sender);
        void __fastcall btnSaveClick(TObject *Sender);
        void __fastcall btnpen1Click(TObject *Sender);
        void __fastcall btnpen2Click(TObject *Sender);
        void __fastcall btnSaveAsClick(TObject *Sender);
        void __fastcall btnScrollBlockClick(TObject *Sender);
        void __fastcall btnmoveClick(TObject *Sender);
        void __fastcall btnSaiBlockClick(TObject *Sender);
        void __fastcall btnWallBlockClick(TObject *Sender);
        void __fastcall btnObjBlockClick(TObject *Sender);
        void __fastcall btnLightBlockClick(TObject *Sender);
        void __fastcall btnObjSelfBlockClick(TObject *Sender);
        void __fastcall btnBlockTopClick(TObject *Sender);
        void __fastcall btnDrawRndObjClick(TObject *Sender);
        void __fastcall PresetObjChange(TObject *Sender);
        void __fastcall PresetObjKeyDown(TObject *Sender, WORD &Key,
               TShiftState Shift);
        void __fastcall PresetObjKeyUp(TObject *Sender, WORD &Key,
              TShiftState Shift);
        void __fastcall btnDrawSFBlockClick(TObject *Sender);
        void __fastcall btnDrawSBlockClick(TObject *Sender);
        void __fastcall btnDrawSAIClick(TObject *Sender);
        void __fastcall btnDrawWFBlockClick(TObject *Sender);
        void __fastcall btnDrawWBlockClick(TObject *Sender);
        void __fastcall btnUndoClick(TObject *Sender);
        void __fastcall btnHEXClick(TObject *Sender);

private:	// User declarations
public:		// User declarations
        CLog *pLog;
        TfrmPBar *frmPBar;
        TfrmProperties *frmProp;
        CUtilites *pUtil;
        CPal *pPal;
        CListFiles *pLstFiles;
        CFrmSet *pFrmSet;
        CProSet *pProSet;
        CMsg *pMsg;
        TList *RndObj; //CRandomObj *pRndObj;
        int iPos; // for Progress Bar
        int mainHeight, mainWidth;

//        IDirectDraw *dd;
        LPDIRECTDRAW7 pDD;

        __fastcall TfrmMDI(TComponent* Owner);
        bool InitClasses(void);
        void OpenPBarForm(void);
        void DeletePBarForm(void);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmMDI *frmMDI;
//---------------------------------------------------------------------------
#endif
