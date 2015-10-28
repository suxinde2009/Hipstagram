%----Begin of octave script
clear;
cd ~/.gimp-octave
load matrixIn.txt;
[rows, cols, bpp] = size(matrixIn)
img = matrixIn;

%----------------------------------------------------------------------
%   BLACK_AND_WHITE: Runs a bw filter on an image
%
%   PARAMETERS::   all parameters are in the range [0,1]
%   alpha       weight for yellow sepia tone
%   beta        weight for desaturation
%   gamma       weight for blue adjustment
%   delta       weight for burning the edges of the image
%   epsilon     used to normalize perturbations.


%black border on the edges of the pic
[rows, cols, bpp] = size(img);

img(1:15, 1:end, :) = 0;
img(rows-15:end, 1:end, :) = 0;
img(1:end,1:15,:) = 0;
img(1:end,cols-15:end,:) = 0;

alpha 	= 0;
beta  	= 0.5;
gamma 	= 0;
delta 	= 0.5;
epsilon = 0.05;

%----------------------------------------------------------------------

s = size(img);

% calculate center
c = s(1:2)/2;
n = sum(c.^2); % normalization constant

% calculate normalized intensity matrix
i = (img(:,:,1) + img(:,:,2) + img(:,:,3))/3;

% calculate desaturation movements
mr = (img(:,:,2) + img(:,:,3))/2 - img(:,:,1);
mg = (img(:,:,1) + img(:,:,3))/2 - img(:,:,2);
mb = (img(:,:,1) + img(:,:,2))/2 - img(:,:,3);

% make a meshgrid
[X,Y] = meshgrid(1:s(2), 1:s(1));

% add yellow for sepia
img(:,:,1:2) = img(:,:,1:2) + alpha*255;

% turn the blue channel into a weighted sum of the original
% blue value and the intensity
img(:,:,3) = (1-gamma)*img(:,:,3) + gamma*i;

% desaturate
img(:,:,1) = img(:,:,1) + mr*beta;
img(:,:,2) = img(:,:,2) + mg*beta;
img(:,:,3) = img(:,:,3) + mb*beta;

% make a matrix (the size of the image) of normally distributed random values
rand = delta*ones(s(1:2)) - normrnd(0,1,s(1:2)).^2*epsilon;

% burn the edges
black = uint8(255 * rand .* ((X-c(2)).^2 + (Y-c(1)).^2)/n);
img(:,:,1) = img(:,:,1) - black;
img(:,:,2) = img(:,:,2) - black;
img(:,:,3) = img(:,:,3) - black;

% bind the image channels between 0 and 255 in case any overflow or
% underflow occured
img(img < 0) = 0;
img(img > 255) = 255;


%----------------------------------------------------------------------

% stretch from 0 to 255
img = img .- min(img(:));
img = 255 * img/(max(img(:)));

%----End of octave script:----
matrixOut = round(img);
matrixOut(matrixOut(:)<0) = 0;
matrixOut(matrixOut(:)>255) = 255;
dims = ndims (matrixOut)

if (dims == 2)
    matrixOut = resize (matrixOut, [size(matrixOut, 1), size(matrixOut, 2), 1]);
end

size(matrixOut)
save matrixOut.txt matrixOut;
%----------------------------------------------------------------------
