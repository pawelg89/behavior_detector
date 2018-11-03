function [ descr_m1, descr_m2, descr_m3, descr_m4 ] = ParseFrameData( frame_data )
%ParseFrameData Parses single frame of descriptor in text format.
%   IN: frame_data  - single frame of descriptor (all methods) as text
%   OUT: descr_m<x> - matrix 2xN containing floating point values of
%                     descriptor (1,:)-X and (2,:)-Y. One per method.
pairs_start = strfind(frame_data, '(');
pairs_end = strfind(frame_data, ')');
descr_m = 0;
p_count = 1;
for j = 1 : length(pairs_start)
    pair = frame_data(pairs_start(j):pairs_end(j));
    x = str2double(pair(2:strfind(pair, ',')-1));
    y = str2double(pair(strfind(pair, ',')+1:length(pair)-1));
    if (x > 1 && y > 1)
        descr_m = descr_m + 1;
        p_count = 1;
    end
    switch(descr_m)
        case {1}
            descr_m1_x(p_count) = x;
            descr_m1_y(p_count) = y;
            p_count = p_count + 1;
        case {2}
            descr_m2_x(p_count) = x;
            descr_m2_y(p_count) = y;
            p_count = p_count + 1;
        case {3}
            descr_m3_x(p_count) = x;
            descr_m3_y(p_count) = y;
            p_count = p_count + 1;
        case {4}
            descr_m4_x(p_count) = x;
            descr_m4_y(p_count) = y;
            p_count = p_count + 1;
    end
end
descr_m1 = [descr_m1_x; descr_m1_y];
descr_m2 = [descr_m2_x; descr_m2_y];
descr_m3 = [descr_m3_x; descr_m3_y];
descr_m4 = [descr_m4_x; descr_m4_y];

end

