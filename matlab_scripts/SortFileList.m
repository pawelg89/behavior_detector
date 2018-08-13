function [ output_args ] = SortFileList( file_list )
%SORTFILELIST Summary of this function goes here
%   Detailed explanation goes here

for i = 1 : length(file_list)
    swap_done = false;
    for j = 2 : length(file_list)
        if length(file_list(j).path) < length(file_list(j-1).path)
            temp = file_list(j-1).path;
            file_list(j-1).path = file_list(j).path;
            file_list(j).path = temp;
            swap_done = true;
        end
    end
    if swap_done == false
        break;
    end
end
output_args = file_list;

end

