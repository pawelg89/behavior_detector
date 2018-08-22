function WriteFileList( path, list, file_name )
%WRITEFILELIST Writes given list to a path.
%   INPUT:
%   path      (required) - string defining write directory,
%   list      (required) - struct containing list of strings to write,
%   file_name (optional) - default: 'list_all', can be set different.
if nargin < 2
    error('Input "path" and "list" are required parameters');
elseif nargin < 3
    file_name = 'list_all';
end
fid = fopen(strcat(path, '/', file_name, '.txt'), 'wt');
fid2 = fopen(strcat(path, '/', file_name,'_frames.txt'), 'wt');
for i = 1:length(list)
    fprintf(fid, '%s\n', list(i).path);
    fprintf(fid2, '%s\n', list(i).path);
end
fclose(fid);
fclose(fid2);

end

