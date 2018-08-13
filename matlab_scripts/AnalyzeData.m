function [ means, mins, maxes ] = AnalyzeData( data_path )
%ANALYZEDATA Summary of this function goes here
%   Detailed explanation goes here
if size(data_path) == 0
    data_path = 'badania_dane\collected_data.txt';
end
collected_data = importdata(data_path);

data_size = size(collected_data.data);
means = zeros(data_size);
mins = zeros(data_size);
maxes = zeros(data_size);
for i = 1 : data_size
    mins(i) = min(collected_data.data(i,:));
    maxes(i) = max(collected_data.data(i,:));
    means(i) = mean(collected_data.data(i,:));
    semilogy(collected_data.data(i,:));
    hold on;
end
headers = strsplit(collected_data.textdata{1});
figure(1);
title('Time analysis')
legend(headers);
xlabel('Frame')
ylabel('Computing time [ms]')
grid on

for i = 1 : data_size
    fprintf('%s min: %3.8f[ms], max: %3.8f[ms], mean: %3.8f[ms]\n', headers{i},mins(i),maxes(i),means(i));    
end
end

