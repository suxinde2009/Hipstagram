# Hipstagram
A GIMP plugin that allows users to apply filters to photos

As a GIMP plugin for image processing it provides several different image filters to convert, <br>
manipulate, filter, visualize images. It is a try to support similar to the Instagram effects for  <br>
GIMP, an open-source image processing program. <br>
<br>
Hipstagram mainly provides an Instagram-alike tool for image processing along with flexibility <br>
for GIMP users and desktop applications. It comes with a complete set of filter commands which  <br>
operate on images.<br>

## How to install Hipstagram:
### on Linux OS:
In order to install the plug-in of Hipstagram to GIMP, you will need prior to:<br>
-   Install GIMP
-   Install Octave
-   Install and load 'image' libraries for Octave<br>

Then you are ready to install our app.<br>
Every hipstagram filter comes with two code files, with .c file and .m file.<br>
You install separately your chosen desired filters by following these steps:<br>
-   Move your desired filters ( .m files ) to the path:  ~./gimp-octave
-   Accordingly, your .c files can be on whatever path you like
-   Go to the path you have the .c files and enter the command on the terminal:
   
   ```sh
   $gimptool-2.0 --install <filtername>.c
   ```
   
-   Open GIMP
-   Check filters on the menu bar: 'Filters -> Ηipstagram -> <your_filter>'
-   Open new image -> apply a Hipstagram filter
-   Enjoy :)

## How to use Hipstagram:

- Download the Hipstagram code. <br>
  (For each filter you want to use, you have to have both the files of C code and Octave code.)
- Install your desired filter (see later)
- Launch GIMP
- Click "Open New Image"
- Apply the filter by clicking: Menu bar->Filters->Hipstagram->name-of-your filter 
- Click the filter
- A small pop-up window comes showing a preview of your image 
- Your new image :)

## Resources:
* Writing a GIMP plug-in:  http://www.gimp.org/docs/plug-in/plug-in.html
* GIMP Octave plug-in: https://sites.google.com/site/elsamuko/gimp/gimp-octave
* retro filter vectorized: 
   http://parsiad.dyndns.org/scripts/format_code.php?noopts=1&file=files/code/retro_filter/retro_filter_vectorized.m&language=matlab
* How to write GIMP plug-in http://www.linux.ie/articles/gimp-plugin/part1/
