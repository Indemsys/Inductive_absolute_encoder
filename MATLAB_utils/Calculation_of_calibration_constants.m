% В рабочем пространстве должен быть загружен массив recdata с записью сигналов от
% 4-х датчиков во время как минимум одного полного поворота мишени и переменная st означающая период выборки сигналов

% Результатом работы скрипта является получение констант AX OX AY OY
% необходимых для вычисления угла и вывод графика угла поворота мишени 
% 

% Находим разностные значения с парных датчиков
X = recdata(1,:) - recdata(3,:); % Вычисляем сигнал X
Y = recdata(2,:) - recdata(4,:); % Вычисляем сигнал Y сдвинутый относительно X на 90 град

% Находим верхние пики  сигнала X
[pX_h] = findpeaks(X,'MinPeakDistance',100,'MinPeakProminence',10000);
% Находим нижние пики  сигнала X
[pX_l] = findpeaks(-X,'MinPeakDistance',100,'MinPeakProminence',10000);
pX_l = -pX_l; % Восстанавливаем знак 
AX = (pX_h(1)- pX_l(1))/2; % Амплитуда X
OX = (pX_h(1)+ pX_l(1))/2; % Смещение  X

% Находим верхние пики  сигнала Y
[pY_h] = findpeaks(Y,'MinPeakDistance',100,'MinPeakProminence',10000); % Указываем чтобы пики не были слишком близко друг от друга и слишком маленькими
% Находим нижние пики  сигнала B
[pY_l] = findpeaks(-Y,'MinPeakDistance',100,'MinPeakProminence',10000);
pY_l = -pY_l; % Восстанавливаем знак 
AY = (pY_h(1)- pY_l(1))/2; % Амплитуда Y
OY = (pY_h(1)+ pY_l(1))/2; % Смещение   Y

X = (X-OX)/AX; % Корректируем сигналы масштабированием и сдвигом
Y = (Y-OY)/AY;

% Готовим окно для вывода графиков
x_axis = linspace(0, st*(length(recdata)-1),length(recdata) ); 
scrsz = get(groot,'ScreenSize');
figure('OuterPosition',[1 50 scrsz(3) scrsz(4)-50]); % 

sbpl = subplot(2,1,1); %  divides the current figure into an m-by-n grid and creates an axes for a subplot in the position specified by p
plot(sbpl, x_axis, X, x_axis, Y); 
title('Масштабированные сигналы с сенсоров');
grid on;

A = atan(X./Y)*180/pi+90;
A = A + (Y>0).*180;
sbpl = subplot(2,1,2);
plot(sbpl, x_axis, A); 
title('Угол поворота мишени');
grid on;

% Дополнительно проверим в каких границах находится полученный угол
[pA_h] = findpeaks(A,'MinPeakDistance',10,'MinPeakProminence',10);
[pA_l] = findpeaks(-A,'MinPeakDistance',10,'MinPeakProminence',10);
pA_l = -pA_l; % Восстанавливаем знак
% Вычищаем рабочее пространство от лишних переменных 
clear x_axis scrsz sbpl pX_h pX_l pY_h pY_l 


