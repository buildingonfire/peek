# peek 
### lightweight image viewer and annotator

# Description
This program will be a small self-contained exe file which opens and views images very quickly and easily, also allowing for basic annotations to be made.  The annotations will not write directly onto the image but will be composited over it and then upon save will be rendered on top of the saved image.
It will be my default tool for opening and viewing images on my home and work machines.  As such, speed and clarity are the most important aspects and the program should be written and optimized accordingly.

## Annotation Controls
- zoom / scroll with the mouse (middle mouse wheel to zoom, hold middle button and drag to pan)
- crop tool
- a small drawing / annotation toolset which includes:
    - Pencil drawing
    - box dragging
    - arrow dragging
    - text entry
    - "clear all annotations" button which will wipe all annotations and show only the original loaded image
- a small collection of attention-grabbing colors to chose from for each of these tools.  Red, Green, Blue, Yellow, Magenta, Black, White
- eraser, which will erase only annotations, not the underlying image.  

## File Controls
- A small set of controls for determining output format and options.  Output formats are only PNG and Jpeg. 
    - All outputs: 
        - Image scale, expressed in 1x, .5x, .25x.  Default = 1x
        - PNG vs JPG output. shown as a toggle.  default = jpg
        - "append to filename" string field.  default = "annotated"
    - PNG option:
        - Transparancy - default = "no"
    - Jpeg option: 
        - Quality, shown as a 40% to 100% slider.  default = 95%.
    - A large "save" button with a small preview string underneath it showing the path and name the file will save to, along with a rough estimate of the filesize.

## Use Notes
- hitting ctrl-s will save a copy of the image in the selected format, in the same directory as the opened file, with the same file name + underscore + indicated string appended to the end of the name before the file extension.  Subsequent ctrl-s presses of the same image will save additional files with "_a", "_b", "_c" etc appended after the append string before the file extension.