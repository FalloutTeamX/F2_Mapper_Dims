//---------------------------------------------------------------------------
#ifndef mainH
#define mainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "utilites.h"
#include "log.h"
#include "map.h"
#include "tileset.h"
#include "frmset.h"
#include "lists.h"
#include "msg.h"
#include "proset.h"
#include "objset.h"
#include "pal.h"
#include "properts.h"
#include <ExtCtrls.hpp>
#include <Graphics.hpp>
#include <ComCtrls.hpp>
#include <Menus.hpp>
#include <Buttons.hpp>
#include <ImgList.hpp>
#include <ddraw.h>
#include "info.h"
// Функции max и min. winterheart, 04.04.2011
#include <algorithm>
using namespace std;
//---------------------------------------------------------------------------
class TfrmEnv : public TForm
{
  typedef signed char SBYTE;

__published:	// IDE-managed Components
        TPanel *imgMap;
        TShape *shp;
        TPanel *imgObj;
        TTabControl *tabc;
        TImage *imgMiniMap;
        TShape *Shape2;
        TShape *Shape3;
        TShape *Shape4;
        TLabel *lblMM;
        TLabel *lblInfo;
        TShape *Shape5;
        TScrollBar *sb;
        TPopupMenu *popupMap;
        TMenuItem *N1;
        TSpeedButton *btnWorld;
        TShape *Shape6;
        TSpeedButton *btnLocal;
        TImageList *ImageList1;
        TMenuItem *piProperties;
        TMenuItem *piDelete;
        TMenuItem *N2;
        TStatusBar *sbar;
        TShape *Shape1;
        TShape *Shape7;
        TLabel *lblinv;
        TScrollBar *sbINV;
        TPanel *imgInv;
        TShape *Shape8;
        TSpeedButton *btnAdd;
        TSpeedButton *btnRemove;
        TSpeedButton *btnChange;
        TLabel *Label1;
   TLabel *lblDir;
        TShape *Shape9;
        TPanel *ObjectPanel;
        TPanel *InvenPanel;
        TPanel *MinimapPanel;
        TPopupMenu *invPopup;
        TMenuItem *piChange;
        TMenuItem *N3;
        TMenuItem *piRemove;
   TMenuItem *piAdd;
   TMenuItem *piChangeMode;
   TMenuItem *piPickDrawObject;
   TMenuItem *N4;
   TSpeedButton *btnRotateLeft;
   TSpeedButton *btnRotateRight;
   TShape *Shape10;
   TComboBox *KeyControl;
   TSpeedButton *btnItemInfo;
   TMenuItem *piTilesToNavigator;
   TMenuItem *piUseObject;
        void __fastcall FormDestroy(TObject *Sender);
        void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
        void __fastcall imgMapMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
        void __fastcall imgMapMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
        void __fastcall imgMapMouseMove(TObject *Sender, TShiftState Shift,
          int X, int Y);
        void __fastcall imgMiniMapMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
        void __fastcall imgMiniMapMouseMove(TObject *Sender,
          TShiftState Shift, int X, int Y);
        void __fastcall imgMiniMapMouseUp(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
        void __fastcall imgMapClick(TObject *Sender);
        void __fastcall popupMapPopup(TObject *Sender);
        void __fastcall piPropertiesClick(TObject *Sender);
        void __fastcall tabcChange(TObject *Sender);
        void __fastcall imgObjMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
        void __fastcall FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
        void __fastcall btnWorldClick(TObject *Sender);
        void __fastcall btnLocalClick(TObject *Sender);
        void __fastcall sbScroll(TObject *Sender, TScrollCode ScrollCode,
          int &ScrollPos);
        void __fastcall piDeleteClick(TObject *Sender);
        void __fastcall imgInvMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
        void __fastcall btnChangeClick(TObject *Sender);
        void __fastcall btnAddClick(TObject *Sender);
        void __fastcall btnRemoveClick(TObject *Sender);
        void __fastcall sbINVChange(TObject *Sender);
        void __fastcall FormKeyUp(TObject *Sender, WORD &Key,
          TShiftState Shift);
        void __fastcall piChangeModeClick(TObject *Sender);
   void __fastcall piPickDrawObjectClick(TObject *Sender);
   void __fastcall lblMMClick(TObject *Sender);
   void __fastcall imgObjDblClick(TObject *Sender);
   void __fastcall btnRotateLeftClick(TObject *Sender);
   void __fastcall btnRotateRightClick(TObject *Sender);
   void __fastcall KeyControlKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
   void __fastcall KeyControlKeyUp(TObject *Sender, WORD &Key,
          TShiftState Shift);
   void __fastcall piTilesToNavigatorClick(TObject *Sender);
   void __fastcall btnItemInfoClick(TObject *Sender);
   void __fastcall piUseObjectClick(TObject *Sender);

private:	// User declarations
        CUtilites *pUtil;
        CLog *pLog;
        CFrmSet *pFrmSet;
        CProSet *pProSet;
        CListFiles *pLstFiles;
        CMsg *pMsg;
        CPal *pPal;
        TList *pRndObj;
        bool mouseBLeft, mouseBLeft2;
        bool mouseBRight, mouseBRight2;
        bool mouseBMiddle;
        bool shiftDown, ctrlDown, altDown;
        int downX, downY, upX, upY; //for Mouse at Map
        int downHexX, downHexY;
        int downTileX, downTileY;
        int cursorX, cursorY;
        int prevX, prevY, have_sel; // for selection-frame drawing
        int TileX, TileY, tile_sel;
        int HexX, HexY;
        int OldX, OldY; // for Locator
        int OldShowObjX, OldShowObjY; // for indicating obj
        HANDLE h_map;
        Graphics::TBitmap *pBMPlocator;
        Classes::TWndMethod OldMapWndProc;
        Classes::TWndMethod OldNavWndProc;
        Classes::TWndMethod OldInvWndProc;
        struct OBJNAVIGATOR
        {
           BYTE nObjType;     // Текущий тип объекта
           DWORD nSelID;      // Текущий выбранный объект для рисования.
           DWORD nNavID[50];  //[8] Резервируем дополнительное место для слотов
           BYTE nShowMode;
           int nCount;
           int nMaxID;
           int cScrollPos[7];  // Текущее положение ползунка скрола для всех категории навигатора
           DWORD cSelectedID;  // Выбранный объект в категории (используется для запоминания выбора)
        } Navigator;
        struct OBJINVENTORY
        {
           BYTE *pObj;
           BYTE *pChildObj[25];  //[3] Резервируем дополнительное место для слотов
           int nItemNum;
           int nInvStartItem;
           int iLevel;
        } Inventory;

        bool drawBlocker;
        bool inventoryDraw;

        struct OBJSELECT
        {
           SBYTE nObjType;    // Текущий тип объекта
           DWORD nSelID;      // Текущий ID объекта
           BYTE  *pSelObj;    // Текущий выбранный объект
        }  CurMapObjSelect;

        void __fastcall NewMapWndProc(TMessage &Msg);
        void __fastcall NewNavWndProc(TMessage &Msg);
        void __fastcall NewInvWndProc(TMessage &Msg);
        void DrawMiniMap(void);
        void RedrawFloor(void);
        void RerawObjects(void);
        void RedrawRoof(void);
        void RedrawObjects(void);
        void PreRedrawBlockers(void);
        void RedrawBlockers(int nBlockType, BYTE nObjType, WORD nFrmID);
        void RedrawHex(void);
        void RedrawLocator(void);
        void RedrawPickObject(bool force, int nID);
        void SelectObjXY(int X, int Y);
        bool SelectObjRegion(int X, int Y, int X2, int Y2);
        void SelectionChanged(void);
        bool SelectTileRegion(int TileMode, int X, int Y, int X2, int Y2);
        void LogSelected(void);

        void ShowBlockers();
        WORD GetFrmID(BYTE nObjType, int nID);
        void LoadObjID(int pID, BYTE typeID);
        bool HexIsBlock(int objHexX, int objHexY);
        void MouseMiddleUp();
        String GetMapItemsInfo();
        DWORD CheckDirection(int direction, DWORD nDir, WORD nMaxDir);
        void SetGlobalObjectDir(int dir = 0);
        int GetIndexSubType(BYTE nObjType, int nID);

public:		// User declarations
        TfrmProperties *frmProp;
        int SelectMode, SelectModeMBM;
        CMap *pMap;
        CTileSet *pTileSet;
        CObjSet *pObjSet;
        int WorldX, OldWorldX;
        int WorldY, OldWorldY;
        int iLevel;
        bool bShowObj[16];
        bool blockOnTop;
        int randomObject;

        LPDIRECTDRAWSURFACE7 dds, dds2Map, dds2Nav, dds2Inv;
        LPDIRECTDRAWCLIPPER ddc, ddcMap, ddcNav, ddcInv;
        TStatusPanel *panelHEX;
        TStatusPanel *panelTILE;
        TStatusPanel *panelObjCount;
        TStatusPanel *panelObjSelected;
        TStatusPanel *panelMSG;

        void ClearSelection(void);
        void ClearFloorSelection(bool NeedRedrawMap);
        void ClearRoofSelection(bool NeedRedrawMap);
        void ClearObjSelection(bool NeedRedrawMap);
        void MoveSelectedObjects(int offsetHexX, int offsetHexY);
        void RotateSelectedObjects(int direction);
        void SetButtonSave(bool State, bool uState = false);
        void RedrawMap(bool StaticRedraw);
        void PrepareNavigator(BYTE nObjType);
        void RedrawNavigator(int highlight = -1);
        void ResetInventoryInfo(void);
        void RedrawInventory(void);

        int ObjectRandomDraw(int index);
        void RandomObjState(bool enable);
        void DrawBlock(int type);
        void UndoDelete(bool undo);
        void MapCaptionInfo();

        void TransBltFrm(CFrame* frm, TControl*, short nDir, short nFrame,
                                       int x, int y, LPDIRECTDRAWSURFACE7 dds2);
        void TransBltMask(CFrame* frm, TControl*, short nDir, short nFrame,
                int x, int y, LPDIRECTDRAWSURFACE7 dds2, int width, int height);
        void TransBltTileRegion(LPDIRECTDRAWSURFACE7 dds2,
                                int TileX1, int TileY1, int TileX2, int TileY2,
                                TColor clr, int wPen = 1);
        void RepaintDDrawWindow(TWinControl *win, LPDIRECTDRAWSURFACE7 dds,
                          LPDIRECTDRAWSURFACE7 dds2, LPDIRECTDRAWCLIPPER ddc);
        void AttachDDraw(TControl *win, LPDIRECTDRAWSURFACE7 *dds, int primary);

        __fastcall TfrmEnv(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmEnv *frmEnv;
//---------------------------------------------------------------------------
#endif
