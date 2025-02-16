for /f "tokens=1-4 delims=/ " %%i in ("%date%") do (
     set dow=%%i
     set month=%%j
     set day=%%k
     set year=%%l
   )

set datestr=%year%_%month%_%day%
set name=sdf_toy
set fname=%name%_%datestr%

xcopy /yq imgui.ini			builds\%fname%\
xcopy /yqs data				builds\%fname%\data\
xcopy /yqs shaders			builds\%fname%\shaders\
copy Release\%name%.exe			builds\%fname%\%name%.exe
copy Debug\%name%.exe			builds\%fname%\%name%_debug.exe
cd builds
zip -r %fname%.zip %fname%