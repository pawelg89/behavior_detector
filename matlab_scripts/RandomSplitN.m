function [ accepted, rejected ] = RandomSplitN( x, n )
%RANDOMSPLITN Summary of this function goes here
%   Detailed explanation goes here
accepted = zeros(1, length(x));
count = 0;
while (length(accepted) ~= n || accepted(1) == -1)
    [accepted, rejected] = RandomSplit(x);
    count = count + 1;
end
if count == 1
    fprintf('[INFO] it took %d try to get it right. \n', count);
else
    fprintf('[INFO] it took %d tries to get it right. \n', count);
end
end

