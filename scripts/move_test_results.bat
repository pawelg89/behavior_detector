echo OFF
echo Hello World
set dir_path=%1
set log_file=%2
set events_dir=%dir_path%\events
shift
shift
echo %dir_path% %log_file% 
mkdir %dir_path%
copy /y parameters.txt %dir_path%
copy /y descriptors_list.txt %dir_path%
copy /y behavior_names_list.txt %dir_path%
move %log_file% %dir_path%
move collected_data.txt %dir_path%
rd /s /q %events_dir%
move events %events_dir%