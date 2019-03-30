%parse_all_detections
% HELP TEXT goes here
%
clear;
pathname = 'dummy_path';
while (pathname ~= 0)
    % Get file via GUI
    home_folder = 'C:\Users\Gardzin\Documents\DOKTORAT\BehaviorDetector2\BehaviorDetector\_BADANIA\check_trained\';
    [filename, pathname, filterindex] = uigetfile(...
        strcat(home_folder, '*.txt'),...
        'Keep choosing files until done, then press "Cancel"');
    if (pathname ~= 0)
        path = strcat(pathname, filename);
        % Parse detections and generate a column easy to import to Excel
        parsed = ParseDetections(path);

        % Write result next to loaded file
        fid = fopen(strcat(pathname, '/parsed_detections.txt'), 'wt');
        fprintf(fid, '%d\n', parsed);
        fclose(fid);
    end
end