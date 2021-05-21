% ParseRiptideLog.m
% ------------------------------------------------------------------------------
% Alex Higgins, higgins AT ece DOT pdx DOT edu
% Created: 14-Jan-2019 15:45:00
%     
% This script contains the procedure to process Riptide MOOS .slog files. These
% files contain various vehicle sensor data from each mission.
% 
% ------------------------------------------------------------------------------
%



%% Setup workspace
clearvars; close all; clc;

%% Load project library
run(['.' filesep 'libuuv.m'])

%% Initialize variables and Open log file
aname = 'Riptide Parse MOOS Log';   % Working set name
fl = 1;     % File handle to log script output [STDOUT=1, STDERR=2]
dts = hndl_timestamp();
% hndl_diary_on([saveDir filesep strrep(lower(aname),' ','_') '_' dts '.log']);
fprintf(fl, [aname ':: Started\n']);
t0 = tic;

%% Task 1 - Parse .slog file
%  
%  
%  

fprintf(fl, 'Task-1\n---\n');
% msnDir = [dataDir filesep 'HenryHaggLake19Sep2018' filesep 'Riptide' filesep 'MOOSLog_19_9_2018_____01_18_25'];
msnDir = [dataDir filesep 'HAgg_2021_02)03' filesep 'Riptide' filesep 'MOOSLog_3_2_2021_____20_22_48'];
fprintf(fl, 'Parse SoundTrap data files in [%s] ...\n', msnDir)
logFile = dir([msnDir filesep '*.slog']);
fprintf(fl, 'Found system log [%s] ...\n', logFile.name)
outFile = [saveDir filesep logFile.name(1:end-5) '.dat'];
% 
fid = fopen([logFile.folder filesep logFile.name],'r');
% Move to initial unixtime of log file
for k=1:3, fgetl(fid); end
% fseek(fid,26,'cof'); init_unixtime = fscanf(fid,'%f\n');
init_unixtime = fscanf(fid,'%%%% LOGSTART               %f\n');
% Reset file pointer and look for column headers
frewind(fid)
for k=1:66, fgetl(fid); end
fseek(fid,3,'cof');
hdrs = strsplit(fgetl(fid));
hdrs = hdrs(1:end-1);
% Reset file pointer and look for first data row/column
frewind(fid)
for k=1:68, fgetl(fid); end
fod = fopen(outFile,'w');
fprintf(fod,'#');
fprintf(fod,' %s',hdrs{:});
fprintf(fod,'\n');
dline = fgetl(fid);
while ischar(dline)
    pline = strsplit(dline);
    if ~strcmp(pline{1},'%%')
        pline = pline(1:end-1);  
    %     dlmwrite(outFile,pline,'-append','Delimiter',',')
        for k=1:60, fprintf(fod, '%s,', pline{k}); end
        fseek(fod,-1,'cof');
        fprintf(fod,'\n');
    end
    dline = fgetl(fid);
end
fclose(fod);
fclose(fid);
% 
data = dlmread(outFile,',',1,0);
data(:,1) = data(:,1)+init_unixtime;
% data(:,1) = datetime(init_unixtime+data(:,1),'ConvertFrom','posixtime','TimeZone','America/Los_Angeles');
% data(:,1) = datetime(init_unixtime+data(:,1),'ConvertFrom','posixtime','Format','yyyy-MM-dd HH:mm:ss.SSS');
% data(:,1) = datetime(init_unixtime+data(:,1),'ConvertFrom','posixtime','Format','yyyy-MM-dd''T''HH:mmXXX');
hdrs_eln = [1 12:18 22:24 33:36 42:45 53 58:59];
data_tbl = array2table(data(:,hdrs_eln),'VariableNames',hdrs(hdrs_eln));
data_tbl.Properties.Description = logFile.name(1:end-5);
data_tbl = rmmissing(data_tbl,'MinNumMissing',21);
data_tbl = fillmissing(data_tbl,'previous');
% data_timetbl = array2timetable(data(:,hdrs_eln(2:end)),'VariableNames',hdrs(hdrs_eln(2:end)),'RowTimes',datetime(data(:,1),'ConvertFrom','posixtime','Format','yyyy-MM-dd HH:mm:ss.SSS'));
% data_timetbl.Properties.Description = logFile.name(1:end-5);
% data_timetbl = rmmissing(data_tbl,'MinNumMissing',21);
% data_timetbl = fillmissing(data_timetbl,'previous');
fod = fopen([outFile(1:end-4) '.csv'],'w');
fprintf(fod,'%s,',hdrs{hdrs_eln});
fseek(fod,-1,'cof');
fprintf(fod,'\n');
fclose(fod);
dlmwrite([outFile(1:end-4) '.csv'],data(:,hdrs_eln),'delimiter',',','-append','Precision',15);
save([outFile(1:end-4) '.mat'],'data_tbl','-mat','-v7.3')

