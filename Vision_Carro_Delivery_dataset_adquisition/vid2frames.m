% 1. Specify the video file and output folder
videoFile = 'Delivery2.mp4';
outputFolder = 'raw_images_224';

% 2. Create the VideoReader object
v = VideoReader(videoFile);

% 3. Create the output directory if it doesn't exist
if ~exist(outputFolder, 'dir')
    mkdir(outputFolder);
end

% 4. Loop through the video and save frames
frameNumber = 0;
targetSize = [224, 224]; % Define your target dimensions

while hasFrame(v)
    % Read the next frame
    frame = readFrame(v); 
    
    % Resize the frame to match your model input
    % 'bilinear' is a good balance between speed and quality
    resizedFrame = imresize(frame, targetSize, 'bilinear');
    
    % Generate a unique file name
    baseFileName = sprintf('frameesp4_%04d.png', frameNumber);
    fullFileName = fullfile(outputFolder, baseFileName);
    
    % Save the resized frame
    imwrite(resizedFrame, fullFileName);
    
    frameNumber = frameNumber + 1;
end

disp(['Successfully extracted and resized ', num2str(frameNumber), ' frames to 224x224.']);