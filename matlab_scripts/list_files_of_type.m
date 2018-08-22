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
path = uigetdir(home_folder, 'Choose root folder for search');
chosen_folders = 0;
list = struct([]);
% Create a listbox to show selected paths
fig = uifigure('Position', [684 458 430 250],...
               'Name', 'Selected directories');
listbox = uilistbox(fig,...
                    'Position', [0 0 1000 fig.Position(4)-3],...
                    'Items', {});
while path ~= 0
    chosen_folders = chosen_folders + 1;
    listbox.Items = [listbox.Items {path}];
    %display(path);
    temp = ListFilesOfType(path, extension, 1);
    list = [list, temp];
    path = uigetdir(home_folder, 'Choose root folder for search');
end
% Save combined list of multiple folders were chosen
if chosen_folders > 1
    write_path = uigetdir(home_folder, 'Choose destination to save list.');
    WriteFileList(write_path, list, input(custom_fn_prompt));
end
