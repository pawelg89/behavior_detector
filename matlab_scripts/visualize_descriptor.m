clear;
home_folder = 'C:/Users/Gardzin/Documents/DOKTORAT/BehaviorDetector2/BehaviorDetector/_DESKRYPTORY';
% Pick file to visualize
[filename, pathname, filterindex] = uigetfile(home_folder, 'Choose descriptor to visualize');
path = strcat(pathname, filename);

text_file = fileread(path);
descriptor = LoadDescriptor(text_file);
[fig_handles] = VisualizeDescriptor(descriptor);