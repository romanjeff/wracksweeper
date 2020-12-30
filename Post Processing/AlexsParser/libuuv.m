% libuuv.m
% ------------------------------------------------------------------------------
% Alex Higgins, higginja AT ece DOT pdx DOT edu
% 27-Jun-2018 10:43:57 
%
% A library of project wide settings, variables, constants, and functions/
% classes. Always called at the top/start of a parent script or func-
% tion when used. This loads all necessary items into the current
% workspace.
%
% ------------------------------------------------------------------------------
%
iif = @(varargin) varargin{2*find([varargin{1:2:end}], 1, 'first')}();
hndl_timestamp = @() sprintf('%4d%02d%02d%02d%02d%02d', int32(datevec(now)));
hndl_diary_on  = @(s) diary(s);
hndl_diary_off = @() evalc('diary off');
% hndl_cmap = @() colormap(1-colormap('bone'));
hndl_cmap = @() colormap('parula');
hndl_fft  = @(x,n) fftshift(fft(x, n));
hndl_ifft = @(x)   fftshift(ifft(ifftshift(x)));
hndl_incr = @(x) x+1;
hndl_decr = @(x) x-1;
hndl_frqs = @(fs,n) iif(~mod(n,2), ... %{ if even %}
                        @() -1/2:1/n:(1/2-1/n), ...
                        1, ...         %{ else odd %}
                        @() [-1/2:1/n:-1/n 0 1/n:1/n:1/2]) * fs;
statusbar = @(x,c,n,LB) iif( (x==1 && n ~= 0), ...
                           @() fprintf(['%+' num2str(n) 's%c'],'',c), ...
                           ~mod(x,LB), ...
                           @() fprintf(['%c\n%+' num2str(n) 's'],c,''), ...
                           1, @() fprintf('%c',c) );

unimu  = char(956);     % Unicode character for mu
unicor = char(8902);    % Unicode character for correlation
unidel = char(916);     % Unicode character for Delta
unideg = char(176);     % Unicode character for degree
unisig = char(963);     % Unicode character for sigma
uniang = char(952);     % Unicode character for theta
uniomg = char(969);     % unicode character for omega
uniphi = char(934);     % unicode character for phi
unibet = char(946);     % unicode character for beta
uniroh = char(961);     % unicode character for roh
unieta = char(958);     % unicode character for eta
unizet = char(950);     % unicode character for zeta
unieps = char(949);     % unicode character for epsilon

% c = 299792458;
% LoadConstant('c')

% Setup project directory structure
tmp = dir('.');
projDir = tmp(1).folder;
homeDir = getenv('HOME');
dataDir = [projDir filesep 'Data'];
srcDir  = projDir;
saveDir = [projDir filesep 'Output'];

addpath(srcDir,'-END')
clear tmp
