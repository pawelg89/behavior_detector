function [ counter ] = TestRandomSplit( x, print )
%TESTRANDOMSPLIT Summary of this function goes here
%   Detailed explanation goes here

stop = false;
counter = 1;
[a, r] = RandomSplit(x);
while ~stop
   [a, r] = RandomSplit(x);
   if ((a(1) == -1) || (r(1) == -1))
       stop = true;
   else
       counter = counter + 1;
   end
end
if print
    fprintf('Counter reached: %d.\n', counter);
    display(a);
    display(r);
end
end

