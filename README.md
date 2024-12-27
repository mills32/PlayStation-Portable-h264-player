H264 SIMPLE PLAYER FOR PSP
--------------------------

This plays video files in AVI format, containing h264 frames and AAC audio (I have to add MP3 audio).
There are some homebrew h264 players for PSP but the code is so complex I decided to create this as a sample code.

Drop any video to convert_PSP.bat to convert videos using ffmpeg (place ffmpeg.exe in the same folder for windows, if you use linux you probably know how what to do).
Videos will be cropped/resized and black bars will be added if necessary to preserve video aspect ratio.

Press triangle to exit program.

I tested with some of these: https://studio.blender.org/films/ and they work fine. (big buck bunny at 60 fps is also working well)


BUGS
----

- 720x480 videos don't work.
- Not all h264 parameters will work, "convert_PSP.bat" contains all parameters configured, in case you want to use other video converter.








REPRODUCTOR DE VIDEOS H264 PARA PSP
-----------------------------------

Éste programa reproduce videos en formato avi, conteniendo frames h264 y audio AAC (tengo que añadir MP3).
Hay otros reproductores de h264 para PSP, pero el código es tan complicado que decidí crear un ejemplo más simple.

Arrastra cualquier video a convert_PSP.bat para convertirlo usando ffmpeg (copia ffmpeg.exe al mismo directorio en windows, si usas linux, es probable que ya sepas qué hacer).
Los videos seran escalados/recortados y se añadirán bandas negras si es necesario, para mantener la relación de aspecto.

Pulsa triangulo para salir del programa.

Lo he probado con algunos de estos videos: https://studio.blender.org/films/. (big buck bunny a 60fps funciona muy bien).

PROBLEMAS
---------

- Videos de tamaño 720x480 no funcionan.
- No todos los parámetros h264 funcionan, "convert_PSP.bat" contiene todos los parámetros configurados si quieres utilizar otro conversor.


