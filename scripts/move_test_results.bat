echo OFF
echo.
echo.
echo MOVE SCRIPT CALLED! Some files may be missing from their original place.
set dir_path=%1
set log_file=%2
set events_dir=%dir_path%\events
shift
shift

rem Create proper directory structure
echo -----------------------------------------------------------------------------------
echo Making path: %dir_path%
mkdir %dir_path%
rem Copy configuration files
echo -----------------------------------------------------------------------------------
echo Copying parameters.txt, descriptors_list.txt, behavior_names_list.txt into selected directory.
copy /y parameters.txt %dir_path%
copy /y descriptors_list.txt %dir_path%
copy /y behavior_names_list.txt %dir_path%
rem Move log file 
echo -----------------------------------------------------------------------------------
echo Moving log file: '%log_file%' into selected directory.
move %log_file% %dir_path%
rem Move collected_data into appropriate directory. It contains all
rem test data collected during runtime.
echo -----------------------------------------------------------------------------------
echo Moving 'collected_data.txt' into selected dir_path.
move collected_data.txt %dir_path%
rem Move dir with detected behaviors
echo -----------------------------------------------------------------------------------
echo events_dir: %events_dir%
rem rd /s /q %events_dir%
move events %events_dir%
echo.
echo.
