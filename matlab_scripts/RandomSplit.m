function [ accepted, rejected ] = RandomSplit( vec_to_split )
%RANDOMSPLIT - is a function that splits vector to two by coin toss.
%   Function visits each element on input vector and performs a check: 
%   -------- CODE --------
%     if (rand >= 0.5)
%       %GOES TO ACCEPTED
%     else
%       %GOES TO REJECTED
%     end
%   ----------------------
%   vec_to_split - [in] vector with numbers to split.
%   accepted     - [out] vector of accepted values; '-1' if no value has
%                  been selected.
%   rejected     - [out] vector of rejected values; '-1' if no value has
%                  been selected.
accepted = zeros(1, length(vec_to_split));
rejected = zeros(1, length(vec_to_split));
accepted_count = 0;
rejected_count = 0;
% Perform split
for i = 1 : length(vec_to_split)
    if (rand >= 0.5)
        accepted_count = accepted_count + 1;
        accepted(accepted_count) = vec_to_split(i);
    else
        rejected_count = rejected_count + 1;
        rejected(rejected_count) = vec_to_split(i);
    end
end
% Copy non zero values to output vector
if accepted_count == 0
    accepted = -1;
else
    accepted = accepted(1:accepted_count);
end
if rejected_count == 0
    rejected = -1;
else
    rejected = rejected(1:rejected_count);
end

end
