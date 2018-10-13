clear;
x = 1:21;
sum = 0;
tries = 1000; 
f = waitbar(0.0, 'asd', 'Name', 'Test RandomSplit()');
msg = GetRandomMessage(i/tries);
for i = 1 : tries
    counter = TestRandomSplit(x, false);
    sum = sum + counter;
    waitbar(i/tries, f, GetRandomMessage(i/tries));
end
avg = floor(sum / tries + 0.5);
fprintf('Average calls to get no split: %d\n', avg);
waitbar(i/tries,...
        f,...
        strcat('Vector of length(', num2str(length(x)), ...
               ') NOT split in <1:', num2str(avg), '> calls'));
pause(5);
close(f);