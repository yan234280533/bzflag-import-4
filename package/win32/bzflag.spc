-file ../../bin/bzflag.exe bzflag.exe
-file ../../bin/bzfs.exe bzfs.exe
-file ../../bin/bzfls.exe bzfls.exe
-file bin/bzfuinst.exe bzfuinst.exe
-pushdir data
-file ../../data/bbolt.rgb bbolt.rgb
-file ../../data/blaser.rgb blaser.rgb
-file ../../data/boxwall.rgb boxwall.rgb
-file ../../data/caution.rgb caution.rgb
-file ../../data/clouds.rgb clouds.rgb
-file ../../data/explode1.rgb explode1.rgb
-file ../../data/fixedbr.rgb fixedbr.rgb
-file ../../data/fixedmr.rgb fixedmr.rgb
-file ../../data/flag.rgb flag.rgb
-file ../../data/flage.rgb flage.rgb
-file ../../data/gbolt.rgb gbolt.rgb
-file ../../data/glaser.rgb glaser.rgb
-file ../../data/ground.rgb ground.rgb
-file ../../data/helvbi.rgb helvbi.rgb
-file ../../data/helvbr.rgb helvbr.rgb
-file ../../data/missile.rgb missile.rgb
-file ../../data/mountain.rgb mountain.rgb
-file ../../data/pbolt.rgb pbolt.rgb
-file ../../data/plaser.rgb plaser.rgb
-file ../../data/pyrwall.rgb pyrwall.rgb
-file ../../data/rbolt.rgb rbolt.rgb
-file ../../data/rlaser.rgb rlaser.rgb
-file ../../data/roof.rgb roof.rgb
-file ../../data/timesbi.rgb timesbi.rgb
-file ../../data/timesbr.rgb timesbr.rgb
-file ../../data/title.rgb title.rgb
-file ../../data/wall.rgb wall.rgb
-file ../../data/ybolt.rgb ybolt.rgb
-file ../../data/ylaser.rgb ylaser.rgb
-file ../../data/boom.wav boom.wav
-file ../../data/explosion.wav explosion.wav
-file ../../data/fire.wav fire.wav
-file ../../data/flag_alert.wav flag_alert.wav
-file ../../data/flag_drop.wav flag_drop.wav
-file ../../data/flag_grab.wav flag_grab.wav
-file ../../data/flag_lost.wav flag_lost.wav
-file ../../data/flag_won.wav flag_won.wav
-file ../../data/laser.wav laser.wav
-file ../../data/pop.wav pop.wav
-file ../../data/ricochet.wav ricochet.wav
-file ../../data/shock.wav shock.wav
-file ../../data/teamgrab.wav teamgrab.wav
-file ../../data/teleport.wav teleport.wav
-file ../../data/phantom.wav phantom.wav
-file ../../data/missile.wav missile.wav
-file ../../data/jump.wav jump.wav
-file ../../data/land.wav land.wav
-file ../../data/killteam.wav killteam.wav
-file ../../data/lock.wav lock.wav
-popdir
-pushdir doc
-file ../../LICENSE LICENSE
-file README.win32.html README.HTM
-readme README.HTM
-popdir
-shortcut bzflag.exe bzflag "" "%i"
-regkey "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall" "bzflag"
-regstring "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\bzflag" "DisplayName" "bzflag"
-regstring "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\bzflag" "UninstallString" "%i\bzfuinst.exe %i"
