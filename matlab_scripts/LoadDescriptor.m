function [ descriptor ] = LoadDescriptor( text_file )
%LoadDescriptor Load descriptor into memory.
%   Loads input descriptor 'text_file' (in text representation) into output
%   descriptor (in floating point values) that reflects spatio-temporal
%   structure of that descriptor.
pattern = 'descriptor[i][.]=';
descr_pos = strfind(text_file, pattern);
descr_pos = [descr_pos length(text_file)];

descriptor_m1 = cell(length(descr_pos)-1, 1);
descriptor_m2 = cell(length(descr_pos)-1, 1);
descriptor_m3 = cell(length(descr_pos)-1, 1);
descriptor_m4 = cell(length(descr_pos)-1, 1);
for i = 1 : length(descr_pos)-1
    frame_data = text_file(descr_pos(i)+length(pattern):descr_pos(i+1)-1);
    
    % Now parse frame data
    [descr_m1, descr_m2, descr_m3, descr_m4] = ParseFrameData(frame_data);
    descriptor_m1{i} = struct('x', descr_m1(1,:), 'y', descr_m1(2,:));
    descriptor_m2{i} = struct('x', descr_m2(1,:), 'y', descr_m2(2,:));%{descr_m2(1,:), descr_m2(2,:)};%{descr_m2};
    descriptor_m3{i} = struct('x', descr_m3(1,:), 'y', descr_m3(2,:));%{descr_m3(1,:), descr_m3(2,:)};%{descr_m3};
    descriptor_m4{i} = struct('x', descr_m4(1,:), 'y', descr_m4(2,:));%{descr_m4(1,:), descr_m4(2,:)};%{descr_m4};
end
descriptor = struct('m1', descriptor_m1, ...
                    'm2', descriptor_m2, ...
                    'm3', descriptor_m3, ...
                    'm4', descriptor_m4);
end




