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
-   install, of course , GIMP application on your pc
-   install Octave program on your pc
-   install and load 'image' libraries for Octave
Then you are ready to install our app.<br>
As we have already stated on introduction, each hipstagram filter comes with two code files. <br>
With .c files and .m files.<br>
You install separately your chosen desired filters by following these steps:<br>
-   move your .m files to the path:  ~./gimp-octave
-   accordingly, your .c files can be on whatever path you like
-   go to the path you have the .c files and enter the command on the terminal:
   $gimptool-2.0 --install <filtername>.c
-   open GIMP by clicking on the icon
-   check filters on the menu bar:
Filters -> Ηipstagram -> <your_filter>.
-   open new image -> apply a Hipstagram filter
-   enjoy :)

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
