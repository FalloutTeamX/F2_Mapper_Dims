object frmConfig: TfrmConfig
  Left = 181
  Top = 416
  BorderIcons = [biSystemMenu]
  BorderStyle = bsDialog
  Caption = 'Mapper configuration'
  ClientHeight = 371
  ClientWidth = 402
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  PixelsPerInch = 96
  TextHeight = 13
  object lblMapperSilver: TLabel
    Left = 8
    Top = 341
    Width = 91
    Height = 28
    AutoSize = False
    Caption = 'Mapper'
    Color = clNone
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clBtnText
    Font.Height = -20
    Font.Name = 'Arial Black'
    Font.Style = [fsBold, fsItalic]
    ParentColor = False
    ParentFont = False
    Transparent = True
  end
  object lblMapperGray: TLabel
    Left = 7
    Top = 340
    Width = 91
    Height = 28
    AutoSize = False
    Caption = 'Mapper'
    Color = clNone
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clSilver
    Font.Height = -20
    Font.Name = 'Arial Black'
    Font.Style = [fsBold, fsItalic]
    ParentColor = False
    ParentFont = False
    Transparent = True
  end
  object gb1: TGroupBox
    Left = 6
    Top = 0
    Width = 390
    Height = 190
    Caption = ' Fallout location '
    TabOrder = 2
    object lbl1: TLabel
      Left = 14
      Top = 45
      Width = 38
      Height = 13
      Caption = 'Dat files'
    end
    object btnBrowse: TButton
      Left = 360
      Top = 17
      Width = 25
      Height = 21
      Hint = 'Click to browse'
      Caption = '...'
      ParentShowHint = False
      ShowHint = True
      TabOrder = 0
      OnClick = btnBrowseClick
    end
    object edGameDir: TEdit
      Left = 8
      Top = 17
      Width = 350
      Height = 21
      ReadOnly = True
      TabOrder = 1
    end
    object lvDAT: TListView
      Left = 8
      Top = 59
      Width = 377
      Height = 121
      Columns = <
        item
          Caption = 'File Name'
          Width = 120
        end
        item
          Caption = 'Size'
          Width = 80
        end
        item
          Caption = 'Description'
          Width = 170
        end>
      GridLines = True
      ReadOnly = True
      TabOrder = 2
      ViewStyle = vsReport
    end
  end
  object btnOK: TButton
    Left = 242
    Top = 343
    Width = 75
    Height = 25
    Caption = '&OK'
    Default = True
    ModalResult = 1
    TabOrder = 0
    OnClick = btnOKClick
  end
  object btnCancel: TButton
    Left = 321
    Top = 343
    Width = 75
    Height = 25
    Cancel = True
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 1
  end
  object gb2: TGroupBox
    Left = 6
    Top = 190
    Width = 390
    Height = 51
    Caption = ' Local data location '
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
    TabOrder = 3
    object edDataDir: TEdit
      Left = 8
      Top = 18
      Width = 350
      Height = 21
      ReadOnly = True
      TabOrder = 0
    end
    object btnBrowse2: TButton
      Left = 360
      Top = 17
      Width = 25
      Height = 21
      Hint = 'Click to browse'
      Caption = '...'
      ParentShowHint = False
      ShowHint = True
      TabOrder = 1
      OnClick = btnBrowse2Click
    end
  end
  object gb3: TGroupBox
    Left = 6
    Top = 244
    Width = 390
    Height = 93
    Caption = 'Options'
    TabOrder = 4
    object chb: TCheckBox
      Left = 7
      Top = 37
      Width = 377
      Height = 17
      Hint = 'Set this checkbox if strings in MSG-files are OEM encoded.'
      Caption = 'Strings conversion from OEM to ANSI'
      ParentShowHint = False
      ShowHint = True
      TabOrder = 0
    end
    object cbAllow: TCheckBox
      Left = 7
      Top = 15
      Width = 377
      Height = 21
      Hint = 
        'If the option is not set, then you can place the object by holdi' +
        'ng down the Ctrl key.'
      Caption = 'Allow placed objects on blocked hex by another objects'
      ParentShowHint = False
      ShowHint = True
      TabOrder = 1
    end
  end
end
