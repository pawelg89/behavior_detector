function [ figure_handles ] = VisualizeDescriptor( descriptor )
%VisualizeDescriptor Draw 3D spatio-temporal descriptor, plot3D.
%   IN: descriptor - generated by LoadDescriptor function.
%   OUT: handles   - cell 1x4 with handles to 4 figures. One per method.

% Descriptor from method 1
close(figure(1));
figure_handles{1} = figure(1);
for i = 1 : length(descriptor)
    temp_x = zeros(1, (length(descriptor(i).m1.x)-1) * 3);
    temp_y = zeros(1, (length(descriptor(i).m1.x)-1) * 3);
    scale_x = descriptor(i).m1.x(1);
    scale_y = descriptor(i).m1.y(1);
    for j = 2 : length(descriptor(i).m1.x)
        if ( descriptor(i).m1.x(j) ~= -1 )
            temp_x(3*j-1) = 0.0; temp_y(3*j-1) = 0.0;
            temp_x(3*j) = descriptor(i).m1.x(j) * scale_x;
            temp_y(3*j) = descriptor(i).m1.y(j) * scale_y;
            temp_x(3*j+1) = 0.0; temp_y(3*j+1) = 0.0;
        end
    end
    plot3(temp_x, i * ones(length(temp_x)), temp_y);
    hold on;
    
end
grid on;
xlabel('X');
ylabel('Frame number');
zlabel('Y');

% Descriptor from method 2
close(figure(2));
figure_handles{2} = figure(2);
for i = 1 : length(descriptor)
    temp_x = zeros(1, (length(descriptor(i).m2.x)-1) * 3);
    temp_y = zeros(1, (length(descriptor(i).m2.x)-1) * 3);
    scale_y = descriptor(i).m2.y(1);
    for j = 2 : length(descriptor(i).m2.x)
        if ( descriptor(i).m2.x(j) ~= -1 )
            temp_x(3*j-1) = 0.0; temp_y(3*j-1) = 0.0;
            temp_x(3*j) = descriptor(i).m2.x(j) * scale_y;
            temp_y(3*j) = descriptor(i).m2.y(j) * scale_y;
            temp_x(3*j+1) = 0.0; temp_y(3*j+1) = 0.0;
        end
    end
    plot3(temp_x, i * ones(length(temp_x)), temp_y);
    hold on;
    
end
grid on;
xlabel('X');
ylabel('Frame number');
zlabel('Y');

% Descriptor from method 3
close(figure(3));
figure_handles{3} = figure(3);
for i = 1 : length(descriptor)
    temp_x = zeros(1, (length(descriptor(i).m3.x)-1) * 3);
    temp_y = zeros(1, (length(descriptor(i).m3.x)-1) * 3);
    scale_y = descriptor(i).m3.y(1);
    for j = 2 : length(descriptor(i).m3.x)
        temp_x(3*j-1) = 0.0; temp_y(3*j-1) = 0.0;
        temp_x(3*j) = descriptor(i).m3.x(j) * scale_y;
        temp_y(3*j) = descriptor(i).m3.y(j) * scale_y;
        temp_x(3*j+1) = 0.0; temp_y(3*j+1) = 0.0;
    end
    plot3(temp_x, i * ones(length(temp_x)), temp_y);
    hold on;
    
end
grid on;
xlabel('X');
ylabel('Frame number');
zlabel('Y');

% Descriptor from method 4
close(figure(4));
figure_handles{4} = figure(4);
for i = 1 : length(descriptor)
    temp_x = zeros(1, (length(descriptor(i).m4.x)-1) * 3);
    temp_y = zeros(1, (length(descriptor(i).m4.x)-1) * 3);
    scale_y = descriptor(i).m4.y(1);
    for j = 2 : length(descriptor(i).m4.x)
        temp_x(3*j-1) = 0.0; temp_y(3*j-1) = 0.0;
        temp_x(3*j) = descriptor(i).m4.x(j) * scale_y;
        temp_y(3*j) = descriptor(i).m4.y(j) * scale_y;
        temp_x(3*j+1) = 0.0; temp_y(3*j+1) = 0.0;
    end
    plot3(temp_x, i * ones(length(temp_x)), temp_y);
    hold on;
    
end
grid on;
xlabel('X');
ylabel('Frame number');
zlabel('Y');


end

