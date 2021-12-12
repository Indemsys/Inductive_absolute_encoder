
    Dim   Board
    DIM   xm,ym    'mouse coordinate
    Dim   seg

'-----------------------------------------------------------------------
Sub PlaceSpiralTrackFormCreate(Sender)

    Set Board = PCBServer.GetCurrentPCBBoard
    If Board is Nothing Then Exit Sub

    If Not Board.ChooseLocation(xm,ym,"Choose Spiral Track start location") Then
    End If

    PlaceSpiralTrackForm.StartX.Text = CoordToMMS(xm)
    PlaceSpiralTrackForm.StartY.Text = CoordToMMS(ym)

End Sub
'-----------------------------------------------------------------------

Sub XPBitBtn1Click(Sender)

    Dim   Via, Track, Arc
    Dim   X0, Y0
    Dim maxr, minr, arcr
    Dim Pol
    Dim segn
    Dim ptn





    ptn = 0
    N  = edSamplesCnt.Text/1
    nn = N/4
    pi = 3.14159265358979323846



    X0 = StartX.Text
    Y0 = StartY.Text
    maxr = edMaxRadius.Text/1.0
    minr = edMinRadius.Text/1.0
    arcr = edExtRadius.Text/1.0

    PlaceSpiralTrackForm.Hide

    PCBServer.PreProcess

    Pol =  PCBServer.PCBObjectFactory(ePolyObject, eNoDimension, eCreate_Default)
    Pol.PolyHatchStyle = ePolySolid
    Pol.NeckWidthThreshold = 0.01/1
    Pol.MitreCorners = False
    Pol.ArcApproximation = 0.001/1

    if arcr <> maxr then
      ptn = 1 + nn*2 + 2  + N/2
    else
      ptn = 1 + nn*2 + N/2
    end if

    Pol.SetState_PointCount ptn

    segn = 0
    Set seg = TPolySegment

    seg.Kind =  ePolySegmentLine     ' ��������� �����
    seg.vx = MMsToCoord(X0+arcr/1)
    seg.vy = MMsToCoord(Y0)

    Pol.SetState_Segments segn, seg
    segn = segn + 1

    ' ������ ������� ���� ���������
    FOR i = 1 TO N/2
      fi = pi*(i*2/N)
      r = arcr
      x = r*cos(fi) + X0
      y = r*sin(fi) + Y0
      seg.Kind =  ePolySegmentLine
      seg.vx = MMsToCoord(x)
      seg.vy = MMsToCoord(y)
      Pol.SetState_Segments segn, seg
      segn = segn + 1

    NEXT



 '   seg.Kind =  ePolySegmentArc      ' ����
 '   seg.cx = MMsToCoord(X0)
 '   seg.cy = MMsToCoord(Y0)
 '   seg.radius = MMsToCoord(arcr/1)
 '   seg.Angle1 = 0
 '   seg.Angle2 = 180
 '
 '   Pol.SetState_Segments segn, seg
 '   segn = segn + 1

    ' ��������� ������� ��� ���������� ���� � ��������� ������ ���������� ������
     if arcr <> maxr then
       x = -maxr/1 + X0
       y = Y0
       seg.Kind =  ePolySegmentLine
       seg.vx = MMsToCoord(x)
       seg.vy = MMsToCoord(y)
       Pol.SetState_Segments segn, seg
       segn = segn + 1
     end if

    ' ������ ������ ���������� ������ ����� �� �����

    FOR i = nn TO 1 STEP -1
      fi = (pi/2)*(i/nn)
      r = (fi * (maxr - minr)*2)/pi + minr/1
      x = r*cos(fi+pi/2) + X0
      y = r*sin(fi+pi/2) + Y0
      seg.Kind =  ePolySegmentLine
      seg.vx = MMsToCoord(x)
      seg.vy = MMsToCoord(y)
      Pol.SetState_Segments segn, seg
      segn = segn + 1

    NEXT

    ' ������ ������ ���������� ������ ����� �� �����

    FOR i = nn TO 1 STEP -1
      fi = (pi/2)*(i/nn)
      r = (fi * (minr-maxr)*2)/pi + maxr/1
      x = r*cos(fi) + X0
      y = r*sin(fi) + Y0
      seg.Kind =  ePolySegmentLine
      seg.vx = MMsToCoord(x)
      seg.vy = MMsToCoord(y)
      Pol.SetState_Segments segn, seg
      segn = segn + 1
    NEXT

    ' ��������� ������� ��� ���������� ���������� ������ � ������� ����
    if arcr <> maxr then
      x = maxr/1 + X0
      y = Y0
      seg.Kind =  ePolySegmentLine
      seg.vx = MMsToCoord(x)
      seg.vy = MMsToCoord(y)
      Pol.SetState_Segments segn, seg
      segn = segn + 1

    end if

    x = arcr/1 + X0
    y = Y0
    seg.Kind =  ePolySegmentLine      ' ��������� �����
    seg.vx = MMsToCoord(x)
    seg.vy = MMsToCoord(y)
    Pol.SetState_Segments segn, seg
    segn = segn + 1



   Board.AddPCBObject(Pol)


   PCBServer.PostProcess
    ' Refresh PCB workspace.
    ResetParameters
    Call AddStringParameter("Action", "Redraw")
    RunProcess("PCB:Zoom")
    'Client.SendMessage "Pcb:PlacePolygonPlane" , "Layer=Top" , 1024  , Nil

  Close
End Sub


'-----------------------------------------------------------------------
Sub Main
    PlaceSpiralTrackForm.showmodal
End Sub

'-----------------------------------------------------------------------
Sub XPBitBtn2Click(Sender)
    close
End Sub


