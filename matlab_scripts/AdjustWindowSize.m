function [ w_out, h_out ] = AdjustWindowSize( fig, path_l, path_n, w, h )
%ADJUSTWINDOWSIZE Summary of this function goes here
%   Detailed explanation goes here
temp = path_l * 6.5;
if (temp > w)
    w_out = temp;
else
    w_out = w;
end

h_out = h;
set(fig, 'Position', [684 458 w_out h_out]);
end

