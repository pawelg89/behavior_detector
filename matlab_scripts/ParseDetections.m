function [ result ] = ParseDetections( file_path )
%ParseDetections Summary of this function goes here
%   Detailed explanation goes here
text_file = fileread(file_path);

% Load just sequence of numbers
for i = 1 : length(text_file)
    if (text_file(i) == ':')
        short = text_file(i + 2 : length(text_file));
        break;
    end
end
short = strcat('| 0', short, ' |');

% Convert to vector of integers
idx_of_spaces = strfind(short, ' ');
detections = zeros(1, length(idx_of_spaces) - 1);
for i = 1 : length(idx_of_spaces) - 1
    detections(i) = str2num(short(idx_of_spaces(i):idx_of_spaces(i+1)));
end

% fill in gaps between numbers with N/D
ctr = 1;
detections_filled = zeros(1, detections(length(detections)));
for i = 1 : length(detections_filled)
    if i == detections(ctr)
        detections_filled(i) = detections(ctr);
        ctr = ctr + 1;
    end
end

% Parse them into a vertical vector
result = detections_filled;
end

