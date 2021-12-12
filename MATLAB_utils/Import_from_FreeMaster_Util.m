fmstr = actxserver ('MCB.PCM');
fmstr.GetCurrentRecorderData;
recdata=cell2mat(fmstr.LastRecorder_data);
recnames=fmstr.LastRecorder_serieNames;
fmstr.ReadVariable('g_fmstr_smpls_period');
st = fmstr.LastVariable_vValue;
fs = 1/st;
endtime = st*length(recdata);

data_for_simulink = transpose(recdata);
t = transpose(linspace(0, st*(length(recdata)-1),length(recdata) ));
data_for_simulink = horzcat(t,data_for_simulink);

%------------------------------------------------------------
% Выводим графики с данными 
%------------------------------------------------------------
colrs = ['rrrrrmbgyckw']; % матрица цветов

x = linspace(0, st*(length(recdata)-1),length(recdata) ); % подготавливаем массив значений для оси X
scrsz = get(groot,'ScreenSize');
figure('OuterPosition',[1 50 scrsz(3) scrsz(4)-50]); % 
n = length(recnames);
%whitebg - Инверсия цветов
for i = 1:n
   d = recdata(i,:); 
   ax = subplot(n,1,i);
   p = plot(ax, x, d); 
   p.Color = colrs(i);
   dmax = max(d);
   dmin = min(d);
   dstd = std(d);
   % 
   title(sprintf('IC%i, Min= %.2f, Max= %.2f, Range= %.2f, Std= %.2f',i, dmin, dmax, dmax-dmin, dstd) );
end

