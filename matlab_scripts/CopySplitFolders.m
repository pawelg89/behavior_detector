
train = [3 4 6 8 12 14 16 17 19 21];
validate = [1 2 5 7 9 10 11 13 15 18 20];

home_folder = 'C:/Users/Gardzin/Documents/DOKTORAT/DHA/Action/';
def_destination = 'C:/Users/Gardzin/Documents/DOKTORAT/BehaviorDetector2/BehaviorDetector/_BADANIA/';

% Ask for source path (cancel stops adding)
source = uigetdir(home_folder, 'Choose source folder to copy');
destination = uigetdir(def_destination, 'Choose destination folder');

for i=1:length(train)
    frompath = strcat(source, '\', num2str(train(i)), '\');
    topath = strcat(destination, '\train\', num2str(train(i)), '\');
    fprintf('Copy from %s to %s.\n', frompath, topath);
    copyfile(frompath, topath);
end
for i=1:length(validate)
    frompath = strcat(source, '\', num2str(validate(i)), '\');
    topath = strcat(destination, '\validate\', num2str(validate(i)), '\');
    fprintf('Copy from %s to %s.\n', frompath, topath);
    copyfile(frompath, topath);
end

