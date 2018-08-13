clear;
output = AnalyzeData('badania_dane\collected_data.txt');

figure(2);
for i = 1 : data_size
    hist(collected_data.data(i,:));
    hold on;
end

