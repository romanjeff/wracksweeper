function [] = desired_heading_depth_speed()
%DESIRED_HEADING_DEPTH_SPEED
% 
%  Determine uUUV speed from parsed MOOS logs. 
%  
%  This file is from Riptide.
% 

[fn, pn, in] = uigetfile('*.slog')
f = load([pn fn]);
fig = figure
set(fig,'name',fn)

h1 = subplot(3,1,1)
hold
title(['DESIRED HEADING (deg)'])
i = find(isfinite(f(:,6)));
plot(f(i,1),f(i,6),'.-')

h2 = subplot(3,1,2)
hold
title(['DESIRED DEPTH (m)'])
i = find(isfinite(f(:,5)));
plot(f(i,1),f(i,5),'.-')


h3 = subplot(3,1,3)
hold
title(['DESIRED SPEED (m/s)'])
i = find(isfinite(f(:,7)));
plot(f(i,1),f(i,7),'.-')

linkaxes([h1 h2 h3],'x')

