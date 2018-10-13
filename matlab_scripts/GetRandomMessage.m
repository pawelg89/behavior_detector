function [ out_message ] = GetRandomMessage( progress )
%GETRANDOMMESSAGE Summary of this function goes here
%   Detailed explanation goes here
perc = floor(progress * 100);
msg_list(1) = struct('message', ...
                     strcat(num2str(perc),'% Program running...'));
msg_list(2) = struct('message', ...
                     strcat(num2str(perc),'% Program is still running...'));
msg_list(3) = struct('message', ...
                     strcat(num2str(perc),'% Dont worry it will eventually finish...'));

idx = floor(progress * length(msg_list) + 1);
if idx <= 0
    idx = 1;
end
if idx > length(msg_list)
    idx = length(msg_list);
end
if (idx >= 1 && idx <= length(msg_list))
    out_message = msg_list(idx).message;
else
    out_message = 'that progress does not look right';
end

end

