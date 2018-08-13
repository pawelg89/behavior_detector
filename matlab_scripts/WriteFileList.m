function WriteFileList( path, list )
%WRITEFILELIST Summary of this function goes here
%   Detailed explanation goes here
fid = fopen(strcat(path, '/list_all.txt'), 'wt');
for i = 1:length(list)
    fprintf(fid, '%s\n', list(i).path);
end
fclose(fid);

end

