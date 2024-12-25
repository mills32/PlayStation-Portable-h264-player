H264 SIMPLE PLAYER FOR PSP
--------------------------

This plays video files in AVI format, containing h264 frames and AAC audio (I have to add MP3 audio).
There are some homebrew h264 players for PSP but the code is so complex I decided to crate this as a sample code.

Drop any video to convert_PSP.bat to convert videos using ffmpeg (windows). If you use linux you probably know how what to do.
Videos will be cropped/resized and black bars will be added if necessary to preserve video aspect ratio.

BUGS
----

- 720x480 videos don't work.
- 60 fps videos should work OK as long as bitrate is 300kbps or lower.
- Not all h264 parameters will work, "convert_PSP.bat" contains all parameters configured, in case you whant to use other video converter.


REPRODUCTOR DE VIDEOS H264 PARA PSP
-----------------------------------

Éste programa reproduce videos en formato avi, conteniendo frames h264 y audio AAC (tengo que añadir MP3).
Hay otros reproductores de h264 para PSP, pero el código es tan complicado que decidí crear un ejemplo más simple.

Arrastra cualquier video a convert_PSP.bat para convertirlo usando ffmpeg (windows). Si usas linux, es probable que ya sepas qué hacer.
Los videos seran escalados/recortados y se añadirán bandas negras si es necesario, para mantener la relación de aspecto.


PROBLEMAS
---------

- Videos de tamaño 720x480 no funcionan.
- Videos a 60 fps deberían funcionar bien mientras la tasa de bits sea 300kbps o inferior
- No todos los parámetros h264 funcionan, "convert_PSP.bat" contiene todos los parámetros configurados si quieres utilizar otro conversor.

