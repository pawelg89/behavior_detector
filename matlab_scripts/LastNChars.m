function [ result ] = LastNChars( input, n )
%LASTNCHARS Returns last 'n' characters of 'input' string.
%   Checks size and if size exceeds length(input) sets n = length(input)
%
if n < 0
    warning('n cannot be negative');
    return
end
if length(input) < n
    warning('n > length(input); setting n=length(input)');
    n = length(input);
end
m = n - 1;
result = input( (length(input) - m): length(input) );
end

