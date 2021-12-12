object PlaceSpiralTrackForm: TPlaceSpiralTrackForm
  Left = 471
  Top = 150
  HorzScrollBar.Visible = False
  VertScrollBar.Visible = False
  BorderIcons = [biSystemMenu]
  Caption = 'Place Spiral Track'
  ClientHeight = 358
  ClientWidth = 462
  Color = clBtnFace
  DragMode = dmAutomatic
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  FormStyle = fsStayOnTop
  OldCreateOrder = False
  OnCreate = PlaceSpiralTrackFormCreate
  PixelsPerInch = 96
  TextHeight = 13
  object Label13: TLabel
    Left = 145
    Top = 177
    Width = 44
    Height = 13
    Caption = 'Start X:'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
    Transparent = True
  end
  object Label14: TLabel
    Left = 177
    Top = 202
    Width = 13
    Height = 13
    Caption = 'Y:'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
    Transparent = True
  end
  object Label16: TLabel
    Left = 14
    Top = 68
    Width = 285
    Height = 13
    Caption = #1052#1072#1082#1089#1080#1084#1072#1083#1100#1085#1099#1081' '#1088#1072#1076#1080#1091#1089' '#1074#1085#1091#1090#1088#1077#1085#1085#1077#1081' '#1082#1088#1080#1074#1086#1081' ('#1084#1084')'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
    Transparent = True
  end
  object Label18: TLabel
    Left = 48
    Top = 118
    Width = 250
    Height = 13
    Caption = #1056#1072#1076#1080#1091#1089' '#1074#1085#1077#1096#1085#1077#1081' '#1086#1082#1088#1091#1078#1085#1086#1089#1090#1080' '#1092#1080#1075#1091#1088#1099' ('#1084#1084')'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
    Transparent = True
  end
  object Label1: TLabel
    Left = 22
    Top = 92
    Width = 278
    Height = 13
    Caption = #1052#1080#1085#1080#1084#1072#1083#1100#1085#1099#1081' '#1088#1072#1076#1080#1091#1089' '#1074#1085#1091#1090#1088#1077#1085#1085#1077#1081' '#1082#1088#1080#1074#1086#1081' ('#1084#1084')'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
    Transparent = True
  end
  object Label2: TLabel
    Left = 118
    Top = 44
    Width = 179
    Height = 13
    Caption = #1050#1086#1083#1080#1095#1077#1089#1090#1074#1086' '#1091#1075#1083#1086#1074#1099#1093' '#1086#1090#1089#1095#1077#1090#1086#1074
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
    Transparent = True
  end
  object XPBitBtn1: TXPBitBtn
    Left = 289
    Top = 316
    Width = 75
    Height = 25
    Caption = 'OK'
    TabOrder = 0
    OnClick = XPBitBtn1Click
  end
  object XPBitBtn2: TXPBitBtn
    Left = 369
    Top = 316
    Width = 75
    Height = 25
    Caption = 'Cancel'
    TabOrder = 1
    OnClick = XPBitBtn2Click
  end
  object StartX: TEdit
    Left = 193
    Top = 177
    Width = 70
    Height = 19
    BevelEdges = []
    BorderStyle = bsNone
    Color = clWhite
    Ctl3D = False
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clBlue
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentCtl3D = False
    ParentFont = False
    TabOrder = 2
    Text = '2000'
  end
  object StartY: TEdit
    Left = 193
    Top = 202
    Width = 70
    Height = 19
    BevelEdges = []
    BorderStyle = bsNone
    Color = clWhite
    Ctl3D = False
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clBlue
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentCtl3D = False
    ParentFont = False
    TabOrder = 3
    Text = '2000'
  end
  object edMaxRadius: TEdit
    Left = 306
    Top = 68
    Width = 94
    Height = 19
    BevelEdges = []
    BorderStyle = bsNone
    Color = clWhite
    Ctl3D = False
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clBlue
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentCtl3D = False
    ParentFont = False
    TabOrder = 4
    Text = '20'
  end
  object edExtRadius: TEdit
    Left = 304
    Top = 118
    Width = 98
    Height = 19
    BevelEdges = []
    BorderStyle = bsNone
    Color = clWhite
    Ctl3D = False
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clBlue
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentCtl3D = False
    ParentFont = False
    TabOrder = 5
    Text = '21'
  end
  object edMinRadius: TEdit
    Left = 306
    Top = 92
    Width = 94
    Height = 19
    BevelEdges = []
    BorderStyle = bsNone
    Color = clWhite
    Ctl3D = False
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clBlue
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentCtl3D = False
    ParentFont = False
    TabOrder = 6
    Text = '15'
  end
  object edSamplesCnt: TEdit
    Left = 306
    Top = 44
    Width = 94
    Height = 19
    BevelEdges = []
    BorderStyle = bsNone
    Color = clWhite
    Ctl3D = False
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clBlue
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    NumbersOnly = True
    ParentCtl3D = False
    ParentFont = False
    TabOrder = 7
    Text = '4096'
  end
end
