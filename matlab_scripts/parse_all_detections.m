%parse_all_detections
% HELP TEXT goes here
%
clear;

% Get file via GUI
home_folder = 'D:/Github/_BADANIA/check_trained/';
[filename, pathname, filterindex] = uigetfile(...
    strcat(home_folder, '*.txt'),...
    'Choose a file');
path = strcat(pathname, filename);

% Parse detections and generate a column easy to import to Excel
parsed = ParseDetections(path);

% Write result next to loaded file
fid = fopen(strcat(pathname, '/parsed_detections.txt'), 'wt');
fprintf(fid, '%s\n', parsed);
fclose(fid);