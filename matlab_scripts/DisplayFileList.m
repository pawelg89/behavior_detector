function DisplayFileList( in )
%DISPLAYFILELIST Simple function to print out 'in' structure.
%   Input param 'in' needs to be of size[1 n] with field 'path'.
for i=1:length(in)
    display(in(i).path);
end

end

