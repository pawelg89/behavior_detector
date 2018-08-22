function [ msg ] = CheckExtension( ext )
%CHECKEXTENSION Checks if extension is either .jpg or .png
%   Return message: 'OK' or 'Unsupported file extension' if check fails.
if strcmp(ext, '.png') || strcmp(ext, '.jpg')
    msg = 'OK';
else
    msg = 'Unsupported file extension';
end

end

