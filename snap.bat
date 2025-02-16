for /f "tokens=1-4 delims=/ " %%i in ("%date%") do (
     set dow=%%i
     set month=%%j
     set day=%%k
     set year=%%l
   )
for /f "tokens=1-2 delims=: " %%i in ("%time%") do (
     set hour=%%i
     set minute=%%j
   )
set datestr=%year%_%month%_%day%_at_%hour%_%minute%
set name=sdf_toy
set fname=%name%_%datestr%

xcopy /yq imgui.ini		builds\%fname%\
xcopy /yqs game                 builds\%fname%\game\
xcopy /yqs engine               builds\%fname%\engine\
xcopy /yqs studio               builds\%fname%\src\
xcopy /yqs wrap                 builds\%fname%\wrap\
xcopy /yqs libs                 builds\%fname%\libs\
xcopy /yqs core                 builds\%fname%\core\
xcopy /yqs math                 builds\%fname%\math\
xcopy /yqs data			builds\%fname%\data\
xcopy /yqs shaders		builds\%fname%\shaders\
copy Release\%name%.exe		builds\%fname%\%name%.exe
copy Debug\%name%.exe		builds\%fname%\%name%_debug.exe
cd builds
zip -r %fname%.zip %fname%