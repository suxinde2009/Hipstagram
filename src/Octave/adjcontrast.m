%----Begin of octave script:--
clear;
cd ~/.gimp-octave
load matrixIn.txt;
[rows, cols, bpp] = size(matrixIn)
img = matrixIn;

%----------------------------------------------------------------------

% ADJCONTRAST - Adjusts image contrast.
%
% PARAMETERS:
%            img    - image to be processed.
%            gain   - controls the actual contrast
%                     A value of about 5 is neutral (little change).
%                     A value of 1 reduces contrast to about 20% of original
%                     A value of 10 increases contrast about 2.5x.
%                     a reasonable range of values to experiment with.
%
%            cutoff - represents the (normalised) grey value about which
%                     contrast is increased or decreased.  An initial
%                     value you might use is 0.5 (the midpoint of the
%                     greyscale) but different images may require
%                     different points of the greyscale to be enhanced.  

gain 	= 10;	% contrast about 2.5x 
cutoff 	= 0.5;	% midpoint of the greyscale


if isa(img,'uint8');
	newim = double(img);
else 
	newim = img;
end

% rescale range 0-1
newim = newim - min(newim(:));
newim = newim ./ max(newim(:));

newim =  1./(1 + exp(gain*(cutoff-newim)));  % apply sigmoid function

%----------------------------------------------------------------------

%stretch from 0 to 255
newim = newim .- min(newim(:));
newim = 255 * newim/(max(newim(:)));


%----End of octave script:----
matrixOut = round(newim);
matrixOut(matrixOut(:)<0)=0;
matrixOut(matrixOut(:)>255)=255;
dims = ndims (matrixOut)

if (dims == 2)
    matrixOut = resize (matrixOut, [size(matrixOut, 1), size(matrixOut, 2), 1]);
end

size(matrixOut)
save matrixOut.txt matrixOut;
%----------------------------------------------------------------------
