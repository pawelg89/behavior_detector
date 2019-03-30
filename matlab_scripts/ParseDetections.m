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
short_to_show = strcat('{', short, '}')

% Parse them into a vertical vector
for i = 1 : length(short)
    if short(i) ~= ' '
        result(i) = short(i);
    else
        result(i) = char(10);
    end
end

% Add lines that are missing in between numbers



end

