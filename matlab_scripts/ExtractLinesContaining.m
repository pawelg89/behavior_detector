function [] = ExtractLinesContaining(filename, line_has)
%Read and display the file fgetl.m one line at a time:
output = fopen(strcat(line_has, filename), 'W+t');
input = fopen(filename);
tline = fgetl(input);
while ischar(tline)
    if strfind(tline, line_has)
        %disp(tline)
        fprintf(output, strcat(tline,'\n'));
    end
    tline = fgetl(input);
end
fclose(input);
fclose(output);