fprintf(1,'Pares files are found at \"%s\"\n', saveDir)
dir([saveDir filesep sprintf('%s.*',msnDir(end-30:end))])

%% Load .MAT file and plot Data

load([saveDir filesep 'MOOSLog_19_9_2018_____19_32_40.mat'])

fObj = figure(1);
fObj.Name = 'IMU_Head';
plot(datetime(data_tbl.TIME,'ConvertFrom','posixtime','TimeZone','America/Los_Angeles','Format','yyyy-MM-dd HH:mm:ss.SSS'),data_tbl.IMU_HEADING,'.');
ylabel('IMU\_Heading')
grid('on')
grid('minor')
title({'Riptide UUV'; 'Hagg Lake 19-Sep-2018';''})

fObj = figure(2);
fObj.Name = 'NAV_Alt';
plot(datetime(data_tbl.TIME,'ConvertFrom','posixtime','TimeZone','America/Los_Angeles','Format','yyyy-MM-dd HH:mm:ss.SSS'),data_tbl.NAV_ALTITUDE,'.');
ylabel('NAV\_Altitude')
grid('on')
grid('minor')
title({'Riptide UUV'; 'Hagg Lake 19-Sep-2018';''})

fObj = figure(3);
fObj.Name = 'GPS_LatLon';
ax = gobjects(2);
ax(1) = subplot(2,1,1);
  plot(datetime(data_tbl.TIME,'ConvertFrom','posixtime','TimeZone','America/Los_Angeles','Format','yyyy-MM-dd HH:mm:ss.SSS'),data_tbl.GPS_LATITUDE,'.');
  ylabel('GPS\_Latitude')
  grid('on')
  grid('minor')
  title({'Riptide UUV'; 'Hagg Lake 19-Sep-2018';''})
ax(2) = subplot(2,1,2);
  plot(datetime(data_tbl.TIME,'ConvertFrom','posixtime','TimeZone','America/Los_Angeles','Format','yyyy-MM-dd HH:mm:ss.SSS'),data_tbl.GPS_LONGITUDE,'.');
  ylabel('GPS\_Longitude')
  grid('on')
  grid('minor')
  title({'Riptide UUV'; 'Hagg Lake 19-Sep-2018';''})
linkaxes(ax,'x')

fObj = figure(4);
fObj.Name = 'Nav_XY';
ax = gobjects(2);
ax(1) = subplot(2,1,1);
  plot(datetime(data_tbl.TIME,'ConvertFrom','posixtime','TimeZone','America/Los_Angeles','Format','yyyy-MM-dd HH:mm:ss.SSS'),data_tbl.NAV_Y,'.');
  ylabel('NAV\_Y')
  grid('on')
  grid('minor')
  title({'Riptide UUV'; 'Hagg Lake 19-Sep-2018';''})
ax(2) = subplot(2,1,2);
  plot(datetime(data_tbl.TIME,'ConvertFrom','posixtime','TimeZone','America/Los_Angeles','Format','yyyy-MM-dd HH:mm:ss.SSS'),data_tbl.NAV_X,'.');
  ylabel('NAV\_X')
  grid('on')
  grid('minor')
  title({'Riptide UUV'; 'Hagg Lake 19-Sep-2018';''})
linkaxes(ax,'x')

% Convert Table into matrix of doubles
raw_data = data_tbl.Variables;

%% Save Figures
% 

fext = 'png';
for n=1:4
    fObj = figure(n);
    figFileName = [saveDir filesep msnDir(end-30:end) '-'  fObj.Name];
    saveas(fObj,figFileName,fext);
    fprintf(1, 'Saved Fig%03d "%s.%s" ...\n', n, figFileName, fext)
end

%% Task 2 - Parse all .slog file(s) in a directory
%  
%  
%  

