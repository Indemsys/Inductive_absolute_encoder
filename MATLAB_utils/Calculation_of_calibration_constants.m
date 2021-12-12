% � ������� ������������ ������ ���� �������� ������ recdata � ������� �������� ��
% 4-� �������� �� ����� ��� ������� ������ ������� �������� ������ � ���������� st ���������� ������ ������� ��������

% ����������� ������ ������� �������� ��������� �������� AX OX AY OY
% ����������� ��� ���������� ���� � ����� ������� ���� �������� ������ 
% 

% ������� ���������� �������� � ������ ��������
X = recdata(1,:) - recdata(3,:); % ��������� ������ X
Y = recdata(2,:) - recdata(4,:); % ��������� ������ Y ��������� ������������ X �� 90 ����

% ������� ������� ����  ������� X
[pX_h] = findpeaks(X,'MinPeakDistance',100,'MinPeakProminence',10000);
% ������� ������ ����  ������� X
[pX_l] = findpeaks(-X,'MinPeakDistance',100,'MinPeakProminence',10000);
pX_l = -pX_l; % ��������������� ���� 
AX = (pX_h(1)- pX_l(1))/2; % ��������� X
OX = (pX_h(1)+ pX_l(1))/2; % ��������  X

% ������� ������� ����  ������� Y
[pY_h] = findpeaks(Y,'MinPeakDistance',100,'MinPeakProminence',10000); % ��������� ����� ���� �� ���� ������� ������ ���� �� ����� � ������� ����������
% ������� ������ ����  ������� B
[pY_l] = findpeaks(-Y,'MinPeakDistance',100,'MinPeakProminence',10000);
pY_l = -pY_l; % ��������������� ���� 
AY = (pY_h(1)- pY_l(1))/2; % ��������� Y
OY = (pY_h(1)+ pY_l(1))/2; % ��������   Y

X = (X-OX)/AX; % ������������ ������� ���������������� � �������
Y = (Y-OY)/AY;

% ������� ���� ��� ������ ��������
x_axis = linspace(0, st*(length(recdata)-1),length(recdata) ); 
scrsz = get(groot,'ScreenSize');
figure('OuterPosition',[1 50 scrsz(3) scrsz(4)-50]); % 

sbpl = subplot(2,1,1); %  divides the current figure into an m-by-n grid and creates an axes for a subplot in the position specified by p
plot(sbpl, x_axis, X, x_axis, Y); 
title('���������������� ������� � ��������');
grid on;

A = atan(X./Y)*180/pi+90;
A = A + (Y>0).*180;
sbpl = subplot(2,1,2);
plot(sbpl, x_axis, A); 
title('���� �������� ������');
grid on;

% ������������� �������� � ����� �������� ��������� ���������� ����
[pA_h] = findpeaks(A,'MinPeakDistance',10,'MinPeakProminence',10);
[pA_l] = findpeaks(-A,'MinPeakDistance',10,'MinPeakProminence',10);
pA_l = -pA_l; % ��������������� ����
% �������� ������� ������������ �� ������ ���������� 
clear x_axis scrsz sbpl pX_h pX_l pY_h pY_l 


