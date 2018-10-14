function [ list ] = ListFilesOfType( path, extension, lvl )
%LISTFILESOFTYPE Function that traverses input 'path' in search of files
%with given 'extension' e.g. "*.jpg", "*.png" or any other.
%   INPUT:
%   path      - global path to a directory that has to be searched,
%   extension - file extension, e.g. .jpg, .png or any other,
%   lvl       - specifies level in directory tree of path, will be used to
%               display loading bar if 'lvl == 1'. Lower levels have 'lvl+1'
%   OUTPUT:
%   list      - output list containing list of all files in depth-first
%               search fashion.

main_dir = dir(path);
simple_dir = struct([]);
for i = 3 : length(main_dir)
    simple_dir(i-2).path = main_dir(i).name;
end
simple_dir = SortFileList(simple_dir);
file_list = struct([]);
list = struct([]);
idx = 1;

if lvl == 1
    f = waitbar(0.0, LastNChars(strrep(path, '\', '/'), 40),...
        'Name', 'Listing files...');
end
for i = 1 : length(simple_dir)
    elem = simple_dir(i).path;
    if (size(strfind(elem, extension)) ~= 0)
        file_list(idx).path = strrep(strcat(path, '/', elem), '\', '/');
        idx = idx + 1;
    end
    if (size(strfind(elem, '.')) == 0)
        temp = ListFilesOfType(strcat(path, '/', elem), extension, lvl + 1);
        list = [list, temp];
    end
    if lvl == 1
        msg = strcat('...',LastNChars(path, 30), '/', elem);
        msg = strrep(msg, '\', '/');
        msg = strrep(msg, '_', '\_');
        waitbar(i/length(simple_dir), f, msg);
    end
end
list = [list, file_list];
WriteFileList(path, list);
if lvl == 1
    close(f);
end

end

