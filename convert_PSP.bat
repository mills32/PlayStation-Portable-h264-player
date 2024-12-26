@echo off
SET VIDEO_NAME=%1%
rem x264 parameters PSP MENU & HOMEBREW PLAYER
SET S1=cabac=1:ref=2:deblock=1,0,0:analyse=0x3,0x131:me=dia:subme=7
SET S2=%S1%:psy=1:psy_rd=1.00,0.00:mixed-refs=1:me_range=16:chroma_me=1
SET S3=%S2%:trellis=1:8x8dct=0:cqm_preset=0:deadzone-inter=21:deadzone-intra=11:fast_pskip=1:chroma_qp_offset=-2
SET S4=%S3%:threads=12:lookahead_threads=2:sliced_threads=0:nr=0:dct-decimate=1:interlaced=0
SET S5=%S4%:bluray_compat=0:constrained_intra=0:bframes=0:b_pyramid=0:b_adapt=1
SET S6=%S5%:b_bias=0:direct="spatial":weightb=1:open_gop=0:weightp=2
SET S7=%S6%:scenecut=40:intra_refresh=0:rc_lookahead=40:mbtree=1:crf=23.0
SET _OPT=%S7%:qcomp=0.60:qpmin=0:qpmax=69:qpstep=4:ip-factor=1.40:aq_mode=1:aq_strength=1.00

rem direct=3 / /

SET TRIM=-ss 00:00:00 -t 00:2:00
SET CONTAINER=mp4
SET VIDEO_CONTAINER=0
SET AUDIO=0
SET AUDIO_FORMAT=-c:a mp3 -ar 44100 -ab 128k
SET SIZE=0
SET SIZE_FILTERS=
SET RATE=1
SET BITRATE=:bitrate=300
SET GOPS=:keyint=65536:keyint_min=65536


echo. 
echo CONVERT VIDEO FOR PLAYSTATION PORTABLE
echo --------------------------------------
echo.                                          
echo INPUT VIDEO = %VIDEO_NAME%


echo.   
echo SELECT VIDEO CONTAINER (default MP4):  
echo - 0 MP4 (XMB menu player);
echo - 1 AVI (Homebrew player);       
set /p VIDEO_CONTAINER=CONTAINER: 
if %VIDEO_CONTAINER% equ 0 (set CONTAINER=mp4)
if %VIDEO_CONTAINER% equ 1 (set CONTAINER=avi)
if %VIDEO_CONTAINER% equ 0 (SET GOPS=:keyint=256:keyint_min=27)
if %VIDEO_CONTAINER% equ 1 (SET GOPS=:keyint=65536:keyint_min=65536)

echo.   
echo SELECT AUDIO FORMAT (default MP3):  
echo - 0 MP3;
echo - 1 AAC;       
set /p AUDIO=CONTAINER: 
if %AUDIO% equ 0 (set AUDIO_FORMAT=-c:a mp3 -ar 44100 -ab 128k)
if %AUDIO% equ 1 (set AUDIO_FORMAT=-c:a aac -ar 44100 -ab 128k -ac 2)


echo.   
echo SELECT VIDEO SIZE:  
echo - 0 480x272;
echo - 1 720x480;       
set /p SIZE=VIDEO SIZE: 
if %SIZE% equ 0 (set SIZE_FILTERS=-vf "scale=(iw*272/ih):272,crop=480:272" -bsf:v "filter_units=remove_types=6|35|38-40")
if %SIZE% equ 1 (set SIZE_FILTERS=-vf "scale=(iw*480/ih):480,crop=720:480" -bsf:v "filter_units=remove_types=6|35|38-40")


echo.   
echo SELECT VIDEO QUALITY (default 300kbps):  
echo - 0 TINY VIDEO LOW QUALITY (200kbps);
echo - 1 REGULAR (300kbps);   
echo - 2 HIGH (450kbps: CRASH/SLOW WHEN MORE THAN 30 FPS);
echo - 3 VERY HIGH (600kbps: CRASH/SLOW WHEN MORE THAN 30FPS); 
echo - 4 ULTRA (1500kbps: WHEN USING 720x480);   
set /p RATE=VIDEO QUALITY: 
if %RATE% equ 0 (set BITRATE=:bitrate=200)
if %RATE% equ 1 (set BITRATE=:bitrate=300)
if %RATE% equ 2 (set BITRATE=:bitrate=450)
if %RATE% equ 3 (set BITRATE=:bitrate=600)
if %RATE% equ 4 (set BITRATE=:bitrate=1100)


ffmpeg -i %VIDEO_NAME% -c:v libx264 -profile:v main %SIZE_FILTERS% -x264-params "%_OPT%%GOPS%%BITRATE%" %AUDIO_FORMAT% %TRIM% psp_video.%CONTAINER%
pause