fprintf(fl, 'Task-2\n---\n');
topDir = [dataDir filesep 'HenryHaggLake19Sep2018' filesep 'Riptide'];
dirs = dir([topDir filesep 'MOOSLog*']);
nd = length(dirs);
dir_cnt = 0;
for k=1:nd
    if (dirs(k).isdir)
        msnDir = [topDir filesep dirs(k).name];
        fprintf(fl, 'Parse SoundTrap data files in [%s] ...\n', msnDir)
        logFile = dir([msnDir filesep '*.slog']);
        fprintf(fl, '\tFound system log [%s]\n', logFile.name)
        outFile = [saveDir filesep logFile.name(1:end-5) '.dat'];
        % 
        fid = fopen([logFile.folder filesep logFile.name],'r');
        % Move to initial unixtime of log file
        for kk=1:3, fgetl(fid); end
        % fseek(fid,26,'cof'); init_unixtime = fscanf(fid,'%f\n');
        init_unixtime = fscanf(fid,'%%%% LOGSTART               %f\n');
        % Reset file pointer and look for column headers
        frewind(fid)
        for kk=1:66, fgetl(fid); end
        fseek(fid,3,'cof');
        hdrs = strsplit(fgetl(fid));
        hdrs = hdrs(1:end-1);
        % Reset file pointer and look for first data row/column
        frewind(fid)
        for kk=1:68, fgetl(fid); end
        fod = fopen(outFile,'w');
        fprintf(fod,'#');
        fprintf(fod,' %s',hdrs{:});
        fprintf(fod,'\n');
        dline = fgetl(fid);
        while ischar(dline)
            pline = strsplit(dline);
            if ~strcmp(pline{1},'%%')
                pline = pline(1:end-1);  
                for kk=1:60, fprintf(fod, '%s,', pline{kk}); end
                fseek(fod,-1,'cof');
                fprintf(fod,'\n');
            end
            dline = fgetl(fid);
        end
        fclose(fod);
        fclose(fid);
        % 
        data = dlmread(outFile,',',1,0);
        data(:,1) = data(:,1)+init_unixtime;
        hdrs_eln = [1 12:18 22:24 33:36 42:45 53 58:59];
        data_tbl = array2table(data(:,hdrs_eln),'VariableNames',hdrs(hdrs_eln));
        data_tbl.Properties.Description = logFile.name(1:end-5);
        data_tbl = rmmissing(data_tbl,'MinNumMissing',21);
        data_tbl = fillmissing(data_tbl,'previous');
        fod = fopen([outFile(1:end-4) '.csv'],'w');
        fprintf(fod,'%s,',hdrs{hdrs_eln});
        fseek(fod,-1,'cof');
        fprintf(fod,'\n');
        fclose(fod);
        dlmwrite([outFile(1:end-4) '.csv'],data(:,hdrs_eln),'delimiter',',','-append','Precision',15);
        fprintf(fl, '\tWrote raw data to [%s] ...\n', [outFile(1:end-4) '.csv'])
        save([outFile(1:end-4) '.mat'],'data_tbl','-mat','-v7.3')
        fprintf(fl, '\tWrote data table to [%s] ...\n', [outFile(1:end-4) '.mat'])
        dir_cnt = hndl_incr(dir_cnt);
    end
end

fprintf(fl, 'Processed %d system logs\n', dir_cnt)

%% Task 3 - Concatenate all parsed .csv file(s) in a directory
%  
%  
%  

fprintf(fl, 'Task-3\n---\n');
files = dir([saveDir filesep 'MOOSLog*.csv']);
nd = length(files);
files_cnt = 0;
data_tbl = readtable([saveDir filesep files(1).name]);
for csvFile={files(2:end).name}
    curFile = char(csvFile);
    fprintf(fl, 'Loading %s ...\n', curFile)
    tmp_tbl = readtable([saveDir filesep curFile]);
    [rows_num, ~] = size(tmp_tbl);
    data_tbl = [data_tbl; tmp_tbl];
    fprintf(fl, '\t Added %d rows to the data table\n', rows_num)
%     hdrs = dlmread([saveDir filesep curFile],',',[0 0 22 1]);
%     data = dlmread([saveDir filesep curFile],',',1,0);
    files_cnt = hndl_incr(files_cnt);
end
data_tbl.Properties.Description = 'MOOSLog_19_9_2018';
data_tbl = fillmissing(data_tbl,'previous');
save([saveDir filesep 'MOOSLog_19_9_2018.mat'],'data_tbl','-mat','-v7.3')

%% Close log file
fprintf(fl, [aname sprintf(':: Completed %.2f (min)\n', toc(t0)/60)]);
fprintf(fl, '\n');
hndl_diary_off();
