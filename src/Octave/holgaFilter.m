%----Begin of octave script, don't edit:--
clear;
cd ~/.gimp-octave
load matrixIn.txt;
[rows, cols, bpp] = size(matrixIn)
img = matrixIn;

%----------------------------------------------------------------------
%  HOLGA_FILTER

s = size(img);

% calculate center
c = s(1:2)/2;
n = sum(c.^2); % normalization constant

img(:,:,1)=0;  % green channel

% make a meshgrid
[X,Y] = meshgrid(1:s(2), 1:s(1));

% burn the edges
black = uint8(255 * rand .* ((X-c(2)).^2 + (Y-c(1)).^2)/n);
img(:,:,1) = img(:,:,1) - black;
img(:,:,2) = img(:,:,2) - black;
img(:,:,3) = img(:,:,3) - black;

%----------------------------------------------------------------------

%stretch from 0 to 255
img = img .- min(img(:));
img = 255 * img/(max(img(:)));

%----End of octave script, don't edit:----
matrixOut = round(img);
matrixOut(matrixOut(:)<0)=0;
matrixOut(matrixOut(:)>255)=255;
dims = ndims (matrixOut)

if (dims == 2)
    matrixOut = resize (matrixOut, [size(matrixOut, 1), size(matrixOut, 2), 1]);
end

size(matrixOut)
save matrixOut.txt matrixOut;
%----------------------------------------------------------------------
