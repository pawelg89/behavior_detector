clear;
% Const strings
home_folder = 'C:/Users/Gardzin/Documents/DOKTORAT/DHA/Action/';
ext_input_prompt = 'What extension file do you want to list? (surround input with apostrophe) \n';
custom_fn_prompt = 'Choose file name with list (surround input with apostrophe):\n';
% Get file extension
extension = input(ext_input_prompt);
ext_check = CheckExtension(extension);
if strcmp(ext_check, 'OK')
    display(extension);
else
    warning(ext_check);
    return
end
% Ask for all folders that have to be added (cancel stops adding)
path = uigetdir(home_folder, 'Choose root folder for search.');
chosen_folders = 0;
list = struct([]);
% Create a listbox to show selected paths
w = 430;
h = 250;
fig = uifigure('Position', [684 458 w h],...
               'Name', 'Selected directories');
listbox = uilistbox(fig,...
                    'Position', [0 0 1000 fig.Position(4)-3],...
                    'Items', {});
last_path = path;
while path ~= 0
    chosen_folders = chosen_folders + 1;
    listbox.Items = [listbox.Items {path}];
    %display(path);
    temp = ListFilesOfType(path, extension, 1);
    list = [list, temp];
    last_path = path;
    [w, h] = AdjustWindowSize(fig, length(path), chosen_folders, w, h);
    path = uigetdir(path, 'Choose another root folder for search.');
end
% Save combined list of multiple folders were chosen
if chosen_folders > 1
    write_path = uigetdir(last_path, 'Choose destination to save list.');
    if write_path ~= 0
        WriteFileList(write_path, list, input(custom_fn_prompt));
    end
end
close(fig);
